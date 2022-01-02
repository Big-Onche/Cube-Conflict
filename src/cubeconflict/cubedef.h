#include "../game/game.h"

//Définition des aptitudes
enum {APT_SOLDAT = 0, APT_MEDECIN, APT_AMERICAIN, APT_NINJA, APT_VAMPIRE, APT_MAGICIEN, APT_KAMIKAZE, APT_FAUCHEUSE, APT_PHYSICIEN, APT_CAMPEUR, APT_COMMANDO, APT_PRETRE, APT_VICKING, APT_JUNKIE, APT_INDIEN, NUMAPTS};

static const struct aptitudesinfo { int apt_degats, apt_resistance, apt_precision, apt_vitesse; const char *apt_tete, *apt_nomFR, *apt_nomEN, *apt_logo; } aptitudes[NUMAPTS] =
{
    { 105,  105,  105,   975, "chapeaux/casquette",  "Soldat",       "Soldier",      "media/interface/hud/soldat.jpg"},     //0
    {  90,  110,   95,  1025, "chapeaux/medic",      "Médecin",      "Medic",        "media/interface/hud/medecin.jpg"},    //1
    { 100,  135,   80,  1150, "chapeaux/aventurier", "Américain",    "American",     "media/interface/hud/americain.jpg"},  //2
    {  85,   90,   75,   850, "chapeaux/bandana",    "Ninja",        "Ninja",        "media/interface/hud/ninja.jpg"},      //3
    { 110,   65,  110,  1050, "chapeaux/cornes",     "Vampire",      "Vampire",      "media/interface/hud/vampire.jpg"},    //4
    { 100,   80,   90,  1000, "chapeaux/magicien",   "Magicien",     "Wizard",       "media/interface/hud/magicien.jpg"},   //5
    { 100,  100,   70,   875, "chapeaux/japonais",   "Kamikaze",     "Kamikaze",     "media/interface/hud/kamikaze.jpg"},   //6
    { 100,   85,   90,  1050, "chapeaux/crane",      "Faucheuse",    "Reaper",       "media/interface/hud/faucheuse.jpg"},  //7
    {  90,   85,   85,  1050, "chapeaux/graduation", "Physicien",    "Physicist",    "media/interface/hud/physicien.jpg"},  //8
    { 100,   60,  125,  1250, "chapeaux/tente",      "Campeur",      "Camper",       "media/interface/hud/campeur.jpg"},    //9
    { 110,  110,  110,   950, "chapeaux/kepi",       "Commando",     "Commando",     "media/interface/hud/commando.jpg"},   //10
    {  85,  105,   85,   950, "chapeaux/saint",      "Prêtre",       "Priest",       "media/interface/hud/pretre.jpg"},     //11
    { 100,  120,   60,  1075, "chapeaux/viking",     "Viking",       "Viking",       "media/interface/hud/viking.jpg"},     //12
    { 100,  110,   85,  1100, "chapeaux/champignon", "Junkie",       "Junkie",       "media/interface/hud/dealer.png"},     //13
    { 100,  100,   75,  1000, "chapeaux/indien",     "Shoshone",     "Shoshone",     "media/interface/hud/indien.jpg"},     //14
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
static const struct smileysinfo { const char *smileydir, *smileyname; int smileyprice; } customssmileys[] =
{
    {"smileys/hap",      "Hap",          0},
    {"smileys/noel",     "Noel",       100},
    {"smileys/malade",   "Malade",     100},
    {"smileys/content",  "Content",    100},
    {"smileys/colere",   "Colère",     250},
    {"smileys/sournois", "Sournois",   250},
    {"smileys/fou",      "Fou",        250},
    {"smileys/cool",     "Cool",       500},
    {"smileys/bug",      "Bug",        500},
};
enum {SMI_HAP = 0, SMI_NOEL, SMI_MALADE, SMI_CONTENT, SMI_COLERE, SMI_SOURNOIS, SMI_FOU, SMI_COOL, SMI_BUG, NUMSMILEYS};
extern int buyedsmileys[NUMSMILEYS];

static const struct capesinfo { const char *team1capedir, *team2capedir, *capename; int capeprice; } customscapes[] =
{
    {"capes/cape_noob",         "capes/cape_noob/orange",       "Noob",              0},
    {"capes/cape_jvc",          "capes/cape_jvc/orange",        "JVC",             100},
    {"capes/cape_coroned",      "capes/cape_coroned/orange",    "Coroned",         100},
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
    {"capes/cape_riche",        "capes/cape_riche/orange",      "Riche",          2000},
};
enum {CAP_CUBE = 0, NUMCAPES};
extern int buyedcapes[NUMCAPES];

static const struct tombesinfo { const char *tombedir, *tombename; int tombeprice; } customstombes[] =
{
    {"tombes/basique",    "Basique",          0},
    {"tombes/merde",      "Merde",           50},
    {"tombes/fleur",      "Fleur",          100},
    {"tombes/cristal",    "Cristal",        100},
    {"tombes/ballon",     "Ballon de foot", 100},
    {"tombes/fuck",       "Fuck",           250},
    {"tombes/monument",   "Monument",       500},
    {"tombes/oeil",       "Oeil",           500},
    {"tombes/couronne",   "Couronne",       500},
    {"tombes/lingots",    "Lingots",       2000},
};
enum {TOM_POOP = 0, NUMTOMBES};
extern int buyedtombes[NUMTOMBES];

//HUD
extern bool suicided;
extern string str_pseudotueur, str_armetueur;
extern int n_aptitudetueur, n_aptitudevictime;

extern int message1, message2, message3, ctfmessage1, ctfmessage2, ctfmessage3, ctfmessage4, ctfmessage5, ctfmessage6;
extern int lastshoot;
extern int zoomfov, zoom, crosshairsize;
extern float crosshairalpha, pourcents;

extern float weapposside, weapposup, maxweapposside, maxweapposup, shieldside, maxshieldside;
extern int nbfps;

extern int parallaxX, parallaxY;

//Ambiances aléatoires
extern int randomambience;
extern float wateramplitude;

//Statistiques & sauvegarde & config élémentaire
extern bool usesteam;
extern string pseudoaleatoire;
extern void genpseudo(bool forcename, int langue);
extern bool conserveurofficiel;
extern int langage, UI_menutabs, UI_tombe, UI_custtab;

extern void loadsave();
extern void writesave();

extern void addxpandcc(int nbxp, int nbcc = 0);
extern int cclvl, needxp, oldneed, neededxp;
extern float pourcents;

extern void addstat(int valeur, int stat);
extern float menustat(int value);
enum {STAT_CC, STAT_XP, STAT_KILLS, STAT_MORTS, STAT_KILLSTREAK, STAT_BOUCLIERBOIS, STAT_BOUCLIERFER, STAT_BOUCLIEROR, STAT_BOUCLIERMAGNETIQUE,
        STAT_PANACHAY, STAT_MANA, STAT_COCHON, STAT_STEROS, STAT_EPO, STAT_JOINT, STAT_CHAMPIS, STAT_ARMES, STAT_SUPERARMES,
        STAT_DRAPEAUX, STAT_WINS, STAT_ARMUREASSIST, STAT_TPSSEC, STAT_TPSMIN, STAT_TPSH, NUMSTATS};
extern int stat[NUMSTATS];

extern void getsteamachievements();
extern void unlockachievement(int achID);
enum {ACH_TRIPLETTE = 0, ACH_PENTAPLETTE, ACH_DECAPLETTE, ACH_ATOME, ACH_WINNER, ACH_ENVOL, ACH_POSTULANT, ACH_STAGIAIRE,
        ACH_SOLDAT, ACH_LIEUTENANT, ACH_MAJOR, ACH_BEAUTIR, ACH_DEFONCE, ACH_PRECIS, ACH_KILLASSIST, ACH_KILLER, ACH_SACAPV,
        ACH_CADENCE, ACH_1HPKILL, ACH_MAXSPEED, ACH_INCREVABLE, ACH_CHANCE, ACH_CPASBIEN, ACH_SUICIDEFAIL, ACH_FUCKYEAH, NUMACHS};
extern bool succes[NUMACHS];

static const struct achinfo { const char *achname, *achnicenameFR, *achnicenameEN, *achdescFR, *achdescEN; } achievements[] =
{
    {"ACH_TRIPLETTE",     "Triple menace",               "Triple threat",            "Tuer 3 ennemis sans mourrir",                                         "Kill 3 enemies without dying"},
    {"ACH_PENTAPLETTE",   "Terreur",                     "Terror",                   "Tuer 5 ennemis sans mourrir",                                         "Kill 5 enemies without dying"},
    {"ACH_DECAPLETTE",    "Invincible !",                "Invincible !",             "Tuer 10 ennemis sans mourrir",                                        "Kill 10 enemies without dying"},
    {"ACH_ATOME",         "La puissance de l'atome",     "The power of the atom",    "Balancer un missile nucléaire",                                       "Launch a nuclear missile"},
    {"ACH_WINNER",        "Winner",                      "Winner",                   "Remporter 1 partie",                                                  "Win 1 game"},
    {"ACH_ENVOL",         "Envol pour le paradis",       "Fly like an eagle",        "Rester au moins 7 secondes dans les airs",                            "Stay at least 7 seconds in the air"},
    {"ACH_POSTULANT",     "Postulant",                   "Applicant",                "Atteindre le niveau 5",                                               "Reach level 5"},
    {"ACH_STAGIAIRE",     "Stagiaire",                   "Trainee",                  "Atteindre le niveau 10",                                              "Reach level 10"},
    {"ACH_SOLDAT",        "Soldat",                      "Soldier",                  "Atteindre le niveau 20",                                              "Reach level 20"},
    {"ACH_LIEUTENANT",    "Lieutenant",                  "Lieutenant",               "Atteindre le niveau 50 (C'est pas mal !)",                            "Reach level 50 (Nice!)"},
    {"ACH_MAJOR",         "Major",                       "Major",                    "Atteindre le niveau 100 (Tu as supporté le jeu jusqu'ici !?)",        "Reach level 100 (that's a lot of grind!)"},
    {"ACH_BEAUTIR",       "Beau tir !",                  "Nice shoot !",             "Tuer quelqu'un à au moins 100 mètres de distance",                    "Kill someone at least 100 meters away"},
    {"ACH_DEFONCE",       "Complètement défoncé",        "Completely stoned",        "Utiliser en même temps le joint, l'EPO, les champis et les stéros",   "Use joint, EPO, shrooms and steroids at the same time"},
    {"ACH_PRECIS",        "Précis comme un boucher",     "Accurate like a butcher",  "50% de précision sur une partie avec au moins 10 éliminations",       "Achieve 50% accuracy with at least 10 kills in a game"},
    {"ACH_KILLASSIST",    "Et ça fait bim, bam, boum",   "Boom, Boom, Boom, Boom",   "Tuer quelqu'un avec l'explosion de ton armure assistée",              "Kill an enemy with your power armor explosion"},
    {"ACH_KILLER",        "Tueur en série",              "Serial killer",            "Tuer 30 ennemis en une seule partie",                                 "Kill 30 enemies in a game"},
    {"ACH_SACAPV",        "Sac à points de vie",         "Too much health points",   "Avoir 200 points de vie (oui c'est possible !)",                      "Have 200 health points (Hint: it's possible!)"},
    {"ACH_CADENCE",       "Le combo ultime !",           "The ultimate combo!",      "Atteindre la cadence de tir la plus élevée possible du jeu",          "Have the highest possible rate of fire in the game"},
    {"ACH_1HPKILL",       "A 1 PV du ragequit !",        "At 1 HP from ragequit",    "Tuer un ennemi en ayant 1 seul point de vie",                         "Kill an enemy with only 1 health point remaining"},
    {"ACH_MAXSPEED",      "Speedy Gonzales",             "Speedy Gonzales",          "Se déplacer à la vitesse la plus rapide possible du jeu",             "Reach the highest speed in the game"},
    {"ACH_INCREVABLE",    "Increvable !",                "Indestructible!",          "Mourir 5 fois maximum dans une partie avec au moins 10 éliminations",       "Only die 5 times in a game with at least 10 kills"},
    {"ACH_CHANCE",        "Larry Silverstein",           "Lucky Larry",              "Avoir la chance d'obtenir une super-arme par hasard, quel bol !",     "Obtain a superweapon on respawn"},
    {"ACH_CPASBIEN",      "C'est pas bien",              "That's not nice",          "Tuer un allié",                                                       "Kill an ally"},
    {"ACH_SUICIDEFAIL",   "Suicide manqué",              "Failed suicide",           "Rester en vie après avoir utilisé une ceinture d'explosifs",          "Stay alive after using your explosives belt"},
    {"ACH_FUCKYEAH",      "Fuck yeah !",                 "Fuck yeah !",              "Tuer un Shoshone en étant Américain",                                 "Kill a Shoshone with the American class"},
};
