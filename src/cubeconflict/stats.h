extern int xpneededfornextlvl, totalneededxp;
extern float pourcents;

extern void loadsave();
extern void writesave();
extern void addstat(int valeur, int stat, bool rewrite = false);
extern void addxpandcc(int nbxp, int nbcc = 0);

//////////////////////////////////////// Statistiques | Player stats ////////////////////////////////////////
enum {STAT_CC, STAT_XP, STAT_LEVEL, STAT_KILLS, STAT_MORTS, STAT_KDRATIO, STAT_DAMMAGERECORD, STAT_KILLSTREAK, STAT_MAXKILLDIST, STAT_WINS, STAT_SUICIDES, STAT_ALLIESTUES, STAT_TIMEPLAYED, STAT_DRAPEAUXENVOL, STAT_DRAPEAUXENRAP, STAT_DRAPEAUXALYREC, //Main game stats
        STAT_ABILITES, STAT_HEALTHREGEN, STAT_HEALTHREGAIN, STAT_MANAREGEN, STAT_MANAREGAIN, //Classes
        STAT_BOUCLIERBOIS, STAT_BOUCLIERFER, STAT_BOUCLIEROR, STAT_BOUCLIERMAGNETIQUE, STAT_ARMUREASSIST, STAT_REPASSIST, //Shields
        STAT_PANACHAY, STAT_MANA, STAT_COCHON, STAT_STEROS, STAT_EPO, STAT_JOINT, STAT_CHAMPIS, STAT_ARMES, STAT_SUPERARMES, //Objects
        STAT_ATOM, STAT_MUNSHOOTED, STAT_TOTALDAMAGEDEALT, STAT_TOTALDAMAGERECIE, //Stupid statistics
        STAT_EMPTY1, STAT_EMPTY2, STAT_EMPTY3, STAT_EMPTY4, STAT_EMPTY5, STAT_EMPTY6, STAT_EMPTY7, STAT_EMPTY8, STAT_EMPTY9, STAT_EMPTY10, //If we need to add new stat without destroying all user's saves
        NUMSTATS};
extern int stat[NUMSTATS];

static const struct statsinfo { const char *statname, *statnicenameFR, *statnicenameEN, *statlogo; } statslist[] =
{
    //Steam name                //French description                //English description               //Stat logo
    //Main game stats
    {"STAT_CC",                 "CubeCoins",                        "CubeCoins",                        "media/interface/hud/cislacoins.png"},  //0
    {"STAT_XP",                 "XP",                               "XP",                               "media/interface/hud/stats.png"},
    {"STAT_LEVEL",              "Niveau",                           "Level",                            "media/interface/hud/stats.png"},
    {"STAT_KILLS",              "Éliminations",                     "Frags",                            "media/interface/hud/flingue.jpg"},
    {"STAT_MORTS",              "Morts",                            "Deaths",                           "media/interface/hud/tombe.png"},
    {"STAT_KDRATIO",            "Ratio morts/éliminations",         "Kills/Deaths ratio",               "media/interface/hud/stats.png"},       //Calculated in calcratio() with STAT_KILLS & STAT_MORTS then called in getstatinfo() STAT_KDRATIO not saved because float shit.
    {"STAT_DAMMAGERECORD",      "Record de dommages en une partie", "Damage record in a single match",  "media/interface/hud/stats.png"},
    {"STAT_KILLSTREAK",         "Meilleure série d'éliminations",   "Best killstreak",                  "media/interface/hud/rage.png"},
    {"STAT_MAXKILLDIST",        "Elimination la plus éloignée",     "Farthest frag",                    "media/interface/hud/campeur.png"},
    {"STAT_WINS",               "Victoires",                        "Victories",                        "media/interface/hud/cool.jpg"},
    {"STAT_SUICIDES",           "Suicides",                         "Suicides",                         "media/interface/hud/fou.jpg"},             //10
    {"STAT_ALLIESTUES",         "Alliés tués",                      "Killed allies",                    "media/interface/hud/sournois_red.jpg"},
    {"STAT_TIMEPLAYED",         "Temps de jeu",                     "Time played",                      "media/interface/hud/chrono.png"},      //Calculated in secs dotime with STAT_TIMEPLAYED, calculated in HH:MM:SS for display in getstatinfo()
    {"STAT_DRAPEAUXENVOL",      "Drapeaux ennemis volés",           "Stolen enemy flags",               "media/interface/hud/drapeau_ennemi.png"},
    {"STAT_DRAPEAUXENRAP",      "Drapeaux ennemis remportés",       "Enemy flags won",                  "media/interface/hud/drapeau_ennemi.png"},
    {"STAT_DRAPEAUXALYREC",     "Drapeaux alliés récupérés",        "Allied flags recovered",           "media/interface/hud/drapeau_allie.png"},
    //Classes
    {"STAT_ABILITES",           "Abilitées utilisées",              "Used abilities",                   "media/interface/hud/stats.png"},
    {"STAT_HEALTHREGEN",        "Santé redonnée aux alliés",        "Health restored to allies",        "media/interface/hud/medecin.jpg"},
    {"STAT_HEALTHREGAIN",       "Santé récupérée grâce aux médecins", "Health recovered with medics",   "media/interface/hud/coeur.png"},
    {"STAT_MANAREGEN",          "Mana redonné aux alliés",          "Mana restored to allies",          "media/interface/hud/mana.png"},
    {"STAT_MANAREGAIN",         "Mana récupéré grâce aux junkies",  "Mana recovered with junkies",      "media/interface/hud/dealer.png"},           //20
    //Shields
    {"STAT_BOUCLIERBOIS",       "Boucliers en bois utilisés",       "Wooden shields used",              "media/interface/hud/bouclier_bois.png"},
    {"STAT_BOUCLIERFER",        "Boucliers en fer utilisés",        "Iron shields used",                "media/interface/hud/bouclier_fer.png"},
    {"STAT_BOUCLIEROR",         "Boucliers en or utilisés",         "Gold shields used",                "media/interface/hud/bouclier_or.png"},
    {"STAT_BOUCLIERMAGNETIQUE", "Boucliers magnétiques utilisés",   "Magnetic shields used",            "media/interface/hud/bouclier_magnetique.png"},
    {"STAT_ARMUREASSIST",       "Armures assistées utilisés",       "Power armors used",                "media/interface/hud/robot.png"},
    {"STAT_REPASSIST",          "Réparations d'armure assistée",    "Power armor repairs",              "media/interface/hud/options.jpg"},
    //Objects
    {"STAT_PANACHAY",           "Panachays consommés",              "Beers drunk",                      "media/interface/hud/coeur.png"},
    {"STAT_MANA",               "Potions de mana consommées",       "Mana potions consumed",            "media/interface/hud/mana.png"},
    {"STAT_COCHON",             "Cochons grillés mangés",           "Grilled pigs eaten",               "media/interface/hud/stats.png"},
    {"STAT_STEROS",             "Cures de stéroïdes",               "Steroids cycles",                  "media/interface/hud/steros.png"},           //30
    {"STAT_EPO",                "Piqures d'EPO",                    "EPO shots",                        "media/interface/hud/epo.png"},
    {"STAT_JOINT",              "Joints fumés",                     "Smoked joints",                    "media/interface/hud/joint.png"},
    {"STAT_CHAMPIS",            "Champignons mangés",               "Shrooms eaten",                    "media/interface/hud/champis.png"},
    {"STAT_ARMES",              "Armes ramassées",                  "Picked up weapons",                "media/interface/hud/chargeur.png"},
    {"STAT_SUPERARMES",         "Super-caisses ramassées",          "Picked Up Super Crates",           "media/interface/hud/stats.png"},
    //Stupid statistics
    {"STAT_ATOM",               "Bombes atomiques tirées",          "Amount of atom bomb fired",        "media/interface/hud/stats.png"},
    {"STAT_MUNSHOOTED",         "Munitions tirées au total",        "Amount of ammo fired",             "media/interface/hud/stats.png"},
    {"STAT_TOTALDAMAGEDEALT",   "Dommages infligés au total",       "Amount of damage dealt",           "media/interface/hud/stats.png"},
    {"STAT_TOTALDAMAGERECIE",   "Dommages reçus au total",          "Amount of damage recieved",        "media/interface/hud/stats.png"},
};

//////////////////////////////////////// Succès | Achievements ////////////////////////////////////////
extern void getsteamachievements();
extern void unlockachievement(int achID);

enum {ACH_TRIPLETTE = 0, ACH_PENTAPLETTE, ACH_DECAPLETTE, ACH_ATOME, ACH_WINNER, ACH_ENVOL, ACH_POSTULANT, ACH_STAGIAIRE,
        ACH_SOLDAT, ACH_LIEUTENANT, ACH_MAJOR, ACH_BEAUTIR, ACH_DEFONCE, ACH_PRECIS, ACH_KILLASSIST, ACH_KILLER, ACH_SACAPV,
        ACH_CADENCE, ACH_1HPKILL, ACH_MAXSPEED, ACH_INCREVABLE, ACH_CHANCE, ACH_CPASBIEN, ACH_SUICIDEFAIL, ACH_FUCKYEAH, ACH_RICHE,
        ACH_TUEURFANTOME, ACH_EPOFLAG, ACH_M32SUICIDE, ACH_ESPIONDEGUISE, ACH_FUCKYOU, ACH_ABUS, ACH_DESTRUCTEUR, ACH_RAGE,
        ACH_DAVIDGOLIATH, ACH_LANCEEPO, ACH_PASLOGIQUE, ACH_JUSTEPOUR, ACH_BRICOLEUR, ACH_NOSCOPE, ACH_THUGPHYSIQUE, ACH_SPAAACE, NUMACHS};
extern bool succes[NUMACHS];

static const struct achinfo { const char *achname, *achnicenameFR, *achnicenameEN, *achdescFR, *achdescEN; } achievements[NUMACHS] =
{
    //Steam name          //French name                  //English name                 //French description                                                    //English description
    {"ACH_TRIPLETTE",     "Triple menace",               "Triple threat",               "Tuer 3 ennemis sans mourrir.",                                          "Kill 3 enemies without dying."},
    {"ACH_PENTAPLETTE",   "Terreur",                     "Terror",                      "Tuer 5 ennemis sans mourrir.",                                          "Kill 5 enemies without dying."},
    {"ACH_DECAPLETTE",    "Invincible !",                "Invincible !",                "Tuer 10 ennemis sans mourrir.",                                         "Kill 10 enemies without dying."},
    {"ACH_ATOME",         "La puissance de l'atome",     "The power of the atom",       "Balancer un missile nucléaire.",                                        "Launch a nuclear missile."},
    {"ACH_WINNER",        "Winner",                      "Winner",                      "Remporter 1 partie.",                                                   "Win 1 game."},
    {"ACH_ENVOL",         "Envol pour le paradis",       "Fly like an eagle",           "Rester au moins 7 secondes dans les airs.",                             "Stay at least 7 seconds in the air."},
    {"ACH_POSTULANT",     "Postulant",                   "Applicant",                   "Atteindre le niveau 5.",                                                "Reach level 5."},
    {"ACH_STAGIAIRE",     "Stagiaire",                   "Trainee",                     "Atteindre le niveau 10.",                                               "Reach level 10."},
    {"ACH_SOLDAT",        "Soldat",                      "Soldier",                     "Atteindre le niveau 20.",                                               "Reach level 20."},
    {"ACH_LIEUTENANT",    "Lieutenant",                  "Lieutenant",                  "Atteindre le niveau 50 (C'est pas mal !)",                              "Reach level 50 (Nice!)"},
    {"ACH_MAJOR",         "Major",                       "Major",                       "Atteindre le niveau 100 (Tu as supporté le jeu jusqu'ici !?)",          "Reach level 100 (that's a lot of grind!)"},
    {"ACH_BEAUTIR",       "Beau tir !",                  "Nice shoot!",                 "Tuer quelqu'un à au moins 100 mètres de distance.",                     "Kill someone at least 100 meters away."},
    {"ACH_DEFONCE",       "Complètement défoncé",        "Completely stoned",           "Utiliser en même temps le joint, l'EPO, les champis et les stéros.",    "Use joint, EPO, shrooms and steroids at the same time."},
    {"ACH_PRECIS",        "Précis comme un boucher",     "Accurate like a butcher",     "50% de précision sur une partie avec au moins 10 éliminations.",        "Achieve 50% accuracy with at least 10 kills in a game."},
    {"ACH_KILLASSIST",    "Armure.exe ne répond pas",    "Armor.exe is not responding", "Tuer quelqu'un avec l'explosion de ton armure assistée.",               "Kill an enemy with your power armor explosion."},
    {"ACH_KILLER",        "Tueur en série",              "Serial killer",               "Tuer 30 ennemis en une seule partie.",                                  "Kill 30 enemies in a game."},
    {"ACH_SACAPV",        "Sac à points de vie",         "Too much health points",      "Avoir 200 points de vie (oui c'est possible !)",                        "Have 200 health points (Hint: it's possible!)"},
    {"ACH_CADENCE",       "Le combo ultime !",           "The ultimate combo!",         "Atteindre la cadence de tir la plus élevée possible du jeu.",           "Have the highest possible rate of fire in the game."},
    {"ACH_1HPKILL",       "A 1 PV du ragequit !",        "At 1 HP from ragequit",       "Tuer un ennemi en ayant 1 seul point de vie.",                          "Kill an enemy with only 1 health point remaining."},
    {"ACH_MAXSPEED",      "Speedy Gonzales",             "Speedy Gonzales",             "Se déplacer à la vitesse la plus rapide possible du jeu.",              "Reach the highest speed in the game."},
    {"ACH_INCREVABLE",    "Increvable !",                "Indestructible!",             "Mourir 5 fois maximum dans une partie avec au moins 10 éliminations.",  "Only die 5 times in a game with at least 10 kills."},
    {"ACH_CHANCE",        "Larry Silverstein",           "Lucky Larry",                 "Avoir la chance d'obtenir une super-arme par hasard, quel bol !",      "Obtain a superweapon on respawn."},
    {"ACH_CPASBIEN",      "C'est pas bien",              "That's not nice",             "Tuer un allié.",                                                        "Kill an ally."},
    {"ACH_SUICIDEFAIL",   "Suicide manqué",              "Failed suicide",              "Rester en vie après avoir utilisé une ceinture d'explosifs.",           "Stay alive after using your explosives belt."},
    {"ACH_FUCKYEAH",      "Fuck yeah !",                 "Fuck yeah !",                 "Tuer un Shoshone en étant Américain.",                                  "Kill a Shoshone with the American class."},
    {"ACH_RICHE",         "Capitaliste",                 "Capitalist",                  "Avoir 1500 CC en réserve.",                                             "Have 1500 CC in reserve."},
    {"ACH_TUEURFANTOME",  "Tueur de l'au-delà",          "Slayer from beyond",          "Tuer un ennemi en étant mort.",                                         "Kill an enemy while being dead."},
    {"ACH_EPOFLAG",       "C'est à ça que ça sert !",    "That's what it's made for!",  "Ramenez un drapeau ennemi en étant boosté par l'EPO.",                  "Bring back an ennemy flag while being boosted by EPO."},
    {"ACH_M32SUICIDE",    "Erreur de calcul",            "Miscalculation",              "Se tuer avec sa propre grenade.",                                       "Kill yourself with your own grenade."},
    {"ACH_ESPIONDEGUISE", "Pas vu, pas pris !",          "Not seen, not caught!",       "Tuer un ennemi en étant déguisé.",                                      "Kill an enemy while disguised."},
    {"ACH_FUCKYOU",       "Ils vont adorer !",           "They will love it!",          "Equiper la tombe \"fuck\".",                                            "Equip the grave \"fuck\"."},
    {"ACH_ABUS",          "Toujours dans l'abus",        "Never enough",                "Avoir une armure assitée, une super-arme et des stéros.",               "Have a power armor, superweapon, and steroids at the same time."},
    {"ACH_DESTRUCTEUR",   "Destructeur",                 "Destructor",                  "Infliger au moins 10000 dégâts en une seule partie.",                   "Deal at least 10000 damage in a single match."},
    {"ACH_RAGE",          "J'ai pris cher",              "I took a lot",                "Accumuler 8 secondes de rage ou plus.",                                 "Accumulate 8 seconds of rage or more."},
    {"ACH_DAVIDGOLIATH",  "David contre Goliath",        "David versus Goliath",        "Tuer un ennemi équipé d'une super-arme avec le pistolet de base",       "Kill an enemy equipped with a super weapon with the basic pistol."},
    {"ACH_LANCEEPO",      "Lance Armstrong",             "Lance Armstrong",             "Utiliser l'EPO en étant Junkie.",                                       "Shoot yourself with EPO while being Junkie."},
    {"ACH_PASLOGIQUE",    "Illogique",                   "Illogical",                   "Tuer un Ninja avec une arme de corps à corps sans être un ninja.",      "Kill a Ninja with a melee weapon without being a ninja."},
    {"ACH_JUSTEPOUR",     "Juste pour être sûr",         "Just to be sure",             "Tuer un ennemi avec une super-arme en étant Américain sous stéros.",    "Kill someone while being an American on steroids with a superweapon."},
    {"ACH_BRICOLEUR",     "Bricoleur du dimanche",       "Sunday handyman",             "Réparer son bouclier en s'infligeant des dégâts.",                      "Repair your shield by inflicting damage to yourself."},
    {"ACH_NOSCOPE",       "No scope",                    "No scope",                    "Tuer un ennemi à la SV-98 sans viser.",                                 "Kill an enemy with a SV-98 without aiming."},
    {"ACH_THUGPHYSIQUE",  "Thug de la physique",         "Physics thug",                "Tuer un ennemi au lance-flammes alors qu'il se trouve dans l'eau.",     "Kill an enemy with a flamethrower while they are in water."},
    {"ACH_SPAAACE",       "Spaaaaaace",                  "Spaaaaaace",                  "Atteindre la limite de hauteur sur la Lune",                            "Reach the height limit on the Moon."},
};
