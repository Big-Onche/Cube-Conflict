#ifndef __SOUND_H__
#define __SOUND_H__

#include <AL/al.h>
#include <AL/alc.h>

#define MAX_ALTS 16
#define MAX_SOURCES 100

enum SoundNames
{
    S_JUMP_BASIC = 0, S_JUMP_NINJA, S_JUMP_ASSIST, S_LAND_BASIC, S_LAND_ASSIST, S_FOOTSTEP,
    S_FOOTSTEP_ASSIST, S_SWIM, S_SPLASH, S_WATER, S_UNDERWATER, S_JUMPPAD,
    S_TELEPORT, S_SPLASH_LAVA, S_DIE_P1, S_DIE,
    SND_AK47, SND_AK47_FAR, SND_FAR_SHOOT, NUMSNDS
};

enum SoundFlags
{
    SOUND_NOCULL        = 1 << 0,   // sound will be played even if camera1 is beyond radius
    SOUND_NOOCCLUSION   = 1 << 1,   // no filter if occlusion
    SOUND_LOWPRIORITY   = 1 << 2,   // sound will not play if source limit is reached
    SOUND_FIXEDPITCH    = 1 << 3    // no random variation in pitch
};

struct Sound
{
    string soundPath;                   // relative path of the sound
    int numAlts;                        // number of alternatives for the same sound id
    int soundVol;                       // volume of the sound
    ALuint bufferId[MAX_ALTS];          // OpenAL buffer ID
};

extern void alInit();
extern void alCleanUp();

extern bool loadSound(Sound& s);
extern void manageSources();
extern void updateListenerPos();

#endif
