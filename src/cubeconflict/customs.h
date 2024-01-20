//Définition des customisations
#ifndef __CUSTOMS_H__
#define __CUSTOMS_H__

extern const char *getcapedir(int cape, bool enemy = false);
extern const char *getgravedir(int cape);

enum {SMI_HAP = 0, SMI_NOEL, SMI_MALADE, SMI_CONTENT, SMI_COLERE, SMI_SOURNOIS, SMI_FOU, SMI_CLINDOEIL, SMI_COOL, SMI_BUG, NUMSMILEYS}; //+10
extern int smiley[NUMSMILEYS];

static const struct smileysinfo { const char *ident; int price; } customsmileys[NUMSMILEYS] =
{
    {"happy",       0},
    {"xmas",        50},
    {"sick",        100},
    {"stoned",      100},
    {"angry",       250},
    {"sly",         250},
    {"crazy",       250},
    {"wink",        500},
    {"cool",        500},
    {"bug",         1500}
};

enum {CAPE_CUBE = 0, CAPE_PAINT1, CAPE_PAINT2, CAPE_ANAT, CAPE_ATOME, CAPE_DOUBLE, CAPE_FLAMES, CAPE_BOUCLE, CAPE_VINTAGE, CAPE_ELITE, CAPE_HIGH, CAPE_RAYONSX, CAPE_RISITAS, CAPE_RICHE, NUMCAPES}; //6
extern int cape[NUMCAPES];

static const struct capesinfo { const char *ident; int price; } customscapes[NUMCAPES] =
{
    {"noob",        0},
    {"paint1",      50},
    {"paint2",      50},
    {"anatomy",     100},
    {"nuclear",     100},
    {"double",      250},
    {"flames",      250},
    {"time",        250},
    {"vintage",     250},
    {"elite",       250},
    {"psych",       500},
    {"xrays",       500},
    {"risitas3d",   500},
    {"rich",        1500}
};

enum {TOM_MERDE = 0, TOM_BASIQUE1, TOM_BASIQUE2, TOM_FLEUR, TOM_CRISTAL, TOM_GOLF, TOM_OEIL, TOM_EXCALIBUR, TOM_COURONNE, TOM_CRIME, TOM_FUCK, TOM_MONUMENT, TOM_LINGOT, NUMGRAVES}; //7
extern int grave[NUMGRAVES];

static const struct gravesinfo { const char *ident; int price; } customstombes[NUMGRAVES] =
{
    {"merde",     0},
    {"basique1",  50},
    {"basique2",  50},
    {"fleur",     100},
    {"cristal",   100},
    {"minigolf",  100},
    {"oeil",      250},
    {"excalibur", 250},
    {"couronne",  250},
    {"crime",     500},
    {"fuck",      500},
    {"monument",  500},
    {"lingots",   1500}
};

#endif
