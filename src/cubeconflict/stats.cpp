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
            playSound(S_LEVELUP, NULL, 0, 0, SND_FIXEDPITCH|SND_NOTIFICATION);
            conoutf(CON_HUDCONSOLE, GAME_LANG ? "\f1LEVEL UP! \fi(Lvl %d)" : "\f1NIVEAU SUPÉRIEUR ! \fi(Niveau %d)", stat[STAT_LEVEL]);
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
        defformatstring(logodir, "%s%s", "media/interface/" ,statslist[*statID].statlogo);
        result(logodir);
    }
);

ICOMMAND(getstatinfo, "ii", (int *statID, bool *onlyvalue),
    if(*statID<0 || *statID>=NUMSTATS) { result(GAME_LANG ? "Invalid ID" : "ID Invalide"); return; }
    string val;
    switch(*statID)
    {
        //Based on STAT_KILLS & STAT_MORTS, STAT_KDRATIO not saved.
        case STAT_KDRATIO:
        {
            formatstring(val, "%s%s%.1f", *onlyvalue ? "" : GAME_LANG ? statslist[*statID].statnicenameEN : statslist[*statID].statnicenameFR, *onlyvalue ? "" : GAME_LANG ? ": " : " : ", killDeathRatio());
            break;
        }
        //Easy secs to HH:MM:SS converter and displayer
        case STAT_TIMEPLAYED:
        {
            int hours = stat[STAT_TIMEPLAYED] / 3600; int mins = (stat[STAT_TIMEPLAYED] % 3600) / 60; int secs = stat[STAT_TIMEPLAYED] % 60;
            formatstring(val, "%s : %d %s%s %d %s%s %d %s%s",  GAME_LANG ? statslist[*statID].statnicenameEN : statslist[*statID].statnicenameFR,
                        hours, GAME_LANG ? "hour" : "heure", hours>1 ? "s" : "", mins, "min", mins>1 ? "s" : "", secs, "sec", secs>1 ? "s" : "");
            break;
        }
        //when we need to display the stat after description (rare cases)
        case STAT_KILLSTREAK:
        case STAT_DAMMAGERECORD:
        case STAT_MAXKILLDIST:
        case STAT_LEVEL:
        case STAT_BASEHACK:
            formatstring(val, "%s%s%d%s", *onlyvalue ? "" : GAME_LANG ? statslist[*statID].statnicenameEN : statslist[*statID].statnicenameFR, *onlyvalue ? "" : GAME_LANG ? ": " : " : ", stat[*statID], *statID!=STAT_MAXKILLDIST ? *statID==STAT_BASEHACK ? GAME_LANG ? " seconds" : " secondes" : "" : GAME_LANG ? " meters" : " mètres");
            break;
        //otherwise, we put the stat value before the description by default
        default: formatstring(val, "%d%s%s", stat[*statID], *onlyvalue ? "" : " ", *onlyvalue ? "" : GAME_LANG ? statslist[*statID].statnicenameEN : statslist[*statID].statnicenameFR);
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
    if(!savefile) { conoutf(GAME_LANG ? "\fcUnable to write your save file !" : "\fcUnable d'écrite votre fichier de sauvegarde !"); return; }

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
        #ifdef _WIN32
            if(IS_USING_STEAM)
            {
                SteamUserStats()->SetAchievement(achievements[achID].achname); //Met le succès à jour côté steam
                SteamUserStats()->StoreStats();
            }
        #endif

        if(newAchievement) // update achievement status and give reward if new achievement
        {
            achievement[achID] = true;
            addReward(25, 25);
            playSound(S_ACHIEVEMENTUNLOCKED, NULL, 0, 0, SND_FIXEDPITCH|SND_NOTIFICATION);
            conoutf(CON_HUDCONSOLE, GAME_LANG ? "\f1ACHIEVEMENT UNLOCKED! \fi(%s)" : "\f1SUCCES DÉBLOQUÉ ! \fi(%s)", GAME_LANG ? achievements[achID].achnicenameEN : achievements[achID].achnicenameFR);
        }
    }
}

void getsteamachievements() //Récupère les succès enregistrés sur steam
{
    #ifdef _WIN32
        int achID = 0;
        loopi(NUMACHS)
        {
            SteamUserStats()->GetAchievement(achievements[achID].achname, &achievement[achID]);
            achID++;
        }
    #endif
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
        defformatstring(logodir, "media/interface/achievements/%s%s.jpg", achievements[*achID].achname, isLocked(*achID) ? "_no" : "_yes");
        result(logodir);
    }
);

ICOMMAND(getachievementname, "i", (int *achID), //gets achievement name for ui
    if(*achID<0 || *achID>=NUMACHS) result(GAME_LANG ? "Invalid ID" : "ID Invalide");
    else result(GAME_LANG ? achievements[*achID].achnicenameEN : achievements[*achID].achnicenameFR);
);

ICOMMAND(getachievementcolor, "i", (int *achID), //gets achievement color status for ui
    if(*achID<0 || *achID>=NUMACHS) intret(0x777777);
    else intret(isLocked(*achID) ? 0xFFC6C6 : 0xD0F3D0);
);
