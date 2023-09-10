#ifndef __SOUND_H__
#define __SOUND_H__

#include <AL/al.h>
#include <AL/alc.h>

#define MAX_ALTS 16
#define MAX_SOURCES 64

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
    // explosions
    S_EXPL_MISSILE, S_EXPL_GRENADE, S_EXPL_FIREWORKS, S_EXPL_NUKE, S_EXPL_KAMIKAZE,
    S_EXPL_PARMOR, S_EXPL_INWATER, S_EXPL_FAR, S_BIGEXPL_FAR, S_FIREWORKSEXPL_FAR,
    // bullets & projectiles
    S_IMPACTBODY, S_IMPACTWOOD, S_IMPACTIRON, S_IMPACTGOLD, S_IMPACTMAGNET,
    S_IMPACTPOWERARMOR, S_IMPACTWATER, S_LITTLERICOCHET, S_BIGRICOCHET, S_IMPACTARROW,
    S_BULLETFLYBY, S_BIGBULLETFLYBY, S_ROCKET, S_MINIROCKET, S_MISSILENUKE,
    S_FLYBYFIREWORKS, S_FLYBYSPOCK, S_FLYBYFLAME, S_FLYBYARROW, S_FLYBYGRAP1, S_FLYBYPLASMA,
    S_FLYBYELEC, S_IMPACTELEC, S_IMPACTPLASMA, S_IMPACTSPOCK, S_IMPACTGRAP1, S_IMPACTLOURDLOIN,
    // items
    S_ITEMHEALTH, S_ITEMMANA, S_COCHON, S_ITEMAMMO, S_ITEMSUPERAMMO, S_ITEMBBOIS,
    S_ITEMBFER, S_ITEMBOR, S_ITEMBMAGNET, S_ITEMARMOUR, S_ITEMPIECEROBOTIQUE,
    S_ITEMCHAMPIS, S_ITEMJOINT, S_ITEMEPO, S_ITEMSTEROS, S_ITEMSPAWN,
    S_ALARME, S_WPLOADSMALL, S_WPLOADMID, S_WPLOADBIG, S_WPLOADFUTUR,
    S_WPLOADALIEN, S_WPLOADSWORD, S_WPLOADCHAINS, S_WPLOADFASTWOOSH, S_WPLOADSLOWWOOSH,
    S_NOAMMO,
    // powerups & item sounds
    S_ROIDS_SHOOT, S_ROIDS_SHOOT_FAR, S_ROIDS_PUPOUT, S_EPO_RUN, S_EPO_PUPOUT,
    S_SHROOMS_PUPOUT, S_ASSISTALARM,
    // physics
    S_DOUILLE, S_BIGDOUILLE, S_CARTOUCHE, S_RGRENADE, S_ECLAIRPROCHE,
    S_ECLAIRLOIN, S_LAVASPLASH,

    NUMSNDS
};

enum SoundFlags
{
    SND_NOCULL        = 1 << 0,     // sound will be played even if camera1 is beyond radius
    SND_NOOCCLUSION   = 1 << 1,     // no filter if occlusion
    SND_LOWPRIORITY   = 1 << 2,     // sound will not play if source limit is almost reached
    SND_FIXEDPITCH    = 1 << 3,     // no random variation in pitch
    SND_LOOPED        = 1 << 4,     // sound is a loop
    SND_MAPSOUND      = 1 << 5,     // sound used in a map
};

struct Sound
{
    string soundPath;               // relative path of the sound
    int numAlts;                    // number of alternatives for the same sound id
    int soundVol;                   // volume of the sound
    bool loaded = false;
    ALuint bufferId[MAX_ALTS];      // OpenAL buffer ID
};

struct SoundSource
{
    ALuint source;
    size_t entityId;
    bool isActive;
};

extern void initSounds();
extern void cleanUpSounds();

extern void updateSoundPosition(size_t entityId, const vec &newPosition, const vec &velocity = vec(0,0,0));
extern void stopLinkedSound(size_t entityId);
extern void updateSounds();
extern void stopAllMapSounds();
extern void stopAllSounds();

extern bool loadSound(Sound& s);
extern void manageSources();

extern void setShroomsEfx(bool enable);
extern const char *getmapsoundname(int n);

extern void soundNearmiss(int sound, const vec &from, const vec &to, int precision = 2048);

#endif
