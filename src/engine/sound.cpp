// sound.cpp: basic positional sound using sdl_mixer

#include "engine.h"
#include "game.h"
#include <sndfile.h>
#include <AL/efx.h>
#include <AL/alext.h>

bool foundDevice = false;
bool noSound = true;
bool noEfx = false;

//VAR(debugsounds, 0, 0, 1);
VARFP(soundvol, 0, 100, 100, alListenerf(AL_GAIN, soundvol/100.f); );
extern void updateMusicVol();
VARFP(musicvol, 0, 50, 100, updateMusicVol());
VARFP(mutesounds, 0, 0, 1, alListenerf(AL_GAIN, mutesounds ? 0.f : soundvol/100.f));
VARP(minimizedmute, 0, 0, 1);
VARP(maxsoundsatonce, 32, 96, MAX_SOURCES);
VARP(soundfreq, 0, 44100, 48000);

int gameSoundId = 0;
Sound gameSounds[NUMSNDS];
ICOMMAND(gamesound, "siii", (char *path, int *alts, int *vol, bool *occl),
    if(gameSoundId >= NUMSNDS) { conoutf(CON_ERROR, "Unable to load sound: %s (id %d is greater than amount of sounds)", path, gameSoundId); return; }
    formatstring(gameSounds[gameSoundId].soundPath, "%s", path);
    gameSounds[gameSoundId].numAlts = *alts;
    gameSounds[gameSoundId].soundVol = *vol;
    gameSounds[gameSoundId].noGeomOcclusion = *occl;
    gameSounds[gameSoundId].loaded = loadSound(gameSounds[gameSoundId]);
    loadSound(gameSounds[gameSoundId]);
    gameSoundId++;
);

#define NUMMAPSNDS 32 // temporary fixed shit
int mapSoundId = 0;
Sound mapSounds[NUMMAPSNDS];
ICOMMAND(mapsound, "siii", (char *path, int *alts, int *vol, bool *occl),
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
    music[musicId].noGeomOcclusion = true;
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

ALuint reverbEffect, auxEffectReverb;

void applyReverbPreset(ALuint auxEffectSlot, const EFXEAXREVERBPROPERTIES& preset)
{
    alGenEffects(1, &reverbEffect);

    alEffecti(reverbEffect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
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

    alAuxiliaryEffectSloti(auxEffectReverb, AL_EFFECTSLOT_EFFECT, reverbEffect);
    //reportSoundError("applyReverbPreset", "Error while setting reverb preset");
}

void setReverbGain(float gain)
{
    alEffectf(reverbEffect, AL_EAXREVERB_GAIN, clamp(gain, 0, 1));
    alAuxiliaryEffectSloti(auxEffectReverb, AL_EFFECTSLOT_EFFECT, reverbEffect);
    //reportSoundError("setReverbGain", "Error while setting reverb gain");
}

void setReverbPreset(int preset)
{
    if(preset >= 0 && preset < numPresets) applyReverbPreset(auxEffectReverb, presets[preset]);
    else applyReverbPreset(auxEffectReverb, EFX_REVERB_PRESET_GENERIC);
}

VARF(mainmapreverb, 0, 0, numPresets, setReverbPreset(mainmapreverb));

ALuint occlusionFilter;

void setOcclusionEffect()
{
    if(noEfx) return;

    alGenFilters(1, &occlusionFilter);
    //reportSoundError("alGenFilters-occlusionFilter", "Error while initing occlusion filter");

    alFilteri(occlusionFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    //reportSoundError("alFilteri-occlusionFilter", "Error while setting occlusion filter");

    alFilterf(occlusionFilter, AL_LOWPASS_GAIN, 0.5f);
    alFilterf(occlusionFilter, AL_LOWPASS_GAINHF, 0.1f);
}

SoundSource sources[MAX_SOURCES];

void initSoundSources()
{
    loopi(MAX_SOURCES)
    {
        alGenSources(1, &sources[i].source);
        //reportSoundError("initSoundSources", "Error while initing sound sources");

        sources[i].isActive = false;
        sources[i].entityId = SIZE_MAX;  // set to SIZE_MAX initially to represent "no entity"
        sources[i].soundType = 0;
        sources[i].noGeomOcclusion = false;
    }
}

ALCcontext* context = NULL;

void initSounds()
{
    ALCdevice* device = alcOpenDevice(NULL); // open default device
    if(!device) { conoutf("Unable to initialize OpenAL (no audio device detected)"); return; }

    ALCint attributes[] = {
        ALC_FREQUENCY, soundfreq,
        ALC_HRTF_SOFT, ALC_FALSE,
        0
    };

    ALCcontext* context = alcCreateContext(device, attributes);
    if(!context) { conoutf("Unable to initialize OpenAL (!context)"); return; }
    alcMakeContextCurrent(context);

    noSound = false;

    if(!alcIsExtensionPresent(device, "ALC_EXT_EFX")) // check for EFX extension support
    {
        conoutf(CON_WARN, "EFX extension not available.");
        noEfx = true;
    }
    else
    {
        getEfxFuncs();
        alGenAuxiliaryEffectSlots(1, &auxEffectReverb);
        setOcclusionEffect();
    }

    alDistanceModel(AL_LINEAR_DISTANCE);
    alDopplerFactor(1.0f);
    alSpeedOfSound(343.3f);
    initSoundSources();
}

bool checkSoundOcclusion(const vec *soundPos)
{
    if(!soundPos || noEfx || (camera1->o.dist(*soundPos) < 50 && camera1->o.dist(*soundPos) > 1000)) return false;

    vec dir = vec(camera1->o).sub(*soundPos);
    const float dist = dir.magnitude();
    dir.mul(1/dist);

    const float toleranceRadius = 27.f;

    const int sectors = 4;
    const int stacks = 4;
    const float sectorAngle = 2 * M_PI / sectors;
    const float stackAngle = M_PI / stacks;

    const vec camPos = camera1->o;

    loopi(sectors)
    {
        loopj(stacks)
        {
            float theta = i * sectorAngle;
            float phi = j * stackAngle;

            vec samplePoint = *soundPos;
            samplePoint.x += toleranceRadius * sin(phi) * cos(theta);
            samplePoint.y += toleranceRadius * sin(phi) * sin(theta);
            samplePoint.z += toleranceRadius * cos(phi);

            const float rayDist = raycube(samplePoint, dir, camPos.dist(samplePoint), RAY_CLIPMAT|RAY_POLY);

            if(rayDist >= dist) return false;
        }
    }
    return true;
}

bool applyUnderwaterFilter(int flags)
{
    if(lookupmaterial(camera1->o)==MAT_WATER || lookupmaterial(camera1->o)==MAT_LAVA)
    {
        setReverbGain(0.1);
        return true;
    }
    else setReverbGain(0.35);
    return false;
}

float getRandomSoundPitch()
{
    return (0.92f + 0.16f * static_cast<float>(rand()) / RAND_MAX) * (game::gamespeed / 100.f);
}

void playSound(int soundId, const vec *soundPos, float maxRadius, float maxVolRadius, int flags, size_t entityId, int soundType)
{
    if(soundId < 0 || soundId > NUMSNDS || noSound || mutesounds) return; // invalid index or openal not initialized or mute

    if(soundPos && !(flags & SND_NOCULL) && camera1->o.dist(*soundPos) > maxRadius + 50) return; // do not play sound too far from camera, except if flag SND_NOCULL

    size_t sourceIndex = SIZE_MAX; // default to invalid index
    int activeSources = 0;

    loopi(maxsoundsatonce)
    {
        if(!sources[i].isActive)
        {
            if(sourceIndex == SIZE_MAX) sourceIndex = i; // find the first inactive source
        }
        else activeSources++; // count active sources
    }

    if(sourceIndex == SIZE_MAX) { conoutf(CON_WARN, "Max sounds at once capacity reached (%d)", maxsoundsatonce); return; } // skip sound if max capacity
    if((flags & SND_LOWPRIORITY) && (activeSources >= maxsoundsatonce / 2)) return; // skip low-priority sounds (distant shoots etc.) when we are already playing a lot of sounds

    sources[sourceIndex].isActive = true;       // now the source is set to active
    sources[sourceIndex].entityId = entityId;
    sources[sourceIndex].soundType = soundType;
    sources[sourceIndex].soundId = soundId;
    sources[sourceIndex].soundFlags = flags;

    Sound& s = (flags & SND_MUSIC) ? music[soundId] : ( (flags & SND_MAPSOUND) ? mapSounds[soundId] : gameSounds[soundId] );
    ALuint source = sources[sourceIndex].source;
    if(s.noGeomOcclusion) sources[sourceIndex].noGeomOcclusion = true;

    ALuint buffer = s.bufferId[s.numAlts ? rnd(s.numAlts + 1) : 0];

    alSourcei(source, AL_BUFFER, buffer); // managing sounds alternatives
    alSourcef(source, AL_GAIN, s.soundVol / 100.f); // managing sound volume
    alSourcef(source, AL_PITCH, !(flags & SND_FIXEDPITCH) && !(flags & SND_MUSIC) ? getRandomSoundPitch() : 1*(game::gamespeed / 100.f)); // managing variations of pitches
    alSourcei(source, AL_LOOPING, (flags & SND_LOOPED) ? AL_TRUE : AL_FALSE); // loop the sound or not

    if(!soundPos)
    {
        ALfloat sourcePos[] = {0.f, 0.f, 0.f};
        alSourcefv(source, AL_POSITION, sourcePos);
        alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE); // hud sound, make the source relative to the listener
    }
    else
    {
        ALfloat sourcePos[] = {soundPos->x, soundPos->z, soundPos->y};
        alSourcefv(source, AL_POSITION, sourcePos);
        alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);

        alSourcef(source, AL_MAX_DISTANCE, maxRadius);
        alSourcef(source, AL_REFERENCE_DISTANCE, maxVolRadius); // Start decreasing volume immediately
        alSourcef(source, AL_ROLLOFF_FACTOR, 1.0f); // For linear decrease over the distance
    }

    if((flags & SND_MUSIC) || (flags & SND_UI))
    {
        alSource3i(source, AL_AUXILIARY_SEND_FILTER, AL_FILTER_NULL, 0, AL_FILTER_NULL);
        alSourcei(source, AL_DIRECT_FILTER, AL_FILTER_NULL);
    }
    else if(!noEfx) // apply efx if available
    {
        ALuint filter = AL_FILTER_NULL;
        if(!(flags & SND_UI) && !(flags & SND_MUSIC) && (applyUnderwaterFilter(flags) || checkSoundOcclusion(soundPos))) filter = occlusionFilter;
        alSource3i(source, AL_AUXILIARY_SEND_FILTER, auxEffectReverb, 0, filter);
        alSourcei(source, AL_DIRECT_FILTER, filter);
    }

    alSourcePlay(source);
    //reportSoundError("alSourcePlay", s.soundPath, buffer);
}

void stopSound(int soundId, int flags)
{
    if(soundId < 0 || soundId > NUMSNDS || noSound) return; // invalid index or openal not initialized

    loopi(maxsoundsatonce)
    {
        if(sources[i].isActive && sources[i].soundId == soundId && sources[i].soundFlags == flags)
        {
            alSourceStop(sources[i].source);
            sources[i].isActive = false;
            break;
        }
    }
}

void stopMusic(int soundId)
{
    stopSound(soundId, SND_MUSIC);
}

void manageSources()
{
    ALint state;
    loopi(maxsoundsatonce)
    {
        if(sources[i].isActive && sources[i].source) // check if the source is active and valid
        {
            alGetSourcei(sources[i].source, AL_SOURCE_STATE, &state);
            //reportSoundError("alGetSourcei-manageSources", "Error querying source state");
            sources[i].isActive = state != AL_STOPPED; // isActive = false if state is AL_STOPPED
        }
    }
}

void updateMusicVol()
{
    loopi(maxsoundsatonce)
    {
        if(sources[i].isActive && (sources[i].soundFlags & SND_MUSIC))
        {
            alSourcef(sources[i].source, AL_GAIN, musicvol/100.f);
            //reportSoundError("alGetSourcei-updateMusicVol", "Error updating music volume");
        }
    }
}

void updateListenerPos()
{
    ALfloat listenerPos[] = {camera1->o.x, camera1->o.z, camera1->o.y}; // updating listener position
    alListenerfv(AL_POSITION, listenerPos);

    camdir.normalize();     // normalize the forward vector

    vec up = vec(0, 0, 1);  // no roll ATM (z-axis is up in cube engine)

    ALfloat orientation[] = {               // combine the forward and up vectors for orientation
        camdir.x, camdir.z, camdir.y,       // forward vector (inverted for some obscure reason)
        up.x, up.z, up.y                    // up vector
    };

    alListenerfv(AL_ORIENTATION, orientation); // set the listener's orientation
    float f = game::hudplayer()->boostmillis[B_SHROOMS] ? 3.f : 70.f;
    alListener3f(AL_VELOCITY, camera1->vel.x/f, camera1->vel.z/f, camera1->vel.y/f); // set the listener's velocity
}

void updateSoundPosition(size_t entityId, const vec &newPosition, const vec &velocity, int soundType)
{
    if(noSound) return;

    loopi(maxsoundsatonce) // loop through all the SoundSources and find the one with the matching entityId
    {
        if(sources[i].entityId == entityId && sources[i].soundType == soundType) // found the correct sound source, now update its position
        {
            bool occlusion = applyUnderwaterFilter(NULL) ? true : ( !sources[i].noGeomOcclusion && (checkSoundOcclusion(&newPosition)) );
            alSourcei(sources[i].source, AL_DIRECT_FILTER, occlusion ? occlusionFilter : AL_FILTER_NULL);
            alSource3f(sources[i].source, AL_VELOCITY, velocity.x, velocity.z, velocity.y);
            alSource3f(sources[i].source, AL_POSITION, newPosition.x, newPosition.z, newPosition.y);
            break;
        }
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
            playSound(sound, &p, 300, 50, SND_LOWPRIORITY);
            return;
        }
    }
}

void stopMapSound(extentity *e)
{
    if(!e) return;

    size_t entityId = e->entityId;

    SoundSource* soundSource = NULL;
    loopi(maxsoundsatonce)
    {
        if(sources[i].entityId == entityId && sources[i].isActive) { soundSource = &sources[i]; break; }
    }

    if(soundSource)
    {
        alSourceStop(soundSource->source);
        //reportSoundError("alSourceStop-stopMapSound", tempformatstring("Sound %d", e->attr1));
        soundSource->isActive = false;
        e->flags &= ~EF_SOUND;
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

        if(e.type!=ET_SOUND) continue;

        if(e.entityId != SIZE_MAX) updateSoundPosition(e.entityId, e.o);
        if(camera1->o.dist(e.o) < e.attr2 + 50) // check for distance + add a slight tolerance for efx sound effects
        {
            if(!(e.flags & EF_SOUND))
            {
                playSound(e.attr1, &e.o, e.attr2, e.attr3, SND_LOOPED|SND_MAPSOUND|SND_FIXEDPITCH, e.entityId);
                e.flags |= EF_SOUND;  // set the flag to indicate that the sound is currently playing
            }
        }
        else if(e.flags & EF_SOUND) stopMapSound(&e);
    }
}

void stopLinkedSound(size_t entityId, int soundType)
{
    SoundSource* soundSource = NULL;
    loopi(maxsoundsatonce) if(sources[i].entityId == entityId && sources[i].soundType == soundType) { soundSource = &sources[i]; break; }

    if(soundSource)
    {
        alSourceStop(soundSource->source);
        //reportSoundError("alSourceStop-stopLinkedSound", "Error while stopping linked sound");
        alSource3f(soundSource->source, AL_VELOCITY, 0.f, 0.f, 0.f);
        soundSource->isActive = false;
    }
}

void stopAllSounds(bool pause)
{
    loopi(maxsoundsatonce)
    {
        if(sources[i].isActive && (pause || !(sources[i].soundFlags & SND_MUSIC)))
        {
            if(pause) { alSourcePause(sources[i].source); continue; }
            alSourceStop(sources[i].source);
            sources[i].isActive = false;
        }
    }
}

void resumeAllSounds()
{
    loopi(maxsoundsatonce)
    {
        if(sources[i].isActive) alSourcePlay(sources[i].source);
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

    ALuint id[MAX_SOURCES];
    loopi(MAX_SOURCES) id[i] = sources[i].source;
    alDeleteSources(MAX_SOURCES, id);

    alDeleteEffects(1, &reverbEffect);
    alDeleteAuxiliaryEffectSlots(1, &auxEffectReverb);

    ALCcontext* context = alcGetCurrentContext();
    ALCdevice* device = alcGetContextsDevice(context);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

void setShroomsEfx(bool enable)
{
    if(enable) applyReverbPreset(auxEffectReverb, EFX_REVERB_PRESET_DRUGGED);
    else setReverbPreset(mainmapreverb);
}

ICOMMAND(playSound, "sii", (char *soundName, bool fixedPitch, bool uiSound),
    loopi(NUMSNDS)
    {
        if(!strcasecmp(soundName, gameSounds[i].soundPath))
        {
            int flags = 0;
            if(fixedPitch) flags |= SND_FIXEDPITCH;
            if(uiSound) flags |= SND_UI;
            playSound(i, NULL, 0, 0, flags);
            return;
        }
    }
    conoutf(CON_ERROR, "sound not found or not loaded: %s", soundName);
);

void playMusic(int musicId)
{
    if(!musicvol) return;
    if(musicId>=0 && musicId<=NUMSONGS) playSound(musicId, NULL, 0, 0, SND_MUSIC);
    else conoutf("Invalid music ID (must be 0 to %d)", NUMSONGS);
    updateMusicVol();
}
ICOMMAND(playmusic, "i", (int *i), playMusic(*i));

const char *getmapsoundname(int n)
{
    if(n < 0 || !mapSounds[n].loaded) return readstr("Misc_InvalidId");
    else return mapSounds[n].soundPath;
}
