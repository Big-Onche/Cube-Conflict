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

static struct aptitudesinfo { int apt_degats, apt_resistance, apt_precision, apt_vitesse; const char *apt_nom; } aptitudes[NUMAPTS] =
{
    { 100,  100, 100,  100, "Soldat"},      //0 ---> OK
    { 80,   75,  100,  100, "Médecin"},     //1 ---> Fix medigun regenscreen & IA
    { 100,  130,  75,  130, "Américain"},   //2 ---> OK
    { 80,   80,   75,   80, "Ninja"},       //3 ---> OK
    { 100,  50,  100,  100, "Vampire"},     //4 ---> OK (Regen screen ?)
    { 95,   75,   90,  100, "Magicien"},    //5 ---> OK
    { 100,  100,  80,   70, "Kamikaze"},    //6 ---> OK
    { 100,  80,   90,  110, "Faucheuse"},   //7 ---> OK
    { 90,   80,   80,  110, "Physicien"},   //8 ---> OK
    { 100,  60,  125,  150, "Campeur"},     //9 ---> OK
    { 120,  110, 110,   85, "Commando"},    //10
    { 90,   90,   80,  120, "Prêtre"},      //11
    { 100,  120,  60,  115, "Viking"},      //12 --> OK
    { 100,  110,  85,  120, "Junkie"},      //13 --> OK
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
static const struct custominfo { const char *chapeau, *smiley, *capeteam1, *capeteam2, *custtombe; } customs[] =
{
    {}, //0 = Rien
    {"chapeaux/sombrero",   "smileys/hap",      "capes/Cape_JVC",           "capes/Cape_JVC/orange",            "tombes/basique"},
    {"chapeaux/lapin",      "smileys/noel",     "capes/Cape_Cisla",         "capes/Cape_Cisla/orange",          "tombes/fleur"},
    {"chapeaux/aureole",    "smileys/malade",   "capes/Cape_Tabasco",       "capes/Cape_Tabasco/orange",        "tombes/fuck"},
    {"chapeaux/cornes",     "smileys/content",  "capes/Cape_CubeEngine",    "capes/Cape_CubeEngine/orange",     "tombes/monument"},
    {"chapeaux/joker",      "smileys/colere",   "capes/Cape_Cislattack",    "capes/Cape_Cislattack/orange",     "tombes/cristal"},
    {"chapeaux/champignon", "smileys/sournois", "capes/Cape_Ruinee",        "capes/Cape_Ruinee/orange"},
    {"chapeaux/couronne",   "smileys/fou",      "capes/Cape_Weed",          "capes/Cape_Weed/orange"},
    {"chapeaux/heaume",     "smileys/cool",     "capes/Cape_Diable",        "capes/Cape_Diable/orange"},
    {"chapeaux/bandana",    "smileys/bug",      "capes/Cape_High",          "capes/Cape_High/orange"},
    {"chapeaux/melon",      "",                 "capes/Cape_Quenelle",      "capes/Cape_Quenelle/orange"},
    {"chapeaux/casque",     "",                 "capes/Cape_Poulet",        "capes/Cape_Poulet/orange"},
    {"chapeaux/helices"},
    {"chapeaux/aventurier"},
    {"chapeaux/bug"},
    {"chapeaux/lapin"},
};

extern void addxp(int nbxp);
extern void addstat(int valeur, int stat);
extern void writesave();

//Messages de kill
extern bool suicided;
extern string str_pseudotueur, str_armetueur;
extern int n_aptitudetueur, n_aptitudevictime;

//HUD
extern int message1, message2, message3;  //Messages HUD
extern int zoomfov, zoom, crosshairsize; //HUD Zoom
extern float crosshairalpha, pourcents;
extern int ccxp, cclvl, needxp, oldneed, neededxp;

extern float weapposside, weapposup, maxweapposside, maxweapposup, shieldside, maxshieldside; //Visée à la mire
extern int nbfps;

extern int parallaxX, parallaxY; //Effet parallax des menus

//Statistiques & sauvegarde
enum {STAT_KILLS, STAT_MORTS, STAT_KILLSTREAK, STAT_BOUCLIERBOIS, STAT_BOUCLIERFER, STAT_BOUCLIEROR, STAT_BOUCLIERMAGNETIQUE};

extern int ccxp, cclvl, needxp, oldneed, neededxp;
extern float pourcents;

extern int stat_kills, stat_morts, stat_killstreak, stat_bouclierbois, stat_bouclierfer, stat_bouclieror, stat_boucliermagnetique;

extern void addstat(int valeur, int stat);
extern float menustat(int value);

extern void writesave();
extern void addxp(int nbxp);
