#ifndef __GAME_H__
#define __GAME_H__

#include "cube.h"

enum { GUN_RAIL = 0, GUN_PULSE, GUN_SMAW, GUN_MINIGUN, GUN_SPOCKGUN, GUN_M32, GUN_LANCEFLAMMES, GUN_UZI, GUN_FAMAS, GUN_MOSSBERG, GUN_HYDRA, GUN_SV98, GUN_SKS, GUN_ARBALETE,
       GUN_AK47, GUN_GRAP1, GUN_ARTIFICE, GUN_GLOCK, GUN_S_NUKE, GUN_S_GAU8, GUN_S_ROQUETTES, GUN_S_CAMPOUZE, NUMGUNS };
enum { A_BLUE, A_GREEN, A_YELLOW, A_MAGNET };     // armour types... take 20/40/60 % off
enum { ACT_IDLE = 0, ACT_SHOOT, ACT_MELEE, NUMACTS };
enum {  ATK_RAIL_SHOOT = 0, ATK_RAIL_MELEE, ATK_PULSE_SHOOT, ATK_PULSE_MELEE,
        ATK_SMAW_SHOOT, ATK_SMAW_MELEE, ATK_MINIGUN_SHOOT, ATK_MINIGUN_MELEE,
        ATK_SPOCKGUN_SHOOT, ATK_SPOCKGUN_MELEE, ATK_M32_SHOOT, ATK_M32_MELEE,
        ATK_LANCEFLAMMES_SHOOT, ATK_LANCEFLAMMES_MELEE, ATK_UZI_SHOOT, ATK_UZI_MELEE,
        ATK_FAMAS_SHOOT, ATK_FAMAS_MELEE, ATK_MOSSBERG_SHOOT, ATK_MOSSBERG_MELEE,
        ATK_HYDRA_SHOOT, ATK_HYDRA_MELEE, ATK_SV98_SHOOT, ATK_SV98_MELEE,
        ATK_SKS_SHOOT, ATK_SKS_MELEE, ATK_ARBALETE_SHOOT, ATK_ARBALETE_MELEE,
        ATK_AK47_SHOOT, ATK_AK47_MELEE, ATK_GRAP1_SHOOT, ATK_GRAP1_MELEE,
        ATK_ARTIFICE_SHOOT, ATK_ARTIFICE_MELEE, ATK_GLOCK_SHOOT, ATK_GLOCK_MELEE,

        //Corps à corps (X armes)

        //Spéciales aptitudes (2 armes)

        //Super armes (4 armes)
        ATK_NUKE_SHOOT, ATK_NUKE_MELEE, ATK_GAU8_SHOOT, ATK_GAU8_MELEE,
        ATK_ROQUETTES_SHOOT, ATK_ROQUETTES_MELEE, ATK_CAMPOUZE_SHOOT, ATK_CAMPOUZE_MELEE,

        NUMATKS
};

#define validgun(n) ((n) >= 0 && (n) < NUMGUNS)
#define validact(n) ((n) >= 0 && (n) < NUMACTS)
#define validatk(n) ((n) >= 0 && (n) < NUMATKS)

#define MAXRAYS 1
#define EXP_SELFDAMDIV 1
#define EXP_SELFPUSH 1.0f
#define EXP_DISTSCALE 0.5f


static const struct attackinfo { int gun, action, anim, vwepanim, hudanim, sound, hudsound, attackdelay, damage, spread, nozoomspread, margin, projspeed, kickamount, range, rays, hitpush, exprad, ttl, use; } attacks[NUMATKS] =
{
    { GUN_RAIL,  ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_FELECTRIQUE, S_FELECTRIQUE,          350,  45,  10,  60, 0,    0,  10, 4000,  1,    30,  0, 0, 0 },
    { GUN_RAIL,  ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK,  S_EPEEATTACK,           500,   1, 400, 600, 0,    0,  10,   14,  1,     0,  0, 0, 0 },
    { GUN_PULSE, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_FUSILPLASMA, S_FUSILPLASMA,          120,  22,  70, 200, 0, 1000,   5, 2000,  1,    50, 25, 0, 0 },
    { GUN_PULSE, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK,  S_EPEEATTACK,           100,  22, 400, 600, 0,    0,   5,   14,  1,     0,  0, 0, 0 },
    { GUN_SMAW,  ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_SMAW,        S_SMAW,                1250,  85,  20, 200, 0,  600,  15, 3000,  1,   800, 75, 0, 0 },
    { GUN_SMAW,  ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK,  S_EPEEATTACK,          1000,  25,  20,  40, 0,    0, -10,   35,  1,     0,  0, 0, 0 },
    { GUN_MINIGUN, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_MINIGUN,   S_MINIGUN,               70,  18, 120, 400, 0, 3000,   5, 2000,  1,    15, 12, 0, 0 },
    { GUN_MINIGUN, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,          100,  22, 400, 600, 0,    0,   0,   14,  1,     0,  0, 0, 0 },
    { GUN_SPOCKGUN, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_SPOCKGUN, S_SPOCKGUN,             250,  40,  30, 150, 0, 1500,   5,  520,  1,    30, 15, 0, 0 },
    { GUN_SPOCKGUN, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,         120,  22, 400, 600, 0,    0,   5,   14,  1,     0,  0, 0, 0 },
    { GUN_M32, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_LANCEGRENADE, S_LANCEGRENADE,         1000, 100,  25, 250, 0,  400,  10, 1000,  1,   800, 80, 1000, 0 },
    { GUN_M32, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,              120,  22, 400, 600, 0,    0,   5,   14,  1,     0,  0, 0, 0 },
    { GUN_LANCEFLAMMES, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_FLAMEATTACK, S_FLAMEATTACK,    19,   5, 500, 700, 0,  750,   2,  280,  4,    10, 25, 0, 0 },
    { GUN_LANCEFLAMMES, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,     120,  22, 400, 600, 0,    0,   5,   14,  1,     0,  0, 0, 0 },
    { GUN_UZI, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_UZI, S_UZI,                            100,  14, 150, 400, 0, 3000,   2, 2000,  1,    10,  7, 0, 0 },
    { GUN_UZI, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,              500,  20, 400, 600, 0,    0,  -2,   14,  1,     0,  0, 0, 0 },
    { GUN_FAMAS, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_FAMAS, S_FAMAS,                       90,  15, 130, 300, 0, 3000,   3, 2000,  1,    20, 10, 0, 0 },
    { GUN_FAMAS, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,            500,  20, 400, 600, 0,    0,  -2,   14,  1,     0,  0, 0, 0 },
    { GUN_MOSSBERG, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_MOSSBERG, S_MOSSBERG,            1200,  13, 400, 400, 0, 2500,  20, 1000, 20,    20,  8, 0, 0 },
    { GUN_MOSSBERG, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,         500,  20, 400, 600, 0,    0,  -2,   14,  1,     0,  0, 0, 0 },
    { GUN_HYDRA, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_HYDRA, S_HYDRA,                      750,   6, 300, 300, 0, 2500,  15,  520, 20,    20,  7, 0, 0 },
    { GUN_HYDRA, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,            500,  20, 400, 600, 0,    0,  -2,   14,  1,     0,  0, 0, 0 },
    { GUN_SV98, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_SV98, S_SV98,                        1500, 110,   1, 600, 0, 5000,  30, 8000,  1,    80, 12, 0, 0 },
    { GUN_SV98, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,             500,  20, 400, 600, 0,    0,  -2,   14,  1,     0,  0, 0, 0 },
    { GUN_SKS, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_SKS, S_SKS,                            420,  50,   5, 150, 0, 4000,  25, 6000,  1,    50,  7, 0, 0 },
    { GUN_SKS, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,              500,  20, 400, 600, 0,    0,  -2,   14,  1,     0,  0, 0, 0 },
    { GUN_ARBALETE, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_ARBALETE, S_ARBALETE,             900,  65,  15, 200, 0, 2000,   7, 2000,  1,    20,  3, 0, 0 },
    { GUN_ARBALETE, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,         500,  20, 400, 600, 0,    0,  -2,   14,  1,     0,  0, 0, 0 },
    { GUN_AK47, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_AK47, S_AK47,                          92,  17, 125, 400, 0, 3000,   7, 1000,  1,    50,  3, 0, 0 },
    { GUN_AK47, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,             500,  20, 400, 600, 0,    0,  -2,   14,  1,     0,  0, 0, 0 },
    { GUN_GRAP1, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_GRAP1, S_GRAP1,                       45,   4,  50, 200, 0, 1000,  -3,  840,  5, -1000,  3, 0, 0 },
    { GUN_GRAP1, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,            500,  20, 400, 600, 0,    0,  -2,   14,  1,     0,  0, 0, 0 },
    { GUN_ARTIFICE, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_ARTIFICE, S_ARTIFICE,             1000, 60, 100, 400, 0,  900,  60,  520,  1,   200, 80, 0, 0 },
    { GUN_ARTIFICE, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,         500,  20, 400, 600, 0,    0,  -2,   14,  1,     0,  0, 0, 0 },
    { GUN_GLOCK, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_GLOCK, S_GLOCK,                      400,  30, 175, 350, 0, 2000,   7, 1000,  1,    30,  3, 0, 0 },
    { GUN_GLOCK, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,            500,  20, 400, 600, 0,    0,  -2,   14,  1,     0,  0, 0, 0 },
    // Super armes
    { GUN_S_NUKE, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_NUKELAUNCH, S_NUKELAUNCH,           400,  30, 175, 350, 0, 2000,   7, 1000,  1,    30,  3, 0, 0 },
    { GUN_S_NUKE, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,           500,  20, 400, 600, 0,    0,  -2,   14,  1,     0,  0, 0, 0 },
    { GUN_S_GAU8, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_GAU8, S_GAU8,                       400,  30, 175, 350, 0, 2000,   7, 1000,  1,    30,  3, 0, 0 },
    { GUN_S_GAU8, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,           500,  20, 400, 600, 0,    0,  -2,   14,  1,     0,  0, 0, 0 },
    { GUN_S_ROQUETTES, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_MINIROQUETTE, S_MINIROQUETTE,  400,  30, 175, 350, 0, 2000,   7, 1000,  1,    30,  3, 0, 0 },
    { GUN_S_ROQUETTES, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,      500,  20, 400, 600, 0,    0,  -2,   14,  1,     0,  0, 0, 0 },
    { GUN_S_CAMPOUZE, ACT_SHOOT, ANIM_SHOOT, ANIM_VWEP_SHOOT, ANIM_GUN_SHOOT, S_CAMPOUZE, S_CAMPOUZE,           400,  30, 175, 350, 0, 2000,   7, 1000,  1,    30,  3, 0, 0 },
    { GUN_S_CAMPOUZE, ACT_MELEE, ANIM_MELEE, ANIM_VWEP_MELEE, ANIM_GUN_MELEE, S_EPEEATTACK, S_EPEEATTACK,       500,  20, 400, 600, 0,    0,  -2,   14,  1,     0,  0, 0, 0 },
};

static const struct guninfo { const char *name, *file, *vwep; int attacks[NUMACTS]; } guns[NUMGUNS] =
{
    { "fusilelectrique", "fusilelectrique", "worldgun/fusilelectrique", { -1, ATK_RAIL_SHOOT, ATK_RAIL_MELEE }, },
    { "fusilplasma", "fusilplasma", "worldgun/fusilplasma",             { -1, ATK_PULSE_SHOOT, ATK_PULSE_MELEE }, },
    { "smaw", "smaw", "worldgun/smaw",                                  { -1, ATK_SMAW_SHOOT, ATK_SMAW_MELEE }, },
    { "minigun", "minigun", "worldgun/minigun",                         { -1, ATK_MINIGUN_SHOOT, ATK_MINIGUN_MELEE }, },
    { "spockgun", "spockgun", "worldgun/spockgun",                      { -1, ATK_SPOCKGUN_SHOOT, ATK_SPOCKGUN_MELEE }, },
    { "m32", "m32", "worldgun/m32",                                     { -1, ATK_M32_SHOOT, ATK_M32_MELEE }, },
    { "lanceflammes", "lanceflammes", "worldgun/lanceflammes",          { -1, ATK_LANCEFLAMMES_SHOOT, ATK_LANCEFLAMMES_MELEE }, },
    { "uzi", "uzi", "worldgun/uzi",                                     { -1, ATK_UZI_SHOOT, ATK_UZI_MELEE }, },
    { "famas", "famas", "worldgun/famas",                               { -1, ATK_FAMAS_SHOOT, ATK_FAMAS_MELEE }, },
    { "mossberg500", "mossberg500", "worldgun/mossberg500",             { -1, ATK_MOSSBERG_SHOOT, ATK_MOSSBERG_MELEE }, },
    { "hydra", "hydra", "worldgun/hydra",                               { -1, ATK_HYDRA_SHOOT, ATK_HYDRA_MELEE }, },
    { "sv_98", "sv_98", "worldgun/sv_98",                               { -1, ATK_SV98_SHOOT, ATK_SV98_MELEE }, },
    { "sks", "sks", "worldgun/sks",                                     { -1, ATK_SKS_SHOOT, ATK_SKS_MELEE }, },
    { "arbalete", "arbalete", "worldgun/arbalete",                      { -1, ATK_ARBALETE_SHOOT, ATK_ARBALETE_MELEE }, },
    { "ak47", "ak47", "worldgun/ak47",                                  { -1, ATK_AK47_SHOOT, ATK_AK47_MELEE }, },
    { "GRAP1", "GRAP1", "worldgun/GRAP1",                               { -1, ATK_GRAP1_SHOOT, ATK_GRAP1_MELEE }, },
    { "feuartifice", "feuartifice", "worldgun/feuartifice",             { -1, ATK_ARTIFICE_SHOOT, ATK_ARTIFICE_MELEE }, },
    { "glock", "glock", "worldgun/glock",                               { -1, ATK_GLOCK_SHOOT, ATK_GLOCK_MELEE }, },
    //Super armes
    { "missilenorko", "missilenorko", "worldgun/missilenorko",          { -1, ATK_NUKE_SHOOT, ATK_NUKE_MELEE }, },
    { "GAU8", "GAU8", "worldgun/GAU8",                                  { -1, ATK_GAU8_SHOOT, ATK_GAU8_MELEE }, },
    { "miniroquettes", "miniroquettes", "worldgun/miniroquettes",       { -1, ATK_ROQUETTES_SHOOT, ATK_ROQUETTES_MELEE }, },
    { "campouze2000", "campouze2000", "worldgun/campouze2000",          { -1, ATK_CAMPOUZE_SHOOT, ATK_CAMPOUZE_MELEE }, },
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

    /*
    {10,    40,    S_ITEMAMMO,   "ARTIFICE",        HICON_FIST, GUN_ARTIFICE},


    {25,    100,   S_ITEMHEALTH, "PANACHAY",        HICON_HEALTH},
    {10,   1000,   S_COCHON,     "COCHON GRILLAY",  HICON_HEALTH},
    {0,       1,   S_ITEMCHAMPIS,"CHAMPIS",         HICON_QUAD},
    {45000, 45000, S_ITEMEPO,    "EPO",             HICON_QUAD},
    {60000, 60000, S_ITEMJOINT,  "JOINT",           HICON_QUAD},
    {100,   125,   S_ITEMARMOUR, "BOUCLIER DE FER",    HICON_GREEN_ARMOUR, A_GREEN},
    {200,   200,   S_ITEMARMOUR, "BOUCLIER D'OR",   HICON_YELLOW_ARMOUR, A_YELLOW},
    {75,     75,   S_ITEMARMOUR, "BOUCLIER EN BOIS",    HICON_GREEN_ARMOUR, A_BLUE},
    {150,    150,  S_ITEMARMOUR, "BOUCLIER MAGNETIQUE",    HICON_GREEN_ARMOUR, A_MAGNET},
    {1, 999999999,         S_PIECE, "CISLACOIN",            HICON_TOKEN},
    {30000, 30000, S_ITEMSTEROS, "STEROIDES",       HICON_QUAD},
    */
};
