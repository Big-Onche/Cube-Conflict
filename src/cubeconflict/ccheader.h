#include "../game/game.h"

extern bool randomevent(int probability);

//Définition des aptitudes
enum {APT_SOLDAT = 0, APT_MEDECIN, APT_AMERICAIN, APT_NINJA, APT_VAMPIRE, APT_MAGICIEN, APT_KAMIKAZE, APT_FAUCHEUSE, APT_PHYSICIEN, APT_CAMPEUR, APT_ESPION, APT_PRETRE, APT_VIKING, APT_JUNKIE, APT_SHOSHONE, NUMAPTS};

static const struct aptitudesinfo { int apt_degats, apt_resistance, apt_precision, apt_vitesse; const char *apt_tete, *apt_nomFR, *apt_nomEN, *apt_logo; } aptitudes[NUMAPTS] =
{
    { 105,  105,  105,   950, "chapeaux/casquette",  "Soldat",       "Soldier",      "media/interface/hud/soldat.jpg"},     //0     APT_SOLDAT
    {  90,  110,   95,   950, "chapeaux/medic",      "Médecin",      "Medic",        "media/interface/hud/medecin.jpg"},    //1     APT_MEDECIN
    { 100,  135,   80,  1300, "chapeaux/aventurier", "Américain",    "American",     "media/interface/hud/americain.jpg"},  //2     APT_AMERICAIN
    {  85,   90,   75,   750, "chapeaux/bandana",    "Ninja",        "Ninja",        "media/interface/hud/ninja.jpg"},      //3     APT_NINJA
    { 110,   65,  110,   950, "chapeaux/cornes",     "Vampire",      "Vampire",      "media/interface/hud/vampire.jpg"},    //4     APT_VAMPIRE
    { 100,   85,   90,  1000, "chapeaux/magicien",   "Magicien",     "Wizard",       "media/interface/hud/magicien.jpg"},   //5     APT_MAGICIEN
    { 100,  100,   70,   850, "chapeaux/japonais",   "Kamikaze",     "Kamikaze",     "media/interface/hud/kamikaze.jpg"},   //6     APT_KAMIKAZE
    { 115,   85,   90,  1050, "chapeaux/crane",      "Faucheuse",    "Reaper",       "media/interface/hud/faucheuse.jpg"},  //7     APT_FAUCHEUSE
    {  90,   85,   85,  1050, "chapeaux/graduation", "Physicien",    "Physicist",    "media/interface/hud/physicien.jpg"},  //8     APT_PHYSICIEN
    { 100,   60,  130,  1250, "chapeaux/tente",      "Campeur",      "Camper",       "media/interface/hud/campeur.jpg"},    //9     APT_CAMPEUR
    {  90,   85,  120,  1100, "chapeaux/melon",      "Espion",       "Spy",          "media/interface/hud/espion.jpg"},     //10    APT_ESPION
    {  85,  105,   85,   950, "chapeaux/saint",      "Prêtre",       "Priest",       "media/interface/hud/pretre.jpg"},     //11    APT_PRETRE
    { 100,  120,   60,  1050, "chapeaux/viking",     "Viking",       "Viking",       "media/interface/hud/viking.jpg"},     //12    APT_VIKING
    { 100,  110,   85,  1100, "chapeaux/champignon", "Junkie",       "Junkie",       "media/interface/hud/dealer.png"},     //13    APT_JUNKIE
    { 100,  100,   75,  1000, "chapeaux/indien",     "Shoshone",     "Shoshone",     "media/interface/hud/indien.jpg"},     //14    APT_SHOSHONE
};

//Définition des customisations
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

static const struct costumeinfo { const char *village, *usine, *faille, *lune, *chateaux, *volcan; } costumes[4] =
{
    {"mapmodel/caisses/caissebois", "mapmodel/caisses/caisse1", "mapmodel/caisses/caissebois",  "mapmodel/rochers/pierre_fonce_esp",    "mapmodel/caisses/caissebois",  "mapmodel/distributeur"},
    {"mapmodel/tonneau",            "mapmodel/caisses/caisse2", "mapmodel/tonneau",             "mapmodel/caisses/caisse2",             "mapmodel/tonneau",             "mapmodel/rochers/pierre_fonce_esp"},
    {"mapmodel/panneau",            "mapmodel/caisses/caisse3", "mapmodel/panneau",             "mapmodel/caisses/caisse3",             "mapmodel/panneau",             "mapmodel/lampadaire"},
    {"mapmodel/arbres/arbre1",      "mapmodel/murjersay",       "mapmodel/arbres/arbre1",       "mapmodel/murjersay",                   "mapmodel/arbres/arbre1",       "mapmodel/murjersay"},
};

namespace gfx
{
    static const struct colors{ int color; } rndcolor[] =
    {
        {0xFF0000},
        {0x00FF00},
        {0x0000FF},
        {0xFFFF00},
        {0x00FFFF},
        {0xFF00FF}
    };

    extern bool champicolor;
    extern void projgunexplosion(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void projexplosion(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void projgunhit(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void instantrayhit(const vec &from, const vec &to, const vec &muzzle, int atk);
    extern const char *getshielddir(int armourtype, int armourval, bool hud = false, bool preload = false);
}

//HUD and MENUS
extern bool suicided, holdflag;
extern string str_pseudotueur, str_armetueur, tempachname;
extern int n_aptitudetueur, n_aptitudevictime;

enum {MSG_OWNKILLSTREAK = 0, MSG_YOUKILLED, MSG_OTHERKILLSTREAK,
        MSG_CTF_TEAMPOINT, MSG_CTF_ENNEMYPOINT, MSG_CTF_TEAMFLAGRECO, MSG_CTF_ENNEMYFLAGRECO, MSG_CTF_TEAMSTOLE, MSG_CTF_ENNEMYSTOLE,
        MSG_LEVELUP, MSG_ACHUNLOCKED, NUMMESSAGE};
extern int message[NUMMESSAGE];

extern int lastshoot, getsort;
extern int zoomfov, zoom, crosshairsize;
extern float crosshairalpha, pourcents;

extern float weapposside, weapposup, maxweapposside, maxweapposup, shieldside, maxshieldside;
extern int nbfps;

extern int parallaxX, parallaxY, langage, UI_menutabs, UI_smiley, UI_cape, UI_tombe, UI_voix, UI_custtab, UI_showsteamnamebtn; //Menu

//Ambiances aléatoires
extern int randomambience;
extern float wateramplitude;

//Statistiques & sauvegarde & config élémentaire
extern bool IS_USING_STEAM, IS_ON_OFFICIAL_SERV, UI_PLAYMUSIC;
extern void getsteamname();
extern void genpseudo(int forcelang = 0);

