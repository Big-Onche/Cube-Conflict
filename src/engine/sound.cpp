// sound.cpp: basic positional sound using sdl_mixer

#include "engine.h"
#include "game.h"
#include <sndfile.h>
#include <AL/efx.h>
#include <AL/alext.h>
#include <limits>
#include <vector>

bool foundDevice = false;
bool noSound = true;
bool noEfx = false;

//VAR(debugsounds, 0, 0, 1);
VARFP(soundvol, 0, 100, 100, alListenerf(AL_GAIN, soundvol/100.f); );
extern void updateMusicVol();
VARFP(musicvol, 0, 50, 100, updateMusicVol());
VARFP(mutesounds, 0, 0, 1, alListenerf(AL_GAIN, mutesounds ? 0.f : soundvol/100.f));
VARP(minimizedmute, 0, 0, 1);
VARP(maxsoundsatonce, 32, 64, MAX_SOURCES);
VARP(soundfreq, 0, 44100, 48000);

int gameSoundId = 0;
Sound gameSounds[NUMSNDS];
ICOMMAND(gamesound, "siii", (char *path, int *alts, int *vol),
    if(gameSoundId >= NUMSNDS) { conoutf(CON_ERROR, "Unable to load sound: %s (id %d is greater than amount of sounds)", path, gameSoundId); return; }
    formatstring(gameSounds[gameSoundId].soundPath, "%s", path);
    gameSounds[gameSoundId].numAlts = *alts;
    gameSounds[gameSoundId].soundVol = *vol;
    gameSounds[gameSoundId].loaded = loadSound(gameSounds[gameSoundId]);
    gameSoundId++;
);

#define NUMMAPSNDS 32 // temporary fixed shit
int mapSoundId = 0;
Sound mapSounds[NUMMAPSNDS];
static vector<int> mapSoundIds;
static int nextMapSoundsUpdate = 0;
static const int MAP_SOUNDS_UPDATE_INTERVAL = 250;
ICOMMAND(mapsound, "siii", (char *path, int *alts, int *vol, int *occl),
    if(mapSoundId >= NUMMAPSNDS) { conoutf(CON_ERROR, "Unable to load map sound: %s (id %d is greater than amount of map sounds)", path, mapSoundId); return; }
    formatstring(mapSounds[mapSoundId].soundPath, "%s", path);
    mapSounds[mapSoundId].numAlts = *alts;
    mapSounds[mapSoundId].soundVol = *vol;
    mapSounds[mapSoundId].noGeomOcclusion = *occl;
    mapSounds[mapSoundId].loaded = loadSound(mapSounds[mapSoundId]);
    mapSoundId++;
);

ICOMMAND(resetmapsounds, "", (),
    mapSoundId = 0;
    mapSoundIds.shrink(0);
    nextMapSoundsUpdate = 0;
);

int musicId = 0;
Sound music[NUMSONGS];
ICOMMAND(gamemusic, "sii", (char *path, int *alts, int *vol),
    if(musicId >= NUMSONGS) { conoutf(CON_ERROR, "Unable to load music: %s (id %d is greater than amount of musics)", path, musicId); return; }
    formatstring(music[musicId].soundPath, "%s", path);
    music[musicId].numAlts = *alts;
    music[musicId].soundVol = *vol;
    music[musicId].loaded = loadSound(music[musicId], true);
    musicId++;
);
/*
void reportSoundError(const char* func, const char* details, int buffer = 0)
{
    if(!debugsounds) return;
    ALenum error = alGetError();
    defformatstring(bufferId, buffer ? " (Buffer ID: %d)" : "", buffer);
    if(error != AL_NO_ERROR) conoutf(CON_ERROR, "OpenAL Error at %s (%s) %d%s", func, details, error, bufferId);
}
*/
bool loadSoundFile(const string& filepath, ALuint buffer) // load a sound using libsndfile
{
    static std::vector<short> soundLoadScratch;
    static bool warnedSampleConversion = false;

    SNDFILE* file;
    SF_INFO sfinfo;
    ALsizei num_bytes;

    file = sf_open(tempformatstring("%s", filepath), SFM_READ, &sfinfo); // open the audio file and check if valid
    if(!file) { conoutf(CON_WARN, "Could not load audio file: %s", filepath); return false; }

    ALenum format;
    switch(sfinfo.channels) // determine the OpenAL format from the number of channels
    {
        case 1: format = AL_FORMAT_MONO16; break;
        case 2: format = AL_FORMAT_STEREO16; break;
        default:
            conoutf(CON_ERROR, "Unsupported channel count in %s (%d instead of 1 or 2)", filepath, sfinfo.channels);
            sf_close(file);
            return false;
    }

    const sf_count_t num_frames = sfinfo.frames; // get the number of frames in the sound file
    if(num_frames < 0)
    {
        conoutf(CON_WARN, "Audio file has invalid frame count: %s", filepath);
        sf_close(file);
        return false;
    }

    const size_t sampleCount = size_t(num_frames) * size_t(sfinfo.channels);
    const size_t maxBytes = size_t(std::numeric_limits<ALsizei>::max());
    if(sampleCount > maxBytes / sizeof(short))
    {
        conoutf(CON_ERROR, "Audio file too large for OpenAL buffer: %s", filepath);
        sf_close(file);
        return false;
    }

    if(soundLoadScratch.size() < sampleCount) soundLoadScratch.resize(sampleCount);

    if(!warnedSampleConversion && (sfinfo.format & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16)
    {
        conoutf(CON_WARN, "Audio source is not PCM16, libsndfile converts at load time: %s", filepath);
        warnedSampleConversion = true;
    }

    // read the sound data into the memory buffer
    sf_count_t framesRead = sf_readf_short(file, soundLoadScratch.data(), num_frames);
    if(framesRead < 0)
    {
        conoutf(CON_WARN, "Could not read audio frames from: %s", filepath);
        sf_close(file);
        return false;
    }

    num_bytes = ALsizei(framesRead * sfinfo.channels * sizeof(short));
    alBufferData(buffer, format, soundLoadScratch.data(), num_bytes, sfinfo.samplerate); // upload the sound data to the OpenAL buffer
    //reportSoundError("alBufferData", filepath, buffer);

    sf_close(file);
    return true;
}

bool loadSound(Sound& s, bool music) // load a sound including his alts
{
    if(noSound) return false;

    bool allLoaded = true;

    loopi(s.numAlts ? s.numAlts + 1 : 1)
    {

        if(s.bufferId[i]) // delete existing buffer if it's valid
        {
            if(alIsBuffer(s.bufferId[i])) alDeleteBuffers(1, &s.bufferId[i]);
            s.bufferId[i] = 0;
        }

        defformatstring(fullPath, s.numAlts ? "media/sounds/%s_%d.wav" : "media/sounds/%s.wav", s.soundPath, i + 1);
        if(music) formatstring(fullPath, s.numAlts ? "media/songs/%s_%d.flac" : "media/songs/%s.flac", s.soundPath, i + 1);

        alGenBuffers(1, &s.bufferId[i]);
        ALenum error = alGetError();
        if(error != AL_NO_ERROR)
        {
            conoutf(CON_ERROR, "Failed to generate buffer for %s: OpenAL error %d", s.soundPath, error);
            s.bufferId[i] = 0;
            allLoaded = false;
            continue;
        }

        bool loaded = loadSoundFile(fullPath, s.bufferId[i]);
        if(!loaded) // if loading failed, delete the unused buffer to prevent leak
        {
            alDeleteBuffers(1, &s.bufferId[i]);
            s.bufferId[i] = 0;
            allLoaded = false;
            conoutf(CON_WARN, "Failed to load sound variant %d for %s", i + 1, s.soundPath);
        }
    }

    return allLoaded;
}

LPALGENAUXILIARYEFFECTSLOTS    alGenAuxiliaryEffectSlots    = NULL;
LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = NULL;
LPALAUXILIARYEFFECTSLOTI       alAuxiliaryEffectSloti       = NULL;
LPALGENEFFECTS                 alGenEffects                 = NULL;
LPALDELETEEFFECTS              alDeleteEffects              = NULL;
LPALISEFFECT                   alIsEffect                   = NULL;
LPALEFFECTI                    alEffecti                    = NULL;
LPALEFFECTF                    alEffectf                    = NULL;
LPALEFFECTFV                   alEffectfv                   = NULL;
LPALGENFILTERS                 alGenFilters                 = NULL;
LPALDELETEFILTERS              alDeleteFilters              = NULL;
LPALISFILTER                   alIsFilter                   = NULL;
LPALFILTERI                    alFilteri                    = NULL;
LPALFILTERF                    alFilterf                    = NULL;
LPALGETFILTERF                 alGetFilterf                 = NULL;

static void getEfxFuncs()
{
    if(noEfx) return;

    alGenAuxiliaryEffectSlots    = (LPALGENAUXILIARYEFFECTSLOTS)   alGetProcAddress("alGenAuxiliaryEffectSlots");
    alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
    alAuxiliaryEffectSloti       = (LPALAUXILIARYEFFECTSLOTI)      alGetProcAddress("alAuxiliaryEffectSloti");
    alGenEffects                 = (LPALGENEFFECTS)                alGetProcAddress("alGenEffects");
    alDeleteEffects              = (LPALDELETEEFFECTS)             alGetProcAddress("alDeleteEffects");
    alIsEffect                   = (LPALISEFFECT)                  alGetProcAddress("alIsEffect");
    alEffecti                    = (LPALEFFECTI)                   alGetProcAddress("alEffecti");
    alEffectf                    = (LPALEFFECTF)                   alGetProcAddress("alEffectf");
    alEffectfv                   = (LPALEFFECTFV)                  alGetProcAddress("alEffectfv");
    alGenFilters                 = (LPALGENFILTERS)                alGetProcAddress("alGenFilters");
    alDeleteFilters              = (LPALDELETEFILTERS)             alGetProcAddress("alDeleteFilters");
    alIsFilter                   = (LPALISFILTER)                  alGetProcAddress("alIsFilter");
    alFilteri                    = (LPALFILTERI)                   alGetProcAddress("alFilteri");
    alFilterf                    = (LPALFILTERF)                   alGetProcAddress("alFilterf");
    alGetFilterf                 = (LPALGETFILTERF)                alGetProcAddress("alGetFilterf");
}

ALuint auxEffectSlots[NUMREVERBS];

void applyReverbPreset(ALuint effectSlot, const EFXEAXREVERBPROPERTIES& preset)
{
    if(noEfx) return;

    ALuint reverbEffect;

    alGenEffects(1, &reverbEffect);
    alEffecti(reverbEffect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
    // Set all the reverb parameters from the preset...
    alEffectf(reverbEffect, AL_REVERB_DENSITY, preset.flDensity);
    alEffectf(reverbEffect, AL_REVERB_DIFFUSION, preset.flDiffusion);
    alEffectf(reverbEffect, AL_REVERB_GAIN, preset.flGain);
    alEffectf(reverbEffect, AL_REVERB_GAINHF, preset.flGainHF);
    alEffectf(reverbEffect, AL_REVERB_DECAY_TIME, preset.flDecayTime);
    alEffectf(reverbEffect, AL_REVERB_DECAY_HFRATIO, preset.flDecayHFRatio);
    alEffectf(reverbEffect, AL_REVERB_REFLECTIONS_GAIN, preset.flReflectionsGain);
    alEffectf(reverbEffect, AL_REVERB_LATE_REVERB_GAIN, preset.flLateReverbGain);
    alEffectf(reverbEffect, AL_REVERB_LATE_REVERB_DELAY, preset.flLateReverbDelay);
    alEffectf(reverbEffect, AL_REVERB_AIR_ABSORPTION_GAINHF, preset.flAirAbsorptionGainHF);
    alEffectf(reverbEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, preset.flRoomRolloffFactor);
    alEffecti(reverbEffect, AL_REVERB_DECAY_HFLIMIT, AL_TRUE);
    // Attach the configured effect to the effect slot
    alAuxiliaryEffectSloti(effectSlot, AL_EFFECTSLOT_EFFECT, reverbEffect);

    alDeleteEffects(1, &reverbEffect); // DELETE THE EFFECT AFTER ATTACHING - the slot holds a reference
}

int reverb[NUMREVERBS];

void configureMapReverbs(int a, int b, int c, int d, int e)
{
    int params[] = {a, b, c, d, e};
    loopi(sizeof(params) / sizeof(params[0]))
    {
        if(params[i] >= 0 && params[i] <= numReverbPresets) applyReverbPreset(auxEffectSlots[i], reverbPresets[params[i]]);
        else conoutf(CON_ERROR, "Parameter %d is out of the valid range (0 to %d).", i + 1, numReverbPresets);
    }
}

ICOMMAND(mapreverbs, "iiiii", (int *a, int *b, int *c, int *d, int *e), configureMapReverbs(*a, *b, *c, *d, *e) );

void applyReverb(ALuint source, int reverb)
{
    if(noEfx) return;
    if(reverb >= 0 && reverb < NUMREVERBS) alSource3i(source, AL_AUXILIARY_SEND_FILTER, auxEffectSlots[reverb], 0, AL_FILTER_NULL);
    else conoutf(CON_ERROR, "Parameter %d is out of the valid range (0 to %d).", reverb, NUMREVERBS);
}

int getReverbZone(vec pos)
{
    bool hudSound = pos.iszero();

    if(game::hudplayer()->boostmillis[B_SHROOMS]) return REV_SHROOMS;
    else if(isUnderWater(camera1->o)) return REV_UNDERWATER;

    if(camera1->o.dist(pos) < 600 || hudSound)
    {
        if(hudSound) pos = camera1->o;
        loopi(4) { if(lookupmaterial(pos) == MAT_REVERB + i) return REV_SECOND + i; }
    }
    return REV_MAIN;
}

SoundSource sounds[MAX_SOURCES];
vector<int> activeSourceIds;
vector<int> freeSourceIds;
int activeIndexInList[MAX_SOURCES];

static inline int activeSourceCount() { return activeSourceIds.length(); }

static inline void activateSource(int id)
{
    sounds[id].isActive = true;
    activeIndexInList[id] = activeSourceIds.length();
    activeSourceIds.add(id);
}

static inline void recycleSource(int activeListIndex)
{
    ASSERT(activeSourceIds.inrange(activeListIndex));
    int id = activeSourceIds[activeListIndex];
    ASSERT(activeIndexInList[id] == activeListIndex);
    int lastIndex = activeSourceIds.length() - 1;
    int movedId = activeSourceIds[lastIndex];

    activeSourceIds[activeListIndex] = movedId;
    activeIndexInList[movedId] = activeListIndex;
    activeSourceIds.pop();

    activeIndexInList[id] = -1;
    sounds[id].isActive = false;
    freeSourceIds.add(id);
}

void initSoundSources()
{
    activeSourceIds.shrink(0);
    freeSourceIds.shrink(0);
    activeSourceIds.reserve(MAX_SOURCES);
    freeSourceIds.reserve(MAX_SOURCES);

    loopi(MAX_SOURCES)
    {
        alGenSources(1, &sounds[i].alSource);
        //reportSoundError("initSoundSources", "Error while initing sound sources");
        if(!noEfx)
        {
            alGenFilters(1, &sounds[i].occlusionFilter);
            alFilteri(sounds[i].occlusionFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
        }
        sounds[i].isActive = false;
        activeIndexInList[i] = -1;
        sounds[i].nextOcclusionCheck = 0;
        sounds[i].lastOcclusionChange = 0;
    }
    loopirev(MAX_SOURCES) freeSourceIds.add(i);
}

void initSounds()
{
    ALCdevice* device = alcOpenDevice(NULL); // open default device
    if(!device) { conoutf("Unable to initialize OpenAL (no audio device detected)"); return; }

    ALCint attributes[] = {
        ALC_FREQUENCY, soundfreq,
        ALC_HRTF_SOFT, ALC_FALSE,
        0
    };

    ALCcontext *context = alcCreateContext(device, attributes);
    if(!context || !alcMakeContextCurrent(context))
    {
        conoutf("Unable to initialize OpenAL (!context)");
        alcCloseDevice(device);
        return;
    }

    noSound = false;

    if(!alcIsExtensionPresent(device, "ALC_EXT_EFX")) // check for EFX extension support
    {
        conoutf(CON_WARN, "EFX extension not available.");
        noEfx = true;
    }
    else
    {
        getEfxFuncs();
        alGenAuxiliaryEffectSlots(NUMREVERBS, auxEffectSlots);
        applyReverbPreset(auxEffectSlots[REV_SHROOMS], EFX_REVERB_PRESET_DRUGGED);
        applyReverbPreset(auxEffectSlots[REV_UNDERWATER], EFX_REVERB_PRESET_UNDERWATER);
    }

    alDistanceModel(AL_LINEAR_DISTANCE);
    alDopplerFactor(1.0f);
    alSpeedOfSound(343.3f);
    initSoundSources();
}

static const float OCCLUSION_MAX_DIST = 1000.f;
static const float OCCLUSION_MAX_DIST_SQ = OCCLUSION_MAX_DIST * OCCLUSION_MAX_DIST;
static const float OCCLUSION_SAMPLE_DIST = 300.f;
static const float OCCLUSION_SAMPLE_DIST_SQ = OCCLUSION_SAMPLE_DIST * OCCLUSION_SAMPLE_DIST;
static const float OCCLUSION_TRIANGLE_AXES[3][2] =
{
    { 1.0f, 0.0f },
    { -0.5f, 0.8660254f },
    { -0.5f, -0.8660254f }
};

bool checkSoundOcclusion(const vec *soundPos, int flags = 0)
{
    if(*soundPos == vec(0, 0, 0) || (flags & SND_NOOCCLUSION) || noEfx) return false;

    const vec &cameraPos = camera1->o;
    float distanceSq = cameraPos.fastsquaredist(*soundPos);
    if(distanceSq > OCCLUSION_MAX_DIST_SQ) return false; // cull distant sounds

    vec hitPos;
    //particle_splash(PART_SPARK, 1, 250, *soundPos, 0x00FF00, 1.f, 1, 0);
    if(raycubelos(*soundPos, cameraPos, hitPos)) return false; // we first try a direct ray check
    else if(distanceSq < OCCLUSION_SAMPLE_DIST_SQ) // more complex check with a tolerance if the sound is closer
    {
        if(distanceSq <= 1e-6f) return false;
        vec dir = vec(cameraPos).sub(*soundPos).normalize(); // Direction vector pointing from sound to camera
        vec right = vec(dir.y, -dir.x, 0.0f);
        float rightLenSq = right.squaredlen();
        if(rightLenSq < 1e-6f) right = vec(1.0f, 0.0f, 0.0f); // fallback when direction is almost vertical
        else right.mul(1.0f / sqrtf(rightLenSq));
        vec perp = vec(0, 0, 0).cross(right, dir); // orthonormal basis for sampling

        float distance = sqrtf(distanceSq);
        float toleranceRadius = 25.f + (distance / 20.f); // increase tolerance according to distance to simulate sound dispersion
        loopi(3) // static triangle sample pattern
        {
            const float sampleX = OCCLUSION_TRIANGLE_AXES[i][0];
            const float sampleY = OCCLUSION_TRIANGLE_AXES[i][1];
            vec samplePoint(
                soundPos->x + (right.x * sampleX + perp.x * sampleY) * toleranceRadius,
                soundPos->y + (right.y * sampleX + perp.y * sampleY) * toleranceRadius,
                soundPos->z + (right.z * sampleX + perp.z * sampleY) * toleranceRadius
            );
            //particle_splash(PART_SPARK, 1, 250, samplePoint, 0x00FF00, 1.f, 1, 0);
            if(raycubelos(samplePoint, cameraPos, hitPos)) return false; // if one of the vertices is not occluded, no occlusion filter is applied
        }
    }
    return true;
}

bool isOccluded(int id)
{
    if(sounds[id].soundFlags & (SND_MUSIC | SND_UI)) return false;
    return isUnderWater(sounds[id].position) || checkSoundOcclusion(&sounds[id].position, sounds[id].soundFlags);
}

static inline int getOcclusionCheckInterval(int id)
{
    float distRatio = clamp(camera1->o.fastsquaredist(sounds[id].position) / OCCLUSION_MAX_DIST_SQ, 0.0f, 1.0f);
    float speedRatio = clamp(sounds[id].velocity.squaredlen() / (250.0f * 250.0f), 0.0f, 1.0f);
    int interval = int(100.0f + distRatio * 150.0f - speedRatio * 75.0f);
    return clamp(interval, 100, 250);
}

float getRandomSoundPitch(int flags)
{
    if(flags & (SND_MUSIC | SND_UI)) return 1;
    float pitchSpeed = (game::gamespeed == 100 ? 1.f : (game::gamespeed / 100.f));
    if(flags & SND_FIXEDPITCH) return pitchSpeed;
    return (0.92f + 0.16f * static_cast<float>(rand()) / RAND_MAX) * pitchSpeed;
}

inline bool canPlaySound(int soundId, int maxRadius, int flags, vec soundPos, bool hasSoundPos)
{
    if(soundId < 0 || soundId >= NUMSNDS || noSound || mutesounds) return false; // invalid index or openal not initialized or mute

    if(hasSoundPos && !(flags & SND_NOCULL) && camera1->o.dist(soundPos) > maxRadius + 50) return false; // do not play sound too far from camera, except if flag SND_NOCULL
    int activeCount = activeSourceCount();
    if((flags & SND_LOWPRIORITY) && (activeCount >= maxsoundsatonce / 2)) return false; // skip low-priority sounds (distant shoots etc.) when we are already playing a lot of sounds
    if(activeCount >= maxsoundsatonce) // no inactive source
    {
        static bool warned = false;
        if(!warned) { conoutf(CON_WARN, "Max sounds at once capacity reached (%d)", maxsoundsatonce); warned = true; }
        return false;
    }
    return true;
}

void playSound(int soundId, vec soundPos, float maxRadius, float maxVolRadius, int flags, size_t entityId, int soundType, float pitch)
{
    bool hasSoundPos = !soundPos.iszero();

    if(!canPlaySound(soundId, maxRadius, flags, soundPos, hasSoundPos)) return;
    if(freeSourceIds.empty()) return;

    int id = freeSourceIds.pop();

    // now we set a shitload of sound parameters
    activateSource(id);               // now the source is set to active
    sounds[id].entityId = entityId;
    sounds[id].soundType = soundType;
    sounds[id].soundId = soundId;
    sounds[id].soundFlags = flags;
    sounds[id].position = soundPos;
    sounds[id].velocity = vec(0, 0, 0);
    sounds[id].lastOcclusionChange = totalmillis;
    sounds[id].nextOcclusionCheck = totalmillis;

    Sound& s = (flags & SND_MUSIC) ? music[soundId] : ( (flags & SND_MAPSOUND) ? mapSounds[soundId] : gameSounds[soundId] );
    ALuint source = sounds[id].alSource;

    if(s.noGeomOcclusion) sounds[id].soundFlags |= SND_NOOCCLUSION;
    ALuint buffer = s.bufferId[s.numAlts ? rnd(s.numAlts + 1) : 0];
    alSourcei(source, AL_BUFFER, buffer); // managing sounds alternatives
    alSourcef(source, AL_GAIN, s.soundVol / 100.f); // managing sound volume
    alSourcef(source, AL_PITCH, pitch ? pitch : getRandomSoundPitch(flags)); // managing variations of pitches
    alSourcei(source, AL_LOOPING, (flags & SND_LOOPED) ? AL_TRUE : AL_FALSE); // loop the sound or not
    ALfloat sourcePos[] = {soundPos.x, soundPos.z, soundPos.y};
    alSourcefv(source, AL_POSITION, sourcePos);
    alSource3f(source, AL_VELOCITY, 0.f, 0.f, 0.f);

    if(!hasSoundPos) alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
    else
    {
        alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);
        alSourcef(source, AL_MAX_DISTANCE, maxRadius);
        alSourcef(source, AL_REFERENCE_DISTANCE, maxVolRadius); // Start decreasing volume immediately
        alSourcef(source, AL_ROLLOFF_FACTOR, 1.0f); // For linear decrease over the distance
    }

    if(sounds[id].soundFlags & (SND_MUSIC | SND_UI))
    {
        alSource3i(source, AL_AUXILIARY_SEND_FILTER, AL_FILTER_NULL, 0, AL_FILTER_NULL);
        alSourcei(source, AL_DIRECT_FILTER, AL_FILTER_NULL);
        sounds[id].lfOcclusionGain = sounds[id].hfOcclusionGain = 1.0f;
        sounds[id].isCurrentlyOccluded = false;
        alFilterf(sounds[id].occlusionFilter, AL_LOWPASS_GAIN, sounds[id].lfOcclusionGain);
        alFilterf(sounds[id].occlusionFilter, AL_LOWPASS_GAINHF, sounds[id].hfOcclusionGain);
    }
    else if(!noEfx) // apply efx if available
    {
        bool occluded = isOccluded(id);

        sounds[id].lfOcclusionGain = occluded ? 0.90f : 1.0f;
        sounds[id].hfOcclusionGain = occluded ? 0.20f : 1.0f;
        sounds[id].isCurrentlyOccluded = occluded;

        alFilterf(sounds[id].occlusionFilter, AL_LOWPASS_GAIN, sounds[id].lfOcclusionGain);
        alFilterf(sounds[id].occlusionFilter, AL_LOWPASS_GAINHF, sounds[id].hfOcclusionGain);
        alSourcei(source, AL_DIRECT_FILTER, sounds[id].occlusionFilter);
        applyReverb(source, getReverbZone(sounds[id].position));
    }

    alSourcePlay(source);
    //reportSoundError("alSourcePlay", s.soundPath, buffer);
}

void stopSound(int soundId, int flags)
{
    if(soundId < 0 || soundId >= NUMSNDS || noSound) return; // invalid index or openal not initialized

    for(int i = 0; i < activeSourceIds.length(); )
    {
        int id = activeSourceIds[i];
        if(sounds[id].soundId == soundId && sounds[id].soundFlags == flags)
        {
            alSourceStop(sounds[id].alSource);
            recycleSource(i);
        }
        else ++i;
    }
}

void stopMusic(int soundId)
{
    stopSound(soundId, SND_MUSIC);
}

void updateMusicVol()
{
    loopv(activeSourceIds)
    {
        int id = activeSourceIds[i];
        if(sounds[id].soundFlags & SND_MUSIC)
        {
            alSourcef(sounds[id].alSource, AL_GAIN, musicvol/100.f);
            //reportSoundError("alGetSourcei-updateMusicVol", "Error updating music volume");
        }
    }
}

void updateListenerPos()
{
    ALfloat listenerPos[] = {camera1->o.x, camera1->o.z, camera1->o.y}; // updating listener position
    alListenerfv(AL_POSITION, listenerPos);

    ALfloat orientation[] = {               // combine the forward and up vectors for orientation
        camdir.x, camdir.z, camdir.y,       // forward vector (inverted for some obscure reason)
        0.0, 1.0, 0.0                       // up vector
    };

    alListenerfv(AL_ORIENTATION, orientation); // set the listener's orientation
    float f = game::hudplayer()->boostmillis[B_SHROOMS] ? 3.f : (game::hudplayer()->boostmillis[B_EPO] ? 20 : 70.f);
    alListener3f(AL_VELOCITY, game::hudplayer()->vel.x/f, game::hudplayer()->vel.z/f, game::hudplayer()->vel.y/f); // set the listener's velocity
}

void updateSoundOcclusion(int id)
{
    if(totalmillis >= sounds[id].nextOcclusionCheck)
    {
        bool occlusion = isOccluded(id);
        if(sounds[id].isCurrentlyOccluded != occlusion)
        {
            sounds[id].isCurrentlyOccluded = occlusion;
            sounds[id].lastOcclusionChange = totalmillis;
        }
        sounds[id].nextOcclusionCheck = totalmillis + getOcclusionCheckInterval(id);
    }

    bool occlusion = sounds[id].isCurrentlyOccluded;
    float targetLF = occlusion ? 0.90f : 1.0f;
    float targetHF = occlusion ? 0.20f : 1.0f;
    bool transitioning = (sounds[id].lfOcclusionGain != targetLF || sounds[id].hfOcclusionGain != targetHF);
    if(!transitioning) return;

    float progress = min(1.0f, (totalmillis - sounds[id].lastOcclusionChange) / 750.f);

    if(progress < 1.0f)
    {
        sounds[id].lfOcclusionGain = lerp(sounds[id].lfOcclusionGain, targetLF, progress);
        sounds[id].hfOcclusionGain = lerp(sounds[id].hfOcclusionGain, targetHF, progress);
    }
    else
    {
        sounds[id].lfOcclusionGain = targetLF;
        sounds[id].hfOcclusionGain = targetHF;
    }

    alFilterf(sounds[id].occlusionFilter, AL_LOWPASS_GAIN, sounds[id].lfOcclusionGain);
    alFilterf(sounds[id].occlusionFilter, AL_LOWPASS_GAINHF, sounds[id].hfOcclusionGain);
    alSourcei(sounds[id].alSource, AL_DIRECT_FILTER, sounds[id].occlusionFilter);
}

void updateSoundPosition(int id)
{
    if(game::ispaused()) return;
    vec pos, vel;
    getEntMovement(sounds[id].entityId, pos, vel);
    sounds[id].position = pos;
    sounds[id].velocity = vel;
    applyReverb(sounds[id].alSource, getReverbZone(pos));
    alSource3f(sounds[id].alSource, AL_VELOCITY, vel.x, vel.z, vel.y);
    alSource3f(sounds[id].alSource, AL_POSITION, pos.x, pos.z, pos.y);
}

void manageSources()
{
    if(noSound) return;

    ALint state;
    const int nonOcclusionFlags = SND_MUSIC | SND_UI;
    for(int i = 0; i < activeSourceIds.length(); )
    {
        int id = activeSourceIds[i];
        alGetSourcei(sounds[id].alSource, AL_SOURCE_STATE, &state);

        if(state == AL_STOPPED)
        {
            recycleSource(i);
            continue;
        }

        if(sounds[id].entityId != SIZE_MAX) updateSoundPosition(id);
        if(!(sounds[id].soundFlags & nonOcclusionFlags)) updateSoundOcclusion(id);
        ++i;
    }
}

void soundNearmiss(int sound, const vec &from, const vec &to, int precision)
{
    if(noSound) return;
    const vec &cameraPos = camera1->o;
    const float nearMissRadiusSq = 64.f * 64.f;
    vec v;
    float d = to.dist(from, v);
    int steps = clamp(int(d*2), 1, precision);
    v.div(steps);
    vec p = from;
    loopi(steps)
    {
        p.add(v);
        if(cameraPos.fastsquaredist(p) <= nearMissRadiusSq)
        {
            playSound(sound, p, 128, 50, SND_LOWPRIORITY|SND_NOOCCLUSION);
            return;
        }
    }
}

void stopMapSound(extentity *e, bool deleteEnt)
{
    if(!e) return;

    size_t entityId = e->entityId;

    for(int i = 0; i < activeSourceIds.length(); )
    {
        int id = activeSourceIds[i];
        if(sounds[id].entityId == entityId)
        {
            alSourceStop(sounds[id].alSource);
            recycleSource(i);

            removeEntityPos(e->entityId);
            if(!deleteEnt) e->flags &= ~EF_SOUND;
        }
        else ++i;
    }
}

void stopAllMapSounds()
{
    const vector<extentity *> &ents = entities::getents();

    loopv(ents)
    {
        extentity &e = *ents[i];
        if(e.type!=ET_SOUND || !(e.flags & EF_SOUND)) continue;
        stopMapSound(&e);
    }
}
ICOMMAND(stopmapsounds, "", (), stopAllMapSounds());

static inline void updateMapSoundCache(const vector<extentity *> &ents)
{
    mapSoundIds.shrink(0);
    mapSoundIds.reserve(ents.length());
    loopv(ents)
    {
        extentity *e = ents[i];
        if(!e) continue;
        if(e->type == ET_SOUND || (e->flags & EF_SOUND)) mapSoundIds.add(i);
    }
    nextMapSoundsUpdate = totalmillis + MAP_SOUNDS_UPDATE_INTERVAL;
}

void checkMapSounds(bool force)
{
    if(mainmenu) return;
    const vector<extentity *> &ents = entities::getents();

    if((editmode && totalmillis >= nextMapSoundsUpdate) || force) updateMapSoundCache(ents);

    const vec &cameraPos = camera1->o;
    for(int i = 0; i < mapSoundIds.length(); )
    {
        int entIndex = mapSoundIds[i];
        if(!ents.inrange(entIndex)) { mapSoundIds.removeunordered(i); continue; }

        extentity *entity = ents[entIndex];
        if(!entity) { mapSoundIds.removeunordered(i); continue; }
        extentity &e = *entity;

        if(e.type == ET_EMPTY)
        {
            stopMapSound(&e, true);
            mapSoundIds.removeunordered(i);
            continue;
        }
        else if(e.type != ET_SOUND)
        {
            if(!(e.flags & EF_SOUND)) mapSoundIds.removeunordered(i);
            else ++i;
            continue;
        }

        float maxDist = e.attr2 + 50.f;
        if(maxDist > 0.f && cameraPos.fastsquaredist(e.o) < maxDist * maxDist) // check for distance + add a slight tolerance for efx sound effects
        {
            updateEntPos(e.entityId, e.o, vec(0, 0, 0));
            if(!(e.flags & EF_SOUND))
            {
                playSound(e.attr1, e.o, e.attr2, e.attr3, SND_LOOPED|SND_MAPSOUND|SND_FIXEDPITCH, e.entityId);
                e.flags |= EF_SOUND;  // set the flag to indicate that the sound is currently playing
            }
        }
        else if(e.flags & EF_SOUND) stopMapSound(&e);
        ++i;
    }
}

void stopLinkedSound(size_t entityId, int soundType, bool clear)
{
    for(int i = 0; i < activeSourceIds.length(); )
    {
        int id = activeSourceIds[i];
        if(sounds[id].entityId == entityId && (sounds[id].soundType == soundType || clear))
        {
            alSourceStop(sounds[id].alSource);
            //reportSoundError("alSourceStop-stopLinkedSound", "Error while stopping linked sound");
            recycleSource(i);
        }
        else ++i;
    }
}

void updateSoundPitch(size_t entityId, int soundType, float pitch)
{
    if(game::gamespeed != 100) pitch *= (game::gamespeed / 100.f);
    loopv(activeSourceIds)
    {
        int id = activeSourceIds[i];
        if(sounds[id].entityId == entityId && sounds[id].soundType == soundType)
        {
            alSourcef(sounds[id].alSource, AL_PITCH, pitch);
            //reportSoundError("updateSoundPitch", "Error while modifying pitch");
        }
    }
}

void stopAllSounds(bool pause)
{
    for(int i = 0; i < activeSourceIds.length(); )
    {
        int id = activeSourceIds[i];
        if(pause || !(sounds[id].soundFlags & SND_MUSIC))
        {
            if(pause)
            {
                alSourcePause(sounds[id].alSource);
                ++i;
            }
            else
            {
                alSourceStop(sounds[id].alSource);
                recycleSource(i);
            }
        }
        else ++i;
    }
}

void resumeAllSounds()
{
    loopv(activeSourceIds)
    {
        int id = activeSourceIds[i];
        alSourcePlay(sounds[id].alSource);
    }
}

void updateSounds()
{
    if(noSound) return;
    manageSources();
    if(!isconnected()) return;
    updateListenerPos();
    checkMapSounds();
}

void cleanUpSounds()
{
    if(noSound) return;

    stopAllSounds();

    loopi(MAX_SOURCES)
    {
        alDeleteSources(1, &sounds[i].alSource);
        if(!noEfx) alDeleteFilters(1, &sounds[i].occlusionFilter);
    }

    if(!noEfx) alDeleteAuxiliaryEffectSlots(NUMREVERBS, auxEffectSlots);
    ALCcontext* context = alcGetCurrentContext();
    ALCdevice* device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

ICOMMAND(playSound, "sii", (char *soundName, bool fixedPitch, bool uiSound),
    loopi(NUMSNDS)
    {
        if(!strcasecmp(soundName, gameSounds[i].soundPath))
        {
            int flags = 0;
            if(fixedPitch) flags |= SND_FIXEDPITCH;
            if(uiSound) flags |= SND_UI;
            playSound(i, vec(0, 0, 0), 0, 0, flags);
            return;
        }
    }
    conoutf(CON_ERROR, "sound not found or not loaded: %s", soundName);
);

void playMusic(int musicId)
{
    if(!musicvol) return;
    if(musicId>=0 && musicId<=NUMSONGS) playSound(musicId, vec(0, 0, 0), 0, 0, SND_MUSIC);
    else conoutf("Invalid music ID (must be 0 to %d)", NUMSONGS);
    updateMusicVol();
}
ICOMMAND(playmusic, "i", (int *i), playMusic(*i));

const char *getmapsoundname(int n)
{
    if(n < 0 || n >= NUMMAPSNDS || !mapSounds[n].loaded) return readstr("Misc_InvalidId");
    else return mapSounds[n].soundPath;
}
