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
static struct aptitudesinfo { int apt_degats, apt_resistance, apt_precision, apt_vitesse; const char *apt_nom; } aptitudes[] =
{
    { 100,  100, 100,  100, "Soldat"},      //0 ---> OK
    { 80,   75,  100,  100, "Médecin"},     //1 ---> Fix medigun regenscreen & IA
    { 100,  130,  75,  130, "Américain"},   //2 ---> OK
    { 80,   80,   75,   80, "Ninja"},       //3 ---> OK
    { 100,  50,  100,  100, "Vampire"},     //4 ---> OK (Regen screen ?)
    { 90,   75,   90,  100, "Magicien"},    //5
    { 100,  100,  85,   70, "Kamikaze"},    //6 ---> OK
    { 100,  80,   90,  110, "Faucheuse"},   //7 ---> OK
    { 90,   80,   80,  110, "Physicien"},   //8
    { 100,  60,  125,  150, "Campeur"},     //9 ---> OK
    { 120,  110, 110,   85, "Commando"},    //10
    { 90,   90,   80,  120, "Prêtre"},      //11
    { 120,  120,  60,  115, "Viking"},      //12---> OK
    { 85,   80,   90,  110, "Junkie"},      //13
};

//Messages de kill
extern bool suicided;
extern string str_pseudotueur, str_armetueur;
extern int n_aptitudetueur, n_aptitudevictime;

//HUD
extern int message1, message2, message3;  //Messages HUD
extern int gamemillismsg; //Décompte HUD
extern int zoomfov, zoom, crosshairsize; //HUD Zoom
extern float crosshairalpha;

extern float weapposside, weapposup, maxweapposside, maxweapposup, shieldside, maxshieldside; //Visée à la mire
extern int nbfps;

extern int parallaxX, parallaxY; //Effet parallax des menus
