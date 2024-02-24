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
            playSound(S_LEVELUP, NULL, 0, 0, SND_UI);
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
    rewrite ? stat[statId] = value : stat[statId] += value;

    autoSave++;
    if(autoSave >= autosavesteps) { writeSave(); autoSave=0; } // save every "autosavesteps"
}

float killDeathRatio()
{
    return (float(stat[STAT_KILLS])/(float(stat[STAT_MORTS]) == 0.f ? 1.f : float(stat[STAT_MORTS])));
}

ICOMMAND(gettotalstats, "", (), intret(NUMSTATS)); // gets nb of stats for ui

ICOMMAND(getstatlogo, "i", (int *statID), // gets stat logo for ui
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
    for(int i = 0; (i < 100 && savepart[i] != '\0'); i++) savepart[i] = savepart[i] - SUPERCRYPTKEY;
    return savepart;
}

void decryptSave(char* line)
{
    for (int i = 0; line[i] != '\0'; i++) { line[i] = line[i] + SUPERCRYPTKEY; }
}

void writeSave() // we write the poorly encrypted value for all stat
{
    stream *savefile = openutf8file("config/stats.cfg", "w");
    if(!savefile) { conoutf(CON_ERROR, "\fc%s", readstr("Console_Error_WriteSave")); return; }

    savefile->printf("version=%d\n", PROTOCOL_VERSION);

    loopi(NUMSTATS) savefile->printf("%s\n", encryptSave(tempformatstring("%s=%d", statslist[i].ident, stat[i])));
    loopi(NUMSMILEYS) savefile->printf("%s\n", encryptSave(tempformatstring("smiley_%s=%d", customsmileys[i].ident, smiley[i] ? max(rnd(256), 1) : 0)));
    loopi(NUMCAPES) savefile->printf("%s\n", encryptSave(tempformatstring("cape_%s=%d", capes[i].name, cape[i] ? max(rnd(256), 1) : 0)));
    loopi(NUMGRAVES) savefile->printf("%s\n", encryptSave(tempformatstring("grave_%s=%d", graves[i].name, grave[i] ? max(rnd(256), 1) : 0)));
    loopi(NUMACHS) savefile->printf("%s\n", encryptSave(tempformatstring("%s=%d", achievementNames[i], achievement[i] ? max(rnd(256), 1) : 0)));
    savefile->close();
}

void giveStarterKit() // starter pack
{
    stat[STAT_LEVEL]++;
    smiley[SMI_HAP] = cape[CAPE_CUBE] = grave[TOM_MERDE] = max(rnd(256), 1);
}

void loadSave()
{
    giveStarterKit(); // always give the starter pack in case of missing/corrupted save

    stream *savefile = openfile("config/stats.cfg", "r");
    if(!savefile) { calcPlayerLevel(); return; }

    char buf[64];

    savefile->getline(buf, sizeof(buf));
    bool oldSave = strncmp(buf, "version=", 8);
    savefile->close();

    savefile = openfile("config/stats.cfg", "r");

    if(oldSave) // no version = old save file = we read it the old way, might remove that in 1.0
    {
        int line = 0;
        int skip = 0;

        while(savefile->getline(buf, sizeof(buf)))
        {
            if(skip > 0) { --skip; continue; }

            decryptSave(buf);

            if (line < NUMSTATS) {
                sscanf(buf, "%i", &stat[line]);
                if (line == NUMSTATS - 1) skip = 10; // skip next 10 lines
            } else if (line < NUMSTATS + NUMSMILEYS) {
                sscanf(buf, "%i", &smiley[line - NUMSTATS]);
                if (line == NUMSTATS + NUMSMILEYS - 1) skip = 10; // skip next 10 lines
            } else if (line < NUMSTATS + NUMSMILEYS + NUMCAPES) {
                sscanf(buf, "%i", &cape[line - NUMSTATS - NUMSMILEYS]);
                if (line == NUMSTATS + NUMSMILEYS + NUMCAPES - 1) skip = 6; // skip next 6 lines
            } else if (line < NUMSTATS + NUMSMILEYS + NUMCAPES + NUMGRAVES) {
                sscanf(buf, "%i", &grave[line - NUMSTATS - NUMSMILEYS - NUMCAPES]);
                if (line == NUMSTATS + NUMSMILEYS + NUMCAPES + NUMGRAVES - 1) skip = 27; // skip next 27 lines
            } else sscanf(buf, "%i", &achievement[line - NUMSTATS - NUMSMILEYS - NUMCAPES - NUMGRAVES]);

            line++;
        }
        writeSave();
    }
    else
    {
        while(savefile->getline(buf, sizeof(buf)))
        {
            decryptSave(buf);

            char key[50];
            int value;

            if(sscanf(buf, "%49[^=]=%d", key, &value) == 2)
            {
                loopi(NUMSTATS) { if(!strcmp(key, statslist[i].ident)) { stat[i] = value; break; } }
                loopi(NUMSMILEYS){ if(!strcmp(key, tempformatstring("smiley_%s", customsmileys[i].ident))) { smiley[i] = value; break; } }
                loopi(NUMCAPES) { if(!strcmp(key, tempformatstring("cape_%s", capes[i].name))) { cape[i] = value; break; } }
                loopi(NUMGRAVES) { if(!strcmp(key, tempformatstring("grave_%s", graves[i].name))) { grave[i] = value; break; } }
                loopi(NUMACHS) { if(!strcmp(key, achievementNames[i])) { achievement[i] = value; break; } }
            }
        }
    }

    savefile->close();
    calcPlayerLevel();
    if(IS_USING_STEAM) getsteamachievements();
}

//////////////////////Achievements//////////////////////
int achievement[NUMACHS];
bool isLocked(int achID) { return !achievement[achID]; }

ICOMMAND(unlockach, "i", (int *achID), if(*achID==ACH_PARKOUR || *achID==ACH_EXAM) unlockAchievement(*achID));

bool canUnlockOffline(int achID)
{
    return achID==ACH_PARKOUR || achID==ACH_FUCKYOU || achID==ACH_EXAM || achID==ACH_TMMONEY || achID==ACH_SURVIVOR || achID==ACH_ELIMINATOR;
}

void unlockAchievement(int achID)
{
    bool newAchievement = isLocked(achID);
    if(canUnlockOffline(achID) || IS_ON_OFFICIAL_SERV)
    {
        if(IS_USING_STEAM)
        {
            SteamUserStats()->SetAchievement(achievementNames[achID]);
            SteamUserStats()->StoreStats();
        }

        if(newAchievement) // update achievement status and give reward if new achievement
        {
            achievement[achID] = true;
            addReward(25, 25);
            playSound(S_ACHIEVEMENTUNLOCKED, NULL, 0, 0, SND_UI);
            conoutf(CON_HUDCONSOLE, "\f1%s\fi (%s)", readstr("GameMessage_AchievementUnlocked"), readstr("Stat_Achievements", achID));
        }
    }
}

void getsteamachievements() // retrieves achievements from steam
{
    loopi(NUMACHS)
    {
        bool unlocked;
        SteamUserStats()->GetAchievement(achievementNames[i], &unlocked);
        achievement[i] = unlocked;
    }
}

ICOMMAND(gettotalach, "", (), intret(NUMACHS)); // gets nb of achievements for ui

ICOMMAND(getunlockedach, "", (), //gets nb of unlocked achievements for ui
    int unlocks = 0;
    loopi(NUMACHS) { if(!isLocked(i)) unlocks++; }
    intret(unlocks);
);

ICOMMAND(getachievementslogo, "i", (int *achID), //gets achievement logo for ui
    if(*achID<0 || *achID>=NUMACHS) result("media/texture/game/notexture.png");
    else
    {
        defformatstring(logodir, "media/interface/achievements/%s%s.jpg", achievementNames[*achID], isLocked(*achID) ? "_n" : "_y");
        result(logodir);
    }
);

ICOMMAND(getachievementstate, "i", (int *achID), intret(!isLocked(*achID)); );

const char *achievementNames[NUMACHS] = {
    "ach_triple", "ach_penta", "ach_deca", "ach_atom", "ach_winner", "ach_fly", "ach_applicant", "ach_trainee",
    "ach_soldier", "ach_lieutenant", "ach_major", "ach_niceshot", "ach_stoned", "ach_accurate", "ach_armorkill", "ach_killer", "ach_hpbag",
    "ach_firerate", "ach_onehpkill", "ach_maxspeed", "ach_indestructible", "ach_luck", "ach_notgood", "ach_suicidefail", "ach_fkyeah", "ach_rich",
    "ach_ghost", "ach_epoflag", "ach_grenadefail", "ach_spy", "ach_fkyou", "ach_enough", "ach_destructor", "ach_rage",
    "ach_davidgoliath", "ach_lanceepo", "ach_illogical", "ach_sure", "ach_handyman", "ach_noscope", "ach_thug", "ach_spaaace",
    "ach_parkour", "ach_exam", "ach_undecided", "ach_washakie", "ach_naturo", "ach_tmmoney", "ach_nice", "ach_takethat", "ach_survivor", "ach_eliminator"
};
