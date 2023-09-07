// sound.cpp: basic positional sound using sdl_mixer

#include "engine.h"
#include "sound.h"
#include "SDL_mixer.h"
#include <sndfile.h>
#include <AL/efx.h>

bool foundDevice = false;
bool noSound = false;
bool noEfx = false;

VAR(debugsounds, 0, 1, 1);
VARP(minimizedsounds, 0, 0, 1);

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

void initSources()
{
    alGenSources(MAX_SOURCES, sources);
    loopi(MAX_SOURCES) sourceActive[i] = false;
}

ALuint reverbEffect;
ALuint auxEffectReverb;

ALuint lowPassFilter;

void setReverbEffect()
{
    alGenAuxiliaryEffectSlots(1, &auxEffectReverb);

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
}

void reportSoundError(const char* func, const char* filepath, int buffer)
{
    if(!debugsounds) return;
    ALenum error = alGetError();
    if(error != AL_NO_ERROR) conoutf(CON_ERROR, "OpenAL Error after %s for %s: %d (Buffer ID: %d)", func, filepath, error, buffer);
}

void setOcclusionEffect()
{
    alGenFilters(1, &lowPassFilter);
    reportSoundError("alGenFilters-lowPassFilter", "N/A", -1);

    alFilteri(lowPassFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    reportSoundError("alFilteri-lowPassFilter", "N/A", -1);

    alFilterf(lowPassFilter, AL_LOWPASS_GAIN, 0.5f);  // Value between 0.0 to 1.0. Adjust to your liking.
    alFilterf(lowPassFilter, AL_LOWPASS_GAINHF, 0.1f);
}

void alInit()
{
    logoutf("init: openal");

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
    if((minimized && !minimizedsounds) || game::ispaused()) stopsounds();
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

        alSource3i(source, AL_AUXILIARY_SEND_FILTER, auxEffectReverb, 0, occluded ? lowPassFilter : AL_FILTER_NULL);
        reportSoundError("alSource3i-auxEffectReverb+lowPassFilter", s.soundPath, s.bufferId[altIndex]);

        alSourcei(source, AL_DIRECT_FILTER, occluded ? lowPassFilter : AL_FILTER_NULL);
        reportSoundError("alSourcei-lowPassFilter", s.soundPath, s.bufferId[altIndex]);
    }

    alSourcePlay(source);
    reportSoundError("alSourcePlay", s.soundPath, s.bufferId[altIndex]);

    sourceActive[sourceIndex] = true;
}




struct soundsample
{
    char *name;
    Mix_Chunk *chunk;

    soundsample() : name(NULL), chunk(NULL) {}
    ~soundsample() { DELETEA(name); }

    void cleanup() { if(chunk) { Mix_FreeChunk(chunk); chunk = NULL; } }
    bool load(const char *dir, bool msg = false);
};

struct soundslot
{
    soundsample *sample;
    int volume;
};

struct soundconfig
{
    int slots, numslots;
    int maxuses;

    bool hasslot(const soundslot *p, const vector<soundslot> &v) const
    {
        return p >= v.getbuf() + slots && p < v.getbuf() + slots+numslots && slots+numslots < v.length();
    }

    int chooseslot() const
    {
        return numslots > 1 ? slots + rnd(numslots) : slots;
    }
};

struct soundchannel
{
    int id;
    bool inuse;
    vec loc;
    soundslot *slot;
    extentity *ent;
    int radius, volume, pan, flags;
    bool dirty;

    soundchannel(int id) : id(id) { reset(); }

    bool hasloc() const { return loc.x >= -1e15f; }
    void clearloc() { loc = vec(-1e16f, -1e16f, -1e16f); }

    void reset()
    {
        inuse = false;
        clearloc();
        slot = NULL;
        ent = NULL;
        radius = 0;
        volume = -1;
        pan = -1;
        flags = 0;
        dirty = false;
    }
};
vector<soundchannel> channels;
int maxchannels = 0;

soundchannel &newchannel(int n, soundslot *slot, const vec *loc = NULL, extentity *ent = NULL, int flags = 0, int radius = 0)
{
    if(ent)
    {
        loc = &ent->o;
        ent->flags |= EF_SOUND;
    }
    while(!channels.inrange(n)) channels.add(channels.length());
    soundchannel &chan = channels[n];
    chan.reset();
    chan.inuse = true;
    if(loc) chan.loc = *loc;
    chan.slot = slot;
    chan.ent = ent;
    chan.flags = 0;
    chan.radius = radius;
    return chan;
}

void freechannel(int n)
{
    if(!channels.inrange(n) || !channels[n].inuse) return;
    soundchannel &chan = channels[n];
    chan.inuse = false;
    if(chan.ent) chan.ent->flags &= ~EF_SOUND;
}

void syncchannel(soundchannel &chan)
{
    if(!chan.dirty) return;
    if(!Mix_FadingChannel(chan.id)) Mix_Volume(chan.id, chan.volume);
    Mix_SetPanning(chan.id, 255-chan.pan, chan.pan);
    chan.dirty = false;
}

void stopchannels()
{
    loopv(channels)
    {
        soundchannel &chan = channels[i];
        if(!chan.inuse) continue;
        Mix_HaltChannel(i);
        freechannel(i);
    }
}

void setmusicvol(int musicvol);
extern int musicvol;
static int curvol = 0;
VARFP(soundvol, 0, 255, 255,
{
    if(!soundvol) { stopchannels(); setmusicvol(0); }
    else if(!curvol) setmusicvol(musicvol);
    curvol = soundvol;
});
VARFP(musicvol, 0, 60, 255, setmusicvol(soundvol ? musicvol : 0));

char *musicfile = NULL, *musicdonecmd = NULL;

Mix_Music *music = NULL;
SDL_RWops *musicrw = NULL;
stream *musicstream = NULL;

void setmusicvol(int musicvol)
{
    if(noSound) return;
    if(music) Mix_VolumeMusic((musicvol*MIX_MAX_VOLUME)/255);
}

void stopmusic()
{
    if(noSound) return;
    DELETEA(musicfile);
    DELETEA(musicdonecmd);
    if(music)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        music = NULL;
    }
    if(musicrw) { SDL_FreeRW(musicrw); musicrw = NULL; }
    DELETEP(musicstream);
}

#ifdef WIN32
#define AUDIODRIVER "directsound winmm"
#else
#define AUDIODRIVER ""
#endif
bool shouldinitaudio = true;
SVARF(audiodriver, AUDIODRIVER, { shouldinitaudio = true; initwarning(GAME_LANG ? "Sound configuration" : "Configuration des sons", INIT_RESET, CHANGE_SOUND); });
VARF(sound, 0, 1, 1, { shouldinitaudio = true; initwarning(GAME_LANG ? "Audio reset" : "Réinitialisation audio", INIT_RESET, CHANGE_SOUND); });
VARF(soundchans, 1, 256, 512, initwarning(GAME_LANG ? "Sound channels" : "Canaux des sons", INIT_RESET, CHANGE_SOUND));
VARF(soundfreq, 0, MIX_DEFAULT_FREQUENCY, 48000, initwarning(GAME_LANG ? "Sampling frequency" : "Fréquence d'échantillonnage", INIT_RESET, CHANGE_SOUND));
VARF(soundbufferlen, 128, 2048, 4096, initwarning(GAME_LANG ? "Sound buffer" : "Mémoire tampon des sons", INIT_RESET, CHANGE_SOUND));

bool initaudio()
{
    static string fallback = "";
    static bool initfallback = true;
    static bool restorefallback = false;
    if(initfallback)
    {
        initfallback = false;
        if(char *env = SDL_getenv("SDL_AUDIODRIVER")) copystring(fallback, env);
    }
    if(!fallback[0] && audiodriver[0])
    {
        vector<char*> drivers;
        explodelist(audiodriver, drivers);
        loopv(drivers)
        {
            restorefallback = true;
            SDL_setenv("SDL_AUDIODRIVER", drivers[i], 1);
            if(SDL_InitSubSystem(SDL_INIT_AUDIO) >= 0)
            {
                drivers.deletearrays();
                return true;
            }
        }
        drivers.deletearrays();
    }
    if(restorefallback)
    {
        restorefallback = false;
    #ifdef WIN32
        SDL_setenv("SDL_AUDIODRIVER", fallback, 1);
    #else
        unsetenv("SDL_AUDIODRIVER");
    #endif
    }
    if(SDL_InitSubSystem(SDL_INIT_AUDIO) >= 0) return true;
    conoutf(CON_ERROR, "sound init failed: %s", SDL_GetError());
    return false;
}

void initsound()
{
    return;
    SDL_version version;
    SDL_GetVersion(&version);
    if(version.major == 2 && version.minor == 0 && version.patch == 6)
    {
        noSound = true;
        if(sound) conoutf(CON_ERROR, "audio is broken in SDL 2.0.6");
        return;
    }

    if(shouldinitaudio)
    {
        shouldinitaudio = false;
        if(SDL_WasInit(SDL_INIT_AUDIO)) SDL_QuitSubSystem(SDL_INIT_AUDIO);
        if(!sound || !initaudio())
        {
            noSound = true;
            return;
        }
    }

    if(Mix_OpenAudio(soundfreq, MIX_DEFAULT_FORMAT, 2, soundbufferlen)<0)
    {
        noSound = true;
        conoutf(CON_ERROR, "sound init failed (SDL_mixer): %s", Mix_GetError());
        return;
    }
    Mix_AllocateChannels(soundchans);
    maxchannels = soundchans;
    noSound = false;
}

void musicdone()
{
    if(music) { Mix_HaltMusic(); Mix_FreeMusic(music); music = NULL; }
    if(musicrw) { SDL_FreeRW(musicrw); musicrw = NULL; }
    DELETEP(musicstream);
    DELETEA(musicfile);
    if(!musicdonecmd) return;
    char *cmd = musicdonecmd;
    musicdonecmd = NULL;
    execute(cmd);
    delete[] cmd;
}

Mix_Music *loadmusic(const char *name)
{
    if(!musicstream) musicstream = openzipfile(name, "rb");
    if(musicstream)
    {
        if(!musicrw) musicrw = musicstream->rwops();
        if(!musicrw) DELETEP(musicstream);
    }
    if(musicrw) music = Mix_LoadMUSType_RW(musicrw, MUS_NONE, 0);
    else music = Mix_LoadMUS(findfile(name, "rb"));
    if(!music)
    {
        if(musicrw) { SDL_FreeRW(musicrw); musicrw = NULL; }
        DELETEP(musicstream);
    }
    return music;
}

void startmusic(char *name, int loops = 0)
{
    if(noSound) return;
    stopmusic();
    if(soundvol && musicvol && *name)
    {
        defformatstring(file, "media/%s", name);
        path(file);
        if(loadmusic(file))
        {
            DELETEA(musicfile);
            DELETEA(musicdonecmd);
            musicfile = newstring(file);
            Mix_PlayMusic(music, loops);
            Mix_VolumeMusic((musicvol*MIX_MAX_VOLUME)/255);
            intret(1);
        }
        else
        {
            conoutf(CON_ERROR, "could not play music: %s", file);
            intret(0);
        }
    }
}

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

static Mix_Chunk *loadwav(const char *name)
{
    Mix_Chunk *c = NULL;
    stream *z = openzipfile(name, "rb");
    if(z)
    {
        SDL_RWops *rw = z->rwops();
        if(rw)
        {
            c = Mix_LoadWAV_RW(rw, 0);
            SDL_FreeRW(rw);
        }
        delete z;
    }
    if(!c) c = Mix_LoadWAV(findfile(name, "rb"));
    return c;
}

bool soundsample::load(const char *dir, bool msg)
{
    if(chunk) return true;
    if(!name[0]) return false;

    static const char * const exts[] = { "", ".wav", ".ogg" };
    string filename;
    loopi(sizeof(exts)/sizeof(exts[0]))
    {
        formatstring(filename, "media/sound/%s%s%s", dir, name, exts[i]);
        if(msg && !i) renderprogress(0, filename);
        path(filename);
        chunk = loadwav(filename);
        if(chunk) return true;
    }

    conoutf(CON_ERROR, "failed to load sample: media/sound/%s%s", dir, name);
    return false;
}

static struct soundtype
{
    hashnameset<soundsample> samples;
    vector<soundslot> slots;
    vector<soundconfig> configs;
    const char *dir;

    soundtype(const char *dir) : dir(dir) {}

    int findsound(const char *name, int vol)
    {
        loopv(configs)
        {
            soundconfig &s = configs[i];
            loopj(s.numslots)
            {
                soundslot &c = slots[s.slots+j];
                if(!strcmp(c.sample->name, name) && (!vol || c.volume==vol)) return i;
            }
        }
        return -1;
    }

    int addslot(const char *name, int vol)
    {
        soundsample *s = samples.access(name);
        if(!s)
        {
            char *n = newstring(name);
            s = &samples[n];
            s->name = n;
            s->chunk = NULL;
        }
        soundslot *oldslots = slots.getbuf();
        int oldlen = slots.length();
        soundslot &slot = slots.add();
        // soundslots.add() may relocate slot pointers
        if(slots.getbuf() != oldslots) loopv(channels)
        {
            soundchannel &chan = channels[i];
            if(chan.inuse && chan.slot >= oldslots && chan.slot < &oldslots[oldlen])
                chan.slot = &slots[chan.slot - oldslots];
        }
        slot.sample = s;
        slot.volume = vol ? vol : 100;
        return oldlen;
    }

    int addsound(const char *name, int vol, int maxuses = 0)
    {
        soundconfig &s = configs.add();
        s.slots = addslot(name, vol);
        s.numslots = 1;
        s.maxuses = maxuses;
        return configs.length()-1;
    }

    void addalt(const char *name, int vol)
    {
        if(configs.empty()) return;
        addslot(name, vol);
        configs.last().numslots++;
    }

    void clear()
    {
        slots.setsize(0);
        configs.setsize(0);
    }

    void reset()
    {
        loopv(channels)
        {
            soundchannel &chan = channels[i];
            if(chan.inuse && slots.inbuf(chan.slot))
            {
                Mix_HaltChannel(i);
                freechannel(i);
            }
        }
        clear();
    }

    void cleanupsamples()
    {
        enumerate(samples, soundsample, s, s.cleanup());
    }

    void cleanup()
    {
        cleanupsamples();
        slots.setsize(0);
        configs.setsize(0);
        samples.clear();
    }

    void preloadsound(int n)
    {
        if(noSound || !configs.inrange(n)) return;
        soundconfig &config = configs[n];
        loopk(config.numslots) slots[config.slots+k].sample->load(dir, true);
    }

    bool playing(const soundchannel &chan, const soundconfig &config) const
    {
        return chan.inuse && config.hasslot(chan.slot, slots);
    }
} gamesounds(""), mapsounds("mapsound/");

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

void soundreset()
{
    gamesounds.reset();
}
COMMAND(soundreset, "");

void mapsoundreset()
{
    mapsounds.reset();
}
COMMAND(mapsoundreset, "");

void resetchannels()
{
    loopv(channels) if(channels[i].inuse) freechannel(i);
    channels.shrink(0);
}

void clear_sound()
{
    closemumble();
    if(noSound) return;
    stopmusic();

    gamesounds.cleanup();
    mapsounds.cleanup();
    Mix_CloseAudio();
    resetchannels();
}

void stopmapsounds()
{
    loopv(channels) if(channels[i].inuse && channels[i].ent)
    {
        Mix_HaltChannel(i);
        freechannel(i);
    }
}

void clearmapsounds()
{
    stopmapsounds();
    mapsounds.clear();
}

VAR(stereo, 0, 1, 1);

int maxsoundradius = 340;

bool updatechannel(soundchannel &chan)
{
    if(!chan.slot) return false;
    float volf = 1.0f, panf = 0.5f;
    if(chan.hasloc())
    {
        vec v;
        float dist = chan.loc.dist(camera1->o, v);
        int rad = 0;
        if(chan.ent)
        {
            rad = chan.ent->attr2;
            if(chan.ent->attr3)
            {
                rad -= chan.ent->attr3;
                dist -= chan.ent->attr3;
            }
        }
        else if(chan.radius > 0) rad = chan.radius;
        if(rad > 0) volf -= clamp(dist/rad, 0.0f, 1.0f); // simple mono distance attenuation
        if(stereo && (v.x != 0 || v.y != 0) && dist>0)
        {
            v.rotate_around_z(-camera1->yaw*RAD);
            panf = 0.5f - 0.5f*v.x/v.magnitude2(); // range is from 0 (left) to 1 (right)
        }
    }
    int vol = clamp(int(volf*soundvol*chan.slot->volume*(MIX_MAX_VOLUME/float(255*255)) + 0.5f), 0, MIX_MAX_VOLUME);
    int pan = clamp(int(panf*255.9f), 0, 255);
    if(vol == chan.volume && pan == chan.pan) return false;
    chan.volume = vol;
    chan.pan = pan;
    chan.dirty = true;
    return true;
}

void reclaimchannels()
{
    loopv(channels)
    {
        soundchannel &chan = channels[i];
        if(chan.inuse && !Mix_Playing(i)) freechannel(i);
    }
}

void syncchannels()
{
    loopv(channels)
    {
        soundchannel &chan = channels[i];
        if(chan.inuse && chan.hasloc() && updatechannel(chan)) syncchannel(chan);
    }
}



VARP(maxsoundsatonce, 0, 256, 512);

VAR(dbgsound, 0, 0, 1);

void preloadsound(int n)
{
    gamesounds.preloadsound(n);
}

void preloadmapsound(int n)
{
    mapsounds.preloadsound(n);
}

void preloadmapsounds()
{
    const vector<extentity *> &ents = entities::getents();
    loopv(ents)
    {
        loadprogress = ((float(i+1)/ents.length())*10)+84;
        extentity &e = *ents[i];
        if(e.type==ET_SOUND) mapsounds.preloadsound(e.attr1);
    }
}

int playsound(int n, const vec *loc, extentity *ent, int flags, int loops, int fade, int chanid, int radius, int expire)
{
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

    soundslot &slot = sounds.slots[config.chooseslot()];
    if(!slot.sample->chunk && !slot.sample->load(sounds.dir)) return -1;

    if(dbgsound) conoutf(CON_DEBUG, "sound: %s%s", sounds.dir, slot.sample->name);

    chanid = -1;
    loopv(channels) if(!channels[i].inuse) { chanid = i; break; }
    if(chanid < 0 && channels.length() < maxchannels) chanid = channels.length();
    if(chanid < 0) loopv(channels) if(!channels[i].volume) { Mix_HaltChannel(i); freechannel(i); chanid = i; break; }
    if(chanid < 0) return -1;

    soundchannel &chan = newchannel(chanid, &slot, loc, ent, flags, radius);
    updatechannel(chan);
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
}

void stopsounds()
{
    loopv(channels) if(channels[i].inuse)
    {
        Mix_HaltChannel(i);
        freechannel(i);
    }
}

bool stopsound(int n, int chanid, int fade)
{
    if(!gamesounds.configs.inrange(n) || !channels.inrange(chanid) || !gamesounds.playing(channels[chanid], gamesounds.configs[n])) return false;
    if(dbgsound) conoutf(CON_DEBUG, "stopsound: %s%s", gamesounds.dir, channels[chanid].slot->sample->name);
    if(!fade || !Mix_FadeOutChannel(chanid, fade))
    {
        Mix_HaltChannel(chanid);
        freechannel(chanid);
    }
    return true;
}

int playsoundname(const char *s, const vec *loc, int vol, int flags, int loops, int fade, int chanid, int radius, int expire)
{
    if(!vol) vol = 750;
    int id = gamesounds.findsound(s, vol);
    if(id < 0) id = gamesounds.addsound(s, vol);
    return playsound(id, loc, NULL, flags, loops, fade, chanid, radius, expire);
}

ICOMMAND(playsound, "i", (int *n), playsound(*n));

ICOMMAND(playsnd, "s", (const char *s), playsoundname(s));

void resetsound()
{
    clearchanges(CHANGE_SOUND);
    if(!noSound)
    {
        gamesounds.cleanupsamples();
        mapsounds.cleanupsamples();
        if(music)
        {
            Mix_HaltMusic();
            Mix_FreeMusic(music);
        }
        if(musicstream) musicstream->seek(0, SEEK_SET);
        Mix_CloseAudio();
    }
    initsound();
    resetchannels();
    if(noSound)
    {
        DELETEA(musicfile);
        DELETEA(musicdonecmd);
        music = NULL;
        gamesounds.cleanupsamples();
        mapsounds.cleanupsamples();
        return;
    }
    if(music && loadmusic(musicfile))
    {
        Mix_PlayMusic(music, musicdonecmd ? 0 : -1);
        Mix_VolumeMusic((musicvol*MIX_MAX_VOLUME)/255);
    }
    else
    {
        DELETEA(musicfile);
        DELETEA(musicdonecmd);
    }
}

COMMAND(resetsound, "");

void soundmenu_cleanup()
{
    stopmusic();
    stopsounds();
    UI_PLAYMUSIC = true;
}

const char *getmapsoundname(int n)
{
    soundslot &slot = mapsounds.slots[mapsounds.configs[n].chooseslot()];
    if(!mapsounds.configs.inrange(n))
    {
        return GAME_LANG ? "\fcInvalid ID" : "\fcID Invalide";
    }
    else return slot.sample->name;
}

#ifdef WIN32

#include <wchar.h>

#else

#include <unistd.h>

#ifdef _POSIX_SHARED_MEMORY_OBJECTS
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <wchar.h>
#endif

#endif

#if defined(WIN32) || defined(_POSIX_SHARED_MEMORY_OBJECTS)
struct MumbleInfo
{
    int version, timestamp;
    vec pos, front, top;
    wchar_t name[256];
};
#endif

#ifdef WIN32
static HANDLE mumblelink = NULL;
static MumbleInfo *mumbleinfo = NULL;
#define VALID_MUMBLELINK (mumblelink && mumbleinfo)
#elif defined(_POSIX_SHARED_MEMORY_OBJECTS)
static int mumblelink = -1;
static MumbleInfo *mumbleinfo = (MumbleInfo *)-1;
#define VALID_MUMBLELINK (mumblelink >= 0 && mumbleinfo != (MumbleInfo *)-1)
#endif

#ifdef VALID_MUMBLELINK
VARFP(mumble, 0, 1, 1, { if(mumble) initmumble(); else closemumble(); });
#else
VARFP(mumble, 0, 0, 1, { if(mumble) initmumble(); else closemumble(); });
#endif

void initmumble()
{
    if(!mumble) return;
#ifdef VALID_MUMBLELINK
    if(VALID_MUMBLELINK) return;

    #ifdef WIN32
        mumblelink = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "MumbleLink");
        if(mumblelink)
        {
            mumbleinfo = (MumbleInfo *)MapViewOfFile(mumblelink, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MumbleInfo));
            if(mumbleinfo) wcsncpy(mumbleinfo->name, L"Cube Conflict", 256);
        }
    #elif defined(_POSIX_SHARED_MEMORY_OBJECTS)
        defformatstring(shmname, "/MumbleLink.%d", getuid());
        mumblelink = shm_open(shmname, O_RDWR, 0);
        if(mumblelink >= 0)
        {
            mumbleinfo = (MumbleInfo *)mmap(NULL, sizeof(MumbleInfo), PROT_READ|PROT_WRITE, MAP_SHARED, mumblelink, 0);
            if(mumbleinfo != (MumbleInfo *)-1) wcsncpy(mumbleinfo->name, L"Cube Conflict", 256);
        }
    #endif
    if(!VALID_MUMBLELINK) closemumble();
#else
    conoutf(CON_ERROR, "Mumble positional audio is not available on this platform.");
#endif
}

void closemumble()
{
#ifdef WIN32
    if(mumbleinfo) { UnmapViewOfFile(mumbleinfo); mumbleinfo = NULL; }
    if(mumblelink) { CloseHandle(mumblelink); mumblelink = NULL; }
#elif defined(_POSIX_SHARED_MEMORY_OBJECTS)
    if(mumbleinfo != (MumbleInfo *)-1) { munmap(mumbleinfo, sizeof(MumbleInfo)); mumbleinfo = (MumbleInfo *)-1; }
    if(mumblelink >= 0) { close(mumblelink); mumblelink = -1; }
#endif
}

static inline vec mumblevec(const vec &v, bool pos = false)
{
    // change from X left, Z up, Y forward to X right, Y up, Z forward
    // 8 cube units = 1 meter
    vec m(-v.x, v.z, v.y);
    if(pos) m.div(8);
    return m;
}

void updatemumble()
{
#ifdef VALID_MUMBLELINK
    if(!VALID_MUMBLELINK) return;

    static int timestamp = 0;

    mumbleinfo->version = 1;
    mumbleinfo->timestamp = ++timestamp;

    mumbleinfo->pos = mumblevec(player->o, true);
    mumbleinfo->front = mumblevec(vec(player->yaw*RAD, player->pitch*RAD));
    mumbleinfo->top = mumblevec(vec(player->yaw*RAD, (player->pitch+90)*RAD));
#endif
}

