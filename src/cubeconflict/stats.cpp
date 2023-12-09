//stats.cpp: where we manage stats, achievements and local saves

#include "game.h"
#include "stats.h"
#include "customs.h"

//////////////////////////////////////////////levels and xp////////////////////////////////////////////
int currentLevel = 1, xpForNextLevel = 50, xpForPreviousLevel = 50, totalXpNeeded = 50;  // base level and xp
float progress = -1;

void calcPlayerLevel()
{
    while(stat[STAT_XP] >= xpForNextLevel) // calc the level based on total xp
    {
        currentLevel++;
        xpForPreviousLevel += currentLevel*2;
        xpForNextLevel += xpForPreviousLevel;
        totalXpNeeded += currentLevel*2;
        if(isconnected())
        {
            playSound(S_LEVELUP, NULL, 0, 0, SND_FIXEDPITCH|SND_UI);
            conoutf(CON_HUDCONSOLE, "\f1%s \fi(%s %d)", readstr("GameMessage_LevelUp"), readstr("Stat_Level"), currentLevel);
        }
    }

    if(stat[STAT_XP]-xpForNextLevel!=0 && totalXpNeeded!=0) progress = float(stat[STAT_XP]-xpForNextLevel)/float(totalXpNeeded);

    stat[STAT_LEVEL] = currentLevel;

    game::player1->level = currentLevel; // gameent level update

    switch(currentLevel) // check for achievement
    {
        case 5: unlockAchievement(ACH_POSTULANT); break;
        case 10: unlockAchievement(ACH_STAGIAIRE); break;
        case 20: unlockAchievement(ACH_SOLDAT); break;
        case 50: unlockAchievement(ACH_LIEUTENANT); break;
        case 100: unlockAchievement(ACH_MAJOR);
    }
}

ICOMMAND(hudlevelprogress, "", (), floatret(fabs(progress)));

//////////////////////////////////////////////player stats////////////////////////////////////////////
int stat[NUMSTATS];
bool incrementWinsStat = true;

void addReward(int xp, int cc) // add xp and cc (cisla coins)
{
    if(!IS_ON_OFFICIAL_SERV) return;
    stat[STAT_XP] += xp;
    stat[STAT_CC] += cc;
    calcPlayerLevel();

    if(stat[STAT_CC] > 1500) unlockAchievement(ACH_RICHE);
}

int autoSave = 0;
VARP(autosavesteps, 1, 100, INT_MAX);

void updateStat(int value, int statId, bool rewrite) // update a stat
{
    if(!IS_ON_OFFICIAL_SERV) return;
    if(rewrite) stat[statId] = value;
    else stat[statId] += value;

    autoSave++;
    if(autoSave >= autosavesteps) { writeSave(); autoSave=0; } // save every "autosavesteps"
}

float killDeathRatio()
{
    return (float(stat[STAT_KILLS])/(float(stat[STAT_MORTS]) == 0.f ? 1.f : float(stat[STAT_MORTS])));
}

//ICOMMAND(gettotalstats, "", (), intret(NUMSTATS)); //gets nb of stats for ui

ICOMMAND(getstatlogo, "i", (int *statID), //gets stat logo for ui
    if(*statID<0 || *statID>=NUMSTATS) result("media/texture/game/notexture.png");
    else
    {
        defformatstring(logodir, "%s%s", "media/interface/", statslist[*statID].statlogo);
        result(logodir);
    }
);

ICOMMAND(getstatinfo, "i", (int *statID),
    if(*statID<0 || *statID>=NUMSTATS) { result(readstr("Misc_InvalidId")); return; }
    string val;
    switch(*statID)
    {
        case STAT_KDRATIO: // based on STAT_KILLS & STAT_MORTS, STAT_KDRATIO not saved.
        {
            formatstring(val, "%.1f", killDeathRatio());
            break;
        }
        case STAT_TIMEPLAYED: // easy secs to HH:MM:SS converter and displayer
        case STAT_BASEHACK:
        {
            int secs = *statID == STAT_TIMEPLAYED ? stat[STAT_TIMEPLAYED] : stat[STAT_BASEHACK];
            int h = secs / 3600;
            int m = (secs % 3600) / 60;
            int s = secs % 60;
            formatstring(val, "%02d:%02d:%02d", h, m, s);
            break;
        }
        default: formatstring(val, "%d%s", stat[*statID], *statID==STAT_MAXKILLDIST ? " m" : "");
    }

    result(val);
);

//////////////////////////////////////////////saves////////////////////////////////////////////
#define SUPERCRYPTKEY 10;
char *encryptSave(char savepart[20]) // simple and easily crackable encryption
{
    int i;

    for(i = 0; (i < 100 && savepart[i] != '\0'); i++)
        savepart[i] = savepart[i] - SUPERCRYPTKEY;

    return savepart;
}

void writeSave() // we write the poorly encrypted value for all stat
{
    stream *savefile = openutf8file("config/stats.cfg", "w");
    if(!savefile) { conoutf(CON_ERROR, "\fc%s", readstr("Console_Error_WriteSave")); return; }

    int saveID = 0;
    loopi(NUMSTATS + NUMCUST + NUMACHS)
    {
        savefile->printf("%s\n", encryptSave(tempformatstring("%d", saveID >= NUMSTATS + NUMCUST ? achievement[saveID-NUMCUST-NUMSTATS] : saveID >= NUMSTATS ? cust[saveID-NUMSTATS] : stat[saveID])));
        saveID++;
    }
    savefile->close();
}

void giveStarterKit() // starter pack
{
    stat[STAT_LEVEL]++;
    cust[SMI_HAP] = cust[CAPE_CUBE] = cust[TOM_MERDE] = cust[VOI_CORTEX] = rnd(99)+1;
}

void loadSave() // we read the poorly encrypted value for all stat
{
    giveStarterKit(); // always give the starter pack in case of missing/corrupted save
    stream *savefile = openfile("config/stats.cfg", "r");
    if(!savefile) { calcPlayerLevel(); return; }

    char buf[50];
    int saveID = 0;

    while(savefile->getline(buf, sizeof(buf)))
    {
        int i;
        // we decrypt the line
        for(i = 0; (i < 500 && buf[i] != '\0'); i++)
            buf[i] = buf[i] + SUPERCRYPTKEY;

        if(saveID < NUMSTATS) sscanf(buf, "%i", &stat[saveID]);
        else if(saveID < NUMCUST + NUMSTATS) sscanf(buf, "%i", &cust[saveID-NUMSTATS]);
        else {int j = 0; sscanf(buf, "%i", &j); achievement[saveID-NUMSTATS-NUMCUST] = j;}

        saveID++;
    }
    savefile->close();
    calcPlayerLevel();
    if(IS_USING_STEAM) getsteamachievements();
}

//////////////////////Achievements//////////////////////
bool achievement[NUMACHS];
bool isLocked(int achID) { return !achievement[achID]; }

ICOMMAND(unlockach, "i", (int *achID), if(*achID==ACH_PARKOUR || *achID==ACH_EXAM) unlockAchievement(*achID));

bool canUnlockOffline(int achID)
{
    return achID==ACH_PARKOUR || achID==ACH_FUCKYOU || achID==ACH_EXAM || achID==ACH_TMMONEY || achID==ACH_SURVIVOR || achID==ACH_ELIMINATOR;
}

void unlockAchievement(int achID)
{
    bool newAchievement = isLocked(achID);
    if(canUnlockOffline(achID) || IS_ON_OFFICIAL_SERV) //Ne débloque que si serveur officiel sauf succès en solo
    {
        if(IS_USING_STEAM)
        {
            SteamUserStats()->SetAchievement(achievementNames[achID]); //Met le succès à jour côté steam
            SteamUserStats()->StoreStats();
        }

        if(newAchievement) // update achievement status and give reward if new achievement
        {
            achievement[achID] = true;
            addReward(25, 25);
            playSound(S_ACHIEVEMENTUNLOCKED, NULL, 0, 0, SND_FIXEDPITCH|SND_UI);
            conoutf(CON_HUDCONSOLE, "\f1%s\fi (%s)", readstr("GameMessage_AchievementUnlocked"), readstr("Stat_Achievements", achID));
        }
    }
}

void getsteamachievements() //Récupère les succès enregistrés sur steam
{
    int achID = 0;
    loopi(NUMACHS)
    {
        SteamUserStats()->GetAchievement(achievementNames[achID], &achievement[achID]);
        achID++;
    }
}

ICOMMAND(gettotalach, "", (), intret(NUMACHS)); //gets nb of achievements for ui

ICOMMAND(getunlockedach, "", (), //gets nb of unlocked achievements for ui
    int totalachunlocked = 0; int achID = 0;
    loopi(NUMACHS)
    {
        if(!isLocked(achID)) totalachunlocked++;
        achID++;
    }
    intret(totalachunlocked);
);

ICOMMAND(getachievementslogo, "i", (int *achID), //gets achievement logo for ui
    if(*achID<0 || *achID>=NUMACHS) result("media/texture/game/notexture.png");
    else
    {
        defformatstring(logodir, "media/interface/achievements/%s%s.jpg", achievementNames[*achID], isLocked(*achID) ? "_no" : "_yes");
        result(logodir);
    }
);

ICOMMAND(getachievementstate, "i", (int *achID), intret(!isLocked(*achID)); );

const char *achievementNames[NUMACHS] = {
    "ACH_TRIPLETTE", "ACH_PENTAPLETTE", "ACH_DECAPLETTE", "ACH_ATOME", "ACH_WINNER", "ACH_ENVOL", "ACH_POSTULANT", "ACH_STAGIAIRE",
    "ACH_SOLDAT", "ACH_LIEUTENANT", "ACH_MAJOR", "ACH_BEAUTIR", "ACH_DEFONCE", "ACH_PRECIS", "ACH_KILLASSIST", "ACH_KILLER", "ACH_SACAPV",
    "ACH_CADENCE", "ACH_1HPKILL", "ACH_MAXSPEED", "ACH_INCREVABLE", "ACH_CHANCE", "ACH_CPASBIEN", "ACH_SUICIDEFAIL", "ACH_FKYEAH", "ACH_RICHE",
    "ACH_TUEURFANTOME", "ACH_EPOFLAG", "ACH_M32SUICIDE", "ACH_ESPIONDEGUISE", "ACH_FKYOU", "ACH_ABUS", "ACH_DESTRUCTEUR", "ACH_RAGE",
    "ACH_DAVIDGOLIATH", "ACH_LANCEEPO", "ACH_PASLOGIQUE", "ACH_JUSTEPOUR", "ACH_BRICOLEUR", "ACH_NOSCOPE", "ACH_THUGPHYSIQUE", "ACH_SPAAACE",
    "ACH_PARKOUR", "ACH_EXAM", "ACH_UNDECIDED", "ACH_WASHAKIE", "ACH_NATURO", "ACH_TMMONEY", "ACH_NICE", "ACH_TAKETHAT", "ACH_SURVIVOR", "ACH_ELIMINATOR"
};
