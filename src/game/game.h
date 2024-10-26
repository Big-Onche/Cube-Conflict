#ifndef __GAME_H__
#define __GAME_H__

#include "cube.h"
#include "sound.h"
#include "engine.h"
#include "customs.h"

extern int lastshoot, getspyability, autowield;

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
    CON_TEAMKILL   = 1<<13,
    CON_HUDCONSOLE = 1<<14,
};

// network quantization scale
#define DMF 16.0f               // for world locations
#define DNF 100.0f              // for normalized vectors
#define DVELF 1.0f              // for playerspeed based velocity vectors

enum                            // static entity types
{
    NOTUSED = ET_EMPTY,         // entity slot not in use in map
    LIGHT = ET_LIGHT,           // lightsource, attr1 = radius, attr 3/4/5 = color r/g/b, attr6 = when?, attr7 = blink
    MAPMODEL = ET_MAPMODEL,     // attr1 = idx, attr2 = yaw, attr3 = pitch, attr4 = roll, attr5 = scale
    PLAYERSTART,                // attr1 = angle, attr2 = team
    ENVMAP = ET_ENVMAP,         // attr1 = radius
    PARTICLES = ET_PARTICLES,
    MAPSOUND = ET_SOUND,
    SPOTLIGHT = ET_SPOTLIGHT,
    DECAL = ET_DECAL,
    // weapons
    I_RAIL, I_PULSE, I_SMAW, I_MINIGUN, I_SPOCKGUN, I_M32, I_LANCEFLAMMES, I_UZI, I_FAMAS, I_MOSSBERG, I_HYDRA, I_SV98, I_SKS, I_ARBALETE, I_AK47, I_GRAP1, I_ARTIFICE, I_MOLOTOV, I_GLOCK,
    I_SUPERARME, I_NULL1, I_NULL2, I_NULL3,
    // items
    I_SANTE, I_BOOSTPV, I_ROIDS, I_SHROOMS, I_EPO, I_JOINT,
    I_WOODSHIELD, I_IRONSHIELD, I_GOLDSHIELD, I_MAGNETSHIELD, I_POWERARMOR,
    I_MANA,
    // other
    TELEPORT,                   // attr1 = idx, attr2 = model, attr3 = tag
    TELEDEST,                   // attr1 = idx, attr2 = angle
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

const int NUMMAINGUNS = 19;
const int NUMMELEEWEAPONS = 4;
const int NUMSUPERWEAPONS = 4;
enum { ACT_IDLE = 0, ACT_SHOOT, NUMACTS };

enum { GUN_ELECTRIC = 0, GUN_PLASMA, GUN_SMAW, GUN_MINIGUN, GUN_SPOCKGUN, GUN_M32, GUN_FLAMETHROWER, GUN_UZI, GUN_FAMAS, GUN_MOSSBERG, GUN_HYDRA, GUN_SV98, GUN_SKS, GUN_CROSSBOW, GUN_AK47, GUN_GRAP1, GUN_FIREWORKS, GUN_MOLOTOV, GUN_GLOCK,
       GUN_S_NUKE, GUN_S_GAU8, GUN_S_ROCKETS, GUN_S_CAMPER,
       GUN_M_BUSTER, GUN_M_HAMMER, GUN_M_MASTER, GUN_M_FLAIL,
       GUN_KAMIKAZE, GUN_POWERARMOR, GUN_NINJA, NUMGUNS };

enum { ATK_ELECTRIC = 0, ATK_PLASMA, ATK_SMAW, ATK_MINIGUN, ATK_SPOCKGUN, ATK_M32, ATK_FLAMETHROWER, ATK_UZI, ATK_FAMAS, ATK_MOSSBERG, ATK_HYDRA, ATK_SV98,ATK_SKS, ATK_CROSSBOW, ATK_AK47, ATK_GRAP1, ATK_FIREWORKS, ATK_MOLOTOV, ATK_GLOCK,
       ATK_S_NUKE, ATK_S_GAU8, ATK_S_ROCKETS, ATK_S_CAMPER,
       ATK_M_BUSTER, ATK_M_HAMMER, ATK_M_MASTER, ATK_M_FLAIL,
       ATK_KAMIKAZE, ATK_POWERARMOR, ATK_NINJA, ATK_NONE, NUMATKS };

enum { A_WOOD = 0, A_IRON, A_GOLD, A_MAGNET, A_POWERARMOR, NUMSHIELDS };
enum { B_ROIDS = 0, B_SHROOMS, B_EPO, B_JOINT, B_RAGE, NUMBOOSTS };
enum { ABILITY_1 = 0, ABILITY_2, ABILITY_3, NUMABILITIES };
enum { M_NONE = 0, M_SEARCH, M_AGGRO, M_RETREAT, M_ATTACKING, M_PAIN, M_SLEEP, M_AIMING, M_FRIENDLY, M_NEUTRAL, M_ANGRY, M_MAX};  // monster states

#define validgun(n) ((n) >= 0 && (n) < NUMGUNS)
#define validshield(n) ((n) >= 0 && (n) < NUMSHIELDS)
#define validact(n) ((n) >= 0 && (n) < NUMACTS)
#define validatk(n) ((n) >= 0 && (n) < NUMATKS)
#define validAbility(n) ((n) >= 0 && (n) < NUMABILITIES)

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

static struct gamemodeinfo { const char *modeId, *mutatorId; int flags; } gamemodes[] =
{   // mode -3, -2, -1, 0, 1
    { "Mode_Invasion",          "",                 M_DMSP | M_LOCAL },
    { "Mode_Tutorial",          "",                 M_TUTORIAL | M_LOCAL },
    { "Mode_Campaign",          "",                 M_CLASSICSP | M_LOCAL },
    { "Mode_Replay",            "",                 M_DEMO | M_LOCAL },
    { "Mode_MapEditor",         "",                 M_EDIT },
    // mode 2, 3, 4, 5
    { "Mode_Deathmatch",        "Mut_Pickup",       M_LOBBY },
    { "Mode_Deathmatch",        "Mut_Random",       M_RANDOM | M_NOAMMO | M_MUNINFINIE },
    { "Mode_Deathmatch",        "Mut_FullStuff",    M_FULLSTUFF },
    { "Mode_Deathmatch",        "Mut_Identical",    M_IDENTIQUE | M_NOAMMO | M_MUNINFINIE },
    // mode 6, 7, 8, 9
    { "Mode_TeamDeathmatch",    "Mut_Pickup",       M_TEAM },
    { "Mode_TeamDeathmatch",    "Mut_Random",       M_RANDOM | M_TEAM | M_NOAMMO | M_MUNINFINIE },
    { "Mode_TeamDeathmatch",    "Mut_FullStuff",    M_FULLSTUFF | M_TEAM },
    { "Mode_TeamDeathmatch",    "Mut_Identical",    M_IDENTIQUE | M_TEAM | M_NOAMMO | M_MUNINFINIE },
    // mode 10, 11, 12, 13
    { "Mode_CaptureTheFlag",    "Mut_Pickup",       M_CTF | M_TEAM },
    { "Mode_CaptureTheFlag",    "Mut_Random",       M_RANDOM | M_CTF | M_TEAM | M_NOAMMO | M_MUNINFINIE },
    { "Mode_CaptureTheFlag",    "Mut_FullStuff",    M_FULLSTUFF | M_CTF | M_TEAM },
    { "Mode_CaptureTheFlag",    "Mut_Identical",    M_IDENTIQUE | M_CTF | M_TEAM | M_NOAMMO | M_MUNINFINIE },
    // mode 14, 15, 16, 17, 18
    { "Mode_Domination",        "Mut_Pickup",       M_CAPTURE | M_TEAM },
    { "Mode_Domination",        "Mut_Random",       M_RANDOM | M_CAPTURE | M_TEAM | M_NOAMMO | M_MUNINFINIE },
    { "Mode_Domination",        "Mut_FullStuff",    M_FULLSTUFF | M_CAPTURE | M_TEAM },
    { "Mode_Domination",        "Mut_Identical",    M_IDENTIQUE | M_CAPTURE | M_TEAM | M_NOAMMO | M_MUNINFINIE },
    { "Mode_Domination",        "Mut_Regen",        M_NOITEMS | M_CAPTURE | M_TEAM | M_REGEN },
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

#define NUMMMNAMES 6
static const char * const mastermodecolors[] = { "",       "\f0",    "\f2",        "\f2",        "\f3",        "\f3" };
static const char * const mastermodeicons[] =  { "server", "server", "serverlock", "serverlock", "serverpriv", "serverpriv" };

// network messages codes, c2s, c2c, s2c
enum { PRIV_NONE = 0, PRIV_MASTER, PRIV_AUTH, PRIV_ADMIN };

enum
{
    N_CONNECT = 0, N_SERVINFO, N_WELCOME, N_INITCLIENT, N_POS, N_TEXT, N_SOUND, N_CDIS,
    N_SHOOT, N_EXPLODE, N_SUICIDE,
    N_DIED, N_DAMAGE, N_VAMPIRE, N_REAPER, N_VIKING, N_PRIEST, N_AFTERBURN,
    N_LAVATOUCH, N_LAVATOUCHFX, N_WATERTOUCH, N_FIRETOUCH,
    N_HITPUSH, N_SHOTFX, N_EXPLODEFX,
    N_TRYSPAWN, N_SPAWNSTATE, N_SPAWN, N_FORCEDEATH,
    N_GUNSELECT, N_TAUNT,
    N_MAPCHANGE, N_MAPVOTE, N_TEAMINFO, N_ITEMSPAWN, N_ITEMPICKUP, N_ITEMACC, N_TELEPORT, N_JUMPPAD,
    N_PING, N_PONG, N_CLIENTPING,
    N_TIMEUP, N_FORCEINTERMISSION,
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
    N_SENDSKIN, N_SENDCLASS, N_REGENALLIES,
    N_REQABILITY, N_GETABILITY,
    N_ANNOUNCE,
    N_CURWEAPON,
    N_BASES, N_BASEINFO, N_BASESCORE, N_SCOREBASE, N_REPAMMO, N_BASEREGEN,
    NUMMSG
};

static const int msgsizes[] =               // size inclusive message token, 0 for variable or not-checked sizes
{
    N_CONNECT, 0, N_SERVINFO, 0, N_WELCOME, 1, N_INITCLIENT, 0, N_POS, 0, N_TEXT, 0, N_SOUND, 2, N_CDIS, 2,
    N_SHOOT, 0, N_EXPLODE, 0, N_SUICIDE, 1,
    N_DIED, 7, N_DAMAGE, 8, N_VAMPIRE, 4, N_REAPER, 4, N_VIKING, 3, N_PRIEST, 3, N_AFTERBURN, 3,
    N_LAVATOUCH, 1, N_LAVATOUCHFX, 2, N_WATERTOUCH, 2, N_FIRETOUCH, 1,
    N_HITPUSH, 7, N_SHOTFX, 10, N_EXPLODEFX, 4,
    N_TRYSPAWN, 1, N_SPAWNSTATE, 39, N_SPAWN, 3, N_FORCEDEATH, 2,
    N_GUNSELECT, 2, N_TAUNT, 1,
    N_MAPCHANGE, 0, N_MAPVOTE, 0, N_TEAMINFO, 0, N_ITEMSPAWN, 2, N_ITEMPICKUP, 2, N_ITEMACC, 4,
    N_PING, 2, N_PONG, 2, N_CLIENTPING, 2,
    N_TIMEUP, 2, N_FORCEINTERMISSION, 1,
    N_SERVMSG, 0, N_ITEMLIST, 0, N_RESUME, 0,
    N_EDITMODE, 2, N_EDITENT, 15, N_EDITF, 16, N_EDITT, 16, N_EDITM, 16, N_FLIP, 14, N_COPY, 14, N_PASTE, 14, N_ROTATE, 15, N_REPLACE, 17, N_DELCUBE, 14, N_CALCLIGHT, 1, N_REMIP, 1, N_EDITVSLOT, 16, N_UNDO, 0, N_REDO, 0, N_NEWMAP, 2, N_GETMAP, 1, N_SENDMAP, 0, N_EDITVAR, 0,
    N_MASTERMODE, 2, N_KICK, 0, N_CLEARBANS, 1, N_CURRENTMASTER, 0, N_SPECTATOR, 3, N_SETMASTER, 0, N_SETTEAM, 0,
    N_LISTDEMOS, 1, N_SENDDEMOLIST, 0, N_GETDEMO, 3, N_SENDDEMO, 0,
    N_DEMOPLAYBACK, 3, N_RECORDDEMO, 2, N_STOPDEMO, 1, N_CLEARDEMOS, 2,
    N_TAKEFLAG, 3, N_RETURNFLAG, 4, N_RESETFLAG, 3, N_TRYDROPFLAG, 1, N_DROPFLAG, 7, N_SCOREFLAG, 9, N_INITFLAGS, 0,
    N_SAYTEAM, 0,
    N_CLIENT, 0,
    N_AUTHTRY, 0, N_AUTHKICK, 0, N_AUTHCHAL, 0, N_AUTHANS, 0, N_REQAUTH, 0,
    N_PAUSEGAME, 0, N_GAMESPEED, 0,
    N_ADDBOT, 3, N_DELBOT, 1, N_INITAI, 0, N_FROMAI, 2, N_BOTLIMIT, 2, N_BOTBALANCE, 2,
    N_MAPCRC, 0, N_CHECKMAPS, 1,
    N_SWITCHNAME, 0, N_SWITCHMODEL, 2, N_SWITCHCOLOR, 2, N_SWITCHTEAM, 2,
    N_SERVCMD, 0,
    N_DEMOPACKET, 0,
    N_SENDSKIN, 3, N_SENDCLASS, 2, N_REGENALLIES, 5,
    N_REQABILITY, 2, N_GETABILITY, 4,
    N_ANNOUNCE, 3,
    N_CURWEAPON, 3,
    N_BASES, 0, N_BASEINFO, 0, N_BASESCORE, 0, N_SCOREBASE, 3, N_REPAMMO, 1, N_BASEREGEN, 7,
    -1
};

#define CC_SERVER_PORT 43000
#define CC_LANINFO_PORT 42998
#define CC_MASTER_PORT 42999
#define PROTOCOL_VERSION 980         // bump when protocol changes
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

static struct itemstat { int add, max, sound; const char *ident; int info; } itemstats[] =
{   // weapons
    {15,    60,    S_ITEMAMMO,  "Weapon_ElectricRifle",     GUN_ELECTRIC},
    {32,   128,    S_ITEMAMMO,  "Weapon_PlasmaRifle",       GUN_PLASMA},
    {6,     24,    S_ITEMAMMO,  "Weapon_Smaw",              GUN_SMAW},
    {80,   320,    S_ITEMAMMO,  "Weapon_Minigun",           GUN_MINIGUN},
    {20,    80,    S_ITEMAMMO,  "Weapon_Spockgun",          GUN_SPOCKGUN},
    {7,     28,    S_ITEMAMMO,  "Weapon_M32",               GUN_M32},
    {50,   200,    S_ITEMAMMO,  "Weapon_Flamethrower",      GUN_FLAMETHROWER},
    {50,   200,    S_ITEMAMMO,  "Weapon_Uzi",               GUN_UZI},
    {60,   240,    S_ITEMAMMO,  "Weapon_Famas",             GUN_FAMAS},
    {10,    40,    S_ITEMAMMO,  "Weapon_Mossberg500",       GUN_MOSSBERG},
    {15,    60,    S_ITEMAMMO,  "Weapon_Hydra",             GUN_HYDRA},
    {8,     32,    S_ITEMAMMO,  "Weapon_Sv98",              GUN_SV98},
    {14,    56,    S_ITEMAMMO,  "Weapon_Sks",               GUN_SKS},
    {12,    48,    S_ITEMAMMO,  "Weapon_Crossbow",          GUN_CROSSBOW},
    {40,   160,    S_ITEMAMMO,  "Weapon_Ak47",              GUN_AK47},
    {70,   280,    S_ITEMAMMO,  "Weapon_Gapb1",             GUN_GRAP1},
    {10,    40,    S_ITEMAMMO,  "Weapon_Fireworks",         GUN_FIREWORKS},
    {8,     32,    S_ITEMAMMO,  "Weapon_Molotov",           GUN_MOLOTOV},
    {30,   120,    S_ITEMAMMO,  "Weapon_Glock",             GUN_GLOCK},
    // superweapons
    {  1,    4,    S_ITEMSUPERAMMO, "BOMBE NUCLEAIRE",      GUN_S_NUKE},
    {300, 1200,    S_ITEMSUPERAMMO, "GAU-8",                GUN_S_GAU8},
    { 40,  120,    S_ITEMSUPERAMMO, "MINI-ROQUETTES",       GUN_S_ROCKETS},
    { 15,   60,    S_ITEMSUPERAMMO, "CAMPOUZE 2000",        GUN_S_CAMPER},
    // items
    {250,     1000, S_ITEMHEALTH,   "Item_Health",          0},
    {500,     2500, S_COCHON,       "Item_GrilledPig",      0},
    {30000,  45000, S_ITEMSTEROS,   "Item_Roids",           B_ROIDS},
    {40000, 120000, S_ITEMCHAMPIS,  "Item_Shrooms",         B_SHROOMS},
    {25000,  50000, S_ITEMEPO,      "Item_Epo",             B_EPO},
    {30000,  90000, S_ITEMJOINT,    "Item_Joint",           B_JOINT},
    {750,      750, S_ITEMBBOIS,    "Item_WoodShield",      A_WOOD},
    {1250,    1250, S_ITEMBFER,     "Item_IronShield",      A_IRON},
    {2000,    2000, S_ITEMBOR,      "Item_GoldShield",      A_GOLD},
    {1500,    1500, S_ITEMBMAGNET,  "Item_MagnetShield",    A_MAGNET},
    {3000,    3000, S_ITEMARMOUR,   "Item_PowerArmor",      A_POWERARMOR},
    {50,       150, S_ITEMMANA,     "Item_Mana",            0}
};


#define MAXRAYS 25
#define EXP_SELFDAMDIV 1
#define EXP_SELFPUSH 1.0f
#define EXP_DISTSCALE 0.5f

static const struct attackinfo { int gun, action, picksound, sound, middistsnd, fardistsnd, specialsounddelay, attackdelay, damage, aimspread, noaimspread, margin, projspeed, kickamount, range, rays, hitpush, exprad, ttl, use; } attacks[NUMATKS] =
{
    // Regular weapons
    { GUN_ELECTRIC,     ACT_SHOOT, S_WPLOADFUTUR,     S_ELECRIFLE,    S_ELECRIFLE_FAR,       S_FAR_LIGHT,   10,  350,  325,  35, 105, 0,    0,  10, 8000,  1,    30,   0, 0, 1},
    { GUN_PLASMA,       ACT_SHOOT, S_WPLOADFUTUR,     S_PLASMARIFLE,  S_PLASMARIFLE_FAR,     S_FAR_LIGHT,   25,   90,  180,  45, 135, 0, 2000,   5, 8000,  1,    50,  25, 0, 1},
    { GUN_SMAW,         ACT_SHOOT, S_WPLOADBIG,       S_SMAW,         S_SMAW_FAR,                     -1,    3, 1250, 1150,  20,  60, 2,  700,  15, 8000,  1,   750, 160, 0, 1},
    { GUN_MINIGUN,      ACT_SHOOT, S_WPLOADMID,       S_MINIGUN,      S_MINIGUN_FAR,         S_FAR_LIGHT,   35,   60,  180,  60, 180, 0, 4250,   5, 8000,  1,    15 ,  7, 0, 1},
    { GUN_SPOCKGUN,     ACT_SHOOT, S_WPLOADALIEN,     S_SPOCKGUN,     S_SPOCKGUN_FAR,        S_FAR_LIGHT,   15,  175,  250,  15, 150, 3, 2250,   5, 8000,  1,    30,  15, 0, 1},
    { GUN_M32,          ACT_SHOOT, S_WPLOADMID,       S_M32,          S_M32_FAR,                      -1,    3, 1000, 1250,  20,  50, 0,  400,  10, 1000,  1,   600, 185, 1000, 1},
    { GUN_FLAMETHROWER, ACT_SHOOT, S_WPLOADMID,       S_FLAMETHROWER, S_FLAMETHROWER_FAR,             -1,   30,  100,   30, 500, 500, 9,    0,   2,  280, 10,    10 ,  0, 0, 1},
    { GUN_UZI,          ACT_SHOOT, S_WPLOADSMALL,     S_UZI,          S_UZI_FAR,             S_FAR_LIGHT,   35,   75,  150,  50, 150, 0, 4250,   2, 8000,  1,    10,   5, 0, 1},
    { GUN_FAMAS,        ACT_SHOOT, S_WPLOADSMALL,     S_FAMAS,        S_FAMAS_FAR,           S_FAR_LIGHT,   30,   90,  140,  40, 120, 0, 4250,   3, 8000,  1,    20,   5, 0, 1},
    { GUN_MOSSBERG,     ACT_SHOOT, S_WPLOADMID,       S_MOSSBERG,     S_MOSSBERG_FAR,    S_FAR_VERYHEAVY,    3, 1200,  115, 500, 500, 0,    0,  20, 1000, 25,    20,   0, 0, 1},
    { GUN_HYDRA,        ACT_SHOOT, S_WPLOADSMALL,     S_HYDRA,        S_HYDRA_FAR,       S_FAR_VERYHEAVY,    4,  315,   75, 300, 300, 0,    0,  15,  600, 15,    20,   0, 0, 1},
    { GUN_SV98,         ACT_SHOOT, S_WPLOADMID,       S_SV98,         S_SV98_FAR,            S_FAR_HEAVY,    2, 1500, 1000,   1, 200, 0, 5250,  30, 8000,  1,    80,   7, 0, 1},
    { GUN_SKS,          ACT_SHOOT, S_WPLOADMID,       S_SKS,          S_SKS_FAR,             S_FAR_HEAVY,   10,  420,  500,   5, 125, 0, 4250,  25, 8000,  1,    50,   7, 0, 1},
    { GUN_CROSSBOW,     ACT_SHOOT, S_WPLOADMID,       S_CROSSBOW,     S_CROSSBOW_FAR,                 -1,    5,  800,  850,  10,  90, 0, 3000,   7, 8000,  1,    20,   3, 45000, 1},
    { GUN_AK47,         ACT_SHOOT, S_WPLOADMID,       S_AK47,         S_AK47_FAR,            S_FAR_LIGHT,   30,   92,  170,  60, 180, 0, 4250,   7, 8000,  1,    50,   5, 0, 1},
    { GUN_GRAP1,        ACT_SHOOT, S_WPLOADFUTUR,     S_GRAP1,        S_GRAP1_FAR,                    -1,   12,  200,  250,  30, 300, 3, 1750,  -4, 8000,  1,  -600,  20, 0, 1},
    { GUN_FIREWORKS,    ACT_SHOOT, S_WPLOADSMALL,     S_FIREWORKS,    S_FIREWORKS_FAR,                -1,    3, 1100,  900,  35, 200, 2, 1500,  35,  600,  1,   500,  80, 300, 1},
    { GUN_MOLOTOV,      ACT_SHOOT, S_WPLOADSLOWWOOSH, S_MOLOTOV,      -1,                             -1,    3, 1350,  500,  20,  50, 0,  300, -10, 1500,  1,   100, 250, 10000, 1},
    { GUN_GLOCK,        ACT_SHOOT, S_WPLOADSMALL,     S_GLOCK,        S_GLOCK_FAR,           S_FAR_LIGHT,   10,  150,  280,  25, 150, 0, 4250,   7, 8000,  1,    30,   3, 0, 1},
    // Super weapons
    { GUN_S_NUKE,       ACT_SHOOT, S_WPLOADBIG,       S_NUKE,         S_NUKE_FAR,             S_NUKE_FAR,    1, 3000,  3250,  20, 300, 2,  200,  10, 2000,  1,   400, 1500, 6000, 1},
    { GUN_S_GAU8,       ACT_SHOOT, S_WPLOADBIG,       S_GAU8,         S_GAU8_FAR,                     -1,   90,   14,   300, 150, 250, 3, 7500,   4, 8000,  1,    80,   20, 0, 1},
    { GUN_S_ROCKETS,    ACT_SHOOT, S_WPLOADBIG,       S_MINIROCKETS,  S_MINIROCKETS_FAR, S_FAR_VERYHEAVY,   14,  170,  2000,  10, 300, 2,  850,   6, 8000,  1,   500,  100, 0, 1},
    { GUN_S_CAMPER,     ACT_SHOOT, S_WPLOADBIG,       S_CAMPOUZE,     S_CAMPOUZE_FAR,    S_FAR_VERYHEAVY,    8,  500,    75,  10,  50, 5,    0,   3, 4000, 10,   150,    8, 0, 1},
    // Melee weapons
    { GUN_M_BUSTER,     ACT_SHOOT, S_WPLOADWHOOSH,    S_SWORD349,     -1, -1,   4, 1000,  600, 1, 1, 20, 0, -10,  28,  1,  50,  0, 0, 0},
    { GUN_M_HAMMER,     ACT_SHOOT, S_WPLOADSLOWWOOSH, S_BANHAMMER,    -1, -1,   3, 1500, 1000, 1, 1, 15, 0,  -5,  30,  1,  10,  0, 0, 0},
    { GUN_M_MASTER,     ACT_SHOOT, S_WPLOADWHOOSH,    S_MASTERSWORD,  -1, -1,   5, 600,   430, 1, 1, 20, 0,  -8,  26,  1,  30,  0, 0, 0},
    { GUN_M_FLAIL,      ACT_SHOOT, S_WPLOADCHAINS,    S_FLAIL,        -1, -1,   4, 1150,  750, 1, 1, 10, 0, -10,  32,  1, 125,  0, 0, 0},
    // Special weapons
    { GUN_KAMIKAZE,     ACT_SHOOT, S_WPLOADFASTWOOSH, -1,           S_EXPL_FAR, S_EXPL_FAR,   1, 1000, 3000, 1, 1,  0, 1,  10, 120,  1, 250, 500, 5, 1},
    { GUN_POWERARMOR,    ACT_SHOOT, -1,                -1,           S_EXPL_FAR, S_EXPL_FAR,   1,  220, 2000, 1, 1,  0, 1,  10,  50,  1, 100, 350, 5, 1},
    { GUN_NINJA,     ACT_SHOOT, S_WPLOADWHOOSH,    S_NINJASABER, -1,         -1,           8,  400,  800, 1, 1, 30, 0, -10,  36,  1,  25,   0, 0, 0},
};

static const struct guninfo { const char *ident, *name; vec2 weapDisp; int maxzoomfov, hudrange, attacks[NUMACTS]; } guns[NUMGUNS] =
{
    // Regular weapons
    { "Weapon_ElectricRifle",   "fusilelectrique",  vec2(65, 26),  60, 1000,  { -1, ATK_ELECTRIC }, },
    { "Weapon_PlasmaRifle",     "fusilplasma",      vec2(28, 10),  60,  750,  { -1, ATK_PLASMA }, },
    { "Weapon_Smaw",            "smaw",             vec2( 8,  8),  85,  500,  { -1, ATK_SMAW }, },
    { "Weapon_Minigun",         "minigun",          vec2(36, 11),  80,  750,  { -1, ATK_MINIGUN }, },
    { "Weapon_Spockgun",        "spockgun",         vec2(52, 20),  70,  500,  { -1, ATK_SPOCKGUN }, },
    { "Weapon_M32",             "m32",              vec2(65, 21),  85,  600,  { -1, ATK_M32 }, },
    { "Weapon_Flamethrower",    "lanceflammes",     vec2(40, 15),  95,  280,  { -1, ATK_FLAMETHROWER }, },
    { "Weapon_Uzi",             "uzi",              vec2(23, 21),  80,  500,  { -1, ATK_UZI }, },
    { "Weapon_Famas",           "famas",            vec2(54, 14),  70,  750,  { -1, ATK_FAMAS }, },
    { "Weapon_Mossberg500",     "mossberg500",      vec2(38, 18),  95,  300,  { -1, ATK_MOSSBERG }, },
    { "Weapon_Hydra",           "hydra",            vec2(42, 20),  95,  300,  { -1, ATK_HYDRA }, },
    { "Weapon_Sv98",            "sv98",             vec2( 1,  3),  30, 2000,  { -1, ATK_SV98 }, },
    { "Weapon_Sks",             "sks",              vec2( 1,  3),  50, 1500,  { -1, ATK_SKS }, },
    { "Weapon_Crossbow",        "arbalete",         vec2( 1,  3),  45, 1000,  { -1, ATK_CROSSBOW }, },
    { "Weapon_Ak47",            "ak47",             vec2(46, 25),  70,  750,  { -1, ATK_AK47 }, },
    { "Weapon_Gapb1",           "gapb1",            vec2(43, 17),  85,  500,  { -1, ATK_GRAP1 }, },
    { "Weapon_Fireworks",       "feuartifice",      vec2(70, 30),  85,  500,  { -1, ATK_FIREWORKS }, },
    { "Weapon_Molotov",         "molotov",          vec2( 8,  8),  90,  500,  { -1, ATK_MOLOTOV }, },
    { "Weapon_Glock",           "glock",            vec2(55, 20),  85,  300,  { -1, ATK_GLOCK }, },
    // Super weapons
    { "Weapon_Nuke",            "missilenorko",     vec2( 8,  3),  85, 2000,  { -1, ATK_S_NUKE }, },
    { "Weapon_Gau8",            "gau8",             vec2(57, 10),  85, 2000,  { -1, ATK_S_GAU8 }, },
    { "Weapon_Minirockets",     "miniroquettes",    vec2(10, 10),  70, 1000,  { -1, ATK_S_ROCKETS }, },
    { "Weapon_Camper2000",      "campouze2000",     vec2(10, 10),  60, 3000,  { -1, ATK_S_CAMPER }, },
    // Melee weapons
    { "Weapon_BusterSword",     "epee349",          vec2(4, 3),    95,  120,  { -1, ATK_M_BUSTER }, },
    { "Weapon_BanHammer",       "marteauban",       vec2(4, 3),    95,  120,  { -1, ATK_M_HAMMER }, },
    { "Weapon_MasterSword",     "mastersword",      vec2(4, 3),    95,  120,  { -1, ATK_M_MASTER }, },
    { "Weapon_Flail",           "fleau",            vec2(4, 3),    95,  120,  { -1, ATK_M_FLAIL }, },
    // Special weapons
    { "Weapon_Explosives",      "kamikaze",         vec2(4, 3),    95,    0,  { -1, ATK_KAMIKAZE }, },
    { "Weapon_PowerArmor",      "assistxpl",        vec2(4, 3),    95,    0,  { -1, ATK_POWERARMOR }, },
    { "Weapon_Saber",           "sabre",            vec2(4, 3),    95,  120,  { -1, ATK_NINJA }, },
};

// Definition of player's classes
enum {C_SOLDIER = 0, C_MEDIC, C_AMERICAN, C_NINJA, C_VAMPIRE, C_WIZARD, C_KAMIKAZE, C_REAPER, C_PHYSICIST, C_CAMPER, C_SPY, C_PRIEST, C_VIKING, C_JUNKIE, C_SHOSHONE, NUMCLASSES};
#define validClass(n) ((n) >= 0 && (n) < NUMCLASSES)

struct ability { const int manacost, duration, cooldown, snd; };
static const struct classesConfig { const char *hatDir; int damage, resistance, accuracy, speed; ability abilities[3]; } classes[NUMCLASSES] =
{                        // classe stats         // ability 1                 // ability 2                // ability 3
    { "hats/soldier",    105,  105,  105,   105, { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },
    { "hats/medic",       90,  115,   95,   105, { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },
    { "hats/american",   100,  135,   80,    85, { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },
    { "hats/ninja",       85,   90,   75,   125, { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },
    { "hats/vampire",    110,   65,  110,   110, { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },
    { "hats/wizard",     100,   85,   90,   100, { {30,  250, 2000, S_WIZ_1}, {40,  4000, 5000, S_WIZ_2}, {60, 3000,  6000, S_WIZ_3} } },
    { "hats/kamikaze",   100,  100,   70,   115, { {0,     0,    0,      -1}, {100, 3000, 3000, S_TIMER}, {0,     0,     0,      -1} } },
    { "hats/reaper",     120,   85,   90,    95, { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },
    { "hats/physicist",   90,   85,   85,    95, { {40, 4000, 3000, S_PHY_1}, {50,  5000, 7000, S_PHY_2}, {65, 6000,  9000, S_PHY_3} } },
    { "hats/camper",     100,   60,  135,    80, { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },
    { "hats/spy",         90,   85,  120,    90, { {40, 4000, 7000, S_SPY_1}, {50,  7000, 7000, S_SPY_2}, {60, 5000, 10000, S_SPY_3} } },
    { "hats/priest",      85,  105,   85,   105, { {30, 4000, 8000, S_PRI_1}, {10,  8000, 8000, S_PRI_2}, {80, 4000, 10000, S_PRI_3} } },
    { "hats/viking",     100,  120,   60,    95, { {0,     0,    0,      -1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },
    { "hats/junkie",     100,  110,   85,    90, { {0,     0,    0,     - 1}, {0,      0,    0,      -1}, {0,     0,     0,      -1} } },
    { "hats/shoshone",   100,  100,   75,   100, { {50, 7500, 7500, S_SHO_1}, {50,  7500, 7500, S_SHO_2}, {50, 7500,  7500, S_SHO_3} } }
};

static const struct armourInfo { const char *name; int absorb, max; } armours[NUMSHIELDS] =
{
    { "wood",    25,  750 },
    { "iron",    50, 1250 },
    { "gold",    75, 2000 },
    { "magnet", 100, 1500 },
    { "power",   85, 3000 }
};

#include "ai.h"

extern int currentIdenticalWeapon;

// inherited by gameent and server clients
struct gamestate
{
    int seed;
    int health, maxhealth, mana;
    int armour, armourtype;
    int boostmillis[NUMBOOSTS], abilitymillis[NUMABILITIES], vampiremillis;
    bool abilityready[NUMABILITIES];
    int afterburnmillis, afterburnatk;
    int gunselect, gunwait;
    int ammo[NUMGUNS];
    bool aiming;
    int aitype, skill;

    gamestate() : maxhealth(1000), aitype(AI_NONE), skill(0) {}

    void baseammo(int gun, int k = 1)
    {
        ammo[gun] = (itemstats[gun-GUN_ELECTRIC].add*k);
    }

    void addammo(int gun, int k = 1, int scale = 1)
    {
        itemstat &is = itemstats[gun-GUN_ELECTRIC];
        ammo[gun] = min(ammo[gun] + (is.add*k)/scale, is.max);
    }

    bool hasmaxammo(int type)
    {
       const itemstat &is = itemstats[type-I_RAIL];
       return ammo[type-I_RAIL+GUN_ELECTRIC]>=is.max;
    }

    bool canpickupitem(int type, int playerClass, bool hasPowerArmor)
    {
        if(type<I_RAIL || type>I_MANA) return false;
        itemstat &is = itemstats[type-I_RAIL];

        switch(type)
        {
            case I_SANTE: return health < maxhealth;

            case I_BOOSTPV: return maxhealth < is.max;

            case I_MANA:
                if(playerClass==C_VAMPIRE) return health < maxhealth;
                else return (playerClass==C_WIZARD || playerClass==C_PHYSICIST || playerClass==C_PRIEST || playerClass==C_SPY || playerClass==C_SHOSHONE) && mana<is.max;

            case I_ROIDS: case I_EPO: case I_JOINT: case I_SHROOMS:
                return boostmillis[type-I_ROIDS] < is.max;

            case I_WOODSHIELD:
            case I_IRONSHIELD:
            case I_GOLDSHIELD:
            case I_MAGNETSHIELD:
                return hasPowerArmor ? (armour < 3000) : (armour < (playerClass==C_SOLDIER ? is.max+(250*(armourtype+1)) : is.max));

            case I_POWERARMOR: return !hasPowerArmor;

            default:
                return ammo[is.info] < is.max*(playerClass==C_AMERICAN ? 1.5f : 1);
        }
    }

    void pickupitem(int type, int playerClass, int aptisort, bool haspowerarmor, int rndsweap)
    {
        if(type<I_RAIL || type>I_MANA) return;
        itemstat &is = itemstats[type-I_RAIL+rndsweap];

        int itemboost = playerClass==C_PRIEST && aptisort ? 2 : 1;

        switch(type)
        {
            case I_BOOSTPV:
                health = min(health+is.add*(playerClass==C_JUNKIE ? 1.5f : itemboost), 2500.0f);
                return;

            case I_SANTE:
                health = min(health+is.add*(playerClass==C_MEDIC ? 2 : itemboost), maxhealth);
                return;

            case I_MANA:
                if(playerClass!=C_VAMPIRE) mana = min(mana+is.add, is.max);
                else health = min(health+250, maxhealth);
                return;

            case I_WOODSHIELD: case I_IRONSHIELD: case I_GOLDSHIELD: case I_MAGNETSHIELD: case I_POWERARMOR:
                if(type==I_POWERARMOR)
                {
                    armourtype = A_POWERARMOR;
                    armour = min(armour+is.add, is.max);
                    health = min(health+300, maxhealth);
                    if(!ammo[GUN_POWERARMOR]) ammo[GUN_POWERARMOR] = 1;
                    return;
                }
                else
                {
                    armourtype = haspowerarmor ? A_POWERARMOR : is.info;
                    if(!haspowerarmor) ammo[GUN_POWERARMOR] = 0;
                    int armourval = haspowerarmor && armour ? (500 * itemboost) : (playerClass==C_SOLDIER ? is.max+(250*(armourtype+1)) : is.max);
                    armour = min(armour + armourval, haspowerarmor ? 3000 : armourval);
                    return;
                }

            case I_ROIDS: case I_EPO: case I_JOINT: case I_SHROOMS:
            {
                float boostboost = playerClass==C_JUNKIE ? 1.5f : itemboost; // cannot find a better var name :)
                boostmillis[type-I_ROIDS] = min(boostmillis[type-I_ROIDS]+is.add*boostboost, is.max*boostboost);
                return;
            }

            default:
                float ammoboost = ((playerClass == C_AMERICAN) ? (type == I_SUPERARME && rndsweap == 0 ? 2 : 1.5f) : 1);
                ammo[is.info] = min(ammo[is.info]+is.add*itemboost*ammoboost, is.max*ammoboost);
        }
    }

    void respawn()
    {
        health = 1000;
        maxhealth = 1000;
        mana = 100;
        loopi(NUMBOOSTS) boostmillis[i] = 0;
        loopi(NUMABILITIES)
        {
            abilitymillis[i] = 0;
            abilityready[i] = true;
        }
        vampiremillis = 0;
        afterburnmillis = 0;
        gunwait = 0;
        seed = rnd(4);
        loopi(NUMGUNS) ammo[i] = 0;
        aiming = false;
    }

    void addMeleeWeapons(int playerClass)
    {
        int weapon = (playerClass == C_NINJA ? GUN_NINJA : GUN_M_BUSTER + rnd(4));
        ammo[weapon] = 1;
        gunselect = weapon;
    }

    void addStarterWeapons(int playerClass, int glockAmmo, int grenadeAmmo)
    {
        ammo[GUN_GLOCK] = glockAmmo;
        ammo[GUN_M32] = (playerClass == C_AMERICAN ? grenadeAmmo * 3 : grenadeAmmo);
    }

    void addSuperWeapon(int playerClass, int gamemode)
    {
        if(playerClass != C_SOLDIER || gamemode == m_tutorial) return;

        int weapon = GUN_S_NUKE + rnd(4);
        if(!rnd(50))
        {
            baseammo(weapon);
            gunselect = weapon;
        }
    }

    void addArmour(int playerClass, int gamemode)
    {
        bool armourBonus = (playerClass == C_SOLDIER);

        if(m_fullstuff)
        {
            armourtype = A_IRON;
            armour = armourBonus ? 1750 : 1250;
            return;
        }
        else if(m_regencapture)
        {
            armourtype = A_WOOD;
            armour = armourBonus ? 550 : 300;
            return;
        }
        else if(m_tutorial) armour = false;
        else
        {
            armourtype = A_WOOD;
            armour = armourBonus ? 1000 : 750;
        }
    }

    void selectGun(int playerClass, int baseWeapon)
    {
        switch(playerClass)
        {
            case C_KAMIKAZE: gunselect = GUN_KAMIKAZE; break;
            case C_NINJA: gunselect = GUN_NINJA; break;
            default: gunselect = baseWeapon;
        }
    }

    void spawnstate(int gamemode, int playerClass)
    {
        int selectedWeapon = GUN_GLOCK;

        if(m_random) // random weapon mutator
        {
            int ranndomWeapon = rnd(NUMMAINGUNS);
            ammo[ranndomWeapon] = 1;
            selectedWeapon = ranndomWeapon;
        }
        else if(m_fullstuff) // multiple weapons mutator
        {
            int numGuns[3];
            loopi(3)
            {
                int newGun;
                bool duplicate;
                do { newGun = rnd(NUMMAINGUNS); duplicate = false;
                    loopj(i) if (numGuns[j] == newGun) {duplicate = true; break; }
                } while (duplicate);
                numGuns[i] = newGun;
                baseammo(numGuns[i], playerClass == C_AMERICAN ? 6 : 4);
            }
            selectedWeapon = numGuns[rnd(3)];
        }
        else if(m_identique) // identical weapon mutator
        {
            ammo[currentIdenticalWeapon] = 1;
            selectedWeapon = currentIdenticalWeapon;
        }
        else if(m_capture) // base capture
        {
            addStarterWeapons(playerClass, m_regencapture ? 10 : 30, 1);
        }
        else if(m_tutorial || m_edit || m_dmsp) // solo game modes
        {
            if(m_dmsp) addStarterWeapons(playerClass, 10, 0);
        }
        else addStarterWeapons(playerClass, 30, 1); // weapon pickup mutator

        if(playerClass == C_KAMIKAZE) ammo[GUN_KAMIKAZE] = 1;
        addArmour(playerClass, gamemode);
        addMeleeWeapons(playerClass);
        selectGun(playerClass, selectedWeapon);
        addSuperWeapon(playerClass, gamemode);
    }

    int dodamage(int damage, bool shieldRegen = false) // just subtract damage here, can set death, etc. later in code calling this
    {
        int ad = damage * (armours[A_WOOD + armourtype].absorb) / 100.f; // let armour absorb when possible

        if(damage > 0)
        {
            if(ad > armour) ad = armour;
            if(shieldRegen && armour) armour = min(armour+ad, armours[A_WOOD + armourtype].max);
            else armour -= ad;
        }

        damage -= ad;
        health -= damage;
        return damage;
    }

    int doregen(int damage)
    {
        if(health < maxhealth)
        {
            health = min(health + damage, maxhealth);
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
static const char * const teamnames[1+MAXTEAMS] = { "", "1", "2" };
static const char * const teamtextcode[1+MAXTEAMS] = { "\fc", "\fd", "\fc" };
static const int teamtextcolor[1+MAXTEAMS] = { 0xFF2222, 0xFFFF22, 0xFF2222 };
static const char * const teamblipcolor[1+MAXTEAMS] = { "_neutral", "_blue", "_red" };
static inline int teamnumber(const char *name) { loopi(MAXTEAMS) if(!strcmp(teamnames[1+i], name)) return 1+i; return 0; }
#define validteam(n) ((n) >= 1 && (n) <= MAXTEAMS)
#define teamname(n) (teamnames[validteam(n) ? (n) : 0])

struct gameent : dynent, gamestate
{
    size_t entityId;
    int weight;                         // affects the effectiveness of hitpush
    int clientnum, privilege, lastupdate, plag, ping;
    int lifesequence;                   // sequence id for each respawn, used in damage test
    int respawned, lastspawn, suicided;
    int lastpain;
    int lastaction, lastattack, lastgunselect, lastshieldswitch, lastshrooms;
    int curdamage, lastcurdamage, curdamagecolor;
    int attacking, gunaccel;
    int lastfootstep;
    int lasttaunt, lastlavatouch;
    int lastpickup, lastpickupmillis, flagpickup, lastbase, lastrepammo, lastweap;
    int killstreak, frags, flags, deaths, totaldamage, totalshots;
    editinfo *edit;
    float deltayaw, deltapitch, deltaroll, newyaw, newpitch, newroll;
    int smoothmillis;

    int lastability[3], lastabilityrequest;
    int attacksound; // 0 = no sound, 1 = close sound, 2 = close + far sound
    bool shieldbroken, powerarmorexploded, powerarmorsound;
    int lastOutOfMap;
    bool wasAttacking, isOutOfMap;
    bool isConnected;

    string name, info;
    int team, playermodel, playercolor, skin[NUMSKINS], character, level;
    float skeletonSize, graveSize;
    ai::aiinfo *ai;
    int ownernum, lastnode;
    gameent *lastkiller;

    vec muzzle, weed, balles;

    gameent() : entityId(entitiesIds::getNewId()), weight(100), clientnum(-1), privilege(PRIV_NONE), lastupdate(0), plag(0), ping(0),
                lifesequence(0), respawned(-1), suicided(-1), lastpain(0), lastfootstep(0), killstreak(0), frags(0), flags(0), deaths(0),
                totaldamage(0), totalshots(0), edit(NULL), smoothmillis(-1), team(0), playermodel(-1), playercolor(0),
                skin{0, 0, 0}, character(0), level(0), ai(NULL), ownernum(-1), muzzle(-1, -1, -1), weed(-1, -1, -1), balles(-1, -1, -1)
    {
        loopi(3) lastability[i] = -1;
        name[0] = info[0] = 0;
        respawn();
    }
    ~gameent()
    {
        freeeditinfo(edit);
        freeeditinfo(edit);

        if(ai) delete ai;
    }

    void hitphyspush(int damage, const vec &dir, gameent *actor, int atk, gameent *target)
    {
        if(target->character == C_AMERICAN) return;
        vec push(dir);
        push.mul((actor==this && attacks[atk].exprad ? EXP_SELFPUSH : 1.0f)*attacks[atk].hitpush*(damage/10)/weight);
        vel.add(push);
    }

    void respawn()
    {
        dynent::reset();
        gamestate::respawn();
        respawned = suicided = -1;
        lastaction = 0;
        lastattack = -1;
        lastspawn = lastmillis;
        lastgunselect = lastmillis;
        lastshieldswitch = lastmillis;
        curdamage = 0;
        lastcurdamage = 0;
        attacking = ACT_IDLE;
        lasttaunt = 0;
        lastlavatouch = 0;
        lastpickup = -1;
        lastpickupmillis = 0;
        flagpickup = 0;
        lastbase = lastrepammo = -1;
        lastnode = -1;
        loopi(3) abilityready[i] = true;
        gunaccel = 0;
        killstreak = 0;
        attacksound = 0;
        powerarmorexploded = false;
        powerarmorsound = false;
        shieldbroken = false;
    }

    int respawnwait(int secs, int delay = 0)
    {
        return max(0, secs - (::lastmillis - lastpain - delay)/1000);
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

extern void playSound(int soundIndex, vec soundPos = vec(0, 0, 0), float maxRadius = 300.f, float maxVolRadius = 10.f, int flags = NULL, size_t entityId = SIZE_MAX, int soundType = 0, float pitch = 0);

extern bool getEntMovement(size_t entityId, vec& pos, vec& vel);
extern void updateEntPos(size_t entityId, const vec& newPos, vec fixedVel = vec(0, 0, 0));
extern void removeEntityPos(size_t entityId);
extern void clearEntsPos();

namespace entities
{
    extern vector<extentity *> ents;

    extern const char *entmdlname(int type);

    extern void preloadentities();
    extern void renderentities();
    extern void resettriggers();
    extern void checktriggers();
    extern void checkitems(gameent *d);

    extern void resetspawns();
    extern void spawnitems(bool force = false);
    extern void putitems(packetbuf &p);
    extern void setspawn(int i, bool on);
    extern void teleport(int n, gameent *d);
    extern void pickupeffects(int n, gameent *d, int rndsweap);
    extern void teleporteffects(gameent *d, int tp, int td, bool local = true);
    extern void jumppadeffects(gameent *d, int jp, bool local = true);

    extern void repammo(gameent *d, int type, bool local = true);
    extern bool validitem(int type);
}

namespace game
{
    //hud
    extern void drawrpgminimap(gameent *d, int w, int h);
    extern int getteamfrags(int team);
    extern string killerName;
    extern int killerWeapon, killerCharacter, killerLevel;
    extern float killerDistance;

    // abilities
    extern void requestAbility(gameent *d, int ability);
    extern void launchAbility(gameent *d, int ability, int millis);
    extern void updateAbilitiesSkills(int curtime, gameent *d);
    extern bool hasAbilities(gameent *d);
    extern bool hasAbilityEnabled(gameent *d, int numAbility);
    extern char *getdisguisement(int seed);

    // game
    extern int gamemode, canMove, gamespeed;

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
        virtual int respawnwait(gameent *d, int delay = 0) { return 0; }
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
    extern const char *teamcolorname(gameent *d, const char *alt);
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
    extern bool powerArmorExploding(gameent *d);
    extern bool kamikazeExploding(gameent *d);
    extern void deathstate(gameent *d, bool restore = false);
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
    struct monster;
    extern vector<monster *> monsters;
    extern void npcdrop(const vec *o, int type);
    extern void clearmonsters();
    extern void preloadmonsters();
    extern void stackmonster(monster *d, physent *o);
    extern void updatemonsters(int curtime);
    extern void rendermonsters();
    extern void suicidemonster(monster *m);
    extern void hitmonster(int damage, monster *m, gameent *at, const vec &vel, int atk);
    extern void monsterkilled(gameent *d);
    extern void endsp();
    extern void spsummary(int accuracy);

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

    // weapon
    struct hitmsg
    {
        int target, lifesequence, info1, info2;
        ivec dir;
    };

    extern vector<hitmsg> hits;

    static const int OFFSETMILLIS = 500;
    extern void checkInventoryGuns();
    typedef void (*inventoryCallback)(int gunId);
    extern void findSpecialWeapon(gameent *d, int baseWeapon, int maxWeapons, inventoryCallback callback);
    extern int getweapon(const char *name);
    extern void updateAttacks(gameent *d, const vec &targ, bool isMonster = false);
    extern void doaction(int act);
    extern void shoteffects(int atk, const vec &from, const vec &to, gameent *d, bool local, int id, int prevaction, bool isMonster = false);
    extern void explode(bool local, gameent *owner, const vec &v, const vec &vel, dynent *safe, int dam, int atk);
    extern void explodeeffects(int atk, gameent *d, bool local, int id = 0);
    extern void damageeffect(int damage, gameent *d, gameent *actor, int atk = 0);
    extern void gibeffect(int damage, const vec &vel, gameent *d);
    extern void hit(int damage, dynent *d, gameent *at, const vec &vel, int atk, float info1, int info2 = 1);
    extern float intersectdist;
    extern bool intersect(dynent *d, const vec &from, const vec &to, float margin = 0, float &dist = intersectdist);
    extern dynent *intersectclosest(const vec &from, const vec &to, gameent *at, float margin = 0, float &dist = intersectdist);
    extern void removeweapons(gameent *owner);
    extern void updateweapons(int curtime);
    extern void gunselect(int gun, gameent *d, bool force = false, bool shortcut = false);
    extern void weaponswitch(gameent *d);
    extern bool isAttacking(gameent *d);

    // scoreboard
    extern void showscores(bool on);
    extern void getbestplayers(vector<gameent *> &best);
    extern void getbestteams(vector<int> &best);
    extern vector<gameent *> bestplayers;
    extern vector<int> bestteams;
    extern void clearteaminfo();
    extern void setteaminfo(int team, int frags);
    extern void removegroupedplayer(gameent *d);

    // render
    extern bool hassuicided;
    inline bool hasShrooms() { return game::hudplayer()->boostmillis[B_SHROOMS]; }
    inline bool hasRoids(gameent *d) { return d->boostmillis[B_ROIDS]; }
    inline bool hasPowerArmor(gameent *d) { return d->armourtype==A_POWERARMOR && d->armour; }
    inline bool hasSuperWeapon(gameent *d) { return d->ammo[GUN_S_NUKE] || d->ammo[GUN_S_GAU8] || d->ammo[GUN_S_ROCKETS] || d->ammo[GUN_S_CAMPER]; }
    struct playermodelinfo { const char *model[MAXTEAMS], *cbmodel; };
    extern void saveGrave(gameent *d);
    extern void clearGraves();
    extern void moveGraves();
    extern const playermodelinfo &getplayermodelinfo(gameent *d);
    extern int getplayercolor(gameent *d, int team);
    enum {T_CLASSE = 0, T_PLAYERMODEL, T_CAPE, T_GRAVE, T_TAUNT};
    extern int chooserandomtraits(int seed, int trait);
    extern void syncplayer();
    extern void swayhudgun(int curtime);
    extern vec hudgunorigin(int gun, const vec &from, const vec &to, gameent *d);
    extern const char *getWeaponDir(int weapon, bool hud = false);
    extern void initWeaponsPaths();
    extern void initCapesPaths();
    extern void initGravesPaths();
    extern void initAssetsPaths();
    extern vec2 hudgunDisp;
}

namespace projectiles
{
    struct projectile
    {
        size_t entityId;
        vec dir, o, from, to, offset;
        float speed;
        gameent *owner;
        int atk;
        bool local;
        int offsetmillis;
        int id;
        int lifetime;
        bool exploded;
        bool inwater;
        int projsound;
        bool soundplaying;

        projectile()
            : entityId(entitiesIds::getNewId()) // initialize the new entityId field here
        {}
    };

    extern vector<projectile> curProjectiles;

    extern void preload();
    extern void add(const vec &from, const vec &to, float speed, bool local, int id, gameent *owner, int atk);
    extern void stain(const projectile &p, const vec &pos, int atk);
    extern float distance(dynent *o, vec &dir, const vec &v, const vec &vel);
    extern void update(int curtime);
    extern void render(int curtime);
    extern void avoid(ai::avoidset &obstacles, float radius);
    extern void remove(gameent *owner);
    extern void clear();
}

enum {BNC_GRENADE = 0, BNC_MOLOTOV, BNC_PIXEL, BNC_BURNINGDEBRIS, BNC_ROCK, BNC_BIGROCK, BNC_CASING, BNC_BIGCASING, BNC_CARTRIDGE, BNC_SCRAP, BNC_GLASS, BNC_LIGHT, NUMBOUNCERS};

namespace bouncers
{
    extern int bouncersfade;

    struct bouncer : physent
    {
        size_t entityId;
        int lifetime, bounces, seed;
        float roll;
        bool local;
        gameent *owner;
        int bouncetype, variant, gun;
        vec offset;
        int offsetmillis;
        int id;
        bool inwater;
        vec particles;

        bouncer() : entityId(entitiesIds::getNewId()), bounces(0), roll(0), variant(0), particles(-1, -1, -1)
        {
            type = ENT_BOUNCE;
        }
    };

    extern vector<bouncer *> curBouncers;

    extern void initPaths();
    extern void preload();
    extern void add(const vec &from, const vec &to, bool local, int id, gameent *owner, int type, int lifetime, int speed, vec2 yawPitch = vec2(0, 0));
    extern void spawn(const vec &p, const vec &vel, gameent *d, int type, int speed = 0, int lifetime = rnd(bouncersfade) + rnd(5000), bool frommonster = false);
    extern void bounceEffect(physent *d, const vec &surface);
    extern void update(int curtime);
    extern void render();
    extern void remove(gameent *owner);
    extern void clear();
}

namespace server
{
    extern const char *modename(int n, const char *unknown = "unknown");
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
    extern bool canspawnitem(int type);
    extern bool pickup(int i, int sender);
    extern bool noInfiniteAmmo(int atk);
    extern int gamemillis;
}

extern bool rndevent(int probability, int probabilityReduce = 0);
extern void createdrop(const vec *o, int type);
extern void trydisconnect(bool local);

extern bool disabledClass[NUMCLASSES];

#endif

