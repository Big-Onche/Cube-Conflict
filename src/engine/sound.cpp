// sound.cpp: basic positional sound using sdl_mixer

#include "engine.h"
#include "game.h"
#include <sndfile.h>
#include <AL/efx.h>
#include <AL/alext.h>
#include <unordered_set>

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
ICOMMAND(mapsound, "siii", (char *path, int *alts, int *vol, int *occl),
    if(mapSoundId >= NUMMAPSNDS) { conoutf(CON_ERROR, "Unable to load map sound: %s (id %d is greater than amount of map sounds)", path, mapSoundId); return; }
    formatstring(mapSounds[mapSoundId].soundPath, "%s", path);
    mapSounds[mapSoundId].numAlts = *alts;
    mapSounds[mapSoundId].soundVol = *vol;
    mapSounds[mapSoundId].noGeomOcclusion = *occl;
    mapSounds[mapSoundId].loaded = loadSound(mapSounds[mapSoundId]);
    mapSoundId++;
);

ICOMMAND(resetmapsounds, "", (), mapSoundId = 0);

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
    SNDFILE* file;
    SF_INFO sfinfo;
    short* membuf;
    sf_count_t num_frames;
    ALsizei num_bytes;

    file = sf_open(tempformatstring("%s", filepath), SFM_READ, &sfinfo); // open the audio file and check if valid
    if(!file) { conoutf(CON_WARN, "Could not load audio file: %s", filepath); return false; }

    num_frames = sfinfo.frames; // get the number of frames in the sound file
    membuf = new short[num_frames * sfinfo.channels];
    if(membuf == nullptr)
    {
        conoutf(CON_ERROR, "Could not allocate memory for the sound data.");
        sf_close(file);
        return false;
    }

    // read the sound data into the memory buffer
    num_frames = sf_readf_short(file, membuf, num_frames);
    num_bytes = num_frames * sizeof(short) * sfinfo.channels;

    ALenum format;
    switch(sfinfo.channels) // determine the OpenAL format from the number of channels
    {
        case 1: format = AL_FORMAT_MONO16; break;
        case 2: format = AL_FORMAT_STEREO16; break;
        default:
            conoutf(CON_ERROR, "Unsupported channel count in %s (%d instead of 1 or 2)", filepath, sfinfo.channels);
            delete[] membuf;
            sf_close(file);
            return false;
    }

    alBufferData(buffer, format, membuf, num_bytes, sfinfo.samplerate); // upload the sound data to the OpenAL buffer
    //reportSoundError("alBufferData", filepath, buffer);

    delete[] membuf; // clean up and free resources
    sf_close(file);
    return true;
}

bool loadSound(Sound& s, bool music) // load a sound including his alts
{
    if(noSound) return false;
    bool loaded = false;
    loopi(s.numAlts ? s.numAlts+1 : 1)
    {
        if(s.bufferId[i]) alDeleteBuffers(1, &s.bufferId[i]); // delete the buffer if it already exists.

        defformatstring(fullPath, s.numAlts ? "media/sounds/%s_%d.wav" : "media/sounds/%s.wav", s.soundPath, i + 1);
        if(music) formatstring(fullPath, s.numAlts ? "media/songs/%s_%d.flac" : "media/songs/%s.flac", s.soundPath, i + 1);

        alGenBuffers(1, &s.bufferId[i]);
        //reportSoundError("alGenBuffers", s.soundPath, s.bufferId[i]);
        loaded = loadSoundFile(fullPath, s.bufferId[i]);
    }
    return loaded;
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

void initSoundSources()
{
    loopi(MAX_SOURCES)
    {
        alGenSources(1, &sounds[i].alSource);
        //reportSoundError("initSoundSources", "Error while initing sound sources");
        if(!noEfx)
        {
            alGenFilters(1, &sounds[i].occlusionFilter);
            alFilteri(sounds[i].occlusionFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
        }
    }
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

const int vertices = 3;
const float sectorAngle = 2 * M_PI / vertices;

bool checkSoundOcclusion(const vec *soundPos, int flags = 0)
{
    if(*soundPos == vec(0, 0, 0) || (flags & SND_NOOCCLUSION) || noEfx) return false;

    float distance = camera1->o.dist(*soundPos);
    if(distance > 1000) return false; // cull distant sounds

    vec hitPos;
    //particle_splash(PART_SPARK, 1, 250, *soundPos, 0x00FF00, 1.f, 1, 0);
    if(raycubelos(*soundPos, camera1->o, hitPos)) return false; // we first try a direct ray check
    else if(distance < 300) // more complex check with a tolerance if the sound is closer
    {
        vec dir = vec(camera1->o).sub(*soundPos).normalize(); // Direction vector pointing from sound to camera
        vec right = vec(0, 0, 0).cross(dir, vec(0, 0, 1)).normalize(); // Recalculating right vector to be orthogonal to dir
        vec perp = vec(0, 0, 0).cross(right, dir).normalize(); // Recalculating perpendicular vector

        loopi(vertices) // creating a triangle that always faces the camera
        {
            float theta = i * sectorAngle + ((totalmillis / 75.f) * M_PI); // spinning the triangle to get a circle of points
            vec displacement = right;

            float toleranceRadius = 25 + (distance / 20.f); // increase tolerance according to distance to simulate sound dispersion

            displacement.mul(cos(theta) * toleranceRadius);

            vec verticalDisplacement = perp;
            verticalDisplacement.mul(sin(theta) * toleranceRadius);

            vec samplePoint = *soundPos;
            samplePoint.add(displacement);
            samplePoint.add(verticalDisplacement);
            //particle_splash(PART_SPARK, 1, 250, samplePoint, 0x00FF00, 1.f, 1, 0);
            if(raycubelos(samplePoint, camera1->o, hitPos)) return false; // if one of the vertices is not occluded, no occlusion filter is applied
        }
    }
    return true;
}

bool isOccluded(int id)
{
    if(sounds[id].soundFlags & (SND_MUSIC | SND_UI)) return false;
    return isUnderWater(sounds[id].position) || checkSoundOcclusion(&sounds[id].position, sounds[id].soundFlags);
}

float getRandomSoundPitch(int flags)
{
    if(flags & (SND_MUSIC | SND_UI)) return 1;
    float pitchSpeed = (game::gamespeed == 100 ? 1.f : (game::gamespeed / 100.f));
    if(flags & SND_FIXEDPITCH) return pitchSpeed;
    return (0.92f + 0.16f * static_cast<float>(rand()) / RAND_MAX) * pitchSpeed;
}

std::unordered_set<size_t> activeSources;

inline bool canPlaySound(int soundId, int maxRadius, int flags, vec soundPos, bool hasSoundPos)
{
    if(soundId < 0 || soundId >= NUMSNDS || noSound || mutesounds) return false; // invalid index or openal not initialized or mute

    if(hasSoundPos && !(flags & SND_NOCULL) && camera1->o.dist(soundPos) > maxRadius + 50) return false; // do not play sound too far from camera, except if flag SND_NOCULL
    if((flags & SND_LOWPRIORITY) && (activeSources.size() >= (size_t)maxsoundsatonce / 2)) return false; // skip low-priority sounds (distant shoots etc.) when we are already playing a lot of sounds
    if(activeSources.size() >= (size_t)maxsoundsatonce) // no inactive source
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

    int id = 0;
    loopi(maxsoundsatonce) { if(!sounds[i].isActive) { id = i; break; } } // search after an inactive source

    // now we set a shitload of sound parameters
    activeSources.insert(id);
    sounds[id].isActive = true;       // now the source is set to active
    sounds[id].entityId = entityId;
    sounds[id].soundType = soundType;
    sounds[id].soundId = soundId;
    sounds[id].soundFlags = flags;
    sounds[id].position = soundPos;
    sounds[id].velocity = vec(0, 0, 0);

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
    if(soundId < 0 || soundId > NUMSNDS || noSound) return; // invalid index or openal not initialized

    for(auto it = activeSources.begin(); it != activeSources.end(); /* no increment here */)
    {
        size_t id = *it;
        if(sounds[id].soundId == soundId && sounds[id].soundFlags == flags)
        {
            alSourceStop(sounds[id].alSource);
            sounds[id].isActive = false;
            it = activeSources.erase(it);
        }
        else it++;
    }
}

void stopMusic(int soundId)
{
    stopSound(soundId, SND_MUSIC);
}

void updateMusicVol()
{
    for(auto it = activeSources.begin(); it != activeSources.end(); it++)
    {
        size_t id = *it;
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
    bool occlusion = isOccluded(id);

    // only update if changing or if we are mid-transition
    bool changed = (sounds[id].isCurrentlyOccluded != occlusion);
    bool transitioning = (sounds[id].lfOcclusionGain != (occlusion ? 0.90f : 1.0f)
                       || sounds[id].hfOcclusionGain != (occlusion ? 0.20f : 1.0f));

    if(!changed && !transitioning) return;

    if(changed)
    {
        sounds[id].isCurrentlyOccluded = occlusion;
        sounds[id].lastOcclusionChange = totalmillis;
    }

    float progress = min(1.0f, (totalmillis - sounds[id].lastOcclusionChange) / 750.f);

    float targetLF = occlusion ? 0.90f : 1.0f;
    float targetHF = occlusion ? 0.20f : 1.0f;

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
    for(auto it = activeSources.begin(); it != activeSources.end(); /* no increment here */)
    {
        size_t id = *it;
        alGetSourcei(sounds[id].alSource, AL_SOURCE_STATE, &state);

        if(state == AL_STOPPED)
        {
            sounds[id].isActive = false;
            it = activeSources.erase(it); // Erase and advance iterator in one step to avoid invalidating the iterator
        }
        else ++it; // Only increment iterator if not erasing

        if(sounds[id].entityId != SIZE_MAX) updateSoundPosition(id);
        updateSoundOcclusion(id);
    }
}

void soundNearmiss(int sound, const vec &from, const vec &to, int precision)
{
    if(noSound) return;
    vec v;
    float d = to.dist(from, v);
    int steps = clamp(int(d*2), 1, 1024);
    v.div(steps);
    vec p = from;
    loopi(steps)
    {
        p.add(v);
        if(camera1->o.dist(p) <= 64)
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

    for(auto it = activeSources.begin(); it != activeSources.end(); /* no increment here */)
    {
        size_t id = *it;
        if(sounds[id].entityId == entityId)
        {
            alSourceStop(sounds[id].alSource);
            sounds[id].isActive = false;
            it = activeSources.erase(it);

            removeEntityPos(e->entityId);
            if(!deleteEnt) e->flags &= ~EF_SOUND;
        }
        else ++it;
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

void checkMapSounds()
{
    if(mainmenu) return;
    const vector<extentity *> &ents = entities::getents();

    loopv(ents)
    {
        extentity &e = *ents[i];

        if(e.type == ET_EMPTY) { stopMapSound(&e, true); continue; }
        else if(e.type != ET_SOUND) continue;

        if(camera1->o.dist(e.o) < e.attr2 + 50) // check for distance + add a slight tolerance for efx sound effects
        {
            updateEntPos(e.entityId, e.o, vec(0, 0, 0));
            if(!(e.flags & EF_SOUND))
            {
                playSound(e.attr1, e.o, e.attr2, e.attr3, SND_LOOPED|SND_MAPSOUND|SND_FIXEDPITCH, e.entityId);
                e.flags |= EF_SOUND;  // set the flag to indicate that the sound is currently playing
            }
        }
        else if(e.flags & EF_SOUND) stopMapSound(&e);
    }
}

void stopLinkedSound(size_t entityId, int soundType, bool clear)
{
    for(auto it = activeSources.begin(); it != activeSources.end(); /* no increment here */)
    {
        size_t id = *it;
        if(sounds[id].entityId == entityId && (sounds[id].soundType == soundType || clear))
        {
            alSourceStop(sounds[id].alSource);
            //reportSoundError("alSourceStop-stopLinkedSound", "Error while stopping linked sound");
            sounds[id].isActive = false;
            it = activeSources.erase(it);
        }
        else it++;
    }
}

void updateSoundPitch(size_t entityId, int soundType, float pitch)
{
    if(game::gamespeed != 100) pitch *= (game::gamespeed / 100.f);
    for(auto it = activeSources.begin(); it != activeSources.end(); it++)
    {
        size_t id = *it;
        if(sounds[id].entityId == entityId && sounds[id].soundType == soundType)
        {
            alSourcef(sounds[id].alSource, AL_PITCH, pitch);
            //reportSoundError("updateSoundPitch", "Error while modifying pitch");
        }
    }
}

void stopAllSounds(bool pause)
{
    for(auto it = activeSources.begin(); it != activeSources.end(); /* no increment here */)
    {
        size_t id = *it;
        if(pause || !(sounds[id].soundFlags & SND_MUSIC))
        {
            if(pause)
            {
                alSourcePause(sounds[id].alSource);
                it++;
            }
            else
            {
                alSourceStop(sounds[id].alSource);
                sounds[id].isActive = false;
                it = activeSources.erase(it);
            }
        }
        else it++;
    }
}

void resumeAllSounds()
{
    for(auto it = activeSources.begin(); it != activeSources.end(); it++)
    {
        size_t id = *it;
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
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(alcGetContextsDevice(context));
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
    if(n < 0 || !mapSounds[n].loaded) return readstr("Misc_InvalidId");
    else return mapSounds[n].soundPath;
}
