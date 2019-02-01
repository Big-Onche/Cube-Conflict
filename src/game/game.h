#ifndef __GAME_H__
#define __GAME_H__

#include "cube.h"

// animations

enum
{
    ANIM_DEAD = ANIM_GAMESPECIFIC, ANIM_DYING,
    ANIM_IDLE, ANIM_RUN_N, ANIM_RUN_NE, ANIM_RUN_E, ANIM_RUN_SE, ANIM_RUN_S, ANIM_RUN_SW, ANIM_RUN_W, ANIM_RUN_NW,
    ANIM_JUMP, ANIM_JUMP_N, ANIM_JUMP_NE, ANIM_JUMP_E, ANIM_JUMP_SE, ANIM_JUMP_S, ANIM_JUMP_SW, ANIM_JUMP_W, ANIM_JUMP_NW,
    ANIM_SINK, ANIM_SWIM,
    ANIM_CROUCH, ANIM_CROUCH_N, ANIM_CROUCH_NE, ANIM_CROUCH_E, ANIM_CROUCH_SE, ANIM_CROUCH_S, ANIM_CROUCH_SW, ANIM_CROUCH_W, ANIM_CROUCH_NW,
    ANIM_CROUCH_JUMP, ANIM_CROUCH_JUMP_N, ANIM_CROUCH_JUMP_NE, ANIM_CROUCH_JUMP_E, ANIM_CROUCH_JUMP_SE, ANIM_CROUCH_JUMP_S, ANIM_CROUCH_JUMP_SW, ANIM_CROUCH_JUMP_W, ANIM_CROUCH_JUMP_NW,
    ANIM_CROUCH_SINK, ANIM_CROUCH_SWIM,
    ANIM_SHOOT, ANIM_MELEE,
    ANIM_PAIN,
    ANIM_EDIT, ANIM_LAG, ANIM_TAUNT, ANIM_WIN, ANIM_LOSE,
    ANIM_GUN_IDLE, ANIM_GUN_SHOOT, ANIM_GUN_MELEE,
    ANIM_VWEP_IDLE, ANIM_VWEP_SHOOT, ANIM_VWEP_MELEE,
    ANIM_GUN_ZOOM_IDLE, ANIM_GUN_ZOOM_SHOOT,
    ANIM_GUN_VIDE, ANIM_GUN_RELOAD,
    NUMANIMS
};

static const char * const animnames[] =
{
    "mapmodel",
    "dead", "dying",
    "idle", "run N", "run NE", "run E", "run SE", "run S", "run SW", "run W", "run NW",
    "jump", "jump N", "jump NE", "jump E", "jump SE", "jump S", "jump SW", "jump W", "jump NW",
    "sink", "swim",
    "crouch", "crouch N", "crouch NE", "crouch E", "crouch SE", "crouch S", "crouch SW", "crouch W", "crouch NW",
    "crouch jump", "crouch jump N", "crouch jump NE", "crouch jump E", "crouch jump SE", "crouch jump S", "crouch jump SW", "crouch jump W", "crouch jump NW",
    "crouch sink", "crouch swim",
    "attack", "melee",
    "pain",
    "edit", "lag", "taunt", "win", "lose",
    "gun idle", "gun shoot", "gun melee",
    "vwep idle", "vwep shoot", "vwep melee"
    "gun zoom idle", "gun zoom shoot",
    "gun vide", "gun reload",
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
    I_S_NUKE, I_S_GAU8, I_S_ROQUETTES, I_S_CAMPOUZE,
    //OBJETS
    I_SANTE, I_BOOSTPV, I_BOOSTDEGATS, I_BOOSTPRECISION, I_BOOSTVITESSE, I_BOOSTGRAVITE,
    I_BOUCLIERBOIS, I_BOUCLIERFER, I_BOUCLIEROR, I_BOUCLIERMAGNETIQUE,

    TELEPORT,                   // attr1 = idx, attr2 = model, attr3 = tag
    TELEDEST,                   // attr1 = angle, attr2 = idx
    JUMPPAD,                    // attr1 = zpush, attr2 = ypush, attr3 = xpush
    FLAG,                       // attr1 = angle, attr2 = team

    MAXENTTYPES,

    I_FIRST = 0,
    I_LAST = -1
};

struct gameentity : extentity
{
};

enum { GUN_RAIL = 0, GUN_PULSE, GUN_SMAW, GUN_MINIGUN, GUN_SPOCKGUN, GUN_M32, GUN_LANCEFLAMMES, GUN_UZI, GUN_FAMAS, GUN_MOSSBERG, GUN_HYDRA, GUN_SV98, GUN_SKS, GUN_ARBALETE, GUN_AK47, GUN_GRAP1, GUN_ARTIFICE, GUN_GLOCK,
       GUN_S_NUKE, GUN_S_GAU8, GUN_S_ROQUETTES, GUN_S_CAMPOUZE,
       GUN_CAC349, GUN_CACMARTEAU, GUN_CACMASTER, GUN_CACFLEAU,
       GUN_KAMIKAZE, GUN_MEDIGUN,
       NUMGUNS };
enum { A_BLUE, A_GREEN, A_YELLOW, A_MAGNET };     // armour types... take 20/40/60 % off
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
        ATK_KAMIKAZE_SHOOT, ATK_MEDIGUN_SHOOT,
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
    M_MELEE      = 1<<9,
    M_BATTLE     = 1<<10,
};

static struct gamemodeinfo
{
    const char *name, *prettyname;
    int flags;
    const char *info;
} gamemodes[] =
{
    { "demo", "Demo", M_DEMO | M_LOCAL, NULL},
    { "Editeur de maps", "Editeur de maps", M_EDIT, "Editeur de maps" },

    //MODE 1, 2, 3, 4
    { "Tue Les Tous", "Tue Les Tous", M_LOBBY, "Tue les tous." },
    { "Tue Les Tous (Aléatoire)", "Tue Les Tous (Aléatoire)", M_LOBBY | M_RANDOM, "Tue les tous.\n (Aléatoire)" },
    { "Tue Les Tous (Full stuff)", "Tue Les Tous (Full stuff)", M_LOBBY | M_FULLSTUFF, "Tue les tous.\n (Full stuff)" },
    { "Tue Les Tous (Corps à corps)", "Tue Les Tous (Corps à corps)", M_LOBBY | M_MELEE, "Tue les tous.\n (Corps à corps)" },
    //MODE 5, 6, 7, 8
    { "Tue Les Tous", "Tue Les Tous", M_LOBBY | M_TEAM, "Tue les tous.\n (Par équipe)" },
    { "Tue Les Tous (Aléatoire)", "Tue Les Tous (Aléatoire)", M_LOBBY | M_RANDOM | M_TEAM, "     Tue les tous.    \n (Aléatoire/Par équipe)" },
    { "Tue Les Tous (Full stuff)", "Tue Les Tous (Full stuff)", M_LOBBY | M_FULLSTUFF | M_TEAM, "Tue les tous.\n (Full stuff)" },
    { "Tue Les Tous (Corps à corps)", "Tue Les Tous (Corps à corps)", M_LOBBY | M_MELEE | M_TEAM, "Tue les tous.\n (Corps à corps)" },

    { "Battle royale", "Battle royale", M_LOBBY | M_BATTLE, "Tue les tous." },
    { "Battle royale (Aléatoire)", "Battle royale (Aléatoire)", M_LOBBY | M_RANDOM | M_BATTLE, "Tue les tous.\n (Aléatoire)" },
    { "Battle royale (Full stuff)", "Battle royale (Full stuff)", M_LOBBY | M_FULLSTUFF | M_BATTLE, "Tue les tous.\n (Full stuff)" },
    { "Battle royale (Corps à corps)", "Battle royale (Corps à corps)", M_LOBBY | M_MELEE | M_BATTLE, "Tue les tous.\n (Corps à corps)" },
    //MODE 13, 14, 15, 16
    { "Battle royale", "Battle royale", M_LOBBY | M_TEAM | M_BATTLE, "Tue les tous.\n (Par équipe)" },
    { "Battle royale (Aléatoire)", "Battle royale (Aléatoire)", M_LOBBY | M_RANDOM | M_TEAM | M_BATTLE, "     Tue les tous.    \n (Aléatoire/Par équipe)" },
    { "Battle royale (Full stuff)", "Battle royale (Full stuff)", M_LOBBY | M_FULLSTUFF | M_TEAM | M_BATTLE, "Tue les tous.\n (Full stuff)" },
    { "Battle royale (Corps à corps)", "Battle royale (Corps à corps)", M_LOBBY | M_MELEE | M_TEAM | M_BATTLE, "Tue les tous.\n (Corps à corps)" },
    //MODE 9, 10, 11, 12
    { "Battle royale", "Battle royale", M_LOBBY | M_BATTLE, "Tue les tous." },
    { "Battle royale (Aléatoire)", "Battle royale (Aléatoire)", M_LOBBY | M_RANDOM | M_BATTLE, "Tue les tous.\n (Aléatoire)" },
    { "Battle royale (Full stuff)", "Battle royale (Full stuff)", M_LOBBY | M_FULLSTUFF | M_BATTLE, "Tue les tous.\n (Full stuff)" },
    { "Battle royale (Corps à corps)", "Battle royale (Corps à corps)", M_LOBBY | M_MELEE | M_BATTLE, "Tue les tous.\n (Corps à corps)" },
    //MODE 13, 14, 15, 16
    { "Battle royale", "Battle royale", M_LOBBY | M_TEAM | M_BATTLE, "Tue les tous.\n (Par équipe)" },
    { "Battle royale (Aléatoire)", "Battle royale (Aléatoire)", M_LOBBY | M_RANDOM | M_TEAM | M_BATTLE, "     Tue les tous.    \n (Aléatoire/Par équipe)" },
    { "Battle royale (Full stuff)", "Battle royale (Full stuff)", M_LOBBY | M_FULLSTUFF | M_TEAM | M_BATTLE, "Tue les tous.\n (Full stuff)" },
    { "Battle royale (Corps à corps)", "Battle royale (Corps à corps)", M_LOBBY | M_MELEE | M_TEAM | M_BATTLE, "Tue les tous.\n (Corps à corps)" },


    { "Capture de drapeau", "Capture de drapeau", M_TEAM | M_CTF, "Capture de drapeau." },
    { "Capture de drapeau (Aléatoire)", "Capture de drapeau (Aléatoire)", M_RANDOM | M_TEAM | M_CTF, "Capture de drapeau.    \n (Aléatoire)" },
    { "Capture de drapeau (Full stuff)", "Capture de drapeau (Full stuff)", M_TEAM | M_FULLSTUFF | M_CTF, "Capture de drapeau." },
    { "Capture de drapeau (Corps à corps)", "Capture de drapeau (Corps à corps)", M_RANDOM | M_TEAM | M_CTF, "Capture de drapeau.    \n (Aléatoire)" },
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
#define m_melee        (m_check(gamemode, M_MELEE))
#define m_battle       (m_check(gamemode, M_BATTLE))

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
    S_JUMP = 0, S_LAND, S_SPLASH2, S_SPLASH1, S_BURN, S_JUMPPAD, S_TELEPORTEUR, S_SANG,

    S_GLOCK, S_UZI, S_MINIGUN, S_MOSSBERG, S_EPEEIDLE, S_EPEEATTACK, S_SMAW, S_FAMAS, S_SPOCKGUN, S_SV98, S_FELECTRIQUE, S_LANCEGRENADE,
    S_ARTIFICE, S_FLAMEATTACK, S_NUKELAUNCH, S_FUSILPLASMA, S_EXPLOSIONARTIFICE, S_EXPLOSION, S_EXPLOSIONGRENADE, S_NUKE, S_KAMIKAZEBOOM,
    S_ARBALETE, S_AK47, S_GRAP1, S_MARTEAUBAN, S_MASTERSWORD, S_FLEAU, S_GAU8, S_MINIROQUETTE, S_CAMPOUZE, S_MEDIGUN, S_HYDRA, S_SKS,

    S_EAU_GLOCK, S_EAU_UZI, S_EAU_MINIGUN, S_EAU_MOSSBERG, S_EAU_CORPSACORPS, S_EAU_SMAW, S_EAU_FAMAS, S_EAU_SPOCKGUN, S_EAU_SV98, S_EAU_FELECTRIQUE, S_EAU_LANCEGRENADE,
    S_EAU_ARTIFICE, S_EAU_FLAMEATTACK, S_EAU_NUKELAUNCH, S_EAU_FUSILPLASMA,
    S_EAU_ARBALETE, S_EAU_AK47, S_EAU_GAU8, S_EAU_MINIROQUETTE, S_EAU_MEDIGUN, S_EAU_KAMIKAZE,

    S_ARTIFICELOIN, S_EXPLOSIONLOIN,
    S_AK47_LOIN, S_FAMAS_LOIN, S_UZI_LOIN, S_SV98_LOIN, S_GLOCK_LOIN, S_MINIGUN_LOIN, S_SKS_LOIN, S_LANCEMISSILE_LOIN, S_MISSILE_LOIN, S_FELECTRIQUE_LOIN, S_FPLASMA_LOIN, S_SPOCK_LOIN, S_MOSSBERG_LOIN, S_HYDRA_LOIN, S_LANCEGRENADE_LOIN, S_ARTIFICE_LOIN, S_ARMESLOIN,

    S_RIFLELOIN, S_BALLECORPS, S_BALLEBOUCLIER, S_BALLEBOUCLIERENT, S_REGENMEDIGUN, S_FLYBY, S_FLYBYSNIPE, S_FLYBYGRAP1, S_FLYBYALIEN, S_FLYBYELEC, S_IMPACT, S_IMPACTLOURDLOIN, S_IMPACTGRAP1, S_IMPACTALIEN, S_IMPACTSNIPE, S_IMPACTELEC,

    S_FAMASLOL, S_BLOHBLOH, S_BOOBARL, S_KALASHLOL, S_ARTIFICELOL,

    S_RECHARGEMENT1, S_RECHARGEMENT2, S_RECHARGEMENT3,

    S_ITEMHEALTH, S_COCHON, S_ITEMAMMO, S_ITEMARMOUR, S_ITEMCHAMPIS, S_ITEMJOINT, S_ITEMEPO, S_ITEMSTEROS, S_WEAPLOAD,

    S_HEARTBEAT, S_ALARME,

    S_DESTRUCTION, S_INVENTAIRE,

    S_MISSILE, S_FUSEE, S_MISSILENUKE, S_FLECHE, S_CARTOUCHE, S_RGRENADE, S_ECLAIRPROCHE, S_ECLAIRLOIN,

    S_SORTLANCE, S_SORTMAGE1, S_SORTMAGE2, S_SORTMAGE3, S_SORTPRETRE1, S_SORTPRETRE2, S_SORTPRETRE3, S_SORTPHY1, S_SORTPHY2, S_SORTPHY3, S_SORTIMPOSSIBLE, S_SORTPRET, S_FAUCHEUSE, S_RAGE,

    S_MENUBOUTON, S_CAISSEENREGISTREUSE,

    S_RISIKILL, S_BIGRISIKILL, S_GIGARISIKILL, S_RISIKILLLOIN, S_BIGRISIKILLLOIN, S_GIGARISIKILLLOIN, S_KILL, S_PIXEL, S_DRAPEAUPRIS, S_DRAPEAUTOMBE, S_DRAPEAUSCORE,

    S_BATTLEKILL,

    S_M_PIXELVOI, S_M_PIXELHIT, S_M_PIXELATK,

    S_M_CHEFVOI, S_M_CHEFHIT,

    S_M_FERMIERVOI, S_M_FERMIERHIT,

    S_PIECE,
    S_PET,

    S_QUENELLE,

    S_NULL,


    S_PUNCH1, S_CG,
    S_ITEMSPAWN, S_NOAMMO, S_PUPOUT,
    S_PAIN1, S_PAIN2, S_PAIN3, S_PAIN4, S_PAIN5,
    S_DIE1, S_DIE2,

    S_V_BASECAP, S_V_BASELOST,
    S_V_FIGHT,
    S_V_BOOST10,
    S_V_QUAD10,
    S_V_RESPAWNPOINT,

    S_FLAGRETURN,
    S_FLAGRESET,

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
    N_SENDHAT, N_SENDCAPE, N_SENDAPTITUDE,
    N_ANNOUNCE,
    NUMMSG
};

static const int msgsizes[] =               // size inclusive message token, 0 for variable or not-checked sizes
{
    N_CONNECT, 0, N_SERVINFO, 0, N_WELCOME, 1, N_INITCLIENT, 0, N_POS, 0, N_TEXT, 0, N_SOUND, 2, N_CDIS, 2,
    N_SHOOT, 0, N_EXPLODE, 0, N_SUICIDE, 1,
    N_DIED, 6, N_DAMAGE, 6, N_VAMPIRE, 6, N_HITPUSH, 7, N_SHOTFX, 10, N_EXPLODEFX, 4,
    N_TRYSPAWN, 1, N_SPAWNSTATE, 9, N_SPAWN, 3, N_FORCEDEATH, 2,
    N_GUNSELECT, 2, N_TAUNT, 1,
    N_MAPCHANGE, 0, N_MAPVOTE, 0, N_TEAMINFO, 0, N_ITEMSPAWN, 2, N_ITEMPICKUP, 2, N_ITEMACC, 3,
    N_PING, 2, N_PONG, 2, N_CLIENTPING, 2,
    N_TIMEUP, 2, N_FORCEINTERMISSION, 1,
    N_SERVMSG, 0, N_ITEMLIST, 0, N_RESUME, 0,
    N_EDITMODE, 2, N_EDITENT, 11, N_EDITF, 16, N_EDITT, 16, N_EDITM, 16, N_FLIP, 14, N_COPY, 14, N_PASTE, 14, N_ROTATE, 15, N_REPLACE, 17, N_DELCUBE, 14, N_CALCLIGHT, 1, N_REMIP, 1, N_EDITVSLOT, 16, N_UNDO, 0, N_REDO, 0, N_NEWMAP, 2, N_GETMAP, 1, N_SENDMAP, 0, N_EDITVAR, 0,
    N_MASTERMODE, 2, N_KICK, 0, N_CLEARBANS, 1, N_CURRENTMASTER, 0, N_SPECTATOR, 3, N_SETMASTER, 0, N_SETTEAM, 0,
    N_LISTDEMOS, 1, N_SENDDEMOLIST, 0, N_GETDEMO, 2, N_SENDDEMO, 0,
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
    N_SENDHAT, 2, N_SENDCAPE, 2, N_SENDAPTITUDE, 2,
    N_ANNOUNCE, 2,
    -1
};

#define TESSERACT_SERVER_PORT 42000
#define TESSERACT_LANINFO_PORT 41998
#define TESSERACT_MASTER_PORT 41999
#define PROTOCOL_VERSION 2              // bump when protocol changes
#define DEMO_VERSION 1                  // bump when demo format changes
#define DEMO_MAGIC "TESSERACT_DEMO\0\0"

struct demoheader
{
    char magic[16];
    int version, protocol;
};

#define MAXNAMELEN 15

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
    {100,  400,    S_ITEMAMMO,   "LANCE-FLAMMES",    HICON_SIZE, GUN_LANCEFLAMMES},
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
    {  1,     4,    S_ITEMAMMO,   "BOMBE NUCLEAIRE", HICON_SIZE, GUN_S_NUKE},
    {250,  1000,    S_ITEMAMMO,   "GAU-8",           HICON_SIZE, GUN_S_GAU8},
    { 30,   120,    S_ITEMAMMO,   "MINI-ROQUETTES",  HICON_SIZE, GUN_S_ROQUETTES},
    { 15,    60,    S_ITEMAMMO,   "CAMPOUZE 2000",   HICON_SIZE, GUN_S_CAMPOUZE},
    //Objets
    {250,     1000, S_ITEMHEALTH, "PANACHAY",            HICON_SIZE},
    {750,     2500, S_COCHON,     "COCHON GRILLAY",      HICON_SIZE},
    {30000,  45000, S_ITEMSTEROS, "STEROIDES",           HICON_SIZE},
    {60000, 120000, S_ITEMCHAMPIS,"CHAMPIS",             HICON_SIZE},
    {45000,  75000, S_ITEMEPO,    "EPO",                 HICON_SIZE},
    {60000,  90000, S_ITEMJOINT,  "JOINT",               HICON_SIZE},
    {750,      750, S_ITEMARMOUR, "BOUCLIER EN BOIS",    HICON_SIZE, A_BLUE},
    {1000,    1250, S_ITEMARMOUR, "BOUCLIER DE FER",     HICON_SIZE, A_GREEN},
    {2000,    2000, S_ITEMARMOUR, "BOUCLIER D'OR",       HICON_SIZE, A_YELLOW},
    {1500,    1500, S_ITEMARMOUR, "BOUCLIER MAGNETIQUE", HICON_SIZE, A_MAGNET},
};

#define validitem(n) false

#define MAXRAYS 1
#define EXP_SELFDAMDIV 1
#define EXP_SELFPUSH 1.0f
#define EXP_DISTSCALE 0.5f


static const struct attackinfo { int gun, action, anim, vwepanim, hudanim, sound, hudsound, farsound1, farsound2, attackdelay, damage, spread, nozoomspread, margin, projspeed, kickamount, range, rays, hitpush, exprad, ttl, use; } attacks[NUMATKS] =
{
    //Armes "normales"
    { GUN_RAIL,         ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_FELECTRIQUE,  S_FELECTRIQUE,  S_FELECTRIQUE_LOIN, S_NULL,     350,  325,  10, 100, 0,    0,  10, 4000,  1,    30,   0, 0, 0},
    { GUN_PULSE,        ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_FUSILPLASMA,  S_FUSILPLASMA,  S_FPLASMA_LOIN, S_NULL,         120,  180,  70, 200, 0, 1000,   5, 2000,  1,    50,  25, 0, 0},
    { GUN_SMAW,         ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_SMAW,         S_SMAW,         S_LANCEMISSILE_LOIN, S_NULL,   1250,  850,  20, 200, 0,  600,  15, 3000,  1,   750, 100, 0, 0},
    { GUN_MINIGUN,      ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_MINIGUN,      S_MINIGUN,      S_MINIGUN_LOIN, S_ARMESLOIN,     70,  180, 120, 400, 0, 3000,   5, 2000,  1,    15 , 12, 0, 0},
    { GUN_SPOCKGUN,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_SPOCKGUN,     S_SPOCKGUN,     S_SPOCK_LOIN, S_NULL,           250,  300,  30, 150, 0, 1500,   5,  520,  1,    30,  15, 0, 0},
    { GUN_M32,          ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_LANCEGRENADE, S_LANCEGRENADE, S_LANCEGRENADE_LOIN, S_NULL,   1000, 1250,  25, 250, 0,  400,  10, 1000,  1,   600, 150, 1000, 0},
    { GUN_LANCEFLAMMES, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_FLAMEATTACK,  S_FLAMEATTACK,  S_NULL, S_NULL,                 100,   90, 450, 250, 0,  750,   2,  280,  3,    10 , 25, 0, 0},
    { GUN_UZI,          ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_UZI,          S_UZI,          S_UZI_LOIN, S_ARMESLOIN,         75,  150, 130, 300, 0, 3000,   2, 2000,  1,    10,   7, 0, 0},
    { GUN_FAMAS,        ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_FAMAS,        S_FAMAS,        S_FAMAS_LOIN, S_ARMESLOIN,       90,  140, 100, 250, 0, 3000,   3, 2000,  1,    20,  10, 0, 0},
    { GUN_MOSSBERG,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_MOSSBERG,     S_MOSSBERG,     S_MOSSBERG, S_RIFLELOIN,       1200,  240, 400, 400, 0, 4000,  20, 1000, 20,    20,   8, 0, 0},
    { GUN_HYDRA,        ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_HYDRA,        S_HYDRA,        S_HYDRA_LOIN, S_RIFLELOIN,      750,  200, 300, 300, 0, 2500,  15,  520, 20,    20,   7, 0, 0},
    { GUN_SV98,         ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_SV98,         S_SV98,         S_SV98_LOIN, S_RIFLELOIN,      1500,  900,   1, 600, 0, 3000,  30, 8000,  1,    80,  12, 0, 0},
    { GUN_SKS,          ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_SKS,          S_SKS,          S_SKS_LOIN, S_RIFLELOIN,        420,  500,   5, 150, 0, 2750,  25, 6000,  1,    50,   7, 0, 0},
    { GUN_ARBALETE,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_ARBALETE,     S_ARBALETE,     S_NULL, S_NULL,                 900,  700,  15, 200, 0, 2000,   7, 2000,  1,    20,   3, 0, 0},
    { GUN_AK47,         ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_AK47,         S_AK47,         S_AK47_LOIN, S_ARMESLOIN,        92,  170, 125, 300, 0, 3000,   7, 1000,  1,    50,   3, 0, 0},
    { GUN_GRAP1,        ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_GRAP1,        S_GRAP1,        S_NULL, S_NULL,                 230,  200,  30, 400, 0, 1500,  -3,  840,  1,  -400, 20, 0, 0},
    { GUN_ARTIFICE,     ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_ARTIFICE,     S_ARTIFICE,     S_ARTIFICE_LOIN, S_NULL,       1000,  600, 100, 400, 0,  900,  60,  520,  1,   500, 80, 0, 0},
    { GUN_GLOCK,        ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_GLOCK,        S_GLOCK,        S_GLOCK_LOIN, S_ARMESLOIN,      400,  280, 175, 350, 0, 2000,   7, 1000,  1,    30,   3, 0, 0},
    // Super armes
    { GUN_S_NUKE, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_NUKELAUNCH, S_NUKELAUNCH, S_NULL, S_NULL,           3000, 5000,  20, 300, 0,  175,  10, 2000,  1,   400, 1280, 0, 0},
    { GUN_S_GAU8, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_GAU8, S_GAU8, S_NULL, S_NULL,                        13,   500, 150, 250, 0, 4000,   1, 4000,  1,    80,   15, 0, 0},
    { GUN_S_ROQUETTES, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_MINIROQUETTE, S_MINIROQUETTE, S_NULL, S_NULL,   170,  800,  10, 300, 0,  700,   6, 2000,  1,   500,   70, 0, 0},
    { GUN_S_CAMPOUZE, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_CAMPOUZE, S_CAMPOUZE, S_NULL, S_NULL,           500,   700,  30,  30, 0, 5000,   3, 8000, 10,   150,    8, 0, 0},
    // Armes corps à corps
    { GUN_CAC349, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_EPEEATTACK, S_EPEEATTACK, S_NULL, S_NULL,           1000, 100, 400, 400, 0,    0, -10,   40, 20,    50,  0, 0, 0},
    { GUN_CACMARTEAU, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_MARTEAUBAN, S_MARTEAUBAN, S_NULL, S_NULL,       1500, 140, 400, 400, 0,    0,  -5,   45, 20,    50,  0, 0, 0},
    { GUN_CACMASTER, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_MASTERSWORD, S_MASTERSWORD, S_NULL, S_NULL,       600,  80, 700, 700, 0,    0,  -8,   40, 20,    50,  0, 0, 0},
    { GUN_CACFLEAU, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_FLEAU, S_FLEAU, S_NULL, S_NULL,                   1250,  90, 200, 200, 0,    0,  -7,   55, 20,    70,  0, 0, 0},
    // Armes spéciales aptitudes
    { GUN_KAMIKAZE, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_KAMIKAZEBOOM, S_KAMIKAZEBOOM, S_NULL, S_NULL,    1000, 3000,   1,   1, 0,    1,  10,   70,  1,   300, 400, 1, 0},
    { GUN_MEDIGUN, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_MEDIGUN, S_MEDIGUN, S_NULL, S_NULL,                100,  -50, 500, 700, 0,  750,   2,  280,  4,    10,  25, 0, 0},
};

static const struct guninfo { const char *name, *file, *vwep, *armedesc; int maxweapposside, maxweapposup, maxzoomfov, attacks[NUMACTS]; } guns[NUMGUNS] =
{
    //Armes "normales"
    { "fusilelectrique", "fusilelectrique", "worldgun/fusilelectrique", "un fusil électrique !",            66,  26, 60,  { -1, ATK_RAIL_SHOOT }, },
    { "fusilplasma", "fusilplasma", "worldgun/fusilplasma",             "un fusil du turfu !",              28,  10, 60,  { -1, ATK_PULSE_SHOOT }, },
    { "smaw", "smaw", "worldgun/smaw",                                  "un lance-roquettes de noob.",       8,   8, 85,  { -1, ATK_SMAW_SHOOT }, },
    { "minigun", "minigun", "worldgun/minigun",                         "un minigun cheaté.",               36,  11, 80,  { -1, ATK_MINIGUN_SHOOT }, },
    { "spockgun", "spockgun", "worldgun/spockgun",                      "un pistolet alien",                52,  20, 70,  { -1, ATK_SPOCKGUN_SHOOT }, },
    { "m32", "m32", "worldgun/m32",                                     "une grenade imprévisible.",        65,  21, 85,  { -1, ATK_M32_SHOOT }, },
    { "lanceflammes", "lanceflammes", "worldgun/lanceflammes",          "un lance-flammes !",               40,  15, 95,  { -1, ATK_LANCEFLAMMES_SHOOT }, },
    { "uzi", "uzi", "worldgun/uzi",                                     "une mitraillette de gangster.",    23,  21, 80,  { -1, ATK_UZI_SHOOT }, },
    { "famas", "famas", "worldgun/famas",                               "une arme made in France",          54,  14, 70,  { -1, ATK_FAMAS_SHOOT }, },
    { "mossberg500", "mossberg500", "worldgun/mossberg500",             "un fusil à pompe de vieux con.",   38,  18, 95,  { -1, ATK_MOSSBERG_SHOOT }, },
    { "hydra", "hydra", "worldgun/hydra",                               "un fusil venant d'un autre jeu !", 92,  39, 95,  { -1, ATK_HYDRA_SHOOT }, },
    { "sv_98", "sv_98", "worldgun/sv_98",                               "un sniper de campeur.",             1,   3, 30,  { -1, ATK_SV98_SHOOT }, },
    { "sks", "sks", "worldgun/sks",                                     "une carabine russe !",              1,   3, 50,  { -1, ATK_SKS_SHOOT }, },
    { "arbalete", "arbalete", "worldgun/arbalete",                      "une flèche de merde !",             1,   3, 45,  { -1, ATK_ARBALETE_SHOOT }, },
    { "ak47", "ak47", "worldgun/ak47",                                  "l'arme à Vladimir Poutine !",      46,  25, 70,  { -1, ATK_AK47_SHOOT }, },
    { "GRAP1", "GRAP1", "worldgun/GRAP1",                               "un projectile rose de tapette !",  43,  17, 85,  { -1, ATK_GRAP1_SHOOT }, },
    { "feuartifice", "feuartifice", "worldgun/feuartifice",             "une arme de Gilet jaune.",         70,  30, 85,  { -1, ATK_ARTIFICE_SHOOT }, },
    { "glock", "glock", "worldgun/glock",                               "un pistolet vraiment pourri.",     55,  20, 85,  { -1, ATK_GLOCK_SHOOT }, },
    //Super armes
    { "missilenorko", "missilenorko", "worldgun/missilenorko",          "une putain de bombe nucléaire !",   8,   3, 85,  { -1, ATK_NUKE_SHOOT }, },
    { "GAU8", "GAU8", "worldgun/GAU8",                                  "un GAU-8 portable !",              57,  10, 85,  { -1, ATK_GAU8_SHOOT }, },
    { "miniroquettes", "miniroquettes", "worldgun/miniroquettes",       "un minigun à roquettes !",         10,  10, 70,  { -1, ATK_ROQUETTES_SHOOT }, },
    { "campouze2000", "campouze2000", "worldgun/campouze2000",          "el famoso Campouze 2000 !",        10,  10, 60,  { -1, ATK_CAMPOUZE_SHOOT }, },
    //Corps à corps
    { "epee349", "armes_cac/epee349", "worldgun/armes_cac/epee349",             "l'épée collector à 349 euros.", 4, 3, 95,   { -1, ATK_CAC349_SHOOT }, },
    { "marteauban", "armes_cac/marteauban", "worldgun/armes_cac/marteauban",    "un marteau de bannissement !",  4, 3, 95,   { -1, ATK_CACMARTEAU_SHOOT }, },
    { "mastersword", "armes_cac/mastersword", "worldgun/armes_cac/mastersword", "une épée légendaire !",         4, 3, 95,   { -1, ATK_CACMASTER_SHOOT }, },
    { "fleau", "armes_cac/fleau", "worldgun/armes_cac/fleau",                   "une boule piquante !",          4, 3, 95,   { -1, ATK_CACFLEAU_SHOOT }, },
    // Armes spéciales aptitudes
    { "kamikaze", "kamikaze", "worldgun/kamikaze",  "une ceinture d'explosifs !",        4, 3, 95,   { -1, ATK_KAMIKAZE_SHOOT }, },
    { "medigun", "medigun", "worldgun/medigun",  "un médigun (c'est pas normal) !",     10, 3, 85,   { -1, ATK_MEDIGUN_SHOOT }, },
};

#include "ai.h"

// inherited by gameent and server clients
struct gamestate
{
    int health, maxhealth;
    int armour, armourtype;
    int steromillis, epomillis, jointmillis, champimillis, ragemillis;
    int gunselect, gunwait;
    int ammo[NUMGUNS];
    int aitype, skill;

    gamestate() : maxhealth(1), aitype(AI_NONE), skill(0) {}

    bool hasmaxammo(int type)
    {
       const itemstat &is = itemstats[type-I_RAIL];
       return ammo[type-I_RAIL+GUN_RAIL]>=is.max;
    }

    bool canpickup(int type, int aptitude)
    {
        //return validitem(type);
        if(type<I_RAIL || type>I_BOUCLIERMAGNETIQUE) return false;
        itemstat &is = itemstats[type-I_RAIL];

        switch(type)
        {
            case I_SANTE:
                //if(classe==2) return true;
                //else
                return health<maxhealth;
                break;
            case I_BOOSTPV: return maxhealth<is.max;
            case I_BOOSTDEGATS: return steromillis<is.max;
            case I_BOOSTPRECISION: return champimillis<is.max;
            case I_BOOSTVITESSE:  return epomillis<is.max;
            case I_BOOSTGRAVITE: return jointmillis<is.max;
            case I_BOUCLIERBOIS:
                if(armour>=750) return false;
            case I_BOUCLIERFER:
                if(armour>=1250) return false;
            case I_BOUCLIERMAGNETIQUE:
                if(armourtype==A_YELLOW && armour>=1500) return false;
            case I_BOUCLIEROR: return !armourtype || armour<is.max;
            default:
                {
                    float aptboost;
                    aptitude == 2 ? aptboost = 1.5f : aptboost = 1;
                    return ammo[is.info]<is.max*aptboost;
                }
        }
    }

    void pickup(int type, int aptitude)
    {
        if(type<I_RAIL || type>I_BOUCLIERMAGNETIQUE) return;
        itemstat &is = itemstats[type-I_RAIL];

        switch(type)
        {
            case I_BOOSTPV:
                //if(maxhealth<1300) maxhealth = maxhealth+100;
                health = min(health+is.add, 2500);
                break;
            case I_SANTE: // boost also adds to health
                health = min(health+is.add, maxhealth);
                break;
            case I_BOUCLIERBOIS:
            case I_BOUCLIERFER:
            case I_BOUCLIEROR:
            case I_BOUCLIERMAGNETIQUE:
                armour = min(armour+is.add, is.max);
                armourtype = is.info;
                break;
            case I_BOOSTDEGATS: steromillis = min(steromillis+is.add, is.max); break;
            case I_BOOSTVITESSE: epomillis = min(epomillis+is.add, is.max); break;
            case I_BOOSTGRAVITE: jointmillis = min(jointmillis+is.add, is.max); break;
            case I_BOOSTPRECISION: champimillis = min(champimillis+is.add, is.max); break;
                //if(classe==14) jointmillis = jointmillis*1.50f;
                break;
            default:
                {
                    float aptboost;
                    aptitude == 2 ? aptboost = 1.5f : aptboost = 1;
                    ammo[is.info] = min(ammo[is.info]+is.add*aptboost, is.max*aptboost);
                }
                break;
                //if(classe==2)
                //{
                //    health = min(health+(is.add)*1.6f, (maxhealth*1.0f));
               //     ammo[GUN_MEDIGUN] = min(ammo[GUN_MEDIGUN]+(is.add)*1.6f, (is.max*1.6f));
               // }
                //else if(classe==12 && sortpretre>0)
                //{
                //    health = min(health+(is.add)*2.0f, (maxhealth*1.0f));
                //}
                //else health = min(health+is.add, maxhealth);
        }
    }


    void respawn()
    {
        health = maxhealth;
        steromillis = 0;
        epomillis = 0;
        jointmillis = 0;
        champimillis = 0;
        ragemillis = 0;
        gunwait = 0;
        loopi(NUMGUNS) ammo[i] = 0;
    }

    void addcacweaps(int gamemode, int aptitude)
    {
        if(aptitude == 3)
        {
            switch(rnd(9))
            {
                case 0: ammo[GUN_CAC349] = 1; ammo[GUN_CACMARTEAU] = 1; break;
                case 1: ammo[GUN_CAC349] = 1; ammo[GUN_CACMASTER] = 1; break;
                case 2: ammo[GUN_CAC349] = 1; ammo[GUN_CACFLEAU] = 1; break;
                case 3: ammo[GUN_CACMARTEAU] = 1; ammo[GUN_CAC349] = 1; break;
                case 4: ammo[GUN_CACMARTEAU] = 1; ammo[GUN_CACMASTER] = 1; break;
                case 5: ammo[GUN_CACMARTEAU] = 1; ammo[GUN_CACFLEAU] = 1; break;
                case 6: ammo[GUN_CACFLEAU] = 1; ammo[GUN_CAC349] = 1; break;
                case 7: ammo[GUN_CACFLEAU] = 1; ammo[GUN_CAC349] = 1; break;
                case 8: ammo[GUN_CACFLEAU] = 1; ammo[GUN_CACMASTER] = 1;  break;
            }
            switch(rnd(4))
            {
                case 0: gunselect = GUN_CAC349; return;
                case 1: gunselect = GUN_CACMARTEAU; return;
                case 2: gunselect = GUN_CACMASTER; return;
                case 3: gunselect = GUN_CACFLEAU; return;
            }
        }
        else if(aptitude==6 || aptitude==9) return;
        else
        {
            switch(rnd(4))
            {
                case 0: ammo[GUN_CAC349] = 1; gunselect = GUN_CAC349; break;
                case 1: ammo[GUN_CACMARTEAU] = 1; gunselect = GUN_CACMARTEAU; break;
                case 2: ammo[GUN_CACMASTER] = 1; gunselect = GUN_CACMASTER; break;
                case 3: ammo[GUN_CACFLEAU] = 1; gunselect = GUN_CACFLEAU; break;
            }
        }
    }

    void addsweaps()
    {
        switch(rnd(30))
        {
            case 0:
            {
                switch(rnd(4))
                {
                    case 0: ammo[GUN_S_ROQUETTES] = 20; gunselect = GUN_S_ROQUETTES; break;
                    case 1: ammo[GUN_S_GAU8] = 250; gunselect = GUN_S_GAU8; break;
                    case 2: ammo[GUN_S_CAMPOUZE] = 10; gunselect = GUN_S_CAMPOUZE; break;
                    case 3: ammo[GUN_S_NUKE] = 1; gunselect = GUN_S_NUKE; break;
                }
            }
        }
    }

    void spawnstate(int gamemode, int aptitude)
    {
        addcacweaps(gamemode, aptitude);

        switch(aptitude)
        {
            case 1: ammo[GUN_MEDIGUN] = 160; break;
            case 6: ammo[GUN_KAMIKAZE] = 1; break;
        }

        if(m_random)
        {
            armourtype = A_BLUE;
            armour = 750;
            int randomarme = rnd(17);
            gunselect = aptitude==1 ? GUN_MEDIGUN : aptitude==6 ? GUN_KAMIKAZE : randomarme;
            ammo[randomarme] = aptitude==2 ? 1.5f*itemstats[randomarme].max : itemstats[randomarme].max;
            if(!m_battle) addsweaps();
            return;
        }
        else if (m_fullstuff)
        {
            armourtype = A_GREEN;
            armour = 1250;
            int randomarmea = rnd(17);
            int randomarmeb = rnd(17);
            int randomarmec = rnd(17);
            ammo[randomarmea] = aptitude==2 ? 1.5f*itemstats[randomarmea].max/2 : itemstats[randomarmea].max/2;
            ammo[randomarmeb] = aptitude==2 ? 1.5f*itemstats[randomarmeb].max/2 : itemstats[randomarmeb].max/2;
            ammo[randomarmec] = aptitude==2 ? 1.5f*itemstats[randomarmec].max/2 : itemstats[randomarmec].max/2;
            gunselect = aptitude==1 ? GUN_MEDIGUN : aptitude==6 ? GUN_KAMIKAZE : randomarmea;
            if(!m_battle) addsweaps();
            return;
        }
        else if (m_melee)
        {
            armourtype = A_BLUE;
            armour = 750;
            if(aptitude==6) ammo[GUN_KAMIKAZE] = 0;
            return;
        }
        else
        {
            armourtype = A_BLUE;
            armour = 750;
            //ammo[GUN_GLOCK] = aptitude==2 ? 45 : 30;
            //ammo[GUN_M32] = aptitude==2 ? 3 : 1;
            //gunselect = GUN_GLOCK;

            ammo[GUN_GRAP1] = 100;
            gunselect = GUN_GRAP1;
            if(!m_battle) addsweaps();
        }
    }

    // just subtract damage here, can set death, etc. later in code calling this
    int dodamage(int damage)
    {
        int ad = damage*(armourtype+1)*25/100; // let armour absorb when possible

        if(damage>0)
        {
            if(ad>armour) ad = armour;
            armour -= ad;
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
static const char * const teamnames[1+MAXTEAMS] = { "", "SWAG", "YOLO" };
static const char * const teamtextcode[1+MAXTEAMS] = { "\fd", "\fd", "\fc" };
static const int teamtextcolor[1+MAXTEAMS] = { 0xFFFF22, 0xFFFF22, 0xFF2222 }; // red
static const int teamscoreboardcolor[1+MAXTEAMS] = { 0xFFFF22, 0xFFFF22, 0xFF2222 };
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
    int attacksound, attackchan, idlesound, idlechan, hurtchan, ragechan;
    int lasttaunt;
    int lastpickup, lastpickupmillis, flagpickup;
    int killstreak, frags, flags, deaths, totaldamage, totalshots;
    editinfo *edit;
    float deltayaw, deltapitch, deltaroll, newyaw, newpitch, newroll;
    int smoothmillis;

    string name, info;
    int team, playermodel, playercolor, customhat, customcape, aptitude;
    ai::aiinfo *ai;
    int ownernum, lastnode;

    vec muzzle, weed;

    gameent() : weight(100), clientnum(-1), privilege(PRIV_NONE), lastupdate(0), plag(0), ping(0), lifesequence(0), respawned(-1), suicided(-1), lastpain(0), attacksound(-1), attackchan(-1), killstreak(0), frags(0), flags(0), deaths(0), totaldamage(0), totalshots(0), edit(NULL), smoothmillis(-1), team(0), playermodel(-1), playercolor(0), ai(NULL), ownernum(-1), muzzle(-1, -1, -1)
    {
        name[0] = info[0] = 0;
        respawn();
    }
    ~gameent()
    {
        freeeditinfo(edit);
        freeeditinfo(edit);
        if(attackchan >= 0) stopsound(attacksound, attackchan);
        if(idlechan >= 0) stopsound(idlesound, idlechan);
        if(hurtchan >= 0) stopsound(S_HEARTBEAT, hurtchan);
        if(ragechan >= 0) stopsound(S_RAGE, ragechan);
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

    void stopheartbeat()
    {
        if(hurtchan >= 0) stopsound(S_HEARTBEAT, hurtchan, 4000);
        hurtchan = -1;
    }

    void stopragesound()
    {
        if(ragechan >= 0) stopsound(S_RAGE, ragechan, 4000);
        ragechan = -1;
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
    extern const char *itemname(int i);
    extern int itemicon(int i);

    extern void preloadentities();
    extern void renderentities();
    extern void checkitems(gameent *d);

    extern void checkstero(int time, gameent *d);
    extern void checkepo(int time, gameent *d);
    extern void checkjoint(int time, gameent *d);
    extern void checkchampi(int time, gameent *d);
    extern void checkrage(int time, gameent *d);

    extern void resetspawns();
    extern void spawnitems(bool force = false);
    extern void putitems(packetbuf &p);
    extern void setspawn(int i, bool on);
    extern void teleport(int n, gameent *d);
    extern void pickupeffects(int n, gameent *d);
    extern void teleporteffects(gameent *d, int tp, int td, bool local = true);
    extern void jumppadeffects(gameent *d, int jp, bool local = true);
}

namespace game
{
    //Fonctions Cube Conflict
    extern void drawmessages(int killstreak, string str_pseudovictime, int n_aptitudevictime, string str_pseudoacteur, int n_killstreakacteur);
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
    extern const char *teamcolorname(gameent *d, const char *alt = "you");
    extern const char *teamcolor(const char *prefix, const char *suffix, int team, const char *alt);
    extern gameent *pointatplayer();
    extern gameent *hudplayer();
    extern gameent *followingplayer();
    extern void stopfollowing();
    extern void checkfollow();
    extern void nextfollow(int dir = 1);
    extern void clientdisconnected(int cn, bool notify = true);
    extern void clearclients(bool notify = true);
    extern void startgame();
    extern void spawnplayer(gameent *);
    extern void deathstate(gameent *d, bool restore = false);
    extern void damaged(int damage, gameent *d, gameent *actor, bool local = true, int atk = 0);
    extern void regened(int damage, gameent *d, gameent *actor, bool local = true);
    extern void killed(gameent *d, gameent *actor);
    extern void timeupdate(int timeremain);
    extern void msgsound(int n, physent *d = NULL);
    extern void drawicon(int icon, float x, float y, float sz = 120);
    const char *mastermodecolor(int n, const char *unknown);
    const char *mastermodeicon(int n, const char *unknown);

    extern void movehudgun(gameent *d);

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
    extern void c2sinfo(bool force = false);
    extern void sendposition(gameent *d, bool reliable = false);

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
    extern void clearragdolls();
    extern void moveragdolls();
    extern const playermodelinfo &getplayermodelinfo(gameent *d);
    extern int getplayercolor(gameent *d, int team);
    extern int chooserandomplayermodel(int seed);
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

