#ifndef __GAME_H__
#define __GAME_H__

#include "cube.h"
#include "engine.h"

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
    "vwep idle", "vwep shoot", "vwep melee"
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
#define DMF 16.0f                // for world locations
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
    //ARMES
    I_RAIL, I_PULSE, I_SMAW, I_MINIGUN, I_SPOCKGUN, I_M32, I_LANCEFLAMMES, I_UZI, I_FAMAS, I_MOSSBERG, I_HYDRA, I_SV98, I_SKS, I_ARBALETE, I_AK47, I_GRAP1, I_ARTIFICE, I_GLOCK,
    I_SUPERARME, I_NULL1, I_NULL2, I_NULL3,
    //OBJETS
    I_SANTE, I_BOOSTPV, I_BOOSTDEGATS, I_BOOSTPRECISION, I_BOOSTVITESSE, I_BOOSTGRAVITE,
    I_BOUCLIERBOIS, I_BOUCLIERFER, I_BOUCLIEROR, I_BOUCLIERMAGNETIQUE, I_ARMUREASSISTEE,
    I_MANA,

    TELEPORT,                   // attr1 = idx, attr2 = model, attr3 = tag
    TELEDEST,                   // attr1 = angle, attr2 = idx
    JUMPPAD,                    // attr1 = zpush, attr2 = ypush, attr3 = xpush
    FLAG,                       // attr1 = angle, attr2 = team

    MAXENTTYPES,
};

struct gameentity : extentity
{
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
        //Corps à corps (X armes)
        ATK_CAC349_SHOOT, ATK_CACMARTEAU_SHOOT,
        ATK_CACMASTER_SHOOT, ATK_CACFLEAU_SHOOT,
        //Spéciales aptitudes (2 armes)
        ATK_KAMIKAZE_SHOOT, ATK_ASSISTXPL_SHOOT, ATK_CACNINJA_SHOOT,
        NUMATKS
};

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
};

static struct gamemodeinfo
{
    const char *nameFR;
    const char *nameEN;
    int flags;
} gamemodes[] =
{
    { "demo", "demo", M_DEMO | M_LOCAL },
    { "Editeur de maps", "Map editor", M_EDIT },

    //MODE 1, 2, 3, 4, 5
    { "Tue Les Tous", "Deathmatch", M_LOBBY },
    { "Tue Les Tous (Aléatoire)", "Deathmatch (Random)", M_RANDOM | M_NOAMMO | M_MUNINFINIE},
    { "Tue Les Tous (Full stuff)", "Deathmatch (Full stuff)", M_FULLSTUFF},
    { "Tue Les Tous (Identique)", "Deathmatch (Identical)", M_IDENTIQUE | M_NOAMMO | M_MUNINFINIE},
    //MODE 6, 7, 8, 9, 10
    { "Tue Les Tous", "Team Deathmatch", M_TEAM },
    { "Tue Les Tous (Aléatoire)", "Team Deathmatch (Random)", M_RANDOM | M_TEAM | M_NOAMMO | M_MUNINFINIE},
    { "Tue Les Tous (Full stuff)", "Team Deathmatch (Full stuff)", M_FULLSTUFF | M_TEAM},
    { "Tue Les Tous (Identique)", "Team Deathmatch (Identical)", M_IDENTIQUE | M_TEAM | M_NOAMMO | M_MUNINFINIE},
    //MODE 11, 12, 13, 14, 15
    { "Capture de drapeau", "Capture the flag", M_CTF | M_TEAM },
    { "Capture de drapeau (Aléatoire)", "Capture the flag (Random)", M_RANDOM | M_CTF | M_TEAM | M_NOAMMO | M_MUNINFINIE},
    { "Capture de drapeau (Full stuff)", "Capture the flag (Full stuff)", M_FULLSTUFF | M_CTF | M_TEAM},
    { "Capture de drapeau (Identique)", "Capture the flag (Identical)", M_IDENTIQUE | M_CTF | M_TEAM | M_NOAMMO | M_MUNINFINIE},
};

#define STARTGAMEMODE (-1)
#define NUMGAMEMODES ((int)(sizeof(gamemodes)/sizeof(gamemodes[0])))

#define m_valid(mode)          ((mode) >= STARTGAMEMODE && (mode) < STARTGAMEMODE + NUMGAMEMODES)
#define m_check(mode, flag)    (m_valid(mode) && gamemodes[(mode) - STARTGAMEMODE].flags&(flag))
#define m_checknot(mode, flag) (m_valid(mode) && !(gamemodes[(mode) - STARTGAMEMODE].flags&(flag)))
#define m_checkall(mode, flag) (m_valid(mode) && (gamemodes[(mode) - STARTGAMEMODE].flags&(flag)) == (flag))

#define m_ctf          (m_check(gamemode, M_CTF))
#define m_teammode     (m_check(gamemode, M_TEAM))
#define m_overtime     (m_check(gamemode, M_OVERTIME))
#define isteam(a,b)    (m_teammode && a==b)

#define m_random       (m_check(gamemode, M_RANDOM))
#define m_fullstuff    (m_check(gamemode, M_FULLSTUFF))
#define m_identique    (m_check(gamemode, M_IDENTIQUE))
#define m_noammo       (m_check(gamemode, M_NOAMMO))
#define m_muninfinie   (m_check(gamemode, M_MUNINFINIE))

#define m_demo         (m_check(gamemode, M_DEMO))
#define m_edit         (m_check(gamemode, M_EDIT))
#define m_lobby        (m_check(gamemode, M_LOBBY))
#define m_timed        (m_checknot(gamemode, M_DEMO|M_EDIT|M_LOCAL))
#define m_botmode      (m_checknot(gamemode, M_DEMO|M_LOCAL))
#define m_mp(mode)     (m_checknot(mode, M_LOCAL))

enum { MM_AUTH = -1, MM_OPEN = 0, MM_VETO, MM_LOCKED, MM_PRIVATE, MM_PASSWORD, MM_START = MM_AUTH, MM_INVALID = MM_START - 1 };

static const char * const mastermodenames[] =  { "auth",   "Public",   "veto",       "Verrouillé",     "Privé",    "Mot de passe" };
static const char * const mastermodecolors[] = { "",       "\f0",    "\f2",        "\f2",        "\f3",        "\f3" };
static const char * const mastermodeicons[] =  { "server", "server", "serverlock", "serverlock", "serverpriv", "serverpriv" };

// hardcoded sounds, defined in sounds.cfg
enum
{
    S_JUMP = 0, S_JUMPASSIST, S_LAND, S_LANDASSIST, S_PAS, S_PASASSIST, S_PASEPO, S_NAGE, S_SPLASH2, S_SPLASH1, S_BURN, S_JUMPPAD, S_TELEPORTEUR, S_SANG, S_ITEMSPAWN, S_NOAMMO,

    //Armes
    S_GLOCK, S_UZI, S_MINIGUN, S_MOSSBERG, S_EPEEIDLE, S_EPEEATTACK, S_SMAW, S_FAMAS, S_SPOCKGUN, S_SV98, S_FELECTRIQUE, S_LANCEGRENADE,
    S_ARTIFICE, S_FLAMEATTACK, S_NUKELAUNCH, S_FUSILPLASMA, S_EXPLOSIONARTIFICE, S_EXPLOSION, S_EXPLOSIONGRENADE, S_NUKE, S_KAMIKAZEBOOM, S_ASSISTBOOM,
    S_EXPLOSIONAVECEAU,
    S_ARBALETE, S_AK47, S_GRAP1, S_MARTEAUBAN, S_MASTERSWORD, S_FLEAU, S_GAU8, S_MINIROQUETTE, S_CAMPOUZE, S_MEDIGUN, S_HYDRA, S_SKS,
    S_EAU_GLOCK, S_EAU_UZI, S_EAU_MINIGUN, S_EAU_MOSSBERG, S_EAU_CORPSACORPS, S_EAU_SMAW, S_EAU_FAMAS, S_EAU_SPOCKGUN, S_EAU_SV98, S_EAU_FELECTRIQUE, S_EAU_LANCEGRENADE,
    S_EAU_ARTIFICE, S_EAU_FLAMEATTACK, S_EAU_NUKELAUNCH, S_EAU_FUSILPLASMA, S_EAU_ARBALETE, S_EAU_AK47, S_EAU_GAU8, S_EAU_MINIROQUETTE, S_EAU_MEDIGUN, S_EAU_KAMIKAZE,

    //Sons au loin
    S_ARTIFICELOIN, S_EXPLOSIONLOIN, S_AK47_LOIN, S_FAMAS_LOIN, S_UZI_LOIN, S_SV98_LOIN, S_GLOCK_LOIN, S_MINIGUN_LOIN, S_SKS_LOIN, S_LANCEMISSILE_LOIN,
    S_FELECTRIQUE_LOIN, S_FPLASMA_LOIN, S_SPOCK_LOIN, S_MOSSBERG_LOIN, S_HYDRA_LOIN, S_LANCEGRENADE_LOIN, S_ARTIFICE_LOIN, S_ARMESLOIN, S_RIFLELOIN,

    //Balles
    S_BALLECORPS,
    S_BALLEBOUCLIERBOIS, S_BALLEBOUCLIERFER, S_BALLEBOUCLIEROR, S_BALLEBOUCLIERMAGNETIQUE, S_BALLEARMUREASSISTENT,
    S_REGENMEDIGUN, S_REGENJUNKIE, S_FLYBY, S_FLYBYSNIPE, S_FLYBYGRAP1, S_FLYBYALIEN, S_FLYBYELEC, S_FLYBYFLAME,
    S_IMPACT, S_IMPACTLOURDLOIN, S_IMPACTGRAP1, S_IMPACTALIEN, S_IMPACTSNIPE, S_IMPACTELEC, S_IMPACTEAU,

    //Armes autre
    S_FAMASLOL, S_BLOHBLOH, S_ARTIFICELOL, S_GRENADELOL,
    S_RECHARGEMENT1, S_RECHARGEMENT2, S_RECHARGEMENT3,

    // Objets
    S_ITEMHEALTH, S_COCHON, S_ITEMAMMO, S_ITEMBBOIS, S_ITEMBFER, S_ITEMBOR, S_ITEMBMAGNET, S_ITEMARMOUR, S_ITEMPIECEROBOTIQUE, S_ITEMCHAMPIS, S_ITEMJOINT, S_ITEMEPO, S_ITEMSTEROS, S_STEROSTIR, S_STEROTIRLOIN, S_WEAPLOAD,
    S_HEARTBEAT, S_ASSISTALARM, S_ALARME,
    S_DESTRUCTION, S_INVENTAIRE,

    //Bruitages physique
    S_MISSILE, S_FUSEE, S_MISSILENUKE, S_MINIMISSILE, S_FLECHE, S_DOUILLE, S_BIGDOUILLE, S_CARTOUCHE, S_RGRENADE, S_ECLAIRPROCHE, S_ECLAIRLOIN,

    // Sorts
    S_SORTLANCE,
    S_SORTMAGE1, S_SORTMAGE2, S_SORTMAGE3,  S_SORTPRETRE1, S_SORTPRETRE2, S_SORTPRETRE3,
    S_SORTPHY1, S_SORTPHY2, S_SORTPHY3, S_SORTINDIEN1, S_SORTINDIEN2, S_SORTINDIEN3,
    S_SORTIMPOSSIBLE, S_SORTPRET,
    S_FAUCHEUSE,

    // Menus
    S_MENUBOUTON, S_CAISSEENREGISTREUSE, S_ERROR,
    S_APT_SOLDAT, S_APT_MEDECIN, S_APT_AMERICAIN, S_APT_NINJA, S_APT_VAMPIRE, S_APT_MAGICIEN, S_APT_KAMIKAZE, S_APT_FAUCHEUSE, S_APT_PHYSICIEN, S_APT_CAMPEUR, S_APT_COMMANDO, S_APT_PRETRE, S_APT_VIKING, S_APT_JUNKIE, S_APT_SHOSHONE,

    //Messages
    S_RISIKILL, S_BIGRISIKILL, S_GIGARISIKILL,
    S_RISIKILLLOIN, S_BIGRISIKILLLOIN, S_GIGARISIKILLLOIN,
    S_ACHIEVEMENTUNLOCKED,
    S_KILL, S_PIXEL, S_DRAPEAUPRIS, S_DRAPEAUTOMBE, S_DRAPEAUSCORE, S_DRAPEAURESET,

    S_CGCORTEX, S_CGVALOCHE, S_CGVIEILLE, S_CGHENDEK, S_CGMOUNIR, S_CGDELAVIER, S_CGPRAUD, S_CGRENE, S_CGRAOULT,

    //Null
    S_NULL, //S_PUPOUT,
    S_DIE1, S_DIE2,

    S_HIT,
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
    N_SENDCAPE, N_SENDTOMBE, N_SENDDANSE, N_SENDAPTITUDE,
    N_ANNOUNCE,
    N_SENDSORT1, N_SENDSORT2, N_SENDSORT3,
    N_IDENTIQUEARME, N_SERVAMBIENT,
    NUMMSG
};

static const int msgsizes[] =               // size inclusive message token, 0 for variable or not-checked sizes
{
    N_CONNECT, 0, N_SERVINFO, 0, N_WELCOME, 1, N_INITCLIENT, 0, N_POS, 0, N_TEXT, 0, N_SOUND, 2, N_CDIS, 2,
    N_SHOOT, 0, N_EXPLODE, 0, N_SUICIDE, 1,
    N_DIED, 6, N_DAMAGE, 6, N_VAMPIRE, 4, N_HITPUSH, 7, N_SHOTFX, 10, N_EXPLODEFX, 4,
    N_TRYSPAWN, 1, N_SPAWNSTATE, 0, N_SPAWN, 3, N_FORCEDEATH, 2,
    N_GUNSELECT, 2, N_TAUNT, 1,
    N_MAPCHANGE, 0, N_MAPVOTE, 0, N_TEAMINFO, 0, N_ITEMSPAWN, 2, N_ITEMPICKUP, 2, N_ITEMACC, 4,
    N_PING, 2, N_PONG, 2, N_CLIENTPING, 2,
    N_TIMEUP, 2, N_FORCEINTERMISSION, 1,
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
    N_SENDCAPE, 2, N_SENDTOMBE, 2, N_SENDDANSE, 2, N_SENDAPTITUDE, 2,
    N_ANNOUNCE, 2,
    N_SENDSORT1, 2, N_SENDSORT2, 2, N_SENDSORT3, 2,
    N_IDENTIQUEARME, 2, N_SERVAMBIENT, 2,
    -1
};

#define CC_SERVER_PORT 43000
#define CC_LANINFO_PORT 42998
#define CC_MASTER_PORT 42999
#define PROTOCOL_VERSION 2           // bump when protocol changes
#define DEMO_VERSION 1                // bump when demo format changes
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


static struct itemstat { int add, max, sound; const char *name; int icon, info; } itemstats[] =
{
    {15,    60,    S_ITEMAMMO,   "FUSIL ELECTRIQUE", HICON_SIZE, GUN_RAIL},
    {25,   100,    S_ITEMAMMO,   "FUSIL PLASMA",     HICON_SIZE, GUN_PULSE},
    {5,     20,    S_ITEMAMMO,   "SMAW",             HICON_SIZE, GUN_SMAW},
    {75,   300,    S_ITEMAMMO,   "MINIGUN",          HICON_SIZE, GUN_MINIGUN},
    {15,    60,    S_ITEMAMMO,   "SPOCKGUN",         HICON_SIZE, GUN_SPOCKGUN},
    {7,     28,    S_ITEMAMMO,   "M32",              HICON_SIZE, GUN_M32},
    {50,   200,    S_ITEMAMMO,   "LANCE-FLAMMES",    HICON_SIZE, GUN_LANCEFLAMMES},
    {50,   200,    S_ITEMAMMO,   "UZI",              HICON_SIZE, GUN_UZI},
    {60,   240,    S_ITEMAMMO,   "FAMAS",            HICON_SIZE, GUN_FAMAS},
    {10,    40,    S_ITEMAMMO,   "MOSSBERG 500",     HICON_SIZE, GUN_MOSSBERG},
    {12,    48,    S_ITEMAMMO,   "HYDRA",            HICON_SIZE, GUN_HYDRA},
    {8,     32,    S_ITEMAMMO,   "SV-98",            HICON_SIZE, GUN_SV98},
    {14,    56,    S_ITEMAMMO,   "SKS",              HICON_SIZE, GUN_SKS},
    {12,    48,    S_ITEMAMMO,   "ARBALETE",         HICON_SIZE, GUN_ARBALETE},
    {40,   160,    S_ITEMAMMO,   "AK-47",            HICON_SIZE, GUN_AK47},
    {70,   280,    S_ITEMAMMO,   "GRAP-1",           HICON_SIZE, GUN_GRAP1},
    {10,    40,    S_ITEMAMMO,   "FEU D'ARTIFICE",   HICON_SIZE, GUN_ARTIFICE},
    {30,   120,    S_ITEMAMMO,   "GLOCK",            HICON_SIZE, GUN_GLOCK},
    //Super armes
    {  1,     1,    S_ITEMAMMO,   "BOMBE NUCLEAIRE", HICON_SIZE, GUN_S_NUKE},
    {300,  1000,    S_ITEMAMMO,   "GAU-8",           HICON_SIZE, GUN_S_GAU8},
    { 40,   120,    S_ITEMAMMO,   "MINI-ROQUETTES",  HICON_SIZE, GUN_S_ROQUETTES},
    { 20,    60,    S_ITEMAMMO,   "CAMPOUZE 2000",   HICON_SIZE, GUN_S_CAMPOUZE},
    //Objets
    {250,     1000, S_ITEMHEALTH,   "PANACHAY",            HICON_SIZE},
    {500,     2500, S_COCHON,       "COCHON GRILLAY",      HICON_SIZE},
    {30000,  45000, S_ITEMSTEROS,   "STEROIDES",           HICON_SIZE},
    {40000, 120000, S_ITEMCHAMPIS,  "CHAMPIS",             HICON_SIZE},
    {45000,  75000, S_ITEMEPO,      "EPO",                 HICON_SIZE},
    {30000,  90000, S_ITEMJOINT,    "JOINT",               HICON_SIZE},
    {750,      750, S_ITEMBBOIS,    "BOUCLIER EN BOIS",    HICON_SIZE, A_BLUE},
    {1250,    1250, S_ITEMBFER,     "BOUCLIER DE FER",     HICON_SIZE, A_GREEN},
    {2000,    2000, S_ITEMBOR,      "BOUCLIER D'OR",       HICON_SIZE, A_YELLOW},
    {1500,    1500, S_ITEMBMAGNET,  "BOUCLIER MAGNETIQUE", HICON_SIZE, A_MAGNET},
    {3000,    3000, S_ITEMARMOUR,   "ARMURE ASSISTEE",     HICON_SIZE, A_ASSIST},
    {50,       150, S_ITEMHEALTH,   "MANA",                HICON_SIZE},

    {50,       150, S_ITEMHEALTH,   "CHAIN",               HICON_SIZE},
};

#define MAXRAYS 50
#define EXP_SELFDAMDIV 1
#define EXP_SELFPUSH 1.0f
#define EXP_DISTSCALE 0.5f


static const struct attackinfo { int gun, action, anim, vwepanim, hudanim, sound, farsound1, farsound2, specialsounddelay, attackdelay, damage, spread, nozoomspread, margin, projspeed, kickamount, range, rays, hitpush, exprad, ttl, use; } attacks[NUMATKS] =
{
    //Armes "normales"
    { GUN_RAIL,         ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_FELECTRIQUE,  S_FELECTRIQUE_LOIN, S_NULL,   10,  350,  325,  35, 105, 0,    0,  10, 8000,  1,    30,   0, 0, 0},
    { GUN_PULSE,        ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_FUSILPLASMA,  S_FPLASMA_LOIN, S_NULL,       25,  120,  180,  45, 135, 0, 1500,   5, 8000,  1,    50,  25, 0, 0},
    { GUN_SMAW,         ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_SMAW,         S_LANCEMISSILE_LOIN, S_NULL,   3, 1250,  850,  20,  60, 0,  600,  15, 8000,  1,   750, 125, 0, 0},
    { GUN_MINIGUN,      ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_MINIGUN,      S_MINIGUN_LOIN, S_ARMESLOIN,  35,   70,  180,  60, 180, 0, 3500,   5, 8000,  1,    15 ,  7, 0, 0},
    { GUN_SPOCKGUN,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_SPOCKGUN,     S_SPOCK_LOIN, S_NULL,         15,  250,  300,  15, 150, 0, 1750,   5, 8000,  1,    30,  15, 0, 0},
    { GUN_M32,          ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_LANCEGRENADE, S_LANCEGRENADE_LOIN, S_NULL,   3, 1000, 1250,  20,  50, 0,  400,  10, 1000,  1,   600, 160, 1000, 0},
    { GUN_LANCEFLAMMES, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_FLAMEATTACK,  S_NULL, S_NULL,               30,  100,   20, 500, 500, 0,    0,   2,  280, 10,    10 ,  0, 0, 0},
    { GUN_UZI,          ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_UZI,          S_UZI_LOIN, S_ARMESLOIN,      35,   75,  150,  50, 150, 0, 3000,   2, 8000,  1,    10,   5, 0, 0},
    { GUN_FAMAS,        ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_FAMAS,        S_FAMAS_LOIN, S_ARMESLOIN,    30,   90,  140,  40, 120, 0, 3500,   3, 8000,  1,    20,   5, 0, 0},
    { GUN_MOSSBERG,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_MOSSBERG,     S_NULL, S_RIFLELOIN,           3, 1200,   50, 500, 500, 0,    0,  20, 1000, 25,    20,   0, 0, 0},
    { GUN_HYDRA,        ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_HYDRA,        S_HYDRA_LOIN, S_RIFLELOIN,     4,  666,   35, 300, 300, 0,    0,  15,  600, 15,    20,   0, 0, 0},
    { GUN_SV98,         ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_SV98,         S_SV98_LOIN, S_RIFLELOIN,      2, 1500,  900,   1, 200, 0, 4500,  30, 8000,  1,    80,   7, 0, 0},
    { GUN_SKS,          ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_SKS,          S_SKS_LOIN, S_RIFLELOIN,      10,  420,  500,   5, 125, 0, 3500,  25, 8000,  1,    50,   7, 0, 0},
    { GUN_ARBALETE,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_ARBALETE,     S_NULL, S_NULL,                5,  900,  850,  10,  90, 0, 2000,   7, 8000,  1,    20,   3, 0, 0},
    { GUN_AK47,         ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_AK47,         S_AK47_LOIN, S_ARMESLOIN,     30,   92,  170,  60, 180, 0, 3000,   7, 8000,  1,    50,   5, 0, 0},
    { GUN_GRAP1,        ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_GRAP1,        S_NULL, S_NULL,               12,  200,  250,  30, 300, 0, 1500,  -4, 8000,  1,  -600,  20, 0, 0},
    { GUN_ARTIFICE,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_ARTIFICE,     S_ARTIFICE_LOIN, S_NULL,       3, 1100,  800,  35, 200, 0, 1200,  45,  520,  1,   500,  80, 0, 0},
    { GUN_GLOCK,        ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_GLOCK,        S_GLOCK_LOIN, S_ARMESLOIN,    10,  400,  280,   5, 150, 0, 2000,   7, 8000,  1,    30,   3, 0, 0},
    // Super armes
    { GUN_S_NUKE,       ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_NUKELAUNCH,   S_NULL, S_NULL,                1, 3000,  3250,  20, 300, 0,  175,  10, 2000,  1,   400, 1500, 0, 0},
    { GUN_S_GAU8,       ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_GAU8,         S_RIFLELOIN, S_RIFLELOIN,     90,   14,   370, 150, 250, 0, 6000,   1, 8000,  1,    80,   20, 0, 0},
    { GUN_S_ROQUETTES,  ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_MINIROQUETTE, S_NULL, S_NULL,               14,  170,  2000,  10, 300, 0,  700,   6, 8000,  1,   500,  100, 0, 0},
    { GUN_S_CAMPOUZE,   ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_CAMPOUZE,     S_NULL, S_RIFLELOIN,           8,  500,   500,  50,  50, 0, 5000,   3, 8000, 10,   150,    8, 0, 0},
    // Armes corps à corps
    { GUN_CAC349,       ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_EPEEATTACK,   S_NULL, S_NULL,   4, 1000,  130, 400, 400, 20, 0, -10,  25,  5,  50,  0, 0, 0},
    { GUN_CACMARTEAU,   ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_MARTEAUBAN,   S_NULL, S_NULL,   3, 1500,  175, 250, 250, 15, 0,  -5,  30,  4,  10,  0, 0, 0},
    { GUN_CACMASTER,    ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_MASTERSWORD,  S_NULL, S_NULL,   5, 600,   100, 700, 700, 20, 0,  -8,  25,  5,  30,  0, 0, 0},
    { GUN_CACFLEAU,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_FLEAU,        S_NULL, S_NULL,   4, 1150,  250, 175, 175, 10, 0, -10,  35,  3, 125,  0, 0, 0},
    // Armes spéciales
    { GUN_KAMIKAZE,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_KAMIKAZEBOOM, S_NULL, S_NULL,   1, 1000, 3000,   1,   1,  0, 1,  10,  120,  1, 250, 425, 1, 0},
    { GUN_ASSISTXPL,    ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_ASSISTBOOM,   S_NULL, S_NULL,   1,  220,  150,   1,   1,  0, 1,  10,   50,  1,  50, 250, 1, 0},
    { GUN_CACNINJA,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_EPEEATTACK,   S_NULL, S_NULL,   8,  400,  140, 325, 325, 30, 0, -10,   40,  6,  25,   0, 0, 0},
};

static const struct guninfo { const char *name, *file, *vwep, *armedescFR, *armedescEN; int maxweapposside, maxweapposup, maxzoomfov, hudrange, attacks[NUMACTS]; } guns[NUMGUNS] =
{
    //Armes "normales"
    { "fusilelectrique", "fusilelectrique", "worldgun/fusilelectrique", "un fusil électrique !", "an electric rifle.",                   66,  26, 60,  1000, { -1, ATK_RAIL_SHOOT }, },
    { "fusilplasma", "fusilplasma", "worldgun/fusilplasma",             "un fusil du turfu !", "a futuristic rifle.",                    28,  10, 60,  750,  { -1, ATK_PULSE_SHOOT }, },
    { "smaw", "smaw", "worldgun/smaw",                                  "un lance-roquettes de noob.", "a noob rocket launcher.",         8,   8, 85,  500,  { -1, ATK_SMAW_SHOOT }, },
    { "minigun", "minigun", "worldgun/minigun",                         "un minigun cheaté.", "a cheated minigun.",                      36,  11, 80,  750,  { -1, ATK_MINIGUN_SHOOT }, },
    { "spockgun", "spockgun", "worldgun/spockgun",                      "un pistolet alien", "an alien gun.",                            52,  20, 70,  500,  { -1, ATK_SPOCKGUN_SHOOT }, },
    { "m32", "m32", "worldgun/m32",                                     "une grenade imprévisible.", "an unpredictable grenade",         65,  21, 85,  600,  { -1, ATK_M32_SHOOT }, },
    { "lanceflammes", "lanceflammes", "worldgun/lanceflammes",          "un lance-flammes !", "a flame thrower!",                        40,  15, 95,  280,  { -1, ATK_LANCEFLAMMES_SHOOT }, },
    { "uzi", "uzi", "worldgun/uzi",                                     "une mitraillette de gangster.", "a gangster's weapon.",         23,  21, 80,  500,  { -1, ATK_UZI_SHOOT }, },
    { "famas", "famas", "worldgun/famas",                               "une arme made in France", "a weapon made in France.",           54,  14, 70,  750,  { -1, ATK_FAMAS_SHOOT }, },
    { "mossberg500", "mossberg500", "worldgun/mossberg500",             "un fusil à pompe de vieux con.", "an old man's shotgun.",       38,  18, 95,  300,  { -1, ATK_MOSSBERG_SHOOT }, },
    { "hydra", "hydra", "worldgun/hydra",                               "un fusil venant d'un autre jeu !", "a gun from another game!",  92,  39, 95,  300,  { -1, ATK_HYDRA_SHOOT }, },
    { "sv_98", "sv_98", "worldgun/sv_98",                               "un sniper de campeur.", "a camper rifle.",                       1,   3, 30, 2000,  { -1, ATK_SV98_SHOOT }, },
    { "sks", "sks", "worldgun/sks",                                     "une carabine russe !", "a russian rifle!",                       1,   3, 50, 1500,  { -1, ATK_SKS_SHOOT }, },
    { "arbalete", "arbalete", "worldgun/arbalete",                      "une flèche de merde !", "a rotten arrow.",                       1,   3, 45, 1000,  { -1, ATK_ARBALETE_SHOOT }, },
    { "ak47", "ak47", "worldgun/ak47",                                  "l'arme à Vladimir Poutine !", "Putin's weapon!",                46,  25, 70,  750,  { -1, ATK_AK47_SHOOT }, },
    { "GRAP1", "GRAP1", "worldgun/GRAP1",                               "un projectile rose de tapette !", "a gay ass pink projectile.", 43,  17, 85,  500,  { -1, ATK_GRAP1_SHOOT }, },
    { "feuartifice", "feuartifice", "worldgun/feuartifice",             "une arme de Gilet jaune.", "a yellow vests weapon",             70,  30, 85,  500,  { -1, ATK_ARTIFICE_SHOOT }, },
    { "glock", "glock", "worldgun/glock",                               "un pistolet vraiment pourri.", "a very bad gun",                55,  20, 85,  300,  { -1, ATK_GLOCK_SHOOT }, },
    //Super armes
    { "missilenorko", "missilenorko", "worldgun/missilenorko",          "une putain de bombe nucléaire !", "a fucking nuclear missile!",    8,   3, 85, 2000,  { -1, ATK_NUKE_SHOOT }, },
    { "GAU8", "GAU8", "worldgun/GAU8",                                  "un GAU-8 portable !", "a portable GAU-8!",                        57,  10, 85, 2000,  { -1, ATK_GAU8_SHOOT }, },
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
    { "sabre",      "armes_cac/sabre",  "worldgun/armes_cac/sabre", "un sabre de ninja !",          "a ninja saber!",                            4, 3, 95, 120,  { -1, ATK_CACNINJA_SHOOT }, },
};

static const struct aptisortsinfo { const char *tex1, *tex2, *tex3; int mana1, mana2, mana3, duree1, duree2, duree3, reload1, reload2, reload3, sound1, sound2, sound3; } sorts[] =
{
    { "media/interface/hud/sortmage1.png", "media/interface/hud/sortmage2.png", "media/interface/hud/sortmage3.png",                    30, 40, 60,  250, 4000, 3000, 2000, 5000, 6000,  S_SORTMAGE1, S_SORTMAGE2, S_SORTMAGE3},        // Magicien
    { "media/interface/hud/sortphysicien1.png", "media/interface/hud/sortphysicien2.png", "media/interface/hud/sortphysicien3.png",     45, 50, 65, 2000, 4000, 6000, 3000, 5000, 9000,  S_SORTPHY1, S_SORTPHY2, S_SORTPHY3},           // Physicien
    { "media/interface/hud/sortpretre1.png", "media/interface/hud/sortpretre2.png", "media/interface/hud/sortpretre3.png",              30, 10, 80, 4000, 5000, 4000, 8000, 5000, 10000, S_SORTPRETRE1, S_SORTPRETRE2, S_SORTPRETRE3},  // Prêtre
    { "media/interface/hud/sortindien1.png", "media/interface/hud/sortindien2.png", "media/interface/hud/sortindien3.png",              50, 50, 50, 7500, 7500, 7500, 7500, 7500, 7500,  S_SORTINDIEN1, S_SORTINDIEN2, S_SORTINDIEN3},  // Indien
};

#include "ai.h"

// inherited by gameent and server clients
struct gamestate
{
    int health, maxhealth, mana;
    int armour, armourtype;
    int steromillis, epomillis, jointmillis, champimillis, ragemillis, vampimillis;
    int aptisort1, aptisort2, aptisort3;
    int gunselect, gunwait;
    int ammo[NUMGUNS];
    int aitype, skill;

    gamestate() : maxhealth(1), aitype(AI_NONE), skill(0) {}

    void baseammo(int gun, int k = 2)
    {
        ammo[gun] = (itemstats[gun-GUN_RAIL].add*k);
    }

    bool hasmaxammo(int type)
    {
       const itemstat &is = itemstats[type-I_RAIL];
       return ammo[type-I_RAIL+GUN_RAIL]>=is.max;
    }

    bool canpickup(int type, int aptitude, int parmourtype)
    {
        if(type<I_RAIL || type>I_MANA) return false;
        itemstat &is = itemstats[type-I_RAIL];

        switch(type)
        {
            case I_SANTE:
                if(parmourtype==4) return health<maxhealth;
                else return health<maxhealth;
            case I_MANA:
                if(aptitude==4) return health<maxhealth;
                else return (aptitude==5 || aptitude==8 || aptitude==11 || aptitude==14) && mana<is.max;
            case I_BOOSTPV: return maxhealth<is.max;
            case I_BOOSTDEGATS: return steromillis<is.max;
            case I_BOOSTPRECISION: return champimillis<is.max;
            case I_BOOSTVITESSE:  return epomillis<is.max;
            case I_BOOSTGRAVITE: return jointmillis<is.max;
            case I_BOUCLIERBOIS:
                if(parmourtype==4) return armour<3000;
                else if(armour>=750) return false;
            case I_BOUCLIERFER:
                if(parmourtype==4) return armour<3000;
                else if(armour>=1250) return false;
            case I_BOUCLIERMAGNETIQUE:
                if(parmourtype==4) return armour<3000;
                else if(armour>=1500) return false;
            case I_BOUCLIEROR:
                if(parmourtype==4) return armour<3000;
                else if(armour>=2000) return false;
            case I_ARMUREASSISTEE: return parmourtype!=4 || armour<is.max;
            default:
                {
                    float aptboost;
                    aptitude == 2 ? aptboost = 1.5f : aptboost = 1;
                    return ammo[is.info]<is.max*aptboost;
                }
        }
    }

    void pickup(int type, int aptitude, int rndsuperweapon, int aptisort, int parmourtype)
    {
        if(type<I_RAIL || type>I_MANA) return;
        itemstat &is = itemstats[type-I_RAIL];

        int boostitem = 1;
        if(aptitude==11 && aptisort>0) boostitem++;

        switch(type)
        {
            case I_BOOSTPV:
                health = min(health+is.add*(aptitude==1 ? 1.5f : boostitem), 2500.0f);
                break;
            case I_SANTE:
                health = min(health+is.add*(aptitude==1 ? 2 : boostitem), maxhealth);
                break;
            case I_MANA:
                if(aptitude!=4) mana = min(mana+is.add, is.max);
                else health = min(health+250, maxhealth);
                break;
            case I_BOUCLIERBOIS:
            case I_BOUCLIERFER:
            case I_BOUCLIEROR:
            case I_BOUCLIERMAGNETIQUE:
            case I_ARMUREASSISTEE:
                armour = min(parmourtype==4 && type!=I_ARMUREASSISTEE ? armour+(aptitude==11 && aptisort>0 ? 1000 : 500) : armour+is.add, parmourtype==4 ? 3000 : is.max);
                if(type==I_ARMUREASSISTEE)health = min(health+300, maxhealth);
                if(parmourtype!=4)armourtype = is.info;
                break;
            case I_BOOSTDEGATS: steromillis = min(steromillis+is.add*(aptitude==13 ? 1.5f : boostitem), is.max*(aptitude==13 ? 1.5f : 1)); break;
            case I_BOOSTVITESSE: epomillis = min(epomillis+is.add*(aptitude==13 ? 1.5f : boostitem), is.max*(aptitude==13 ? 1.5f : 1)); break;
            case I_BOOSTGRAVITE: jointmillis = min(jointmillis+is.add*(aptitude==13 ? 1.5f : boostitem), is.max*(aptitude==13 ? 1.5f : 1)); break;
            case I_BOOSTPRECISION: champimillis = min(champimillis+is.add*(aptitude==13 ? 1.5f : boostitem), is.max*(aptitude==13 ? 1.5f : 1)); break;
                break;
            case I_SUPERARME:
                {
                    float aptboost;
                    aptitude == 2 ? aptboost = 2 : aptboost = 1;
                    ammo[is.info+rndsuperweapon] = min(ammo[is.info+rndsuperweapon]+is.add*boostitem*aptboost, is.max*aptboost);
                }
            default:
                {
                    float aptboost;
                    aptitude == 2 ? aptboost = 1.5f : aptboost = 1;
                    ammo[is.info] = min(ammo[is.info]+is.add*boostitem*aptboost, is.max*aptboost);
                }
                break;
        }
    }

    void respawn()
    {
        health = maxhealth;
        mana = 100;
        steromillis = 0;
        epomillis = 0;
        jointmillis = 0;
        champimillis = 0;
        ragemillis = 0;
        vampimillis = 0;
        gunwait = 0;
        aptisort1 = 0;
        aptisort2 = 0;
        aptisort3 = 0;
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
            armour = 750;
            int randomarme = rnd(17);
            gunselect = aptitude==6 ? GUN_KAMIKAZE : aptitude==3 ? GUN_CACNINJA : randomarme;
            ammo[randomarme] = aptitude==2 ? 1.5f*itemstats[randomarme].max : itemstats[randomarme].max;
            if(aptitude==0) addsweaps();
            return;
        }
        else if (m_fullstuff)
        {
            armourtype = A_GREEN;
            armour = 1250;
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
            armour = 750;
            gunselect = aptitude==6 ? GUN_KAMIKAZE : aptitude==3 ? GUN_CACNINJA : cnidentiquearme;
            ammo[cnidentiquearme] = aptitude==2 ? 1.5f*itemstats[cnidentiquearme].max : itemstats[cnidentiquearme].max;
            if(aptitude==0) addsweaps();
            return;
        }
        else
        {
            armourtype = A_BLUE;
            armour = 750;
            ammo[GUN_GLOCK] = aptitude==2 ? 45 : 30;
            ammo[GUN_M32] = aptitude==2 ? 3 : 1;
            gunselect = aptitude==6 ? GUN_KAMIKAZE : aptitude==3 ? GUN_CACNINJA : GUN_GLOCK;
            if(aptitude==0) addsweaps();
        }
    }

    // just subtract damage here, can set death, etc. later in code calling this
    int dodamage(int damage, int aptitude, int aptisort)
    {
        int ad = damage*(armourtype+1)*(armourtype==A_ASSIST && armour>0 ? 16 : 25)/100; // let armour absorb when possible

        if(damage>0)
        {
            if(ad>armour) ad = armour;
            if(aptitude==8 && aptisort>0 && armour>0) armour = min(armour+ad, armourtype==A_BLUE ? 750 : armourtype==A_GREEN ? 1250 : armourtype==A_YELLOW ? 2000 : armourtype==A_MAGNET ? 1500 : 4000);
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

static const char * const teamnames[1+MAXTEAMS] = { "", "YORARIEN", "YORAKEKCHOSE" };
static const char * const teamtextcode[1+MAXTEAMS] = { "\fc", "\fd", "\fc" };
static const int teamtextcolor[1+MAXTEAMS] = { 0xFF2222, 0xFFFF22, 0xFF2222 };
static const char * const teamblipcolor[1+MAXTEAMS] = { "_neutral", "_blue", "_red" };
static inline int teamnumber(const char *name) { loopi(MAXTEAMS) if(!strcmp(teamnames[1+i], name)) return 1+i; return 0; }
#define validteam(n) ((n) >= 1 && (n) <= MAXTEAMS)
#define teamname(n) (teamnames[validteam(n) ? (n) : 0])

struct gameent : dynent, gamestate
{
    int weight;                         // affects the effectiveness of hitpush
    int clientnum, privilege, lastupdate, plag, ping;
    int lifesequence;                   // sequence id for each respawn, used in damage test
    int respawned, suicided;
    int lastpain;
    int lastaction, lastattack;
    int attacking;
    int lastfootstep, attacksound, attackchan, hurtchan, dansechan, sortchan, alarmchan;
    int lasttaunt;
    int lastpickup, lastpickupmillis, flagpickup;
    int killstreak, frags, flags, deaths, totaldamage, totalshots;
    editinfo *edit;
    float deltayaw, deltapitch, deltaroll, newyaw, newpitch, newroll;
    int smoothmillis;

    int lastspecial1update, lastspecial2update, lastspecial3update;
    bool sort1pret = true, sort2pret = true, sort3pret = true;

    string name, info;
    int team, playermodel, playercolor, customcape, customtombe, customdanse, aptitude, level;
    float skeletonfade, tombepop;
    ai::aiinfo *ai;
    int ownernum, lastnode;

    vec muzzle, weed, balles, assist;

    gameent() : weight(100), clientnum(-1), privilege(PRIV_NONE), lastupdate(0), plag(0), ping(0), lifesequence(0), respawned(-1), suicided(-1), lastpain(0), lastfootstep(0), attacksound(-1), attackchan(-1), dansechan(-1), sortchan(-1), killstreak(0), frags(0), flags(0), deaths(0), totaldamage(0), totalshots(0), edit(NULL), smoothmillis(-1), team(0), playermodel(-1), playercolor(0), customcape(0), customtombe(0), customdanse(0), aptitude(0), level(0), ai(NULL), ownernum(-1), muzzle(-1, -1, -1)
    {
        name[0] = info[0] = 0;
        respawn();
    }
    ~gameent()
    {
        freeeditinfo(edit);
        freeeditinfo(edit);
        if(attackchan >= 0) stopsound(attacksound, attackchan);
        if(hurtchan >= 0) stopsound(S_HEARTBEAT, hurtchan);
        //if(dansechan >= 0) stopsound(S_DANSE1+ai->customdanse, alarmchan);
        //if(sortchan >= 0) stopsound(S_ASSISTALARM, alarmchan);
        if(alarmchan >= 0) stopsound(S_ASSISTALARM, alarmchan);
        if(ai) delete ai;
    }

    void hitpush(int damage, const vec &dir, gameent *actor, int atk)
    {
        vec push(dir);
        push.mul((actor==this && attacks[atk].exprad ? EXP_SELFPUSH : 1.0f)*attacks[atk].hitpush*(damage/10)/weight);
        //if(actor->aptitude==2) push.mul(0);
        vel.add(push);
    }

    void stopattacksound()
    {
        if(attackchan >= 0) stopsound(attacksound, attackchan, 250);
        attacksound = attackchan = -1;
    }

    void stopdansesound(gameent *d)
    {
        if(dansechan >= 0) stopsound(S_CGCORTEX+(d->customdanse), dansechan, 150);
        dansechan = -1;
    }

    void stopsortsound(gameent *d)
    {
        int neededdata = 0;
        switch(d->aptitude) {case 8: neededdata++; break; case 11: neededdata+=2; case 14: neededdata+=3;}
        if(sortchan >= 0) stopsound(d->aptisort1 ? sorts[neededdata].sound1 : d->aptisort2 ? sorts[neededdata].sound2 : sorts[neededdata].sound3, sortchan, 50);
        sortchan = -1;
    }

    void stopheartbeat()
    {
        if(hurtchan >= 0) stopsound(S_HEARTBEAT, hurtchan, 2500);
        hurtchan = -1;
    }

    void stopassit()
    {
        if(alarmchan >= 0) stopsound(S_ASSISTALARM, alarmchan, 100);
        alarmchan = -1;
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
        lastnode = -1;
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
    extern void checkitems(gameent *d);

    extern void checkboosts(int time, gameent *d);
    extern void checkaptiskill(int time, gameent *d);

    extern void resetspawns();
    extern void spawnitems(bool force = false);
    extern void putitems(packetbuf &p);
    extern void setspawn(int i, bool on);
    extern void teleport(int n, gameent *d);
    extern void pickupeffects(int n, gameent *d, int rndsuperweapon);
    extern void teleporteffects(gameent *d, int tp, int td, bool local = true);
    extern void jumppadeffects(gameent *d, int jp, bool local = true);
}

namespace game
{
    //Fonctions Cube Conflict
    extern void drawmessages(int killstreak, string str_pseudovictime, int n_aptitudevictime, string str_pseudoacteur, int n_killstreakacteur, float killdistance);
    extern void updatespecials(gameent *d);
    //

    extern int gamemode, battlevivants;

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
        virtual void pickspawn(gameent *d) { findplayerspawn(d, -1, m_teammode ? d->team : 0); }
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

    // game
    extern int nextmode;
    extern string clientmap;
    extern bool intermission;
    extern int maptime, maprealtime, maplimit;
    extern gameent *player1;
    extern vector<gameent *> players, clients;
    extern int lastspawnattempt;
    extern int lasthit;
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
    extern void spawnplayer(gameent *);
    extern void deathstate(gameent *d, gameent *actor, bool restore = false);
    extern void damaged(int damage, gameent *d, gameent *actor, bool local = true, int atk = 0);
    extern void killed(gameent *d, gameent *actor);
    extern void timeupdate(int timeremain);
    extern void msgsound(int n, physent *d = NULL);
    extern void drawicon(int icon, float x, float y, float sz = 120);
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

    // weapon
    extern int getweapon(const char *name);
    extern void shoot(gameent *d, const vec &targ);
    extern void shoteffects(int atk, const vec &from, const vec &to, gameent *d, bool local, int id, int prevaction);
    extern void explode(bool local, gameent *owner, const vec &v, const vec &vel, dynent *safe, int dam, int atk);
    extern void explodeeffects(int atk, gameent *d, bool local, int id = 0);
    extern void damageeffect(int damage, gameent *d, gameent *actor, bool thirdperson = true, int atk = 0);
    extern void regeneffect(int damage, gameent *d, gameent *actor, bool thirdperson = true);
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
    extern void gunselect(int gun, gameent *d);
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
        const char *model[1+MAXTEAMS], *hudguns[1+MAXTEAMS],
                   *icon[1+MAXTEAMS];
        bool ragdoll;
    };

    extern void saveragdoll(gameent *d);
    extern void savetombe(gameent *d);
    extern void clearragdolls();
    extern void moveragdolls();
    extern const playermodelinfo &getplayermodelinfo(gameent *d);
    extern int getplayercolor(gameent *d, int team);
    extern int chooserandomplayermodel(int seed);
    extern void syncplayer();
    extern void swayhudgun(int curtime);
    extern vec hudgunorigin(int gun, const vec &from, const vec &to, gameent *d);

    // sorts
    extern void aptitude(gameent *d, int skill);
    extern int getteamfrags(int team);
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

#endif

