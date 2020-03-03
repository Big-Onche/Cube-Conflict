#include "../game/game.h"

//enum { APT_SPEC_DURABLE = 0, APT_SPEC_BLINDE, APT_SPEC_DOPE, APT_SPEC_SURARME, APT_SPEC_CHANCEUX, NUMAPTSSPECS };

//static struct aptitudesspecs { int apt_spec; float boostpv, boostarmure, boostbonus, boostmunitions; } aptitspecs[NUMAPTSSPECS] =
//{
//    { APT_SPEC_DURABLE,     1.25f,  1.00f,  1.00f,  1.00f},
//    { APT_SPEC_BLINDE,      1.00f,  1.25f,  1.00f,  1.00f},
//    { APT_SPEC_DOPE,        1.00f,  1.00f,  1.25f,  1.00f},
//    { APT_SPEC_SURARME,     1.00f,  1.00f,  1.00f,  1.25f},
//    { APT_SPEC_CHANCEUX,    1.00f,  1.00f,  1.00f,  1.00f}, // Changés aléatoirement à chaque vie
//};

//Définition des aptitudes


enum {APT_SOLDAT = 0, APT_MEDECIN, APT_AMERICAIN, APT_NINJA, APT_VAMPIRE, APT_MAGICIEN, APT_KAMIKAZE, APT_FAUCHEUSE, APT_PHYSICIEN, APT_CAMPEUR, APT_COMMANDO, APT_PRETRE, APT_VICKING, APT_JUNKIE, NUMAPTS};

static const struct aptitudesinfo { int apt_degats, apt_resistance, apt_precision, apt_vitesse; const char *apt_tete, *apt_nom; } aptitudes[NUMAPTS] =
{
    { 105,  105, 105,   95, "chapeaux/casquette",   "Soldat"},      //0 ---> OK
    { 80,   75,  100,  100, "chapeaux/medic",       "Médecin"},     //1 ---> Fix medigun regenscreen & IA
    { 100,  130,  75,  130, "chapeaux/aventurier",  "Américain"},   //2 ---> OK
    { 80,   80,   75,   80, "chapeaux/bandana",     "Ninja"},       //3 ---> OK
    { 100,  50,  100,  100, "chapeaux/cornes",      "Vampire"},      //4 ---> OK (Regen screen ?)
    { 95,   75,   90,  100, "chapeaux/magicien",    "Magicien"},    //5 ---> OK
    { 100,  100,  80,   70, "chapeaux/japonais",    "Kamikaze"},    //6 ---> OK
    { 100,  80,   90,  110, "chapeaux/crane",       "Faucheuse"},   //7 ---> OK
    { 90,   80,   80,  110, "chapeaux/graduation",  "Physicien"},   //8 ---> OK
    { 100,  60,  125,  150, "chapeaux/tente",       "Campeur"},     //9 ---> OK
    { 115,  115, 115,   85, "chapeaux/kepi",        "Commando"},    //10 --> OK
    {  80,  105,  80,   90, "chapeaux/saint",       "Prêtre"},      //11 --> OK
    { 100,  120,  60,  115, "chapeaux/viking",      "Viking"},      //12 --> OK
    { 100,  110,  85,  120, "chapeaux/champignon",  "Junkie"},      //13 --> OK
};

static const struct aptisortsinfo { const char *tex1, *tex2, *tex3; int mana1, mana2, mana3, duree1, duree2, duree3, reload1, reload2, reload3, sound1, sound2, sound3; } sorts[] =
{
    { "media/interface/hud/sortmage1.png", "media/interface/hud/sortmage2.png", "media/interface/hud/sortmage3.png",                    30, 40, 60,  250, 5000, 3000, 2000, 9000, 6000,  S_SORTMAGE1, S_SORTMAGE2, S_SORTMAGE3},        // Magicien
    { "media/interface/hud/sortphysicien1.png", "media/interface/hud/sortphysicien2.png", "media/interface/hud/sortphysicien3.png",     45, 50, 65, 2000, 4000, 6000, 3000, 5000, 9000,  S_SORTPHY1, S_SORTPHY2, S_SORTPHY3},           // Physicien
    { "media/interface/hud/sortpretre1.png", "media/interface/hud/sortpretre2.png", "media/interface/hud/sortpretre3.png",              20, 30, 80, 4000, 5000, 4000, 8000, 5000, 10000, S_SORTPRETRE1, S_SORTPRETRE2, S_SORTPRETRE3},  // Prêtre
};

//Définition des boucliers
static const struct shieldsinfo { const char *bois, *fer, *gold, *magnetique, *hudbois, *hudfer, *hudgold, *hudmagnetique; } shields[] =
{
    {"worldshield/bois/100", "worldshield/fer/100", "worldshield/or/100", "worldshield/magnetique/100", "hudshield/bois/100", "hudshield/fer/100", "hudshield/or/100", "hudshield/magnetique/100"},
    {"worldshield/bois/80",  "worldshield/fer/80",  "worldshield/or/80",  "worldshield/magnetique/80",  "hudshield/bois/80",  "hudshield/fer/80",  "hudshield/or/80",  "hudshield/magnetique/80"},
    {"worldshield/bois/60",  "worldshield/fer/60",  "worldshield/or/60",  "worldshield/magnetique/60",  "hudshield/bois/60",  "hudshield/fer/60",  "hudshield/or/60",  "hudshield/magnetique/60"},
    {"worldshield/bois/40",  "worldshield/fer/40",  "worldshield/or/40",  "worldshield/magnetique/40",  "hudshield/bois/40",  "hudshield/fer/40",  "hudshield/or/40",  "hudshield/magnetique/40"},
    {"worldshield/bois/20",  "worldshield/fer/20",  "worldshield/or/20",  "worldshield/magnetique/20" , "hudshield/bois/20",  "hudshield/fer/20",  "hudshield/or/20",  "hudshield/magnetique/20"},
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
    {"smileys/sournois", "capes/cape_ruinee",       "capes/cape_ruinee/orange"},
    {"smileys/fou",      "capes/cape_weed",         "capes/cape_weed/orange"},
    {"smileys/cool",     "capes/cape_larry",        "capes/cape_larry/orange"},
    {"smileys/bug",      "capes/cape_high",         "capes/cape_high/orange"},
    {"",                 "capes/cape_spartiate",    "capes/cape_spartiate/orange"},
    {"",                 "capes/cape_risitasbg",    "capes/cape_risitasbg/orange"},
};

extern void addxp(int nbxp);
extern void addstat(int valeur, int stat);
extern void writesave();

//Messages de kill
extern bool suicided;
extern string str_pseudotueur, str_armetueur;
extern int n_aptitudetueur, n_aptitudevictime;

//HUD
extern int message1, message2, message3, ctfmessage1, ctfmessage2, ctfmessage3, ctfmessage4, ctfmessage5, ctfmessage6;  //Messages HUD
extern int zoomfov, zoom, crosshairsize; //HUD Zoom
extern float crosshairalpha, pourcents;
extern int ccxp, cclvl, needxp, oldneed, neededxp;

extern float weapposside, weapposup, maxweapposside, maxweapposup, shieldside, maxshieldside; //Visée à la mire
extern int nbfps;

extern int parallaxX, parallaxY; //Effet parallax des menus

extern float wateramplitude;

//Statistiques & sauvegarde
enum {STAT_KILLS, STAT_MORTS, STAT_KILLSTREAK, STAT_BOUCLIERBOIS, STAT_BOUCLIERFER, STAT_BOUCLIEROR, STAT_BOUCLIERMAGNETIQUE,
        STAT_PANACHAY, STAT_MANA, STAT_COCHON, STAT_STEROS, STAT_EPO, STAT_JOINT, STAT_CHAMPIS, NUMSTATS};
extern int stat[NUMSTATS];

extern int ccxp, cclvl, needxp, oldneed, neededxp;
extern float pourcents;

extern void addstat(int valeur, int stat);
extern float menustat(int value);

extern void writesave();
extern void addxp(int nbxp);
