#ifndef __SOUND_H__
#define __SOUND_H__

#include <AL/al.h>
#include <AL/alc.h>

#define MAX_ALTS 16
#define MAX_SOURCES 100

enum SoundNames
{   // players sounds
    S_JUMP_BASIC = 0, S_JUMP_NINJA, S_JUMP_ASSIST, S_LAND_BASIC, S_LAND_ASSIST, S_FOOTSTEP,
    S_FOOTSTEP_ASSIST, S_SWIM, S_SPLASH, S_WATER, S_UNDERWATER, S_JUMPPAD,
    S_TELEPORT, S_SPLASH_LAVA, S_DIE_P1, S_DIE,
    // close weapon shoot sounds
    S_ELECRIFLE, S_PLASMARIFLE, S_PLASMARIFLE_SFX,
    S_SMAW, S_MINIGUN, S_SPOCKGUN, S_M32, S_FLAMETHROWER,
    S_UZI, S_FAMAS, S_MOSSBERG, S_HYDRA, S_SV98,
    S_SKS, S_CROSSBOW, S_AK47, S_GRAP1, S_FIREWORKS,
    S_GLOCK, S_NUKE, S_GAU8, S_MINIROCKETS, S_CAMPOUZE,
    S_SWORD349, S_BANHAMMER, S_MASTERSWORD, S_FLAIL, S_NINJASABER,
    // far weapon shoot sounds
    S_ELECRIFLE_FAR, S_PLASMARIFLE_FAR, S_SMAW_FAR, S_MINIGUN_FAR, S_SPOCKGUN_FAR,
    S_M32_FAR, S_FLAMETHROWER_FAR, S_UZI_FAR, S_FAMAS_FAR, S_MOSSBERG_FAR,
    S_HYDRA_FAR, S_SV98_FAR, S_SKS_FAR, S_CROSSBOW_FAR, S_AK47_FAR,
    S_GRAP1_FAR, S_FIREWORKS_FAR, S_GLOCK_FAR, S_NUKE_FAR, S_GAU8_FAR,
    S_MINIROCKETS_FAR, S_CAMPOUZE_FAR, S_FAR_LIGHT, S_FAR_HEAVY, S_FAR_VERYHEAVY,



    NUMSNDS

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
