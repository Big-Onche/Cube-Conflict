//Définition des customisations
#ifndef __CUSTOMS_H__
#define __CUSTOMS_H__

enum {SKIN_CAPE, SKIN_GRAVE, SKIN_TAUNT, NUMSKINS};

enum {D_COMMON = 0, D_UNCOMMON, D_RARE, D_LEGENDARY, D_GODLY, NUMDROPS};

enum {SMI_HAP = 0, SMI_NOEL, SMI_MALADE, SMI_CONTENT, SMI_COLERE, SMI_SOURNOIS, SMI_FOU, SMI_CLINDOEIL, SMI_COOL, SMI_BUG, NUMSMILEYS}; //+10
extern int smiley[NUMSMILEYS];
#define validSmiley(n) ((n) >= 0 && (n) < NUMSMILEYS)

static const struct smileysinfo { const char *ident; int oldprice, price, value; } customsmileys[NUMSMILEYS] =
{
    {"happy",       0,      0,      D_COMMON},
    {"xmas",        50,     25,     D_COMMON},
    {"sick",        100,    75,     D_UNCOMMON},
    {"stoned",      100,    75,     D_UNCOMMON},
    {"angry",       250,    125,    D_RARE},
    {"sly",         250,    125,    D_RARE},
    {"crazy",       250,    125,    D_RARE},
    {"wink",        500,    250,    D_LEGENDARY},
    {"cool",        500,    250,    D_LEGENDARY},
    {"bug",         1500,   500,    D_GODLY}
};

enum {CAPE_CUBE = 0, CAPE_PAINT1, CAPE_PAINT2, CAPE_ANAT, CAPE_ATOME, CAPE_DOUBLE, CAPE_FLAMES, CAPE_BOUCLE, CAPE_VINTAGE, CAPE_ELITE, CAPE_HIGH, CAPE_RAYONSX, CAPE_RISITAS, CAPE_RICHE, NUMCAPES}; //6
extern int cape[NUMCAPES];
#define validCape(n) ((n) >= 0 && (n) < NUMCAPES)

static const struct capesConfig { const char *name; int price, value; } capes[NUMCAPES] =
{
    {"noob",        0,      D_COMMON},
    {"paint1",      25,     D_COMMON},
    {"paint2",      25,     D_COMMON},
    {"anatomy",     75,     D_UNCOMMON},
    {"nuclear",     75,     D_UNCOMMON},
    {"double",      125,    D_RARE},
    {"flames",      125,    D_RARE},
    {"time",        125,    D_RARE},
    {"vintage",     125,    D_RARE},
    {"elite",       125,    D_RARE},
    {"psych",       250,    D_LEGENDARY},
    {"xrays",       250,    D_LEGENDARY},
    {"risitas3d",   250,    D_LEGENDARY},
    {"rich",        500,    D_GODLY}
};

enum {TOM_MERDE = 0, TOM_BASIQUE1, TOM_BASIQUE2, TOM_FLEUR, TOM_CRISTAL, TOM_GOLF, TOM_OEIL, TOM_EXCALIBUR, TOM_COURONNE, TOM_CRIME, TOM_FUCK, TOM_MONUMENT, TOM_LINGOT, NUMGRAVES}; //7
extern int grave[NUMGRAVES];
#define validGrave(n) ((n) >= 0 && (n) < NUMGRAVES)

static const struct gravesConfig { const char *name; int price, value; } graves[NUMGRAVES] =
{
    {"merde",         0,    D_COMMON},
    {"basique1",     25,    D_COMMON},
    {"basique2",     25,    D_COMMON},
    {"fleur",        75,    D_UNCOMMON},
    {"cristal",      75,    D_UNCOMMON},
    {"minigolf",     75,    D_UNCOMMON},
    {"oeil",        125,    D_RARE},
    {"excalibur",   125,    D_RARE},
    {"couronne",    125,    D_RARE},
    {"crime",       250,    D_LEGENDARY},
    {"fuck",        250,    D_LEGENDARY},
    {"monument",    250,    D_LEGENDARY},
    {"lingots",     500,    D_GODLY}
};

#endif
