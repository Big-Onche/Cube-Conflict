//Définition des customisations

enum {SMI_HAP, SMI_NOEL, SMI_MALADE, SMI_CONTENT, SMI_COLERE, SMI_SOURNOIS, SMI_FOU, SMI_CLINDOEIL, SMI_COOL, SMI_BUG, SMI_11, SMI_12, SMI_13, SMI_14, SMI_15, SMI_16, SMI_17, SMI_18, SMI_19, SMI_20,
        CAPE_CUBE, CAPE_JVC, CAPE_CORONED, CAPE_ATOME, CAPE_JESUSECO, CAPE_DOUBLE, CAPE_FLAMES, CAPE_BOUCLE, CAPE_VINTAGE, CAPE_ELITE, CAPE_HIGH, CAPE_RAYONSX, CAPE_RISITAS, CAPE_RICHE, CAPE_15, CAPE_16, CAPE_17, CAPE_18, CAPE_19, CAPE_20,
        TOM_MERDE, TOM_BASIQUE1, TOM_BASIQUE2, TOM_FLEUR, TOM_CRISTAL, TOM_GOLF, TOM_OEIL, TOM_EXCALIBUR, TOM_COURONNE, TOM_CRIME, TOM_FUCK, TOM_MONUMENT, TOM_LINGOT, TOM_14, TOM_15, TOM_16, TOM_17, TOM_18, TOM_19, TOM_20,
        VOI_CORTEX, VOI_VALOCHE, VOI_VIEILLE, VOI_HENDEK, VOI_MILI1, VOI_MILI2, VOI_MOUNIR, VOI_DELAVIER, VOI_PRAUD, VOI_MALLEVILLE, VOI_11, VOI_12, VOI_13, VOI_14, VOI_15, VOI_16, VOI_17, VOI_18, VOI_19, VOI_20, NUMCUST};
extern int cust[NUMCUST];

static const struct smileysinfo { int smileyprice; } customsmileys[10] = { {0}, {50}, {100}, {100}, {250}, {250}, {250}, {500}, {500}, {1500}, };

static const struct capesinfo { const char *capedir; int capeprice; } customscapes[14] =
{
    {"noob",        0},
    {"triggered",   50},
    {"anatomy",     50},
    {"nuclear",     100},
    {"paint",       100},
    {"double",      250},
    {"flames",      250},
    {"time",        250},
    {"vintage",     250},
    {"elite",       250},
    {"psych",       500},
    {"xrays",       500},
    {"risitas3d",   500},
    {"rich",        1500},
};

static const struct tombesinfo { const char *tombedir; int tombeprice; } customstombes[13] =
{
    {"tombes/merde",     0},
    {"tombes/basique1",  50},
    {"tombes/basique2",  50},
    {"tombes/fleur",     100},
    {"tombes/cristal",   100},
    {"tombes/minigolf",  100},
    {"tombes/oeil",      250},
    {"tombes/excalibur", 250},
    {"tombes/couronne",  250},
    {"tombes/crime",     500},
    {"tombes/fuck",      500},
    {"tombes/monument",  500},
    {"tombes/lingots",   1500},
};

static const struct danceinfo { const char *dancename; int danceprice; } customsdance[11] =
{
    {"Cortex",        0},
    {"Valoche",      50},
    {"Vieille",     100},
    {"Hendek",      100},
    {"Militaire 1", 250},
    {"Militaire 2", 250},
    {"Mounir",      250},
    {"Delavier",    500},
    {"Praud",       500},
    {"Malleville",  1500},
    {"Raoult",      1500},
};
