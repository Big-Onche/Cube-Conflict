#ifndef __STATS_H__
#define __STATS_H__

#ifdef _WIN32
    #include "steam_api.h"
    #include "isteamuserstats.h"
#endif

extern int xpForNextLevel, totalXpNeeded;
extern bool incrementWinsStat;

extern void loadSave();
extern void writeSave();
extern void updateStat(int value, int statId, bool rewrite = false);
extern void addReward(int xp, int cc = 0);

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
    {"STAT_KILLS",              "Éliminations",                     "Frags",                            "hud/flingue.jpg"},
    {"STAT_MORTS",              "Morts",                            "Deaths",                           "hud/grave.png"},
    {"STAT_KDRATIO",            "Ratio morts/éliminations",         "Kills/Deaths ratio",               "hud/stats.png"},       //Calculated in calcratio() with STAT_KILLS & STAT_MORTS then called in getstatinfo() STAT_KDRATIO not saved because float shit.
    {"STAT_DAMMAGERECORD",      "Record de dommages en une partie", "Damage record in a single match",  "hud/checkbox_on.jpg"},
    {"STAT_KILLSTREAK",         "Meilleure série d'éliminations",   "Best killstreak",                  "hud/b_rage.png"},
    {"STAT_MAXKILLDIST",        "Elimination la plus éloignée",     "Farthest frag",                    "hud/campeur.png"},
    {"STAT_WINS",               "Victoires",                        "Victories",                        "hud/cool.jpg"},
    {"STAT_SUICIDES",           "Suicides",                         "Suicides",                         "hud/fou.jpg"},             //10
    {"STAT_ALLIESTUES",         "Alliés tués",                      "Killed allies",                    "hud/sournois_red.jpg"},
    {"STAT_TIMEPLAYED",         "Temps de jeu",                     "Time played",                      "hud/timer.png"},      //Calculated in secs with STAT_TIMEPLAYED in dotime(), calculated in HH:MM:SS for display in getstatinfo()
    {"STAT_DRAPEAUXENVOL",      "Drapeaux ennemis volés",           "Stolen enemy flags",               "hud/drapeau_ennemi.png"},
    {"STAT_DRAPEAUXENRAP",      "Drapeaux ennemis remportés",       "Enemy flags won",                  "hud/drapeau_ennemi.png"},
    {"STAT_DRAPEAUXALYREC",     "Drapeaux alliés récupérés",        "Allied flags recovered",           "hud/drapeau_allie.png"},
    {"STAT_BASEHACK",           "Temps passé à hacker",             "Time spent hacking",               "hud/radio_off.jpg"},
    //Classes
    {"STAT_ABILITES",           "Abilitées utilisées",              "Used abilities",                   "hud/stats.png"},
    {"STAT_HEALTHREGEN",        "Santé redonnée aux alliés",        "Health restored to allies",        "apt_logo/1.png"},
    {"STAT_HEALTHREGAIN",       "Santé récupérée grâce aux médecins", "Health recovered with medics",   "hud/health.png"},
    {"STAT_MANAREGEN",          "Mana redonné aux alliés",          "Mana restored to allies",          "hud/mana.png"},
    {"STAT_MANAREGAIN",         "Mana récupéré grâce aux junkies",  "Mana recovered with junkies",      "apt_logo/13.png"},           //20
    //Shields
    {"STAT_BOUCLIERBOIS",       "Boucliers en bois utilisés",       "Wooden shields used",              "hud/s_wood.png"},
    {"STAT_BOUCLIERFER",        "Boucliers en fer utilisés",        "Iron shields used",                "hud/s_iron.png"},
    {"STAT_BOUCLIEROR",         "Boucliers en or utilisés",         "Gold shields used",                "hud/s_gold.png"},
    {"STAT_BOUCLIERMAGNETIQUE", "Boucliers magnétiques utilisés",   "Magnetic shields used",            "hud/s_magnet.png"},
    {"STAT_ARMUREASSIST",       "Armures assistées utilisés",       "Power armors used",                "hud/s_power.png"},
    {"STAT_REPASSIST",          "Réparations d'armure assistée",    "Power armor repairs",              "hud/options.jpg"},
    //Objects
    {"STAT_PANACHAY",           "Panachays consommés",              "Beers drunk",                      "hud/health.png"},
    {"STAT_MANA",               "Potions de mana consommées",       "Mana potions consumed",            "hud/mana.png"},
    {"STAT_COCHON",             "Cochons grillés mangés",           "Grilled pigs eaten",               "hud/stats.png"},
    {"STAT_STEROS",             "Cures de stéroïdes",               "Steroids cycles",                  "hud/b_roids.png"},           //30
    {"STAT_EPO",                "Piqures d'EPO",                    "EPO shots",                        "hud/b_epo.png"},
    {"STAT_JOINT",              "Joints fumés",                     "Smoked joints",                    "hud/b_joint.png"},
    {"STAT_CHAMPIS",            "Champignons mangés",               "Shrooms eaten",                    "hud/b_shrooms.png"},
    {"STAT_ARMES",              "Armes ramassées",                  "Picked up weapons",                "hud/loader.png"},
    {"STAT_SUPERARMES",         "Super-caisses ramassées",          "Picked Up Super Crates",           "hud/stats.png"},
    //Stupid statistics
    {"STAT_ATOM",               "Bombes atomiques tirées",          "Amount of atom bomb fired",        "hud/stats.png"},
    {"STAT_MUNSHOOTED",         "Munitions tirées au total",        "Amount of ammo fired",             "hud/stats.png"},
    {"STAT_TOTALDAMAGEDEALT",   "Dommages infligés au total",       "Amount of damage dealt",           "hud/stats.png"},
    {"STAT_TOTALDAMAGERECIE",   "Dommages reçus au total",          "Amount of damage recieved",        "hud/stats.png"}
};

//////////////////////////////////////// Succès | Achievements ////////////////////////////////////////
extern void getsteamachievements();
extern void unlockAchievement(int achID);

enum {ACH_TRIPLETTE = 0, ACH_PENTAPLETTE, ACH_DECAPLETTE, ACH_ATOME, ACH_WINNER, ACH_ENVOL, ACH_POSTULANT, ACH_STAGIAIRE,
        ACH_SOLDAT, ACH_LIEUTENANT, ACH_MAJOR, ACH_BEAUTIR, ACH_DEFONCE, ACH_PRECIS, ACH_KILLASSIST, ACH_KILLER, ACH_SACAPV,
        ACH_CADENCE, ACH_1HPKILL, ACH_MAXSPEED, ACH_INCREVABLE, ACH_CHANCE, ACH_CPASBIEN, ACH_SUICIDEFAIL, ACH_FUCKYEAH, ACH_RICHE,
        ACH_TUEURFANTOME, ACH_EPOFLAG, ACH_M32SUICIDE, ACH_ESPIONDEGUISE, ACH_FUCKYOU, ACH_ABUS, ACH_DESTRUCTEUR, ACH_RAGE,
        ACH_DAVIDGOLIATH, ACH_LANCEEPO, ACH_PASLOGIQUE, ACH_JUSTEPOUR, ACH_BRICOLEUR, ACH_NOSCOPE, ACH_THUGPHYSIQUE, ACH_SPAAACE,
        ACH_PARKOUR, ACH_EXAM, ACH_UNDECIDED, ACH_WASHAKIE, ACH_NATURO, ACH_TMMONEY, ACH_NICE, ACH_TAKETHAT, ACH_SURVIVOR, ACH_ELIMINATOR, NUMACHS};

extern bool achievement[NUMACHS];
extern const char *achievementNames[NUMACHS];

#endif
