#ifndef __STATS_H__
#define __STATS_H__

#include "steam_api.h"

extern int xpForNextLevel, totalXpNeeded;

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
        NUMSTATS};
extern int stat[NUMSTATS];

static const struct statsinfo { const char *ident, *statlogo; } statslist[] = // main game stats
{   // id                       // logo
    {"stat_cc",                 "hud/cislacoins.png"},          // 0
    {"stat_xp",                 "hud/stats.png"},
    {"stat_level",              "hud/stats.png"},
    {"stat_kills",              "hud/flingue.jpg"},
    {"stat_deaths",             "hud/grave.png"},
    {"stat_kdratio",            "hud/stats.png"},               // calculated in calcratio() with STAT_KILLS & STAT_MORTS then displayed with getstatinfo()
    {"stat_damagesrec",         "hud/checkbox_on.jpg"},
    {"stat_killstreak",         "hud/b_rage.png"},
    {"stat_maxkilldist",        "hud/campeur.png"},
    {"stat_wins",               "hud/cool.jpg"},
    {"stat_suicides",           "hud/fou.jpg"},                 // 10
    {"stat_teamkills",          "hud/sournois_red.jpg"},
    {"stat_timeplayed",         "hud/timer.png"},               // seconds, displayed in HH:MM:SS with getstatinfo()
    {"stat_flagsstolen",        "hud/drapeau_ennemi.png"},
    {"stat_flagsscores",        "hud/drapeau_ennemi.png"},
    {"stat_allyflagsrec",       "hud/drapeau_allie.png"},
    {"stat_basehack",           "hud/radio_off.jpg"},           // seconds, displayed in HH:MM:SS with getstatinfo()
    // classes
    {"stat_abilities",          "hud/stats.png"},
    {"stat_healthregen",        "apt_logo/1.png"},
    {"stat_healthregain",       "hud/health.png"},
    {"stat_manaregen",          "hud/mana.png"},
    {"stat_manaregain",         "apt_logo/13.png"},             //20
    // shields
    {"stat_woodshields",        "hud/s_wood.png"},
    {"stat_ironshields",        "hud/s_iron.png"},
    {"stat_goldshields",        "hud/s_gold.png"},
    {"stat_magnetshields",      "hud/s_magnet.png"},
    {"stat_powerarmors",        "hud/s_power.png"},
    {"stat_parmorrepairs",      "hud/options.jpg"},
    // items
    {"stat_panachay",           "hud/health.png"},
    {"stat_mana",               "hud/mana.png"},
    {"stat_grilledpigs",        "hud/stats.png"},
    {"stat_roids",              "hud/b_roids.png"},              //30
    {"stat_epo",                "hud/b_epo.png"},
    {"stat_joint",              "hud/b_joint.png"},
    {"stat_shrooms",            "hud/b_shrooms.png"},
    {"stat_weapons",            "hud/loader.png"},
    {"stat_superweapons",       "hud/flingue.jpg"},
    // stupid stats
    {"stat_totalnukes",         "hud/stats.png"},
    {"stat_totalammofired",     "hud/stats.png"},
    {"stat_totaldamagedealth",  "hud/stats.png"},
    {"stat_totaldamagereci",    "hud/stats.png"}
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

extern int achievement[NUMACHS];
extern const char *achievementNames[NUMACHS];

#endif
