// sound.cpp: basic positional sound using sdl_mixer

#include "engine.h"
#include "sound.h"
#include <sndfile.h>
#include <AL/efx.h>

bool foundDevice = false;
bool noSound = false;
bool noEfx = false;

VAR(debugsounds, 0, 1, 1);
VARP(minimizedsounds, 0, 0, 1);
VAR(stereo, 0, 1, 1);
VARP(soundvol, 0, 255, 255);
VARP(musicvol, 0, 255, 255);
VARP(soundfreq, 0, 44100, 48000);
VARP(maxsoundsatonce, 0, 256, 512);

#define NUMMAPSNDS 32
Sound gameSounds[NUMSNDS];
Sound mapSounds[NUMMAPSNDS];

ALuint sources[MAX_SOURCES];
bool sourceActive[MAX_SOURCES] = {false};
extentity* sourceOwners[MAX_SOURCES] = {NULL};

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
}

void reportSoundError(const char* func, const char* filepath, int buffer)
{
    if(!debugsounds) return;
    ALenum error = alGetError();
    if(error != AL_NO_ERROR) conoutf(CON_ERROR, "OpenAL Error after %s for %s: %d (Buffer ID: %d)", func, filepath, error, buffer);
}

ALuint reverbEffect;
ALuint auxEffectReverb;

void setReverbEffect()
{
    alGenAuxiliaryEffectSlots(1, &auxEffectReverb);
    reportSoundError("alGenAuxiliaryEffectSlots-auxEffectReverb", "N/A", -1);

    alGenEffects(1, &reverbEffect);
    alEffecti(reverbEffect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
    alEffectf(reverbEffect, AL_REVERB_DENSITY, 1.0f);
    alEffectf(reverbEffect, AL_REVERB_DIFFUSION, 1.0f);
    alEffectf(reverbEffect, AL_REVERB_GAIN, 0.5f);
    alEffectf(reverbEffect, AL_REVERB_GAINHF, 0.9f);
    alEffectf(reverbEffect, AL_REVERB_DECAY_TIME, 1.5f);
    alEffectf(reverbEffect, AL_REVERB_DECAY_HFRATIO, 0.5f);
    alEffectf(reverbEffect, AL_REVERB_REFLECTIONS_GAIN, 0.5f);
    alEffectf(reverbEffect, AL_REVERB_LATE_REVERB_GAIN, 1.0f);
    alEffectf(reverbEffect, AL_REVERB_LATE_REVERB_DELAY, 0.15f);
    alEffectf(reverbEffect, AL_REVERB_AIR_ABSORPTION_GAINHF, 1.0f);
    alEffectf(reverbEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, 0.0f);
    alEffecti(reverbEffect, AL_REVERB_DECAY_HFLIMIT, AL_TRUE);

    alAuxiliaryEffectSloti(auxEffectReverb, AL_EFFECTSLOT_EFFECT, reverbEffect);
    reportSoundError("alAuxiliaryEffectSloti-reverbEffect", "N/A", -1);
}

ALuint occlusionFilter;

void setOcclusionEffect()
{
    alGenFilters(1, &occlusionFilter);
    reportSoundError("alGenFilters-occlusionFilter", "N/A", -1);

    alFilteri(occlusionFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    reportSoundError("alFilteri-occlusionFilter", "N/A", -1);

    alFilterf(occlusionFilter, AL_LOWPASS_GAIN, 0.5f);  // Value between 0.0 to 1.0. Adjust to your liking.
    alFilterf(occlusionFilter, AL_LOWPASS_GAINHF, 0.1f);
}

void initSources()
{
    alGenSources(MAX_SOURCES, sources);
    loopi(MAX_SOURCES) sourceActive[i] = false;
}

void alInit()
{
    ALCdevice* device = alcOpenDevice(NULL); // open default device
    if(!device) { conoutf("Unable to initialize OpenAL (no audio device detected)"); noSound = true; return; }

    ALCint attributes[] = {
        ALC_FREQUENCY, 44100,
        0
    };

    ALCcontext* context = alcCreateContext(device, attributes);
    if(!context) { conoutf("Unable to initialize OpenAL (!context)"); noSound = true; return; }
    alcMakeContextCurrent(context);

    if(!alcIsExtensionPresent(device, "ALC_EXT_EFX")) // check for EFX extension support
    {
        conoutf(CON_WARN, "EFX extension not available.");
        noEfx = true;
    }
    else
    {
        getEfxFuncs();
        setReverbEffect();
        setOcclusionEffect();
    }

    alDistanceModel(AL_LINEAR_DISTANCE);
    initSources();
}

void alCleanUp()
{
    if(noSound) return;
    alDeleteSources(MAX_SOURCES, sources);
    alDeleteEffects(1, &reverbEffect);
    alDeleteAuxiliaryEffectSlots(1, &auxEffectReverb);
    ALCcontext* context = alcGetCurrentContext();
    ALCdevice* device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

ICOMMAND(gamesound, "isii", (int *id, char *path, int *alts, int *vol),
    formatstring(gameSounds[*id].soundPath, "%s", path);
    gameSounds[*id].numAlts = *alts;
    gameSounds[*id].soundVol = *vol;
    loadSound(gameSounds[*id]);
);

ICOMMAND(mapsound, "isii", (int *id, char *path, int *alts, int *vol),
    formatstring(mapSounds[*id].soundPath, "%s", path);
    mapSounds[*id].numAlts = *alts;
    mapSounds[*id].soundVol = *vol;
    loadSound(mapSounds[*id]);
);

void loadSoundFile(const string& filepath, ALuint buffer) // load a sound using libsndfile
{
    SNDFILE* file;
    SF_INFO sfinfo;
    short* membuf;
    sf_count_t num_frames;
    ALsizei num_bytes;

    file = sf_open(tempformatstring("%s", filepath), SFM_READ, &sfinfo); // open the audio file and check if valid
    if(!file) { conoutf(CON_WARN, "Could not open audio in %s: %s", filepath, sf_strerror(file)); return; }

    num_frames = sfinfo.frames; // get the number of frames in the sound file
    membuf = new short[num_frames * sfinfo.channels];
    if(membuf == nullptr)
    {
        conoutf(CON_ERROR, "Could not allocate memory for the sound data.");
        sf_close(file);
        return;
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
            return;
    }

    alBufferData(buffer, format, membuf, num_bytes, sfinfo.samplerate); // upload the sound data to the OpenAL buffer
    reportSoundError("alBufferData", filepath, buffer);

    delete[] membuf; // clean up and free resources
    sf_close(file);
}

bool loadSound(Sound& s) // load a sound including his alts
{
    loopi(s.numAlts ? s.numAlts+1 : 1)
    {
        if(s.bufferId[i]) alDeleteBuffers(1, &s.bufferId[i]); // delete the buffer if it already exists.

        defformatstring(fullPath, s.numAlts ? "media/sounds/%s_%d.wav" : "media/sounds/%s.wav", s.soundPath, i + 1);
        alGenBuffers(1, &s.bufferId[i]);
        reportSoundError("alGenBuffers", s.soundPath, s.bufferId[i]);
        loadSoundFile(fullPath, s.bufferId[i]);
    }
    return true;
}

bool preloadAllSounds()
{
    loopi(NUMSNDS) { if (!loadSound(gameSounds[i])) return false; }
    loopi(NUMMAPSNDS)  { if (!loadSound(mapSounds[i])) return false; }
    return true;
}

int findInactiveSource()
{
    loopi(MAX_SOURCES) { if(!sourceActive[i]) return i; }
    return -1;
}

bool checkSoundOcclusion(const vec *soundPos)
{
    if(!soundPos) return false;  // HUD sounds are never occluded

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

void manageSources()
{
    ALint state, looping;
    loopi(MAX_SOURCES)
    {
        if(sourceActive[i])
        {
            alGetSourcei(sources[i], AL_SOURCE_STATE, &state);
            alGetSourcei(sources[i], AL_LOOPING, &looping);
            reportSoundError("alGetSourcei-manageSources", "N/A", -1);
            if(state == AL_STOPPED)
            {
                sourceActive[i] = false;
                sourceOwners[i] = NULL;
            }
        }
    }
}

int countActiveSources()
{
    int count = 0;
    loopi(MAX_SOURCES) { if (sourceActive[i]) count++; }
    return count;
}

void updateListenerPos()
{
    ALfloat listenerPos[] = {camera1->o.x, camera1->o.z, camera1->o.y}; // Updating listener position
    alListenerfv(AL_POSITION, listenerPos);

    camdir.normalize(); // Normalize the forward vector

    vec up = vec(0, 0, 1); // no roll ATM (z-axis is up in cube engine)

    ALfloat orientation[] = {               // Combine the forward and up vectors for orientation
        camdir.x, camdir.z, camdir.y,    // Forward vector (inverted for some obscure reason)
        up.x, up.z, up.y                    // Up vector
    };

    alListenerfv(AL_ORIENTATION, orientation); // Set the listener's orientation
}

void stopMapSound(extentity *e)
{
    loopi(MAX_SOURCES)
    {
        if(sourceActive[i] && sourceOwners[i] == e)
        {
            alSourceStop(sources[i]);
            sourceActive[i] = false;
            sourceOwners[i] = nullptr;
            e->flags &= ~EF_SOUND; // Clear the sound flag for the entity
        }
    }
}

void checkMapSounds()
{
    if(mainmenu) return;
    const vector<extentity *> &ents = entities::getents();

    loopv(ents)
    {
        extentity &e = *ents[i];
        if(e.type!=ET_SOUND) continue;
        if(camera1->o.dist(e.o) < e.attr2)
        {
            if(!(e.flags & EF_SOUND))
            {
                playSound(e.attr1, &e.o, e.attr2, e.attr3, SND_LOOPED|SND_NOOCCLUSION|SND_MAPSOUND|SND_FIXEDPITCH, &e);
                e.flags |= EF_SOUND;  // set the flag to indicate that the sound is currently playing
            }
        }
        else if(e.flags & EF_SOUND) stopMapSound(&e);
    }
}

void updateSounds()
{
    if(noSound) return;
    if((minimized && !minimizedsounds) || game::ispaused()); //stopsounds();
    else
    {
        manageSources();
        updateListenerPos();
        checkMapSounds();
    }
}

void playSound(int soundIndex, const vec *soundPos, float maxRadius, float maxVolRadius, int flags, extentity* owner)
{
    if(soundIndex < 0 || soundIndex > NUMSNDS || noSound) return; // Invalid index or openal not initialized

    if(soundPos && !(flags & SND_NOCULL) && camera1->o.dist(*soundPos) > maxRadius + 50) return; // do not play sound too far from camera, except if flag SND_NOCULL

    int sourceIndex = findInactiveSource();
    if((flags & SND_LOWPRIORITY) && (countActiveSources() >= 0.80f * MAX_SOURCES)) return; // skip low priority sounds (distant shoots etc.) when we are close (85%) to max capacity
    else if(sourceIndex == -1)
    {
        conoutf(CON_WARN, "Max sounds at once capacity reached (%d)", MAX_SOURCES); // all sources are active, skip the sound
        return;
    }

    Sound& s = (flags & SND_MAPSOUND) ? mapSounds[soundIndex] : gameSounds[soundIndex];
    ALuint source = sources[sourceIndex];

    int altIndex = (s.numAlts > 0) ? rnd(s.numAlts+1) : 0;
    ALuint buffer = s.bufferId[altIndex];

    alSourcei(source, AL_BUFFER, buffer); // managing sounds alternatives
    alSourcef(source, AL_GAIN, s.soundVol / 100.f); // managing sound volume
    if(!(flags & SND_FIXEDPITCH)) // managing variations of pitches
    {
        float randomPitch = 0.9f + 0.2f * ((float)rand() / (float)RAND_MAX);
        alSourcef(source, AL_PITCH, randomPitch);
    }
    else alSourcef(source, AL_PITCH, 1.f);

    if(flags & SND_LOOPED) alSourcei(source, AL_LOOPING, AL_TRUE); // Loop the sound
    else alSourcei(source, AL_LOOPING, AL_FALSE); // Don't loop the sound

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

    if(!noEfx) // apply efx if available
    {
        bool occluded = checkSoundOcclusion(soundPos) && !(flags & SND_NOOCCLUSION);

        alSource3i(source, AL_AUXILIARY_SEND_FILTER, auxEffectReverb, 0, occluded ? occlusionFilter : AL_FILTER_NULL);
        reportSoundError("alSource3i-auxEffectReverb+occlusionFilter", s.soundPath, s.bufferId[altIndex]);

        alSourcei(source, AL_DIRECT_FILTER, occluded ? occlusionFilter : AL_FILTER_NULL);
        reportSoundError("alSourcei-occlusionFilter", s.soundPath, s.bufferId[altIndex]);
    }

    alSourcePlay(source);
    reportSoundError("alSourcePlay", s.soundPath, s.bufferId[altIndex]);

    sourceActive[sourceIndex] = true;
}

/*
COMMANDN(music, startmusic, "ss");

static struct songsinfo { string file; int loops; } songs[] =
{
    {"songs/menu.ogg", 0},
    {"songs/pause.ogg", 9},
    {"songs/premission.ogg", 0},
};

void musicmanager(int track) //CubeConflict, gestion des musiques
{
    if(musicstream || track < 0 || track > 2) return;
    startmusic(songs[track].file, songs[track].loops);
}


void registersound(char *name, int *vol) { intret(gamesounds.addsound(name, *vol, 0)); }
COMMAND(registersound, "si");

void mapsound(char *name, int *vol, int *maxuses) { intret(mapsounds.addsound(name, *vol, *maxuses < 0 ? 0 : max(1, *maxuses))); }
COMMAND(mapsound, "sii");

void altsound(char *name, int *vol) { gamesounds.addalt(name, *vol); }
COMMAND(altsound, "si");

void altmapsound(char *name, int *vol) { mapsounds.addalt(name, *vol); }
COMMAND(altmapsound, "si");

ICOMMAND(numsounds, "", (), intret(gamesounds.configs.length()));
ICOMMAND(nummapsounds, "", (), intret(mapsounds.configs.length()));
*/

int playsound(int n, const vec *loc, extentity *ent, int flags, int loops, int fade, int chanid, int radius, int expire)
{
    /*
    if(noSound || !soundvol || (minimized && !minimizedsounds) || n==-1) return -1;

    soundtype &sounds = ent || flags&SND_MAP ? mapsounds : gamesounds;
    if(!sounds.configs.inrange(n)) { conoutf(CON_WARN, "unregistered sound: %d", n); return -1; }
    soundconfig &config = sounds.configs[n];

    if(loc && radius > 0)
    {
        // cull sounds that are unlikely to be heard
        if(camera1->o.dist(*loc) > 1.5f*radius)
        {
            if(channels.inrange(chanid) && sounds.playing(channels[chanid], config))
            {
                Mix_HaltChannel(chanid);
                freechannel(chanid);
            }
            return -1;
        }
    }

    if(chanid < 0)
    {
        if(config.maxuses)
        {
            int uses = 0;
            loopv(channels) if(sounds.playing(channels[i], config) && ++uses >= config.maxuses) return -1;
        }

        // avoid bursts of sounds with heavy packetloss and in sp
        static int soundsatonce = 0, lastsoundmillis = 0;
        if(totalmillis == lastsoundmillis) soundsatonce++; else soundsatonce = 1;
        lastsoundmillis = totalmillis;
        if(maxsoundsatonce && soundsatonce > maxsoundsatonce) return -1;
    }

    if(channels.inrange(chanid))
    {
        soundchannel &chan = channels[chanid];
        if(sounds.playing(chan, config))
        {
            if(loc) chan.loc = *loc;
            else if(chan.hasloc()) chan.clearloc();
            return chanid;
        }
    }
    if(fade < 0) return -1;

    //soundslot &slot = sounds.slots[config.chooseslot()];
    //if(!slot.sample->chunk && !slot.sample->load(sounds.dir)) return -1;

    //if(dbgsound) conoutf(CON_DEBUG, "sound: %s%s", sounds.dir, slot.sample->name);

    chanid = -1;
    loopv(channels) if(!channels[i].inuse) { chanid = i; break; }
    if(chanid < 0 && channels.length() < maxchannels) chanid = channels.length();
    if(chanid < 0) loopv(channels) if(!channels[i].volume) { Mix_HaltChannel(i); freechannel(i); chanid = i; break; }
    if(chanid < 0) return -1;

    //soundchannel &chan = newchannel(chanid, &slot, loc, ent, flags, radius);
    //updatechannel(chan);
    int playing = -1;
    if(fade)
    {
        Mix_Volume(chanid, chan.volume);
        playing = expire >= 0 ? Mix_FadeInChannelTimed(chanid, slot.sample->chunk, loops, fade, expire) : Mix_FadeInChannel(chanid, slot.sample->chunk, loops, fade);
    }
    else playing = expire >= 0 ? Mix_PlayChannelTimed(chanid, slot.sample->chunk, loops, expire) : Mix_PlayChannel(chanid, slot.sample->chunk, loops);
    if(playing >= 0) syncchannel(chan);
    else freechannel(chanid);
    return playing;
    */
    return false;
}


/*
int playsoundname(const char *s, const vec *loc, int vol, int flags, int loops, int fade, int chanid, int radius, int expire)
{
    if(!vol) vol = 750;
    int id = gamesounds.findsound(s, vol);
    if(id < 0) id = gamesounds.addsound(s, vol);
    return playsound(id, loc, NULL, flags, loops, fade, chanid, radius, expire);
}

ICOMMAND(playsound, "i", (int *n), playsound(*n));

ICOMMAND(playsnd, "s", (const char *s), playsoundname(s));


void soundmenu_cleanup()
{
    stopmusic();
    stopsounds();
    UI_PLAYMUSIC = true;
}
*/

const char *getmapsoundname(int n)
{
    //soundslot &slot = mapsounds.slots[mapsounds.configs[n].chooseslot()];
    //if(!mapsounds.configs.inrange(n))
    //{
        return GAME_LANG ? "\fcInvalid ID" : "\fcID Invalide";
    //}
    //else return slot.sample->name;
}




