#include "../game/game.h"

//Définition des aptitudes
enum {APT_SOLDAT = 0, APT_MEDECIN, APT_AMERICAIN, APT_NINJA, APT_VAMPIRE, APT_MAGICIEN, APT_KAMIKAZE, APT_FAUCHEUSE, APT_PHYSICIEN, APT_CAMPEUR, APT_COMMANDO, APT_PRETRE, APT_VICKING, APT_JUNKIE, APT_INDIEN, NUMAPTS};

static const struct aptitudesinfo { int apt_degats, apt_resistance, apt_precision, apt_vitesse; const char *apt_tete, *apt_nomFR, *apt_nomEN, *apt_logo; } aptitudes[NUMAPTS] =
{
    { 105,  105,  105,   95, "chapeaux/casquette",  "Soldat",       "Soldier",      "media/interface/hud/soldat.jpg"},     //0
    {  90,  110,   95,  105, "chapeaux/medic",      "Médecin",      "Medic",        "media/interface/hud/medecin.jpg"},    //1
    { 100,  135,   80,  130, "chapeaux/aventurier", "Américain",    "American",     "media/interface/hud/americain.jpg"},  //2
    {  85,   90,   75,   70, "chapeaux/bandana",    "Ninja",        "Ninja",        "media/interface/hud/ninja.jpg"},      //3
    { 110,   65,  110,  110,  "chapeaux/cornes",    "Vampire",      "Vampire",      "media/interface/hud/vampire.jpg"},    //4
    { 100,   80,   90,  100, "chapeaux/magicien",   "Magicien",     "Magician",     "media/interface/hud/magicien.jpg"},   //5
    { 100,  100,   70,   75, "chapeaux/japonais",   "Kamikaze",     "Kamikaze",     "media/interface/hud/kamikaze.jpg"},   //6
    { 100,   85,   90,  110, "chapeaux/crane",      "Faucheuse",    "Reaper",       "media/interface/hud/faucheuse.jpg"},  //7
    {  90,   85,   85,  110, "chapeaux/graduation", "Physicien",    "Physicist",    "media/interface/hud/physicien.jpg"},  //8
    { 100,   60,  125,  150, "chapeaux/tente",      "Campeur",      "Camper",       "media/interface/hud/campeur.jpg"},    //9
    { 110,  110,  110,   90, "chapeaux/kepi",       "Commando",     "Commando",     "media/interface/hud/commando.jpg"},   //10
    {  85,  105,   85,   90, "chapeaux/saint",      "Prêtre",       "Priest",       "media/interface/hud/pretre.jpg"},     //11
    { 100,  120,   60,  115, "chapeaux/viking",     "Viking",       "Viking",       "media/interface/hud/viking.jpg"},     //12
    { 100,  110,   85,  120, "chapeaux/champignon", "Junkie",       "Junkie",       "media/interface/hud/dealer.png"},     //13
    { 100,  100,   75,  100, "chapeaux/indien",     "Shoshone",     "Shoshone",     "media/interface/hud/indien.jpg"},     //14
};

//Définition des boucliers
static const struct shieldsinfo { const char *bois, *fer, *gold, *magnetique, *hudbois, *hudfer, *hudgold, *hudmagnetique, *hudassistee; } shields[] =
{
    {"worldshield/bois/100", "worldshield/fer/100", "worldshield/or/100", "worldshield/magnetique/100", "hudshield/bois/100", "hudshield/fer/100", "hudshield/or/100", "hudshield/magnetique/100",  "hudshield/armureassistee/bleu"},
    {"worldshield/bois/80",  "worldshield/fer/80",  "worldshield/or/80",  "worldshield/magnetique/80",  "hudshield/bois/80",  "hudshield/fer/80",  "hudshield/or/80",  "hudshield/magnetique/80",   "hudshield/armureassistee/vert"},
    {"worldshield/bois/60",  "worldshield/fer/60",  "worldshield/or/60",  "worldshield/magnetique/60",  "hudshield/bois/60",  "hudshield/fer/60",  "hudshield/or/60",  "hudshield/magnetique/60",   "hudshield/armureassistee/jaune"},
    {"worldshield/bois/40",  "worldshield/fer/40",  "worldshield/or/40",  "worldshield/magnetique/40",  "hudshield/bois/40",  "hudshield/fer/40",  "hudshield/or/40",  "hudshield/magnetique/40",   "hudshield/armureassistee/orange"},
    {"worldshield/bois/20",  "worldshield/fer/20",  "worldshield/or/20",  "worldshield/magnetique/20",  "hudshield/bois/20",  "hudshield/fer/20",  "hudshield/or/20",  "hudshield/magnetique/20",   "hudshield/armureassistee/rouge"},
};

//Définition des customisations
static const struct custominfo { const char *smiley, *capeteam1, *capeteam2, *custtombe; } customs[] =
{
    {}, //0 = Rien
    {"smileys/hap",      "capes/cape_JVC",          "capes/cape_JVC/orange",            "tombes/basique"},
    {"smileys/noel",     "capes/cape_cisla",        "capes/cape_cisla/orange",          "tombes/fleur"},
    {"smileys/malade",   "capes/cape_atome",        "capes/cape_atome/orange",          "tombes/fuck"},
    {"smileys/content",  "capes/cape_cubeengine",   "capes/cape_cubeengine/orange",     "tombes/monument"},
    {"smileys/colere",   "capes/cape_cislattack",   "capes/cape_cislattack/orange",     "tombes/cristal"},
    {"smileys/sournois", "capes/cape_ruinee",       "capes/cape_ruinee/orange",         "tombes/couronne"},
    {"smileys/fou",      "capes/cape_weed",         "capes/cape_weed/orange",           "tombes/merde"},
    {"smileys/cool",     "capes/cape_larry",        "capes/cape_larry/orange",          "tombes/lingots"},
    {"smileys/bug",      "capes/cape_high",         "capes/cape_high/orange",           "tombes/oeil"},
    {"",                 "capes/cape_depardieu",    "capes/cape_depardieu/orange"},
    {"",                 "capes/cape_risitasbg",    "capes/cape_risitasbg/orange"},
};

//HUD
extern bool suicided;
extern string str_pseudotueur, str_armetueur;
extern int n_aptitudetueur, n_aptitudevictime;

extern int message1, message2, message3, ctfmessage1, ctfmessage2, ctfmessage3, ctfmessage4, ctfmessage5, ctfmessage6;
extern int lastshoot;
extern int zoomfov, zoom, crosshairsize;
extern float crosshairalpha, pourcents;
extern int ccxp, cclvl, needxp, oldneed, neededxp;

extern float weapposside, weapposup, maxweapposside, maxweapposup, shieldside, maxshieldside;
extern int nbfps;

extern int parallaxX, parallaxY;

//Ambiances aléatoires
extern int randomambience;
extern float wateramplitude;

//Statistiques & sauvegarde
extern int langage;
extern string pseudoaleatoire;
extern void genpseudo(bool forcename, int langue);
extern bool conserveurofficiel;

enum {STAT_KILLS, STAT_MORTS, STAT_KILLSTREAK, STAT_BOUCLIERBOIS, STAT_BOUCLIERFER, STAT_BOUCLIEROR, STAT_BOUCLIERMAGNETIQUE,
        STAT_PANACHAY, STAT_MANA, STAT_COCHON, STAT_STEROS, STAT_EPO, STAT_JOINT, STAT_CHAMPIS, STAT_ARMES, STAT_SUPERARMES,
        STAT_DRAPEAUX, STAT_WINS, STAT_ARMUREASSIST, STAT_TPSSEC, STAT_TPSMIN, STAT_TPSH, NUMSTATS};
extern int stat[NUMSTATS];
extern void addstat(int valeur, int stat);
extern float menustat(int value);

enum {ACH_TRIPLE, ACH_QUINTE, ACH_INVINCIBLE, ACH_COLLECTIONNEUR, ACH_ALCOOLIQUE, ACH_MANAMANIA, ACH_PROTECTION,
        ACH_JUNKIE, ACH_WINNER, ACH_EPICWINNER, ACH_FLAGRUNNER, ACH_CHEATER, ACH_NEKFEU, NUMSUCCES};
extern int succes[NUMSUCCES];
extern void unlockachievement(const char* ID);

extern void addxp(int nbxp);
extern int ccxp, cclvl, needxp, oldneed, neededxp;
extern float pourcents;

extern void writesave();
extern bool usesteam;
