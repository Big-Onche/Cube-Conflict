//stats.cpp: where we manage stats, achievements and local saves

#include "game.h"
#include "stats.h"
#include "customs.h"

//////////////////////Gestion de l'xp et niveaux//////////////////////
int cclvl = 1, xpneededfornextlvl = 50, xpneededforprevlvl = 50, totalneededxp = 50;
float pourcents = -1;

void genlvl() //Calcule le niveau du joueur
{
    while(stat[STAT_XP] >= xpneededfornextlvl) //Ajoute un niveau et augmente l'xp demandé pour le niveau suivant
    {
        cclvl++;
        xpneededforprevlvl += cclvl*2;
        xpneededfornextlvl += xpneededforprevlvl;
        totalneededxp += cclvl*2;
        if(isconnected())
        {
            playSound(S_LEVELUP, NULL, 0, 0, SND_FIXEDPITCH|SND_NOTIFICATION);
            conoutf(CON_HUDCONSOLE, GAME_LANG ? "\f1LEVEL UP! \fi(Lvl %d)" : "\f1NIVEAU SUPÉRIEUR ! \fi(Niveau %d)", stat[STAT_LEVEL]);
        }
    }

    if(stat[STAT_XP]-xpneededfornextlvl!=0 && totalneededxp!=0) pourcents = float(stat[STAT_XP]-xpneededfornextlvl)/float(totalneededxp);

    switch(cclvl) //Regarde si il y a un succès à déverrouiller
    {
        case 5: unlockachievement(ACH_POSTULANT); break;
        case 10: unlockachievement(ACH_STAGIAIRE); break;
        case 20: unlockachievement(ACH_SOLDAT); break;
        case 50: unlockachievement(ACH_LIEUTENANT); break;
        case 100: unlockachievement(ACH_MAJOR);
    }

    game::player1->level = cclvl; //Actualise le lvl pour l'envoyer en multijoueur
    stat[STAT_LEVEL] = cclvl;
}

ICOMMAND(hudlevelprogress, "", (), floatret(fabs(pourcents)));

//////////////////////Gestion des statistiques//////////////////////
int stat[NUMSTATS];
bool updatewinstat = true;

void addxpandcc(int nbxp, int nbcc) // Ajoute l'xp et/ou les CC
{
    if(!IS_ON_OFFICIAL_SERV) return;
    stat[STAT_XP] += nbxp;
    stat[STAT_CC] += nbcc;
    genlvl(); //Recalcule le niveau

    if(stat[STAT_CC] > 1500) unlockachievement(ACH_RICHE);
}

int savejustincase = 0;
void addstat(int valeur, int statID, bool rewrite) // Ajoute la stat très simplement
{
    if(!IS_ON_OFFICIAL_SERV) return;
    if(rewrite) stat[statID] = valeur;
    else stat[statID] += valeur;

    savejustincase++;
    if(savejustincase==100) {writesave(); savejustincase=0;} //Sauvegarde toutes les 100 stats ajoutés (ça monte vite avec les munitions tirées), ça limite la casse en cas de crash
}

float kdratio() // kill/death ratio calculation
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
    if(*statID<0 || *statID>=NUMSTATS) {result(GAME_LANG ? "Invalid ID" : "ID Invalide"); return;}
    string val;
    switch(*statID)
    {
        //Based on STAT_KILLS & STAT_MORTS, STAT_KDRATIO not saved.
        case STAT_KDRATIO:
        {
            formatstring(val, "%s%s%.1f", *onlyvalue ? "" : GAME_LANG ? statslist[*statID].statnicenameEN : statslist[*statID].statnicenameFR, *onlyvalue ? "" : GAME_LANG ? ": " : " : ", kdratio());
            break;
        }
        //Easy secs to HH:MM:SS converter and displayer
        case STAT_TIMEPLAYED:
        {
            int tmpsec = stat[STAT_TIMEPLAYED]; int tmpmin = 0; int tmphr = 0;
            while(tmpsec > 59) { tmpmin++ ; tmpsec-=60; }
            while(tmpmin > 59) { tmphr++ ; tmpmin-=60; }

            formatstring(val, "%s : %d %s%s %d %s%s %d %s%s",  GAME_LANG ? statslist[*statID].statnicenameEN : statslist[*statID].statnicenameFR,
                        tmphr, GAME_LANG ? "hour" : "heure", tmphr>1 ? "s" : "",
                        tmpmin, "min", tmpmin>1 ? "s" : "",
                        tmpsec, "sec", tmpsec>1 ? "s" : "");
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

//////////////////////Gestion des sauvegardes//////////////////////
#define SUPERCRYPTKEY 10;
char *encryptsave(char savepart[20]) //simple and easily crackable encryption
{
    int i;

    for(i = 0; (i < 100 && savepart[i] != '\0'); i++)
        savepart[i] = savepart[i] - SUPERCRYPTKEY;

    return savepart;
}

void writesave() //we write the poorly encrypted value for all stat
{
    stream *savefile = openutf8file("config/stats.cfg", "w");
    if(!savefile) { conoutf(GAME_LANG ? "\fcUnable to write your save file !" : "\fcUnable d'écrite votre fichier de sauvegarde !"); return; }

    int saveID = 0;
    loopi(NUMSTATS + NUMCUST + NUMACHS)
    {
        savefile->printf("%s\n", encryptsave(tempformatstring("%d", saveID >= NUMSTATS + NUMCUST ? succes[saveID-NUMCUST-NUMSTATS] : saveID >= NUMSTATS ? cust[saveID-NUMSTATS] : stat[saveID])));
        saveID++;
    }
    savefile->close();
}

void givestarterkit() //Avant tout le joueur a les customisations de base et le niveau 1, même en cas de sauvegarde corrompue
{
    stat[STAT_LEVEL]++;
    cust[SMI_HAP] = cust[CAPE_CUBE] = cust[TOM_MERDE] = cust[VOI_CORTEX] = rnd(99)+1;
}

void loadsave() //we read the poorly encrypted value for all stat
{
    givestarterkit(); //-> on donne toujours le kit de départ
    stream *savefile = openfile("config/stats.cfg", "r");
    if(!savefile) {genlvl(); return;}

    char buf[50];
    int saveID = 0;

    while(savefile->getline(buf, sizeof(buf)))
    {
        int i;
        //we decrypt the line
        for(i = 0; (i < 500 && buf[i] != '\0'); i++)
            buf[i] = buf[i] + SUPERCRYPTKEY;

        if(saveID < NUMSTATS) sscanf(buf, "%i", &stat[saveID]);
        else if(saveID < NUMCUST + NUMSTATS) sscanf(buf, "%i", &cust[saveID-NUMSTATS]);
        else {int j = 0; sscanf(buf, "%i", &j); succes[saveID-NUMSTATS-NUMCUST] = j;}

        saveID++;
    }
    savefile->close();
    genlvl(); //Génère le niveau après le chargement des statistiques
}

//////////////////////Gestion des succès//////////////////////
bool succes[NUMACHS];
bool achievementlocked(int achID) {return !succes[achID];} //Succès verrouillé ? OUI = TRUE, NON = FALSE

ICOMMAND(unlockach, "i", (int *achID), if(*achID==ACH_PARKOUR || *achID==ACH_EXAM) unlockachievement(*achID));

bool canunlockoffline(int achID)
{
    return achID==ACH_PARKOUR || achID==ACH_FUCKYOU || achID==ACH_EXAM || achID==ACH_TMMONEY || achID==ACH_SURVIVOR || achID==ACH_ELIMINATOR;
}

void unlockachievement(int achID) //Débloque le succès
{
    bool newach = achievementlocked(achID);
    if(canunlockoffline(achID) || IS_ON_OFFICIAL_SERV) //Ne débloque que si serveur officiel sauf succès en solo
    {
        #ifdef _WIN32
            if(IS_USING_STEAM)
            {
                SteamUserStats()->SetAchievement(achievements[achID].achname); //Met le succès à jour côté steam
                SteamUserStats()->StoreStats();
            }
        #endif

        if(newach)
        {
            succes[achID] = true; //Met le succès à jour côté client
            addxpandcc(25, 25);
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
            SteamUserStats()->GetAchievement(achievements[achID].achname, &succes[achID]);
            achID++;
        }
    #endif
}

ICOMMAND(gettotalach, "", (), intret(NUMACHS)); //gets nb of achievements for ui

ICOMMAND(getunlockedach, "", (), //gets nb of unlocked achievements for ui
    int totalachunlocked = 0; int achID = 0;
    loopi(NUMACHS)
    {
        if(!achievementlocked(achID)) totalachunlocked++;
        achID++;
    }
    intret(totalachunlocked);
);

ICOMMAND(getachievementslogo, "i", (int *achID), //gets achievement logo for ui
    if(*achID<0 || *achID>=NUMACHS) result("media/texture/game/notexture.png");
    else
    {
        defformatstring(logodir, "media/interface/achievements/%s%s.jpg", achievements[*achID].achname, achievementlocked(*achID) ? "_no" : "_yes");
        result(logodir);
    }
);

ICOMMAND(getachievementname, "i", (int *achID), //gets achievement name for ui
    if(*achID<0 || *achID>=NUMACHS) result(GAME_LANG ? "Invalid ID" : "ID Invalide");
    else result(GAME_LANG ? achievements[*achID].achnicenameEN : achievements[*achID].achnicenameFR);
);

ICOMMAND(getachievementcolor, "i", (int *achID), //gets achievement color status for ui
    if(*achID<0 || *achID>=NUMACHS) intret(0x777777);
    else intret(achievementlocked(*achID) ? 0xFFC6C6 : 0xD0F3D0);
);
