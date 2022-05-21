//Définition des customisations
enum {SMI_HAP, SMI_NOEL, SMI_MALADE, SMI_CONTENT, SMI_COLERE, SMI_SOURNOIS, SMI_FOU, SMI_CLINDOEIL, SMI_COOL, SMI_BUG, SMI_11, SMI_12, SMI_13, SMI_14, SMI_15, SMI_16, SMI_17, SMI_18, SMI_19, SMI_20,
        CAPE_CUBE, CAPE_JVC, CAPE_CORONED, CAPE_ATOME, CAPE_JESUSECO, CAPE_WEED, CAPE_FLAMES, CAPE_BOUCLE, CAPE_VINTAGE, CAPE_ELITE, CAPE_HIGH, CAPE_RAYONSX, CAPE_RISITAS, CAPE_RICHE, CAPE_15, CAPE_16, CAPE_17, CAPE_18, CAPE_19, CAPE_20,
        TOM_MERDE, TOM_BASIQUE1, TOM_BASIQUE2, TOM_FLEUR, TOM_CRISTAL, TOM_GOLF, TOM_OEIL, TOM_EXCALIBUR, TOM_COURONNE, TOM_CRIME, TOM_FUCK, TOM_MONUMENT, TOM_LINGOT, TOM_14, TOM_15, TOM_16, TOM_17, TOM_18, TOM_19, TOM_20,
        VOI_CORTEX, VOI_VALOCHE, VOI_VIEILLE, VOI_HENDEK, VOI_MILI1, VOI_MILI2, VOI_MOUNIR, VOI_DELAVIER, VOI_PRAUD, VOI_MALLEVILLE, VOI_11, VOI_12, VOI_13, VOI_14, VOI_15, VOI_16, VOI_17, VOI_18, VOI_19, VOI_20, NUMCUST};
extern int cust[NUMCUST];

static const struct smileysinfo { const char *smileydir, *smileyname; int smileyprice; } customssmileys[10] =
{
    {"smileys/hap",         "Hap",           0},
    {"smileys/noel",        "Noel",          0},
    {"smileys/malade",      "Malade",      100},
    {"smileys/content",     "Content",     100},
    {"smileys/colere",      "Colère",      250},
    {"smileys/sournois",    "Sournois",    250},
    {"smileys/fou",         "Fou",         250},
    {"smileys/clindoeil",   "Clin d'oeil", 500},
    {"smileys/cool",        "Cool",        500},
    {"smileys/bug",         "Bug",        1500},
};

static const struct capesinfo { const char *team1capedir, *team2capedir, *capename; int capeprice; } customscapes[14] =
{
    {"capes/cape_noob",         "capes/cape_noob/orange",       "Noob",              0},
    {"capes/cape_jvc",          "capes/cape_jvc/orange",        "JVC",              50},
    {"capes/cape_coroned",      "capes/cape_coroned/orange",    "Coroned",          50},
    {"capes/cape_atome",        "capes/cape_atome/orange",      "Atome",           100},
    {"capes/cape_jesuseco",     "capes/cape_jesuseco/orange",   "Issou ECO+",      100},
    {"capes/cape_weed",         "capes/cape_weed/orange",       "Weed",            100},
    {"capes/cape_flames",       "capes/cape_flames/orange",     "Flames",          250},
    {"capes/cape_boucle",       "capes/cape_boucle/orange",     "Boucle",          250},
    {"capes/cape_vintage",      "capes/cape_vintage/orange",    "Vintage",         250},
    {"capes/cape_elite",        "capes/cape_elite/orange",      "Elite",           250},
    {"capes/cape_high",         "capes/cape_high/orange",       "Défoncé",         500},
    {"capes/cape_rayonsx",      "capes/cape_rayonsx/orange",    "Rayons X",        500},
    {"capes/cape_risitas",      "capes/cape_risitas/orange",    "Risitas",         500},
    {"capes/cape_riche",        "capes/cape_riche/orange",      "Riche",          1500},
};

static const struct tombesinfo { const char *tombedir, *tombemenudir, *tombename; int tombeprice; } customstombes[13] =
{
    {"tombes/merde",        "tombes/merde",     "Merde",            0},
    {"tombes/basique1",     "tombes/basique1",  "Basique 1",       50},
    {"tombes/basique2",     "tombes/basique2",  "Basique 2",       50},
    {"tombes/fleur",        "tombes/fleur",     "Fleur",          100},
    {"tombes/cristal",      "tombes/cristal",   "Cristal",        100},
    {"tombes/minigolf",     "tombes/minigolf",  "minigolf",       100},
    {"tombes/oeil",         "tombes/oeil/menu", "Oeil",           250},
    {"tombes/excalibur",    "tombes/excalibur", "Excalibur",      250},
    {"tombes/couronne",     "tombes/couronne",  "Couronne",       250},
    {"tombes/crime",        "tombes/crime",     "Crime",          500},
    {"tombes/fuck",         "tombes/fuck",      "Fuck",           500},
    {"tombes/monument",     "tombes/monument",  "Monument",       500},
    {"tombes/lingots",      "tombes/lingots",   "Lingots",       1500},
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
