#ifndef __STATS_H__
#define __STATS_H__

extern int xpneededfornextlvl, totalneededxp;

extern bool updatewinstat;

extern void loadsave();
extern void writesave();
extern void addstat(int valeur, int stat, bool rewrite = false);
extern void addxpandcc(int nbxp, int nbcc = 0);

//////////////////////////////////////// Statistiques | Player stats ////////////////////////////////////////
enum {STAT_CC, STAT_XP, STAT_LEVEL, STAT_KILLS, STAT_MORTS, STAT_KDRATIO, STAT_DAMMAGERECORD, STAT_KILLSTREAK, STAT_MAXKILLDIST, STAT_WINS, STAT_SUICIDES, STAT_ALLIESTUES, STAT_TIMEPLAYED, STAT_DRAPEAUXENVOL, STAT_DRAPEAUXENRAP, STAT_DRAPEAUXALYREC, STAT_BASEHACK, //Main game stats
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
    {"STAT_CC",                 "Cube Coins",                       "Cube Coins",                       "hud/cislacoins.png"},  //0
    {"STAT_XP",                 "XP",                               "XP",                               "hud/stats.png"},
    {"STAT_LEVEL",              "Niveau",                           "Level",                            "hud/stats.png"},
    {"STAT_KILLS",              "�liminations",                     "Frags",                            "hud/flingue.jpg"},
    {"STAT_MORTS",              "Morts",                            "Deaths",                           "hud/grave.png"},
    {"STAT_KDRATIO",            "Ratio morts/�liminations",         "Kills/Deaths ratio",               "hud/stats.png"},       //Calculated in calcratio() with STAT_KILLS & STAT_MORTS then called in getstatinfo() STAT_KDRATIO not saved because float shit.
    {"STAT_DAMMAGERECORD",      "Record de dommages en une partie", "Damage record in a single match",  "hud/checkbox_on.jpg"},
    {"STAT_KILLSTREAK",         "Meilleure s�rie d'�liminations",   "Best killstreak",                  "hud/rage.png"},
    {"STAT_MAXKILLDIST",        "Elimination la plus �loign�e",     "Farthest frag",                    "hud/campeur.png"},
    {"STAT_WINS",               "Victoires",                        "Victories",                        "hud/cool.jpg"},
    {"STAT_SUICIDES",           "Suicides",                         "Suicides",                         "hud/fou.jpg"},             //10
    {"STAT_ALLIESTUES",         "Alli�s tu�s",                      "Killed allies",                    "hud/sournois_red.jpg"},
    {"STAT_TIMEPLAYED",         "Temps de jeu",                     "Time played",                      "hud/chrono.png"},      //Calculated in secs with STAT_TIMEPLAYED in dotime(), calculated in HH:MM:SS for display in getstatinfo()
    {"STAT_DRAPEAUXENVOL",      "Drapeaux ennemis vol�s",           "Stolen enemy flags",               "hud/drapeau_ennemi.png"},
    {"STAT_DRAPEAUXENRAP",      "Drapeaux ennemis remport�s",       "Enemy flags won",                  "hud/drapeau_ennemi.png"},
    {"STAT_DRAPEAUXALYREC",     "Drapeaux alli�s r�cup�r�s",        "Allied flags recovered",           "hud/drapeau_allie.png"},
    {"STAT_BASEHACK",           "Temps pass� � hacker",             "Time spent hacking",               "hud/radio_off.jpg"},
    //Classes
    {"STAT_ABILITES",           "Abilit�es utilis�es",              "Used abilities",                   "hud/stats.png"},
    {"STAT_HEALTHREGEN",        "Sant� redonn�e aux alli�s",        "Health restored to allies",        "apt_logo/1.png"},
    {"STAT_HEALTHREGAIN",       "Sant� r�cup�r�e gr�ce aux m�decins", "Health recovered with medics",   "hud/coeur.png"},
    {"STAT_MANAREGEN",          "Mana redonn� aux alli�s",          "Mana restored to allies",          "hud/mana.png"},
    {"STAT_MANAREGAIN",         "Mana r�cup�r� gr�ce aux junkies",  "Mana recovered with junkies",      "apt_logo/13.png"},           //20
    //Shields
    {"STAT_BOUCLIERBOIS",       "Boucliers en bois utilis�s",       "Wooden shields used",              "hud/bouclier_bois.png"},
    {"STAT_BOUCLIERFER",        "Boucliers en fer utilis�s",        "Iron shields used",                "hud/bouclier_fer.png"},
    {"STAT_BOUCLIEROR",         "Boucliers en or utilis�s",         "Gold shields used",                "hud/bouclier_or.png"},
    {"STAT_BOUCLIERMAGNETIQUE", "Boucliers magn�tiques utilis�s",   "Magnetic shields used",            "hud/bouclier_magnetique.png"},
    {"STAT_ARMUREASSIST",       "Armures assist�es utilis�s",       "Power armors used",                "hud/robot.png"},
    {"STAT_REPASSIST",          "R�parations d'armure assist�e",    "Power armor repairs",              "hud/options.jpg"},
    //Objects
    {"STAT_PANACHAY",           "Panachays consomm�s",              "Beers drunk",                      "hud/coeur.png"},
    {"STAT_MANA",               "Potions de mana consomm�es",       "Mana potions consumed",            "hud/mana.png"},
    {"STAT_COCHON",             "Cochons grill�s mang�s",           "Grilled pigs eaten",               "hud/stats.png"},
    {"STAT_STEROS",             "Cures de st�ro�des",               "Steroids cycles",                  "hud/steros.png"},           //30
    {"STAT_EPO",                "Piqures d'EPO",                    "EPO shots",                        "hud/epo.png"},
    {"STAT_JOINT",              "Joints fum�s",                     "Smoked joints",                    "hud/joint.png"},
    {"STAT_CHAMPIS",            "Champignons mang�s",               "Shrooms eaten",                    "hud/champis.png"},
    {"STAT_ARMES",              "Armes ramass�es",                  "Picked up weapons",                "hud/chargeur.png"},
    {"STAT_SUPERARMES",         "Super-caisses ramass�es",          "Picked Up Super Crates",           "hud/stats.png"},
    //Stupid statistics
    {"STAT_ATOM",               "Bombes atomiques tir�es",          "Amount of atom bomb fired",        "hud/stats.png"},
    {"STAT_MUNSHOOTED",         "Munitions tir�es au total",        "Amount of ammo fired",             "hud/stats.png"},
    {"STAT_TOTALDAMAGEDEALT",   "Dommages inflig�s au total",       "Amount of damage dealt",           "hud/stats.png"},
    {"STAT_TOTALDAMAGERECIE",   "Dommages re�us au total",          "Amount of damage recieved",        "hud/stats.png"}
};

//////////////////////////////////////// Succ�s | Achievements ////////////////////////////////////////
extern void getsteamachievements();
extern void unlockachievement(int achID);

enum {ACH_TRIPLETTE = 0, ACH_PENTAPLETTE, ACH_DECAPLETTE, ACH_ATOME, ACH_WINNER, ACH_ENVOL, ACH_POSTULANT, ACH_STAGIAIRE,
        ACH_SOLDAT, ACH_LIEUTENANT, ACH_MAJOR, ACH_BEAUTIR, ACH_DEFONCE, ACH_PRECIS, ACH_KILLASSIST, ACH_KILLER, ACH_SACAPV,
        ACH_CADENCE, ACH_1HPKILL, ACH_MAXSPEED, ACH_INCREVABLE, ACH_CHANCE, ACH_CPASBIEN, ACH_SUICIDEFAIL, ACH_FUCKYEAH, ACH_RICHE,
        ACH_TUEURFANTOME, ACH_EPOFLAG, ACH_M32SUICIDE, ACH_ESPIONDEGUISE, ACH_FUCKYOU, ACH_ABUS, ACH_DESTRUCTEUR, ACH_RAGE,
        ACH_DAVIDGOLIATH, ACH_LANCEEPO, ACH_PASLOGIQUE, ACH_JUSTEPOUR, ACH_BRICOLEUR, ACH_NOSCOPE, ACH_THUGPHYSIQUE, ACH_SPAAACE,
        ACH_PARKOUR, ACH_EXAM, ACH_UNDECIDED, ACH_WASHAKIE, ACH_NATURO, ACH_TMMONEY, ACH_NICE, ACH_TAKETHAT, ACH_SURVIVOR, ACH_ELIMINATOR, NUMACHS};
extern bool succes[NUMACHS];

static const struct achinfo { const char *achname, *achnicenameFR, *achnicenameEN; } achievements[NUMACHS] =
{
    //Steam name          //French name                  //English name
    {"ACH_TRIPLETTE",     "Triple menace",               "Triple threat"},
    {"ACH_PENTAPLETTE",   "Terreur",                     "Terror"},
    {"ACH_DECAPLETTE",    "Invincible !",                "Invincible!"},
    {"ACH_ATOME",         "La puissance de l'atome",     "The power of the atom"},
    {"ACH_WINNER",        "Winner",                      "Winner"},
    {"ACH_ENVOL",         "Envol pour le paradis",       "Fly like an eagle"},
    {"ACH_POSTULANT",     "Postulant",                   "Applicant"},
    {"ACH_STAGIAIRE",     "Stagiaire",                   "Trainee"},
    {"ACH_SOLDAT",        "Soldat",                      "Soldier"},
    {"ACH_LIEUTENANT",    "Lieutenant",                  "Lieutenant"},
    {"ACH_MAJOR",         "Major",                       "Major"},
    {"ACH_BEAUTIR",       "Beau tir !",                  "Nice shoot!"},
    {"ACH_DEFONCE",       "Compl�tement d�fonc�",        "Completely stoned"},
    {"ACH_PRECIS",        "Pr�cis comme un boucher",     "Accurate like a butcher"},
    {"ACH_KILLASSIST",    "Armure.exe ne r�pond pas",    "Armor.exe is not responding"},
    {"ACH_KILLER",        "Tueur en s�rie",              "Serial killer"},
    {"ACH_SACAPV",        "Sac � points de vie",         "Too much health points"},
    {"ACH_CADENCE",       "Le combo ultime !",           "The ultimate combo!"},
    {"ACH_1HPKILL",       "A 1 PV du ragequit !",        "At 1 HP from ragequit"},
    {"ACH_MAXSPEED",      "Speedy Gonzales",             "Speedy Gonzales"},
    {"ACH_INCREVABLE",    "Increvable !",                "Indestructible"},
    {"ACH_CHANCE",        "Larry Silverstein",           "Lucky Larry"},
    {"ACH_CPASBIEN",      "C'est pas bien",              "That's not nice"},
    {"ACH_SUICIDEFAIL",   "Suicide manqu�",              "Failed suicide"},
    {"ACH_FKYEAH",        "Fuck yeah !",                 "Fuck yeah!"},
    {"ACH_RICHE",         "Capitaliste",                 "Capitalist"},
    {"ACH_TUEURFANTOME",  "Tueur de l'au-del�",          "Slayer from beyond"},
    {"ACH_EPOFLAG",       "C'est � �a que �a sert !",    "That's what it's made for!"},
    {"ACH_M32SUICIDE",    "Erreur de calcul",            "Miscalculation"},
    {"ACH_ESPIONDEGUISE", "Pas vu, pas pris !",          "Not seen, not caught!"},
    {"ACH_FKYOU",         "Ils vont adorer !",           "They will love it!"},
    {"ACH_ABUS",          "Toujours dans l'abus",        "Never enough"},
    {"ACH_DESTRUCTEUR",   "Destructeur",                 "Destructor"},
    {"ACH_RAGE",          "J'ai pris cher",              "I took a lot"},
    {"ACH_DAVIDGOLIATH",  "David contre Goliath",        "David versus Goliath"},
    {"ACH_LANCEEPO",      "Lance Armstrong",             "Lance Armstrong"},
    {"ACH_PASLOGIQUE",    "Illogique",                   "Illogical"},
    {"ACH_JUSTEPOUR",     "Juste pour �tre s�r",         "Just to be sure"},
    {"ACH_BRICOLEUR",     "Bricoleur du dimanche",       "Sunday handyman"},
    {"ACH_NOSCOPE",       "No scope",                    "No scope"},
    {"ACH_THUGPHYSIQUE",  "Thug de la physique",         "Physics thug"},
    {"ACH_SPAAACE",       "Aller plus haut",             "Spaaaaaace"},
    {"ACH_PARKOUR",       "Le patron du parkour",        "The parkour master"},
    {"ACH_EXAM",          "Examen surprise !",           "Surprise exam!"},
    {"ACH_UNDECIDED",     "Ind�cis",                     "Undecided"},
    {"ACH_WASHAKIE",      "Washakie",                    "Washakie"},
    {"ACH_NATURO",        "Naturopathe",                 "Naturotherapist"},
    {"ACH_TMMONEY",       "Dans le train !",             "Take my money!"},
    {"ACH_NICE",          "Minette",                     "Nice"},
    {"ACH_TAKETHAT",      "Abattre l'ennemi",            "Take that!"},
    {"ACH_SURVIVOR",      "Survivant",                   "Survivor"},
    {"ACH_ELIMINATOR",    "Purificateur",                "Eliminator"}
};

#endif
