#ifndef __GAME_H__
#define __GAME_H__

#include "cube.h"
#include "engine.h"

extern int cnidentiquearme, nextcnweapon, servambient;
extern int lastshoot, getspyability;

// animations
enum
{
    ANIM_DEAD = ANIM_GAMESPECIFIC, ANIM_DYING, ANIM_IDLE,
    ANIM_FORWARD, ANIM_BACKWARD, ANIM_LEFT, ANIM_RIGHT,
    ANIM_JUMP, ANIM_SINK, ANIM_SWIM,
    ANIM_CROUCH, ANIM_CROUCH_FORWARD, ANIM_CROUCH_BACKWARD, ANIM_CROUCH_LEFT, ANIM_CROUCH_RIGHT,
    ANIM_CROUCH_JUMP, ANIM_CROUCH_SINK, ANIM_CROUCH_SWIM,
    ANIM_SHOOT, ANIM_MELEE,
    ANIM_PAIN,
    ANIM_EDIT, ANIM_LAG, ANIM_TAUNT, ANIM_WIN, ANIM_LOSE,
    ANIM_GUN_IDLE, ANIM_GUN_SHOOT, ANIM_GUN_MELEE,
    ANIM_VWEP_IDLE, ANIM_VWEP_SHOOT, ANIM_VWEP_MELEE,
    ANIM_TRIGGER,
    NUMANIMS
};

static const char * const animnames[] =
{
    "mapmodel",
    "dead", "dying", "idle",
    "forward", "backward", "left", "right",
    "jump", "sink", "swim",
    "crouch", "crouchforward", "crouchbackward", "crouchleft", "crouchright",
    "crouch jump", "crouch sink", "crouch swim",
    "attack", "melee",
    "pain",
    "edit", "lag", "taunt", "win", "lose",
    "gun idle", "gun shoot", "gun melee",
    "vwep idle", "vwep shoot", "vwep melee",
    "trigger"
};

// console message types
enum
{
    CON_CHAT       = 1<<8,
    CON_TEAMCHAT   = 1<<9,
    CON_GAMEINFO   = 1<<10,
    CON_FRAG_SELF  = 1<<11,
    CON_FRAG_OTHER = 1<<12,
    CON_TEAMKILL   = 1<<13
};

// network quantization scale
#define DMF 16.0f               // for world locations
#define DNF 100.0f              // for normalized vectors
#define DVELF 1.0f              // for playerspeed based velocity vectors

enum                            // static entity types
{
    NOTUSED = ET_EMPTY,         // entity slot not in use in map
    LIGHT = ET_LIGHT,           // lightsource, attr1 = radius, attr2 = intensity
    MAPMODEL = ET_MAPMODEL,     // attr1 = yaw, attr2 = idx, attr3 = pitch, attr4 = scale, attr5 = roll
    PLAYERSTART,                // attr1 = angle, attr2 = team
    ENVMAP = ET_ENVMAP,         // attr1 = radius
    PARTICLES = ET_PARTICLES,
    MAPSOUND = ET_SOUND,
    SPOTLIGHT = ET_SPOTLIGHT,
    DECAL = ET_DECAL,
    // weapons
    I_RAIL, I_PULSE, I_SMAW, I_MINIGUN, I_SPOCKGUN, I_M32, I_LANCEFLAMMES, I_UZI, I_FAMAS, I_MOSSBERG, I_HYDRA, I_SV98, I_SKS, I_ARBALETE, I_AK47, I_GRAP1, I_ARTIFICE, I_GLOCK,
    I_SUPERARME, I_NULL1, I_NULL2, I_NULL3,
    // items
    I_SANTE, I_BOOSTPV, I_BOOSTDEGATS, I_BOOSTPRECISION, I_BOOSTVITESSE, I_BOOSTGRAVITE,
    I_BOUCLIERBOIS, I_BOUCLIERFER, I_BOUCLIEROR, I_BOUCLIERMAGNETIQUE, I_ARMUREASSISTEE,
    I_MANA,
    // other
    TELEPORT,                   // attr1 = idx, attr2 = model, attr3 = tag
    TELEDEST,                   // attr1 = angle, attr2 = idx
    JUMPPAD,                    // attr1 = zpush, attr2 = ypush, attr3 = xpush
    FLAG,                       // attr1 = angle, attr2 = team
    BASE,                       // attr2 = name alias
    // singler player
    MONSTER,                    // attr1 = monstertype, attr2 = yaw, attr3 = pitch, attr4 = tag
    RESPAWNPOINT,
    TRIGGER_ZONE,               // attr1 = tag, attr2 = type, attr3 = radius
    // utility
    CAMERA_POS,                 // attr1 = tag, attr2 = yaw, attr3 = pitch, attr4 = roll
    MAXENTTYPES,
};

enum
{
    TRIGGER_RESET = 0,
    TRIGGERING,
    TRIGGERED,
    TRIGGER_RESETTING,
    TRIGGER_DISAPPEARED
};

struct gameentity : extentity
{
    int triggerstate, lasttrigger;

    gameentity() : triggerstate(TRIGGER_RESET), lasttrigger(0) {}
};

enum { GUN_RAIL = 0, GUN_PULSE, GUN_SMAW, GUN_MINIGUN, GUN_SPOCKGUN, GUN_M32, GUN_LANCEFLAMMES, GUN_UZI, GUN_FAMAS, GUN_MOSSBERG, GUN_HYDRA, GUN_SV98, GUN_SKS, GUN_ARBALETE, GUN_AK47, GUN_GRAP1, GUN_ARTIFICE, GUN_GLOCK,
       GUN_S_NUKE, GUN_S_GAU8, GUN_S_ROQUETTES, GUN_S_CAMPOUZE,
       GUN_CAC349, GUN_CACMARTEAU, GUN_CACMASTER, GUN_CACFLEAU,
       GUN_KAMIKAZE, GUN_ASSISTXPL, GUN_CACNINJA,
       NUMGUNS };
enum { A_BLUE, A_GREEN, A_YELLOW, A_MAGNET, A_ASSIST };
enum { ACT_IDLE = 0, ACT_SHOOT, NUMACTS };
enum {  ATK_RAIL_SHOOT = 0, ATK_PULSE_SHOOT,
        ATK_SMAW_SHOOT, ATK_MINIGUN_SHOOT,
        ATK_SPOCKGUN_SHOOT, ATK_M32_SHOOT,
        ATK_LANCEFLAMMES_SHOOT, ATK_UZI_SHOOT,
        ATK_FAMAS_SHOOT, ATK_MOSSBERG_SHOOT,
        ATK_HYDRA_SHOOT, ATK_SV98_SHOOT,
        ATK_SKS_SHOOT, ATK_ARBALETE_SHOOT,
        ATK_AK47_SHOOT, ATK_GRAP1_SHOOT,
        ATK_ARTIFICE_SHOOT, ATK_GLOCK_SHOOT,
        //Super armes (4 armes)
        ATK_NUKE_SHOOT, ATK_GAU8_SHOOT,
        ATK_ROQUETTES_SHOOT, ATK_CAMPOUZE_SHOOT,
        //Corps à corps (4 armes)
        ATK_CAC349_SHOOT, ATK_CACMARTEAU_SHOOT,
        ATK_CACMASTER_SHOOT, ATK_CACFLEAU_SHOOT,
        //Spéciales aptitudes/objets (3 armes)
        ATK_KAMIKAZE_SHOOT, ATK_ASSISTXPL_SHOOT, ATK_CACNINJA_SHOOT,
        NUMATKS
};
enum { M_NONE = 0, M_SEARCH, M_AGGRO, M_RETREAT, M_ATTACKING, M_PAIN, M_SLEEP, M_AIMING, M_FRIENDLY, M_NEUTRAL, M_ANGRY, M_MAX};  // monster states

#define validgun(n) ((n) >= 0 && (n) < NUMGUNS)
#define validact(n) ((n) >= 0 && (n) < NUMACTS)
#define validatk(n) ((n) >= 0 && (n) < NUMATKS)

enum
{
    M_TEAM       = 1<<0,
    M_CTF        = 1<<1,
    M_OVERTIME   = 1<<2,
    M_EDIT       = 1<<3,
    M_DEMO       = 1<<4,
    M_LOCAL      = 1<<5,
    M_LOBBY      = 1<<6,
    M_RANDOM     = 1<<7,
    M_FULLSTUFF  = 1<<8,
    M_IDENTIQUE  = 1<<9,
    M_NOAMMO     = 1<<10,
    M_MUNINFINIE = 1<<11,
    M_CAPTURE    = 1<<12,
    M_REGEN      = 1<<13,
    M_NOITEMS    = 1<<14,
    M_DMSP       = 1<<15,
    M_CLASSICSP  = 1<<16,
    M_TUTORIAL   = 1<<17,
};

static struct gamemodeinfo
{
    const char *nameFR;
    const char *nameEN;
    int flags;
} gamemodes[] =
{
    { "Invasion", "Invasion", M_DMSP | M_LOCAL },               // -3
    { "Tutoriel", "Tutorial", M_TUTORIAL | M_LOCAL },   // -2
    { "SP", "SP", M_CLASSICSP | M_LOCAL },              // -1
    { "demo", "demo", M_DEMO | M_LOCAL },               // 0
    { "Editeur de maps", "Map editor", M_EDIT },        // 1

    //MODE 1, 2, 3, 4
    { "Tue Les Tous (Collecte d'armes)",   "Deathmatch (Weapon pickup)",        M_LOBBY },
    { "Tue Les Tous (Armes aléatoire)",   "Deathmatch (Random weapons)",        M_RANDOM | M_NOAMMO | M_MUNINFINIE},
    { "Tue Les Tous (Armes multiples)",    "Deathmatch (Multiple weapons)",     M_FULLSTUFF},
    { "Tue Les Tous (Armes identiques)",   "Deathmatch (Identical weapons)",    M_IDENTIQUE | M_NOAMMO | M_MUNINFINIE},
    //MODE 5, 6, 7, 8
    { "Tue Les Tous (Collecte d'armes)",   "Team Deathmatch (Weapon pickup)",          M_TEAM },
    { "Tue Les Tous (Arme aléatoire)",     "Team Deathmatch (Random weapon)",          M_RANDOM | M_TEAM | M_NOAMMO | M_MUNINFINIE},
    { "Tue Les Tous (Armes multiples)",    "Team Deathmatch (Multiple weapons)",       M_FULLSTUFF | M_TEAM},
    { "Tue Les Tous (Armes identiques)",   "Team Deathmatch (Identical weapons)",      M_IDENTIQUE | M_TEAM | M_NOAMMO | M_MUNINFINIE},
    //MODE 9, 10, 11, 12
    { "Capture de drapeau (Collecte d'armes)",  "Capture the flag (Weapon pickup)",       M_CTF | M_TEAM },
    { "Capture de drapeau (Arme aléatoire)",    "Capture the flag (Random weapon)",       M_RANDOM | M_CTF | M_TEAM | M_NOAMMO | M_MUNINFINIE},
    { "Capture de drapeau (Full stuff)",        "Capture the flag (Multiple weapons)",    M_FULLSTUFF | M_CTF | M_TEAM},
    { "Capture de drapeau (Identique)",         "Capture the flag (Identical)",           M_IDENTIQUE | M_CTF | M_TEAM | M_NOAMMO | M_MUNINFINIE},
    //MODE 13, 14, 15, 16, 17
    { "Conquête (Collecte d'armes)",        "Domination (Weapon pickup)",       M_CAPTURE | M_TEAM},
    { "Conquête (Arme aléatoire)",          "Domination (Random weapon)",       M_RANDOM | M_CAPTURE | M_TEAM | M_NOAMMO | M_MUNINFINIE},
    { "Conquête (Armes multiples)",         "Domination (Multiple weapons)",    M_FULLSTUFF | M_CAPTURE | M_TEAM},
    { "Conquête (Armes identiques)",        "Domination (Identical weapons)",   M_IDENTIQUE | M_CAPTURE | M_TEAM | M_NOAMMO | M_MUNINFINIE},
    { "Conquête (Régénération)",            "Domination (Regeneration)",        M_NOITEMS | M_CAPTURE | M_TEAM | M_REGEN},
};

#define STARTGAMEMODE (-3)
#define NUMGAMEMODES ((int)(sizeof(gamemodes)/sizeof(gamemodes[0])))

#define m_valid(mode)          ((mode) >= STARTGAMEMODE && (mode) < STARTGAMEMODE + NUMGAMEMODES)
#define m_check(mode, flag)    (m_valid(mode) && gamemodes[(mode) - STARTGAMEMODE].flags&(flag))
#define m_checknot(mode, flag) (m_valid(mode) && !(gamemodes[(mode) - STARTGAMEMODE].flags&(flag)))
#define m_checkall(mode, flag) (m_valid(mode) && (gamemodes[(mode) - STARTGAMEMODE].flags&(flag)) == (flag))
#define m_checkonly(mode, flag, exclude) (m_valid(mode) && (gamemodes[(mode) - STARTGAMEMODE].flags&((flag)|(exclude))) == (flag))

#define m_teammode     (m_check(gamemode, M_TEAM))
#define m_overtime     (m_check(gamemode, M_OVERTIME))
#define isteam(a,b)    (m_teammode && a==b)

#define m_random       (m_check(gamemode, M_RANDOM))
#define m_fullstuff    (m_check(gamemode, M_FULLSTUFF))
#define m_identique    (m_check(gamemode, M_IDENTIQUE))
#define m_muninfinie   (m_check(gamemode, M_MUNINFINIE))

#define m_noitems      (m_check(gamemode, M_NOITEMS))
#define m_noammo       (m_check(gamemode, M_NOAMMO))

#define m_ctf          (m_check(gamemode, M_CTF))
#define m_capture      (m_check(gamemode, M_CAPTURE))
#define m_regencapture (m_checkall(gamemode, M_CAPTURE | M_REGEN))

#define m_demo         (m_check(gamemode, M_DEMO))
#define m_edit         (m_check(gamemode, M_EDIT))
#define m_lobby        (m_check(gamemode, M_LOBBY))
#define m_timed        (m_checknot(gamemode, M_DEMO|M_EDIT|M_LOCAL))
#define m_botmode      (m_checknot(gamemode, M_DEMO|M_LOCAL))
#define m_mp(mode)     (m_checknot(mode, M_LOCAL))

#define m_sp           (m_check(gamemode, M_DMSP | M_CLASSICSP))
#define m_dmsp         (m_check(gamemode, M_DMSP))
#define m_classicsp    (m_check(gamemode, M_CLASSICSP))
#define m_tutorial     (m_check(gamemode, M_TUTORIAL))

enum { MM_AUTH = -1, MM_OPEN = 0, MM_VETO, MM_LOCKED, MM_PRIVATE, MM_PASSWORD, MM_START = MM_AUTH, MM_INVALID = MM_START - 1 };

static const char * const mastermodenames_fr[] = { "Auth", "Publique", "Veto",     "Verrouillé", "Privé",      "Mot de passe" };
static const char * const mastermodenames[] =  { "Auth",   "Public",   "Veto",     "Locked",     "Private",    "Password" };
static const char * const mastermodecolors[] = { "",       "\f0",    "\f2",        "\f2",        "\f3",        "\f3" };
static const char * const mastermodeicons[] =  { "server", "server", "serverlock", "serverlock", "serverpriv", "serverpriv" };

// hardcoded sounds, defined in sounds.cfg
enum
{
    // player movements sounds
    S_JUMP_BASIC = 0, S_JUMP_NINJA, S_JUMP_ASSIST, S_LAND_BASIC, S_LAND_ASSIST, S_FOOTSTEP,     // 0-5
    S_FOOTSTEP_ASSIST, S_SWIM, S_SPLASH, S_WATER, S_UNDERWATER, S_JUMPPAD,                      // 6-10
    S_TELEPORT, S_SPLASH_LAVA, S_DIE_P1, S_DIE,                                                 // 11-12

    // close weapon shoot sounds
    S_ELECRIFLE, S_PLASMARIFLE, S_PLASMARIFLE_SFX,                                              // 13-15
    S_SMAW, S_MINIGUN, S_SPOCKGUN, S_M32, S_FLAMETHROWER,                                       // 16-20
    S_UZI, S_FAMAS, S_MOSSBERG, S_HYDRA, S_SV98,                                                // 21-25
    S_SKS, S_CROSSBOW, S_AK47, S_GRAP1, S_FIREWORKS,                                            // 26-30
    S_GLOCK, S_NUKE, S_GAU8, S_MINIROCKETS, S_CAMPOUZE,                                         // 31-35
    S_SWORD349, S_BANHAMMER, S_MASTERSWORD, S_FLAIL, S_NINJASABER,                              // 36-40

    // far weapon shoot sounds
    S_ELECRIFLE_FAR, S_PLASMARIFLE_FAR, S_SMAW_FAR, S_MINIGUN_FAR, S_SPOCKGUN_FAR,              // 41-45
    S_M32_FAR, S_FLAMETHROWER_FAR, S_UZI_FAR, S_FAMAS_FAR, S_MOSSBERG_FAR,                      // 46-50
    S_HYDRA_FAR, S_SV98_FAR, S_SKS_FAR, S_CROSSBOW_FAR, S_AK47_FAR,                             // 51-55
    S_GRAP1_FAR, S_FIREWORKS_FAR, S_GLOCK_FAR, S_NUKE_FAR, S_GAU8_FAR,                          // 56-60
    S_MINIROCKETS_FAR, S_CAMPOUZE_FAR, S_FAR_LIGHT, S_FAR_HEAVY, S_FAR_VERYHEAVY,               // 61-65

    // explosions
    S_EXPL_MISSILE, S_EXPL_GRENADE, S_EXPL_FIREWORKS, S_EXPL_NUKE, S_EXPL_KAMIKAZE,             // 66-70
    S_EXPL_PARMOR, S_EXPL_INWATER, S_EXPL_FAR,S_BIGEXPL_FAR, S_FIREWORKSEXPL_FAR,               // 71-75

    // bullets & projectiles
    S_IMPACTBODY, S_IMPACTWOOD, S_IMPACTIRON, S_IMPACTGOLD, S_IMPACTMAGNET,                     // 76-80
    S_IMPACTPOWERARMOR, S_IMPACTWATER, S_LITTLERICOCHET, S_BIGRICOCHET, S_IMPACTARROW,          // 81-85
    S_BULLETFLYBY, S_BIGBULLETFLYBY, S_ROCKET, S_MINIROCKET, S_MISSILENUKE,                     // 86-90
    S_FLYBYFIREWORKS, S_FLYBYSPOCK, S_FLYBYFLAME, S_FLYBYARROW, S_FLYBYGRAP1, S_FLYBYPLASMA,
    S_FLYBYELEC, S_IMPACTELEC, S_IMPACTSPOCK, S_IMPACTGRAP1, S_IMPACTLOURDLOIN,

    // items
    S_ITEMHEALTH, S_ITEMMANA, S_COCHON, S_ITEMAMMO, S_ITEMSUPERAMMO, S_ITEMBBOIS,
    S_ITEMBFER, S_ITEMBOR, S_ITEMBMAGNET, S_ITEMARMOUR, S_ITEMPIECEROBOTIQUE,
    S_ITEMCHAMPIS, S_ITEMJOINT, S_ITEMEPO, S_ITEMSTEROS, S_ITEMSPAWN,
    S_ALARME, S_WPLOADSMALL, S_WPLOADMID, S_WPLOADBIG, S_WPLOADFUTUR,
    S_WPLOADALIEN, S_WPLOADSWORD, S_WPLOADCHAINS, S_WPLOADFASTWOOSH, S_WPLOADSLOWWOOSH,
    S_NOAMMO,

    // physics
    S_DOUILLE, S_BIGDOUILLE, S_CARTOUCHE, S_RGRENADE, S_ECLAIRPROCHE,
    S_ECLAIRLOIN, S_LAVASPLASH,

    // classes & spells
    S_SORTLANCE, S_SORTIMPOSSIBLE, S_SORTPRET, S_TIMER, S_FAUCHEUSE,
    S_RAGETIR, S_REGENMEDIGUN, S_REGENJUNKIE, S_WIZ_1, S_WIZ_2,
    S_WIZ_3, S_PHY_1, S_PHY_1_WOOD, S_PHY_1_IRON,  S_PHY_1_GOLD,
    S_PHY_1_MAGNET, S_PHY_1_POWERARMOR, S_PHY_2, S_PHY_3, S_SPY_1,
    S_SPY_2, S_SPY_3, S_PRI_1, S_PRI_2, S_PRI_2_2,
    S_PRI_3, S_SHO_1, S_SHO_2, S_SHO_3,

    // notifications
    S_KILL, S_HIT, S_ACHIEVEMENTUNLOCKED, S_LEVELUP, S_DRAPEAUPRIS,
    S_DRAPEAUTOMBE, S_DRAPEAUSCORE, S_DRAPEAURESET, S_TERMINAL_HACKED, S_TERMINAL_LOST,
    S_TERMINAL_HACKED_E, S_TERMINAL_LOST_E, S_TERMINAL_ALARM, S_TERMINAL_ENTER,
    S_KS_X3, S_KS_X5, S_KS_X7, S_KS_X3_FAR, S_KS_X5_FAR,
    S_KS_X7_FAR, S_NOTIFICATION, S_INVASION, S_ALIEN_INVASION,

    // ui
    S_CLICK, S_SCROLLUP, S_SCROLLDOWN, S_CAISSEENREGISTREUSE, S_ERROR,
    S_APT_SOLDAT, S_APT_MEDECIN, S_APT_AMERICAIN, S_APT_NINJA, S_APT_VAMPIRE,
    S_APT_MAGICIEN, S_APT_KAMIKAZE, S_APT_FAUCHEUSE, S_APT_PHYSICIEN, S_APT_CAMPEUR,
    S_APT_ESPION, S_APT_PRETRE, S_APT_VIKING, S_APT_JUNKIE, S_APT_SHOSHONE,

    // powerups & item sounds
    S_ROIDS_SHOOT, S_ROIDS_SHOOT_FAR, S_ROIDS_PUPOUT, S_EPO_RUN, S_EPO_PUPOUT,
    S_SHROOMS_PUPOUT, S_ASSISTALARM,

    //npcs
    S_ALIEN_H, S_ALIEN_P, S_ALIEN_A, S_ALIEN_D,

    //quotes (fr only)
    S_CGCORTEX, S_CGVALOCHE, S_CGVIEILLE, S_CGHENDEK, S_CGMILITAIREA, S_CGMILITAIREB, S_CGMOUNIR, S_CGDELAVIER, S_CGPRAUD, S_CGRENE, S_CGRAOULT
};

// network messages codes, c2s, c2c, s2c
enum { PRIV_NONE = 0, PRIV_MASTER, PRIV_AUTH, PRIV_ADMIN };

enum
{
    N_CONNECT = 0, N_SERVINFO, N_WELCOME, N_INITCLIENT, N_POS, N_TEXT, N_SOUND, N_CDIS,
    N_SHOOT, N_EXPLODE, N_SUICIDE,
    N_DIED, N_DAMAGE, N_VAMPIRE, N_HITPUSH, N_SHOTFX, N_EXPLODEFX,
    N_TRYSPAWN, N_SPAWNSTATE, N_SPAWN, N_FORCEDEATH,
    N_GUNSELECT, N_TAUNT,
    N_MAPCHANGE, N_MAPVOTE, N_TEAMINFO, N_ITEMSPAWN, N_ITEMPICKUP, N_ITEMACC, N_TELEPORT, N_JUMPPAD,
    N_PING, N_PONG, N_CLIENTPING,
    N_TIMEUP, N_PREMISSION, N_FORCEINTERMISSION,
    N_SERVMSG, N_ITEMLIST, N_RESUME,
    N_EDITMODE, N_EDITENT, N_EDITF, N_EDITT, N_EDITM, N_FLIP, N_COPY, N_PASTE, N_ROTATE, N_REPLACE, N_DELCUBE, N_CALCLIGHT, N_REMIP, N_EDITVSLOT, N_UNDO, N_REDO, N_NEWMAP, N_GETMAP, N_SENDMAP, N_CLIPBOARD, N_EDITVAR,
    N_MASTERMODE, N_KICK, N_CLEARBANS, N_CURRENTMASTER, N_SPECTATOR, N_SETMASTER, N_SETTEAM,
    N_LISTDEMOS, N_SENDDEMOLIST, N_GETDEMO, N_SENDDEMO,
    N_DEMOPLAYBACK, N_RECORDDEMO, N_STOPDEMO, N_CLEARDEMOS,
    N_TAKEFLAG, N_RETURNFLAG, N_RESETFLAG, N_TRYDROPFLAG, N_DROPFLAG, N_SCOREFLAG, N_INITFLAGS,
    N_SAYTEAM,
    N_CLIENT,
    N_AUTHTRY, N_AUTHKICK, N_AUTHCHAL, N_AUTHANS, N_REQAUTH,
    N_PAUSEGAME, N_GAMESPEED,
    N_ADDBOT, N_DELBOT, N_INITAI, N_FROMAI, N_BOTLIMIT, N_BOTBALANCE,
    N_MAPCRC, N_CHECKMAPS,
    N_SWITCHNAME, N_SWITCHMODEL, N_SWITCHCOLOR, N_SWITCHTEAM,
    N_SERVCMD,
    N_DEMOPACKET,
    N_SENDCAPE, N_SENDTOMBE, N_SENDDANSE, N_SENDAPTITUDE, N_REGENALLIES,
    N_REQABILITY, N_GETABILITY,
    N_ANNOUNCE,
    N_IDENTIQUEARME, N_SERVAMBIENT,
    N_BASES, N_BASEINFO, N_BASESCORE, N_CNBASESCORE, N_REPAMMO, N_BASEREGEN,
    NUMMSG
};

static const int msgsizes[] =               // size inclusive message token, 0 for variable or not-checked sizes
{
    N_CONNECT, 0, N_SERVINFO, 0, N_WELCOME, 1, N_INITCLIENT, 0, N_POS, 0, N_TEXT, 0, N_SOUND, 2, N_CDIS, 2,
    N_SHOOT, 0, N_EXPLODE, 0, N_SUICIDE, 1,
    N_DIED, 7, N_DAMAGE, 6, N_VAMPIRE, 5, N_HITPUSH, 7, N_SHOTFX, 10, N_EXPLODEFX, 4,
    N_TRYSPAWN, 1, N_SPAWNSTATE, 0, N_SPAWN, 3, N_FORCEDEATH, 2,
    N_GUNSELECT, 2, N_TAUNT, 1,
    N_MAPCHANGE, 0, N_MAPVOTE, 0, N_TEAMINFO, 0, N_ITEMSPAWN, 2, N_ITEMPICKUP, 2, N_ITEMACC, 4,
    N_PING, 2, N_PONG, 2, N_CLIENTPING, 2,
    N_TIMEUP, 2, N_PREMISSION, 2, N_FORCEINTERMISSION, 1,
    N_SERVMSG, 0, N_ITEMLIST, 0, N_RESUME, 0,
    N_EDITMODE, 2, N_EDITENT, 11, N_EDITF, 16, N_EDITT, 16, N_EDITM, 16, N_FLIP, 14, N_COPY, 14, N_PASTE, 14, N_ROTATE, 15, N_REPLACE, 17, N_DELCUBE, 14, N_CALCLIGHT, 1, N_REMIP, 1, N_EDITVSLOT, 16, N_UNDO, 0, N_REDO, 0, N_NEWMAP, 2, N_GETMAP, 1, N_SENDMAP, 0, N_EDITVAR, 0,
    N_MASTERMODE, 2, N_KICK, 0, N_CLEARBANS, 1, N_CURRENTMASTER, 0, N_SPECTATOR, 3, N_SETMASTER, 0, N_SETTEAM, 0,
    N_LISTDEMOS, 1, N_SENDDEMOLIST, 0, N_GETDEMO, 3, N_SENDDEMO, 0,
    N_DEMOPLAYBACK, 3, N_RECORDDEMO, 2, N_STOPDEMO, 1, N_CLEARDEMOS, 2,
    N_TAKEFLAG, 3, N_RETURNFLAG, 4, N_RESETFLAG, 3, N_TRYDROPFLAG, 1, N_DROPFLAG, 7, N_SCOREFLAG, 9, N_INITFLAGS, 0,
    N_SAYTEAM, 0,
    N_CLIENT, 0,
    N_AUTHTRY, 0, N_AUTHKICK, 0, N_AUTHCHAL, 0, N_AUTHANS, 0, N_REQAUTH, 0,
    N_PAUSEGAME, 0, N_GAMESPEED, 0,
    N_ADDBOT, 2, N_DELBOT, 1, N_INITAI, 0, N_FROMAI, 2, N_BOTLIMIT, 2, N_BOTBALANCE, 2,
    N_MAPCRC, 0, N_CHECKMAPS, 1,
    N_SWITCHNAME, 0, N_SWITCHMODEL, 2, N_SWITCHCOLOR, 2, N_SWITCHTEAM, 2,
    N_SERVCMD, 0,
    N_DEMOPACKET, 0,
    N_SENDCAPE, 2, N_SENDTOMBE, 2, N_SENDDANSE, 2, N_SENDAPTITUDE, 2, N_REGENALLIES, 4,
    N_REQABILITY, 2, N_GETABILITY, 4,
    N_ANNOUNCE, 3,
    N_IDENTIQUEARME, 2, N_SERVAMBIENT, 2,
    N_BASES, 0, N_BASEINFO, 0, N_BASESCORE, 0, N_CNBASESCORE, 3, N_REPAMMO, 1, N_BASEREGEN, 7,
    -1
};

#define CC_SERVER_PORT 43000
#define CC_LANINFO_PORT 42998
#define CC_MASTER_PORT 42999
#define PROTOCOL_VERSION 10          // bump when protocol changes
#define DEMO_VERSION 1               // bump when demo format changes
#define DEMO_MAGIC "CC_DEMO\0\0"

struct demoheader
{
    char magic[16];
    int version, protocol;
};

#define MAXNAMELEN 25

enum
{
    HICON_RED_FLAG = 0,
    HICON_BLUE_FLAG,

    HICON_X       = 20,
    HICON_Y       = 1650,
    HICON_TEXTY   = 1644,
    HICON_STEP    = 490,
    HICON_SIZE    = 120,
    HICON_SPACE   = 40
};

static struct itemstat { int add, max, sound; const char *name_fr, *name_en; int icon, info; } itemstats[] =
{   // weapons
    {15,    60,    S_ITEMAMMO,  "FUSIL ELECTRIQUE", "ELECTRIC RIFLE",   HICON_SIZE, GUN_RAIL},
    {32,   128,    S_ITEMAMMO,  "FUSIL PLASMA",     "PLASMA RIFLE",     HICON_SIZE, GUN_PULSE},
    {5,     20,    S_ITEMAMMO,  "SMAW",             "SMAW",             HICON_SIZE, GUN_SMAW},
    {80,   320,    S_ITEMAMMO,  "MINIGUN",          "MINIGUN",          HICON_SIZE, GUN_MINIGUN},
    {20,    80,    S_ITEMAMMO,  "SPOCKGUN",         "SPOCKGUN",         HICON_SIZE, GUN_SPOCKGUN},
    {7,     28,    S_ITEMAMMO,  "M32",              "M32",              HICON_SIZE, GUN_M32},
    {50,   200,    S_ITEMAMMO,  "LANCE-FLAMMES",    "FLAMETHROWER",     HICON_SIZE, GUN_LANCEFLAMMES},
    {50,   200,    S_ITEMAMMO,  "UZI",              "UZI",              HICON_SIZE, GUN_UZI},
    {60,   240,    S_ITEMAMMO,  "FAMAS",            "FAMAS",            HICON_SIZE, GUN_FAMAS},
    {10,    40,    S_ITEMAMMO,  "MOSSBERG 500",     "MOSSBERG 500",     HICON_SIZE, GUN_MOSSBERG},
    {15,    60,    S_ITEMAMMO,  "HYDRA",            "HYDRA",            HICON_SIZE, GUN_HYDRA},
    {8,     32,    S_ITEMAMMO,  "SV-98",            "SV-98",            HICON_SIZE, GUN_SV98},
    {14,    56,    S_ITEMAMMO,  "SKS",              "SKS",              HICON_SIZE, GUN_SKS},
    {12,    48,    S_ITEMAMMO,  "ARBALETE",         "CROSSBOW",         HICON_SIZE, GUN_ARBALETE},
    {40,   160,    S_ITEMAMMO,  "AK-47",            "AK-47",            HICON_SIZE, GUN_AK47},
    {70,   280,    S_ITEMAMMO,  "GAPB-1",           "GAPB-1",           HICON_SIZE, GUN_GRAP1},
    {10,    40,    S_ITEMAMMO,  "FEU D'ARTIFICE",   "FIREWORKS",        HICON_SIZE, GUN_ARTIFICE},
    {30,   120,    S_ITEMAMMO,  "GLOCK",            "GLOCK",            HICON_SIZE, GUN_GLOCK},
    // superweapons
    {  1,    4,    S_ITEMSUPERAMMO, "BOMBE NUCLEAIRE", "NUCLEAR MISSLE",    HICON_SIZE, GUN_S_NUKE},
    {300, 1200,    S_ITEMSUPERAMMO, "GAU-8",           "GAU-8",             HICON_SIZE, GUN_S_GAU8},
    { 40,  120,    S_ITEMSUPERAMMO, "MINI-ROQUETTES",  "ROCKETS MINIGUN",   HICON_SIZE, GUN_S_ROQUETTES},
    { 15,   60,    S_ITEMSUPERAMMO, "CAMPOUZE 2000",   "CAMPER 2000",       HICON_SIZE, GUN_S_CAMPOUZE},
    // items
    {250,     1000, S_ITEMHEALTH,   "PANACHAY",            "HEALTH",         HICON_SIZE},
    {500,     2500, S_COCHON,       "COCHON GRILLAY",      "HEALTH BOOST",   HICON_SIZE},
    {30000,  45000, S_ITEMSTEROS,   "STEROIDES",           "ROIDS",          HICON_SIZE},
    {40000, 120000, S_ITEMCHAMPIS,  "CHAMPIS",             "SHROOMS",        HICON_SIZE},
    {40000,  60000, S_ITEMEPO,      "EPO",                 "EPO",            HICON_SIZE},
    {30000,  90000, S_ITEMJOINT,    "JOINT",               "JOINT",          HICON_SIZE},
    {750,      750, S_ITEMBBOIS,    "BOUCLIER EN BOIS",    "WOOD SHIELD",    HICON_SIZE, A_BLUE},
    {1250,    1250, S_ITEMBFER,     "BOUCLIER DE FER",     "IRON SHIELD",    HICON_SIZE, A_GREEN},
    {2000,    2000, S_ITEMBOR,      "BOUCLIER D'OR",       "GOLD SHIELD",    HICON_SIZE, A_YELLOW},
    {1500,    1500, S_ITEMBMAGNET,  "BOUCLIER MAGNETIQUE", "MAGNET SHIELD",  HICON_SIZE, A_MAGNET},
    {3000,    3000, S_ITEMARMOUR,   "ARMURE ASSISTEE",     "POWERARMOR",     HICON_SIZE, A_ASSIST},
    {50,       150, S_ITEMMANA,     "MANA",                "MANA",           HICON_SIZE},
    {50,       150, S_ITEMHEALTH,   "CHAIN",               "CHAIN",          HICON_SIZE}
};

#define MAXRAYS 25
#define EXP_SELFDAMDIV 1
#define EXP_SELFPUSH 1.0f
#define EXP_DISTSCALE 0.5f

static const struct attackinfo { int gun, action, anim, vwepanim, hudanim, picksound, sound, middistsnd, fardistsnd, specialsounddelay, attackdelay, damage, spread, nozoomspread, margin, projspeed, kickamount, range, rays, hitpush, exprad, ttl, use; } attacks[NUMATKS] =
{
    //Armes "normales"
    { GUN_RAIL,         ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADFUTUR,     S_ELECRIFLE,    S_ELECRIFLE_FAR,    S_FAR_LIGHT, 10,  350,  325,  35, 105, 0,    0,  10, 8000,  1,    30,   0, 0, 0},
    { GUN_PULSE,        ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADFUTUR,     S_PLASMARIFLE,  S_PLASMARIFLE_FAR,  S_FAR_LIGHT, 25,   90,  180,  45, 135, 0, 1500,   5, 8000,  1,    50,  25, 0, 0},
    { GUN_SMAW,         ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADBIG,       S_SMAW,         S_SMAW_FAR,                  -1,  3, 1250,  850,  20,  60, 2,  600,  15, 8000,  1,   750, 125, 0, 0},
    { GUN_MINIGUN,      ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADMID,       S_MINIGUN,      S_MINIGUN_FAR,      S_FAR_LIGHT, 35,   60,  180,  60, 180, 0, 3500,   5, 8000,  1,    15 ,  7, 0, 0},
    { GUN_SPOCKGUN,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADALIEN,     S_SPOCKGUN,     S_SPOCKGUN_FAR,     S_FAR_LIGHT, 15,  175,  250,  15, 150, 3, 1750,   5, 8000,  1,    30,  15, 0, 0},
    { GUN_M32,          ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADMID,       S_M32,          S_M32_FAR,                   -1,  3, 1000, 1250,  20,  50, 0,  400,  10, 1000,  1,   600, 160, 1000, 0},
    { GUN_LANCEFLAMMES, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADMID,       S_FLAMETHROWER, S_FLAMETHROWER_FAR,          -1, 30,  100,   38, 500, 500, 9,    0,   2,  280, 10,    10 ,  0, 0, 0},
    { GUN_UZI,          ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADSMALL,     S_UZI,          S_UZI_FAR,          S_FAR_LIGHT, 35,   75,  150,  50, 150, 0, 3000,   2, 8000,  1,    10,   5, 0, 0},
    { GUN_FAMAS,        ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADSMALL,     S_FAMAS,        S_FAMAS_FAR,        S_FAR_LIGHT, 30,   90,  140,  40, 120, 0, 3500,   3, 8000,  1,    20,   5, 0, 0},
    { GUN_MOSSBERG,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADMID,       S_MOSSBERG,     S_MOSSBERG_FAR, S_FAR_VERYHEAVY,  3, 1200,  115, 500, 500, 0,    0,  20, 1000, 25,    20,   0, 0, 0},
    { GUN_HYDRA,        ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADSMALL,     S_HYDRA,        S_HYDRA_FAR,    S_FAR_VERYHEAVY,  4,  315,   75, 300, 300, 0,    0,  15,  600, 15,    20,   0, 0, 0},
    { GUN_SV98,         ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADMID,       S_SV98,         S_SV98_FAR,         S_FAR_HEAVY,  2, 1500,  900,   1, 200, 0, 4500,  30, 8000,  1,    80,   7, 0, 0},
    { GUN_SKS,          ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADMID,       S_SKS,          S_SKS_FAR,          S_FAR_HEAVY, 10,  420,  500,   5, 125, 0, 3500,  25, 8000,  1,    50,   7, 0, 0},
    { GUN_ARBALETE,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADMID,       S_CROSSBOW,     S_CROSSBOW_FAR,              -1,  5,  900,  850,  10,  90, 0, 2000,   7, 8000,  1,    20,   3, 45000, 0},
    { GUN_AK47,         ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADMID,       S_AK47,         S_AK47_FAR,         S_FAR_LIGHT, 30,   92,  170,  60, 180, 0, 3000,   7, 8000,  1,    50,   5, 0, 0},
    { GUN_GRAP1,        ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADFUTUR,     S_GRAP1,        S_GRAP1_FAR,                 -1, 12,  200,  250,  30, 300, 3, 1500,  -4, 8000,  1,  -600,  20, 0, 0},
    { GUN_ARTIFICE,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADSMALL,     S_FIREWORKS,    S_FIREWORKS_FAR,             -1,  3, 1100,  800,  35, 200, 0, 1200,  45,  520,  1,   500,  80, 250, 0},
    { GUN_GLOCK,        ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADSMALL,     S_GLOCK,        S_GLOCK_FAR,        S_FAR_LIGHT, 10,  100,  280,   5, 150, 0, 2000,   7, 8000,  1,    30,   3, 0, 0},
    // Super armes
    { GUN_S_NUKE,       ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADBIG,       S_NUKE,         S_NUKE_FAR,          S_NUKE_FAR,   1, 3000,  3250,  20, 300, 2,  175,  10, 2000,  1,   400, 1500, 6000, 0},
    { GUN_S_GAU8,       ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADBIG,       S_GAU8,         -1,                           -1, 90,   14,   370, 150, 250, 3, 6000,   9, 8000,  1,    80,   20, 0, 0},
    { GUN_S_ROQUETTES,  ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADBIG,       S_MINIROCKETS,  S_MINIROCKETS_FAR, S_FAR_VERYHEAVY,   14,  170,  2000,  10, 300, 2,  700,   6, 8000,  1,   500,  100, 0, 0},
    { GUN_S_CAMPOUZE,   ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADBIG,       S_CAMPOUZE,     S_CAMPOUZE_FAR,    S_FAR_VERYHEAVY,    8,  500,   500,  50,  50, 5, 5000,   3, 8000, 10,   150,    8, 0, 0},
    // Armes corps à corps
    { GUN_CAC349,       ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADSWORD,     S_SWORD349,     -1, -1,   4, 1000,  600, 1, 1, 20, 0, -10,  28,  1,  50,  0, 0, 0},
    { GUN_CACMARTEAU,   ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADSLOWWOOSH, S_BANHAMMER,    -1, -1,   3, 1500, 1000, 1, 1, 15, 0,  -5,  30,  1,  10,  0, 0, 0},
    { GUN_CACMASTER,    ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADSWORD,     S_MASTERSWORD,  -1, -1,   5, 600,   430, 1, 1, 20, 0,  -8,  26,  1,  30,  0, 0, 0},
    { GUN_CACFLEAU,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADCHAINS,    S_FLAIL,        -1, -1,   4, 1150,  750, 1, 1, 10, 0, -10,  32,  1, 125,  0, 0, 0},
    // Armes spéciales
    { GUN_KAMIKAZE,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADFASTWOOSH, S_EXPL_KAMIKAZE, S_EXPL_FAR, S_EXPL_FAR,   1, 1000, 4000, 1, 1,  0, 1,  10, 120,  1, 250, 500, 5, 0},
    { GUN_ASSISTXPL,    ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADBIG,       S_EXPL_PARMOR,   S_EXPL_FAR, S_EXPL_FAR,   1,  220, 1500, 1, 1,  0, 1,  10,  50,  1, 100, 350, 5, 0},
    { GUN_CACNINJA,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_WPLOADSWORD,     S_NINJASABER,            -1,         -1,   8,  400,  900, 1, 1, 30, 0, -10,  36,  1,  25,   0, 0, 0}
};

static const struct guninfo { const char *name, *file, *vwep, *armedescFR, *armedescEN; int maxweapposside, maxweapposup, maxzoomfov, hudrange, attacks[NUMACTS]; } guns[NUMGUNS] =
{
    //Armes "normales"
    { "fusilelectrique", "fusilelectrique", "worldgun/fusilelectrique", "un fusil électrique !", "an electric rifle.",                   65,  26, 60, 1000,  { -1, ATK_RAIL_SHOOT }, },
    { "fusilplasma", "fusilplasma", "worldgun/fusilplasma",             "un fusil du turfu !", "a futuristic rifle.",                    28,  10, 60,  750,  { -1, ATK_PULSE_SHOOT }, },
    { "smaw", "smaw", "worldgun/smaw",                                  "un lance-roquettes de noob.", "a noob rocket launcher.",         8,   8, 85,  500,  { -1, ATK_SMAW_SHOOT }, },
    { "minigun", "minigun", "worldgun/minigun",                         "un minigun cheaté.", "a cheated minigun.",                      36,  11, 80,  750,  { -1, ATK_MINIGUN_SHOOT }, },
    { "spockgun", "spockgun", "worldgun/spockgun",                      "un pistolet alien", "an alien gun.",                            52,  20, 70,  500,  { -1, ATK_SPOCKGUN_SHOOT }, },
    { "m32", "m32", "worldgun/m32",                                     "une grenade imprévisible.", "an unpredictable grenade",         65,  21, 85,  600,  { -1, ATK_M32_SHOOT }, },
    { "lanceflammes", "lanceflammes", "worldgun/lanceflammes",          "un lance-flammes !", "a flame thrower!",                        40,  15, 95,  280,  { -1, ATK_LANCEFLAMMES_SHOOT }, },
    { "uzi", "uzi", "worldgun/uzi",                                     "une mitraillette de gangster.", "a gangster's weapon.",         23,  21, 80,  500,  { -1, ATK_UZI_SHOOT }, },
    { "famas", "famas", "worldgun/famas",                               "une arme made in France", "a weapon made in France.",           54,  14, 70,  750,  { -1, ATK_FAMAS_SHOOT }, },
    { "mossberg500", "mossberg500", "worldgun/mossberg500",             "un fusil à pompe de vieux con.", "an old man's shotgun.",       38,  18, 95,  300,  { -1, ATK_MOSSBERG_SHOOT }, },
    { "hydra", "hydra", "worldgun/hydra",                               "un fusil venant d'un autre jeu !", "a gun from another game!",  44,  20, 95,  300,  { -1, ATK_HYDRA_SHOOT }, },
    { "sv98", "sv98", "worldgun/sv98",                                  "un sniper de campeur.", "a camper rifle.",                       1,   3, 30, 2000,  { -1, ATK_SV98_SHOOT }, },
    { "sks", "sks", "worldgun/sks",                                     "une carabine russe !", "a russian rifle!",                       1,   3, 50, 1500,  { -1, ATK_SKS_SHOOT }, },
    { "arbalete", "arbalete", "worldgun/arbalete",                      "une flèche de merde !", "a rotten arrow.",                       1,   3, 45, 1000,  { -1, ATK_ARBALETE_SHOOT }, },
    { "ak47", "ak47", "worldgun/ak47",                                  "l'arme à Vladimir Poutine !", "Putin's weapon!",                46,  25, 70,  750,  { -1, ATK_AK47_SHOOT }, },
    { "gapb1", "gapb1", "worldgun/gapb1",                               "un projectile rose de tapette !", "a gay ass pink projectile.", 43,  17, 85,  500,  { -1, ATK_GRAP1_SHOOT }, },
    { "feuartifice", "feuartifice", "worldgun/feuartifice",             "une arme de Gilet jaune.", "a yellow vests weapon",             70,  30, 85,  500,  { -1, ATK_ARTIFICE_SHOOT }, },
    { "glock", "glock", "worldgun/glock",                               "un pistolet vraiment pourri.", "a very bad gun",                55,  20, 85,  300,  { -1, ATK_GLOCK_SHOOT }, },
    //Super armes
    { "missilenorko", "missilenorko", "worldgun/missilenorko",          "une putain de bombe nucléaire !", "a fucking nuclear missile!",    8,   3, 85, 2000,  { -1, ATK_NUKE_SHOOT }, },
    { "gau8", "gau8", "worldgun/gau8",                                  "un GAU-8 portable !", "a portable GAU-8!",                        57,  10, 85, 2000,  { -1, ATK_GAU8_SHOOT }, },
    { "miniroquettes", "miniroquettes", "worldgun/miniroquettes",       "un minigun à roquettes !", "a missiles minigun",                  10,  10, 70, 1000,  { -1, ATK_ROQUETTES_SHOOT }, },
    { "campouze2000", "campouze2000", "worldgun/campouze2000",          "el famoso Campouze 2000 !", "the famous Camp-2000!",              10,  10, 60, 3000,  { -1, ATK_CAMPOUZE_SHOOT }, },
    //Corps à corps
    { "epee349", "armes_cac/epee349", "worldgun/armes_cac/epee349",             "l'épée collector à 349 euros.", "a 386$ collector sword.",      4, 3, 95, 120,  { -1, ATK_CAC349_SHOOT }, },
    { "marteauban", "armes_cac/marteauban", "worldgun/armes_cac/marteauban",    "un marteau de bannissement !", "the Ban Hammer.",               4, 3, 95, 120,  { -1, ATK_CACMARTEAU_SHOOT }, },
    { "mastersword", "armes_cac/mastersword", "worldgun/armes_cac/mastersword", "une épée légendaire !", "a legendary sword!",                   4, 3, 95, 120,  { -1, ATK_CACMASTER_SHOOT }, },
    { "fleau", "armes_cac/fleau", "worldgun/armes_cac/fleau",                   "une boule piquante !", "a spiky ball.",                         4, 3, 95, 120,  { -1, ATK_CACFLEAU_SHOOT }, },
    // Armes spéciales aptitudes
    { "kamikaze",   "kamikaze",         "worldgun/kamikaze",        "une ceinture d'explosifs !",   "an explosives ISIS's made belt!",           4, 3, 95, 0,  { -1, ATK_KAMIKAZE_SHOOT }, },
    { "assistxpl",  "assistxpl",        "worldgun/assistxpl",       "une armure assistée !",        "powered combat armor!",                     4, 3, 95, 0,  { -1, ATK_ASSISTXPL_SHOOT }, },
    { "sabre",      "armes_cac/sabre",  "worldgun/armes_cac/sabre", "un sabre de ninja !",          "a ninja saber!",                            4, 3, 95, 120,  { -1, ATK_CACNINJA_SHOOT }, }
};

//Définition des aptitudes
enum {APT_SOLDAT = 0, APT_MEDECIN, APT_AMERICAIN, APT_NINJA, APT_VAMPIRE, APT_MAGICIEN, APT_KAMIKAZE, APT_FAUCHEUSE, APT_PHYSICIEN, APT_CAMPEUR, APT_ESPION, APT_PRETRE, APT_VIKING, APT_JUNKIE, APT_SHOSHONE, NUMAPTS};

struct ability { const int manacost, duration, cooldown, snd; };
static const struct aptitudesinfo { int apt_degats, apt_resistance, apt_precision, apt_vitesse; const char *apt_nomFR, *apt_nomEN; ability abilities[3];} aptitudes[NUMAPTS] =
{   // classe stats           // classe name               // ability 1               // ability 2                // ability 3
    { 105,  105,  105,   950, "Soldat",     "Soldier",   { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },  // 0 APT_SOLDAT
    {  90,  115,   95,   950, "Médecin",    "Medic",     { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },  // 1 APT_MEDECIN
    { 100,  135,   80,  1300, "Américain",  "American",  { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },  // 2 APT_AMERICAIN
    {  85,   90,   75,   750, "Ninja",      "Ninja",     { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },  // 3 APT_NINJA
    { 110,   65,  110,   950, "Vampire",    "Vampire",   { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },  // 4 APT_VAMPIRE
    { 100,   85,   90,  1000, "Magicien",   "Wizard",    { {30,  250, 2000, S_WIZ_1}, {40,  4000, 5000, S_WIZ_2}, {60, 3000,  6000, S_WIZ_3} } },  // 5 APT_MAGICIEN
    { 100,  100,   70,   850, "Kamikaze",   "Kamikaze",  { {0,     0,    0,      -1}, {100, 5000, 5000, S_TIMER}, {0,     0,     0,      -1} } },  // 6 APT_KAMIKAZE
    { 120,   85,   90,  1050, "Faucheuse",  "Reaper",    { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },  // 7 APT_FAUCHEUSE
    {  90,   85,   85,  1050, "Physicien",  "Physicist", { {45, 4000, 3000, S_PHY_1}, {50,  5000, 7000, S_PHY_2}, {65, 6000,  9000, S_PHY_3} } },  // 8 APT_PHYSICIEN
    { 100,   60,  135,  1250, "Campeur",    "Camper",    { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },  // 9 APT_CAMPEUR
    {  90,   85,  120,  1100, "Espion",     "Spy",       { {40, 4000, 7000, S_SPY_1}, {50,  7000, 7000, S_SPY_2}, {60, 5000, 10000, S_SPY_3} } },  // 10 APT_ESPION
    {  85,  105,   85,   950, "Prêtre",     "Priest",    { {30, 4000, 8000, S_PRI_1}, {10,  8000, 8000, S_PRI_2}, {80, 4000, 10000, S_PRI_3} } },  // 11 APT_PRETRE
    { 100,  120,   60,  1050, "Viking",     "Viking",    { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },  // 12 APT_VIKING
    { 100,  110,   85,  1100, "Junkie",     "Junkie",    { {0,     0,    0,     - 1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },  // 13 APT_JUNKIE
    { 100,  100,   75,  1000, "Shoshone",   "Shoshone",  { {50, 7500, 7500, S_SHO_1}, {50,  7500, 7500, S_SHO_2}, {50, 7500,  7500, S_SHO_3} } }   // 14 APT_SHOSHONE
};

#include "ai.h"

// inherited by gameent and server clients
struct gamestate
{
    int health, maxhealth, mana;
    int armour, armourtype;
    int boostmillis[4], ragemillis, vampimillis;
    int abilitymillis[3], aptiseed;
    int gunselect, gunwait;
    int ammo[NUMGUNS];
    int aitype, skill;

    gamestate() : maxhealth(1), aitype(AI_NONE), skill(0) {}

    void baseammo(int gun, int k = 2)
    {
        ammo[gun] = (itemstats[gun-GUN_RAIL].add*k);
    }

    void addammo(int gun, int k = 1, int scale = 1)
    {
        itemstat &is = itemstats[gun-GUN_RAIL];
        ammo[gun] = min(ammo[gun] + (is.add*k)/scale, is.max);
    }

    bool hasmaxammo(int type)
    {
       const itemstat &is = itemstats[type-I_RAIL];
       return ammo[type-I_RAIL+GUN_RAIL]>=is.max;
    }

    bool canpickupitem(int type, int aptitude, bool haspowerarmor)
    {
        if(type<I_RAIL || type>I_MANA) return false;
        itemstat &is = itemstats[type-I_RAIL];

        switch(type)
        {
            case I_SANTE: return health<maxhealth;

            case I_BOOSTPV: return maxhealth<is.max;

            case I_MANA:
                if(aptitude==APT_VAMPIRE) return health<maxhealth;
                else return (aptitude==APT_MAGICIEN || aptitude==APT_PHYSICIEN || aptitude==APT_PRETRE || aptitude==APT_ESPION || aptitude==APT_SHOSHONE) && mana<is.max;

            case I_BOOSTDEGATS: case I_BOOSTVITESSE: case I_BOOSTGRAVITE: case I_BOOSTPRECISION:
                return boostmillis[type-I_BOOSTDEGATS]<is.max;

            case I_BOUCLIERBOIS: return haspowerarmor ? armour<3000 : armour < (aptitude==APT_SOLDAT ? 1000 : is.max);

            case I_BOUCLIERFER: return haspowerarmor ? armour<3000 : armour < (aptitude==APT_SOLDAT ? 1750 : is.max);

            case I_BOUCLIERMAGNETIQUE: return haspowerarmor ? armour<3000 : armour < (aptitude==APT_SOLDAT ? 2500 : is.max);

            case I_BOUCLIEROR: return haspowerarmor ? armour<3000 : armour < (aptitude==APT_SOLDAT ? 2750 : is.max);

            case I_ARMUREASSISTEE: return !haspowerarmor;

            default: return ammo[is.info]<is.max*(aptitude==APT_AMERICAIN ? 1.5f : 1);
        }
    }

    void pickupitem(int type, int aptitude, int aptisort, bool haspowerarmor, int rndsweap)
    {
        if(type<I_RAIL || type>I_MANA) return;
        itemstat &is = itemstats[type-I_RAIL+rndsweap];

        int itemboost = aptitude==APT_PRETRE && aptisort>0 ? 2 : 1;

        switch(type)
        {
            case I_BOOSTPV:
                health = min(health+is.add*(aptitude==APT_MEDECIN ? 1.5f : itemboost), 2500.0f);
                return;

            case I_SANTE:
                health = min(health+is.add*(aptitude==APT_MEDECIN ? 2 : itemboost), maxhealth);
                return;

            case I_MANA:
                if(aptitude!=APT_VAMPIRE) mana = min(mana+is.add, is.max);
                else health = min(health+250, maxhealth);
                return;

            case I_BOUCLIERBOIS: case I_BOUCLIERFER: case I_BOUCLIEROR: case I_BOUCLIERMAGNETIQUE: case I_ARMUREASSISTEE:
                if(type==I_ARMUREASSISTEE)
                {
                    armourtype = A_ASSIST;
                    armour = min(armour+is.add, is.max);
                    health = min(health+300, maxhealth);
                    if(ammo[GUN_ASSISTXPL]<=1) ammo[GUN_ASSISTXPL] = 1;
                    return;
                }
                else
                {
                    armourtype = haspowerarmor ? A_ASSIST : is.info;
                    int armourval = haspowerarmor && armour> 0 ? 500*itemboost : aptitude==APT_SOLDAT ? is.max+(250*(armourtype+1)) : is.max;
                    armour = min(armour+armourval, haspowerarmor ? 3000 : armourval);
                    return;
                }

            case I_BOOSTDEGATS: case I_BOOSTVITESSE: case I_BOOSTGRAVITE: case I_BOOSTPRECISION:
                {
                    int boostboost = aptitude==APT_JUNKIE ? 1.5f : itemboost; //cannot find a better var name :)
                    boostmillis[type-I_BOOSTDEGATS] = min(boostmillis[type-I_BOOSTDEGATS]+is.add*boostboost, is.max*boostboost);
                    return;
                }

            default:
                float ammoboost = aptitude==APT_AMERICAIN ? 1.5f : 1;
                ammo[is.info] = min(ammo[is.info]+is.add*itemboost*ammoboost, is.max*ammoboost);
        }
    }

    void respawn()
    {
        health = maxhealth;
        mana = 100;
        loopi(4) boostmillis[i] = 0;
        ragemillis = 0;
        vampimillis = 0;
        gunwait = 0;
        loopi(3) abilitymillis[i] = 0;
        aptiseed = rnd(4);
        loopi(NUMGUNS) ammo[i] = 0;
    }

    void addcacweaps(int gamemode, int aptitude)
    {
        switch(rnd(4))
        {
            case 0: ammo[GUN_CAC349] = 1; gunselect = GUN_CAC349; break;
            case 1: ammo[GUN_CACMARTEAU] = 1; gunselect = GUN_CACMARTEAU; break;
            case 2: ammo[GUN_CACMASTER] = 1; gunselect = GUN_CACMASTER; break;
            case 3: ammo[GUN_CACFLEAU] = 1; gunselect = GUN_CACFLEAU; break;
        }
    }

    void addsweaps()
    {
        switch(rnd(200))
        {
            case 0: ammo[GUN_S_ROQUETTES] = 20; gunselect = GUN_S_ROQUETTES; break;
            case 1: ammo[GUN_S_GAU8] = 250; gunselect = GUN_S_GAU8; break;
            case 2: ammo[GUN_S_CAMPOUZE] = 15; gunselect = GUN_S_CAMPOUZE; break;
            case 3: ammo[GUN_S_NUKE] = 1; gunselect = GUN_S_NUKE; break;
        }
    }

    void spawnstate(int gamemode, int aptitude)
    {
        if(aptitude!=3) addcacweaps(gamemode, aptitude);

        switch(aptitude)
        {
            case 3: ammo[GUN_CACNINJA] = 1; break;
            case 6: ammo[GUN_KAMIKAZE] = 1; break;
        }
        if(m_random)
        {
            armourtype = A_BLUE;
            armour = aptitude==APT_SOLDAT ? 1000 : 750;
            int randomarme = rnd(17);
            gunselect = aptitude==6 ? GUN_KAMIKAZE : aptitude==3 ? GUN_CACNINJA : randomarme;
            ammo[randomarme] = aptitude==2 ? 1.5f*itemstats[randomarme].max : itemstats[randomarme].max;
            if(aptitude==0) addsweaps();
            return;
        }
        else if (m_fullstuff)
        {
            armourtype = A_GREEN;
            armour = aptitude==APT_SOLDAT ? 1750 : 1250;
            int spawngun1 = rnd(17), spawngun2, spawngun3;
            gunselect = spawngun1;
            baseammo(spawngun1, 4);
            do spawngun2 = rnd(17); while(spawngun1==spawngun2);
            baseammo(spawngun2, 4);
            do spawngun3 = rnd(17); while(spawngun1==spawngun3 && spawngun2==spawngun3);
            baseammo(spawngun3, 4);
            gunselect = aptitude==6 ? GUN_KAMIKAZE : aptitude==3 ? GUN_CACNINJA : spawngun1;
            if(aptitude==0) addsweaps();
            return;
        }
        else if(m_identique)
        {
            loopi(17) baseammo(i);
            armourtype = A_BLUE;
            armour = aptitude==APT_SOLDAT ? 1000 : 750;
            gunselect = aptitude==6 ? GUN_KAMIKAZE : aptitude==3 ? GUN_CACNINJA : cnidentiquearme;
            ammo[cnidentiquearme] = aptitude==2 ? 1.5f*itemstats[cnidentiquearme].max : itemstats[cnidentiquearme].max;
            if(aptitude==0) addsweaps();
            return;
        }
        else if(m_capture)
        {
            armourtype = A_BLUE;
            armour = m_regencapture ? aptitude==APT_SOLDAT ? 550 : 300 : 750;
            ammo[GUN_GLOCK] = aptitude==2 ? m_regencapture ? 15 : 45 : m_regencapture ? 10 : 30;
            ammo[GUN_M32] = aptitude==2 ? 3 : 1;
            gunselect = aptitude==6 ? GUN_KAMIKAZE : aptitude==3 ? GUN_CACNINJA : GUN_GLOCK;
            if(aptitude==0) addsweaps();
            return;
        }
        else if(m_sp || m_tutorial)
        {
            armourtype = A_BLUE;
            armour = 0;
            health = 1000;
            mana = 100;

            if(m_dmsp)
            {
                armour = 750;
                ammo[GUN_GLOCK] = aptitude==2 ? 100 : 50;
                gunselect = GUN_GLOCK;
            }
        }
        else
        {
            armourtype = A_BLUE;
            armour = aptitude==APT_SOLDAT ? 1000 : 750;
            ammo[GUN_GLOCK] = aptitude==2 ? 45 : 30;
            ammo[GUN_M32] = aptitude==2 ? 3 : 1;
            gunselect = aptitude==6 ? GUN_KAMIKAZE : aptitude==3 ? GUN_CACNINJA : GUN_GLOCK;
            if(aptitude==0) addsweaps();
        }
    }

    // just subtract damage here, can set death, etc. later in code calling this
    int dodamage(int damage, int aptitude, int aptisort)
    {
        int absorbfactor = 0;
        switch(armourtype)
        {
            case A_BLUE: absorbfactor=25; break;
            case A_GREEN: absorbfactor=50; break;
            case A_YELLOW: absorbfactor=75; break;
            case A_MAGNET: absorbfactor=100; break;
            case A_ASSIST: absorbfactor=85; break;
        }

        int ad = damage*(absorbfactor)/100.f; // let armour absorb when possible

        if(damage>0)
        {
            if(ad>armour) ad = armour;
            if(aptitude==8 && aptisort>0 && armour>0) armour = min(armour+ad, armourtype==A_BLUE ? 750 : armourtype==A_GREEN ? 1250 : armourtype==A_YELLOW ? 2000 : armourtype==A_MAGNET ? 1500 : 3000);
            else armour -= ad;
        }
        damage -= ad;
        health -= damage;
        return damage;
    }

    int doregen(int damage)
    {
        if(health < maxhealth) {
            health += damage;
            if(health>maxhealth) health = maxhealth;
            return damage;
        }
        else return 0;
    }

    int hasammo(int gun, int exclude = -1)
    {
        return validgun(gun) && gun != exclude && ammo[gun] > 0;
    }
};

#define MAXTEAMS 2

static const char * const teamnames_FR[1+MAXTEAMS] = { "", "Yorarien", "Yakekchose" };
static const char * const teamnames_EN[1+MAXTEAMS] = { "", "Democrats", "Republicans" };
static const char * const teamtextcode[1+MAXTEAMS] = { "\fc", "\fd", "\fc" };
static const int teamtextcolor[1+MAXTEAMS] = { 0xFF2222, 0xFFFF22, 0xFF2222 };
static const char * const teamblipcolor[1+MAXTEAMS] = { "_neutral", "_blue", "_red" };
static inline int teamnumber(const char *name) { loopi(MAXTEAMS) if(!strcmp(GAME_LANG ? teamnames_EN[1+i] : teamnames_FR[1+i], name)) return 1+i; return 0; }
#define validteam(n) ((n) >= 1 && (n) <= MAXTEAMS)
#define teamname_FR(n) (teamnames_FR[validteam(n) ? (n) : 0])
#define teamname_EN(n) (teamnames_EN[validteam(n) ? (n) : 0])

struct gameent : dynent, gamestate
{
    int weight;                         // affects the effectiveness of hitpush
    int clientnum, privilege, lastupdate, plag, ping;
    int lifesequence;                   // sequence id for each respawn, used in damage test
    int respawned, suicided;
    int lastpain;
    int lastaction, lastattack;
    int attacking, gunaccel;
    int lastfootstep, attacksound, attackchan, dansesound, dansechan, alarmchan, waterchan;
    int lasttaunt;
    int lastpickup, lastpickupmillis, flagpickup, lastbase, lastrepammo;
    int killstreak, frags, flags, deaths, totaldamage, totalshots;
    editinfo *edit;
    float deltayaw, deltapitch, deltaroll, newyaw, newpitch, newroll;
    int smoothmillis;

    int abisnd[3], abichan[3];
    int lastability[3];
    bool abilityready[3], playerexploded;

    string name, info;
    int team, playermodel, playercolor, customcape, customtombe, customdanse, aptitude, level;
    float skeletonfade, tombepop;
    ai::aiinfo *ai;
    int ownernum, lastnode;

    vec muzzle, weed, balles, assist;

    gameent() : weight(100), clientnum(-1), privilege(PRIV_NONE), lastupdate(0), plag(0), ping(0), lifesequence(0), respawned(-1), suicided(-1), lastpain(0), lastfootstep(0), attacksound(-1), attackchan(-1), dansesound(-1), dansechan(-1), killstreak(0), frags(0), flags(0), deaths(0), totaldamage(0), totalshots(0), edit(NULL), smoothmillis(-1), team(0), playermodel(-1), playercolor(0), customcape(0), customtombe(0), customdanse(0), aptitude(0), level(0), ai(NULL), ownernum(-1), muzzle(-1, -1, -1)
    {
        loopi(3)
        {
            abisnd[i] = -1;
            abichan[i] = -1;
            lastability[i] = -1;
        }
        name[0] = info[0] = 0;
        respawn();
    }
    ~gameent()
    {
        freeeditinfo(edit);
        freeeditinfo(edit);

        if(attackchan >= 0) stopsound(attacksound, attackchan);
        if(dansesound >= 0) stopsound(dansesound, dansechan);
        loopi(3) if(abisnd[i] >= 0) stopsound(abisnd[i], abisnd[i]);
        if(alarmchan >= 0) stopsound(S_ASSISTALARM, alarmchan);
        if(ai) delete ai;
    }

    void hitpush(int damage, const vec &dir, gameent *actor, int atk, gameent *target)
    {
        if(target->aptitude==2) return;
        vec push(dir);
        push.mul((actor==this && attacks[atk].exprad ? EXP_SELFPUSH : 1.0f)*attacks[atk].hitpush*(damage/10)/weight);
        vel.add(push);
    }

    void stopattacksound(gameent *d)
    {
        if(attackchan >= 0) { stopsound(attacksound, attackchan, d->gunselect==GUN_S_GAU8 ? 150 : 250); attacksound = attackchan = -1; }
    }

    void stopdansesound(gameent *d)
    {
        if(dansechan >= 0) { stopsound(S_CGCORTEX+(d->customdanse), dansechan, 50); dansesound = dansechan = -1; }
    }

    void stopabisound(gameent *d)
    {
        loopi(3) if(abichan[i] >= 0) { stopsound(abisnd[i], abichan[i], 50); abisnd[i] = abichan[i] = -1; }
    }

    void stoppowerarmorsound()
    {
        if(alarmchan >= 0) { stopsound(S_ASSISTALARM, alarmchan, 100); alarmchan = -1; }
    }

    void stopunderwatersound()
    {
        if(waterchan >= 0) { stopsound(S_UNDERWATER, waterchan, 200); waterchan = -1; }
    }


    void respawn()
    {
        dynent::reset();
        gamestate::respawn();
        respawned = suicided = -1;
        lastaction = 0;
        lastattack = -1;
        attacking = ACT_IDLE;
        lasttaunt = 0;
        lastpickup = -1;
        lastpickupmillis = 0;
        flagpickup = 0;
        lastbase = lastrepammo = -1;
        lastnode = -1;
        loopi(3) abilityready[i] = true;
        gunaccel = 0;
        playerexploded = false;
    }

    void startgame()
    {
        killstreak = frags = flags = deaths = 0;
        totaldamage = totalshots = 0;
        maxhealth = 1000;
        lifesequence = -1;
        respawned = suicided = -2;
    }
};

struct teamscore
{
    int team, score;
    teamscore() {}
    teamscore(int team, int n) : team(team), score(n) {}

    static bool compare(const teamscore &x, const teamscore &y)
    {
        if(x.score > y.score) return true;
        if(x.score < y.score) return false;
        return x.team < y.team;
    }
};

static inline uint hthash(const teamscore &t) { return hthash(t.team); }
static inline bool htcmp(int team, const teamscore &t) { return team == t.team; }

struct teaminfo
{
    int frags;

    teaminfo() { reset(); }

    void reset() { frags = 0; }
};

namespace entities
{
    extern vector<extentity *> ents;

    extern const char *entmdlname(int type);

    extern void preloadentities();
    extern void renderentities();
    extern void resettriggers();
    extern void checktriggers();
    extern void checkitems(gameent *d);

    extern void checkboosts(int time, gameent *d);
    extern void checkaptiskill(int time, gameent *d);

    extern void resetspawns();
    extern void spawnitems(bool force = false);
    extern void putitems(packetbuf &p);
    extern void setspawn(int i, bool on);
    extern void teleport(int n, gameent *d);
    extern void pickupeffects(int n, gameent *d, int rndsweap);
    extern void teleporteffects(gameent *d, int tp, int td, bool local = true);
    extern void jumppadeffects(gameent *d, int jp, bool local = true);

    extern void repammo(gameent *d, int type, bool local = true);
}

namespace game
{
    //hud
    extern void drawmessages(int killstreak, string str_pseudovictime, int n_aptitudevictime, string str_pseudoacteur, int n_killstreakacteur, float killdistance);
    extern void drawrpgminimap(gameent *d, int w, int h);
    extern int getteamfrags(int team);

    // abilities and boosts
    enum abilities {ABILITY_1 = 0, ABILITY_2, ABILITY_3, NUMABILITIES};
    enum boosts {B_ROIDS = 0, B_SHROOMS, B_EPO, B_JOINT, NUMBOOSTS};
    extern void aptitude(gameent *d, int skill, bool request = true);
    extern void updatespecials(gameent *d);

    // game
    extern int gamemode;

    struct clientmode
    {
        virtual ~clientmode() {}

        virtual void preload() {}
        virtual float clipconsole(float w, float h) { return 0; }
        virtual void drawhud(gameent *d, int w, int h) {}
        virtual void rendergame() {}
        virtual void respawned(gameent *d) {}
        virtual void setup() {}
        virtual void checkitems(gameent *d) {}
        virtual int respawnwait(gameent *d) { return 0; }
        virtual float ratespawn(gameent *d, const extentity &e) { return 1.0f; }
        virtual void pickspawn(gameent *d) { findplayerspawn(d, -1, m_teammode && !m_capture ? d->team : 0); }
        virtual void senditems(packetbuf &p) {}
        virtual void removeplayer(gameent *d) {}
        virtual void gameover() {}
        virtual bool hidefrags() { return false; }
        virtual int getteamscore(int team) { return 0; }
        virtual void getteamscores(vector<teamscore> &scores) {}
        virtual void aifind(gameent *d, ai::aistate &b, vector<ai::interest> &interests) {}
        virtual bool aicheck(gameent *d, ai::aistate &b) { return false; }
        virtual bool aidefend(gameent *d, ai::aistate &b) { return false; }
        virtual bool aipursue(gameent *d, ai::aistate &b) { return false; }
    };

    extern clientmode *cmode;
    extern void setclientmode();
    extern int nextmode;
    extern string clientmap;
    extern bool intermission;
    extern bool premission;
    extern int maptime, maprealtime, maplimit;
    extern gameent *player1;
    extern vector<gameent *> players, clients;
    extern int lastspawnattempt;
    extern int lasthit;
    extern int respawnent;
    extern int following;
    extern int smoothmove, smoothdist;

    extern bool clientoption(const char *arg);
    extern gameent *getclient(int cn);
    extern gameent *newclient(int cn);
    extern const char *colorname(gameent *d, const char *name = NULL, const char *alt = NULL, const char *color = "");
    extern const char *teamcolorname(gameent *d, const char *alt = "Tu");
    extern const char *teamcolor(int team);
    extern gameent *pointatplayer();
    extern gameent *hudplayer();
    extern gameent *followingplayer(gameent *fallback = NULL);
    extern void stopfollowing();
    extern void checkfollow();
    extern void nextfollow(int dir = 1);
    extern void clientdisconnected(int cn, bool notify = true);
    extern void clearclients(bool notify = true);
    extern void startgame();
    extern float proximityscore(float x, float lower, float upper);
    extern void spawnplayer(gameent *);
    extern void deathstate(gameent *d, gameent *actor, bool restore = false);
    extern void damaged(int damage, gameent *d, gameent *actor, bool local = true, int atk = 0);
    extern void killed(gameent *d, gameent *actor, int atk);
    extern void timeupdate(int timeremain);
    extern void msgsound(int n, physent *d = NULL);
    const char *mastermodecolor(int n, const char *unknown);
    const char *mastermodeicon(int n, const char *unknown);

    extern void movehudgun(gameent *d);
    extern void aptichange(int aptitude);
    extern void armechange(int arme);

    // client
    extern bool connected, remote, demoplayback;
    extern string servdesc;
    extern vector<uchar> messages;

    extern int parseplayer(const char *arg);
    extern void ignore(int cn);
    extern void unignore(int cn);
    extern bool isignored(int cn);
    extern bool addmsg(int type, const char *fmt = NULL, ...);
    extern void switchname(const char *name);
    extern void switchteam(const char *name);
    extern void switchplayermodel(int playermodel);
    extern void switchplayercolor(int playercolor);
    extern void sendmapinfo();
    extern void stopdemo();
    extern void changemap(const char *name, int mode);
    extern void calcmode();
    extern void c2sinfo(bool force = false);
    extern void sendposition(gameent *d, bool reliable = false);

    // single player
    enum {D_COMMON = 0, D_UNCOMMON, D_RARE, D_LEGENDARY, D_GODLY, NUMDROPS};
    struct monster;
    extern vector<monster *> monsters;
    extern void npcdrop(const vec *o, int type);

    // minimap
    extern float minimapalpha;
    extern float calcradarscale();
    extern void setradartex();
    extern void drawradar(float x, float y, float s);
    extern void drawminimap(gameent *d, float x, float y, float s);
    extern void drawplayerblip(gameent *d, float x, float y, float s, float blipsize = 1);
    extern void drawnpcs(gameent *d, float x, float y, float s);
    extern void setbliptex(int team, const char *type = "");
    extern void drawteammate(gameent *d, float x, float y, float s, gameent *o, float scale, float blipsize = 1);

    extern void clearmonsters();
    extern void preloadmonsters();
    extern void stackmonster(monster *d, physent *o);
    extern void updatemonsters(int curtime);
    extern void rendermonsters();
    extern void suicidemonster(monster *m);
    extern void hitmonster(int damage, monster *m, gameent *at, const vec &vel, int atk);
    extern void monsterkilled();
    extern void endsp();
    extern void spsummary(int accuracy);

    // weapon
    extern int getweapon(const char *name);
    extern void shoot(gameent *d, const vec &targ);
    extern void shoteffects(int atk, const vec &from, const vec &to, gameent *d, bool local, int id, int prevaction);
    extern void explode(bool local, gameent *owner, const vec &v, const vec &vel, dynent *safe, int dam, int atk);
    extern void explodeeffects(int atk, gameent *d, bool local, int id = 0);
    extern void damageeffect(int damage, gameent *d, gameent *actor, bool thirdperson = true, int atk = 0);
    extern void gibeffect(int damage, const vec &vel, gameent *d);
    extern float intersectdist;
    extern bool intersect(dynent *d, const vec &from, const vec &to, float margin = 0, float &dist = intersectdist);
    extern dynent *intersectclosest(const vec &from, const vec &to, gameent *at, float margin = 0, float &dist = intersectdist);
    extern void clearbouncers();
    extern void updatebouncers(int curtime);
    extern void removebouncers(gameent *owner);
    extern void renderbouncers();
    extern void clearprojectiles();
    extern void updateprojectiles(int curtime);
    extern void removeprojectiles(gameent *owner);
    extern void renderprojectiles();
    extern void preloadbouncers();
    extern void removeweapons(gameent *owner);
    extern void updateweapons(int curtime);
    extern void gunselect(int gun, gameent *d, bool force = false);
    extern void weaponswitch(gameent *d);
    extern void avoidweapons(ai::avoidset &obstacles, float radius);

    // scoreboard
    extern void showscores(bool on);
    extern void getbestplayers(vector<gameent *> &best);
    extern void getbestteams(vector<int> &best);
    extern void clearteaminfo();
    extern void setteaminfo(int team, int frags);
    extern void removegroupedplayer(gameent *d);

    // render
    struct playermodelinfo
    {
        const char *model[1+MAXTEAMS], *hudgun;
        bool ragdoll;
    };

    extern void savetombe(gameent *d);
    extern void clearragdolls();
    extern void moveragdolls();
    extern const playermodelinfo &getplayermodelinfo(gameent *d);
    extern int getplayercolor(gameent *d, int team);
    extern int chooserandomtraits(int seed, int trait);
    extern void syncplayer();
    extern void swayhudgun(int curtime);
    extern vec hudgunorigin(int gun, const vec &from, const vec &to, gameent *d);
}

namespace server
{
    extern const char *modename(int n, const char *unknown = "unknown");
    extern const char *modeprettyname(int n, const char *unknown = "unknown");
    extern const char *mastermodename(int n, const char *unknown = "unknown");
    extern void startintermission();
    extern void stopdemo();
    extern void timeupdate(int secs);
    extern const char *getdemofile(const char *file, bool init);
    extern void forcemap(const char *map, int mode);
    extern void forcepaused(bool paused);
    extern void forcegamespeed(int speed);
    extern void hashpassword(int cn, int sessionid, const char *pwd, char *result, int maxlen = MAXSTRLEN);
    extern int msgsizelookup(int msg);
    extern bool serveroption(const char *arg);
    extern bool delayspawn(int type);
    extern bool pickup(int i, int sender);
}

extern char *rndname(bool firstpart, bool fem, int lang);
extern bool randomevent(int probability);
extern void addsleep(int *msec, char *cmd);
extern void createdrop(const vec *o, int type);
extern void trydisconnect(bool local);

#endif

