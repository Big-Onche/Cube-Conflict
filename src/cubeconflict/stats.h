#ifndef __STATS_H__
#define __STATS_H__

#include "steam_api.h"
#include "isteamuserstats.h"

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

static const struct statsinfo { const char *statname, *statlogo; } statslist[] = // main game stats
{   // id                       // logo
    {"STAT_CC",                 "hud/cislacoins.png"},          // 0
    {"STAT_XP",                 "hud/stats.png"},
    {"STAT_LEVEL",              "hud/stats.png"},
    {"STAT_KILLS",              "hud/flingue.jpg"},
    {"STAT_MORTS",              "hud/grave.png"},
    {"STAT_KDRATIO",            "hud/stats.png"},               // calculated in calcratio() with STAT_KILLS & STAT_MORTS then displayed with getstatinfo()
    {"STAT_DAMMAGERECORD",      "hud/checkbox_on.jpg"},
    {"STAT_KILLSTREAK",         "hud/b_rage.png"},
    {"STAT_MAXKILLDIST",        "hud/campeur.png"},
    {"STAT_WINS",               "hud/cool.jpg"},
    {"STAT_SUICIDES",           "hud/fou.jpg"},                 // 10
    {"STAT_ALLIESTUES",         "hud/sournois_red.jpg"},
    {"STAT_TIMEPLAYED",         "hud/timer.png"},               // seconds, displayed in HH:MM:SS with getstatinfo()
    {"STAT_DRAPEAUXENVOL",      "hud/drapeau_ennemi.png"},
    {"STAT_DRAPEAUXENRAP",      "hud/drapeau_ennemi.png"},
    {"STAT_DRAPEAUXALYREC",     "hud/drapeau_allie.png"},
    {"STAT_BASEHACK",           "hud/radio_off.jpg"},           // seconds, displayed in HH:MM:SS with getstatinfo()
    // classes
    {"STAT_ABILITES",           "hud/stats.png"},
    {"STAT_HEALTHREGEN",        "apt_logo/1.png"},
    {"STAT_HEALTHREGAIN",       "hud/health.png"},
    {"STAT_MANAREGEN",          "hud/mana.png"},
    {"STAT_MANAREGAIN",         "apt_logo/13.png"},             //20
    // shields
    {"STAT_BOUCLIERBOIS",       "hud/s_wood.png"},
    {"STAT_BOUCLIERFER",        "hud/s_iron.png"},
    {"STAT_BOUCLIEROR",         "hud/s_gold.png"},
    {"STAT_BOUCLIERMAGNETIQUE", "hud/s_magnet.png"},
    {"STAT_ARMUREASSIST",       "hud/s_power.png"},
    {"STAT_REPASSIST",          "hud/options.jpg"},
    // items
    {"STAT_PANACHAY",           "hud/health.png"},
    {"STAT_MANA",               "hud/mana.png"},
    {"STAT_COCHON",             "hud/stats.png"},
    {"STAT_STEROS",             "hud/b_roids.png"},              //30
    {"STAT_EPO",                "hud/b_epo.png"},
    {"STAT_JOINT",              "hud/b_joint.png"},
    {"STAT_CHAMPIS",            "hud/b_shrooms.png"},
    {"STAT_ARMES",              "hud/loader.png"},
    {"STAT_SUPERARMES"          "hud/stats.png"},
    // stupid stats
    {"STAT_ATOM",               "hud/stats.png"},
    {"STAT_MUNSHOOTED",         "hud/stats.png"},
    {"STAT_TOTALDAMAGEDEALT",   "hud/stats.png"},
    {"STAT_TOTALDAMAGERECIE",   "hud/stats.png"}
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
