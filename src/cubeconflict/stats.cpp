//stats.cpp: where we manage stats, achievements and local saves

#include "steam_api.h"
#include "ccheader.h"
#include "stats.h"
#include "customisation.h"

using namespace std;

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
        if(isconnected()) {playsound(S_LEVELUP); message[MSG_LEVELUP]=totalmillis;}
    }

    float pour1 = totalneededxp, pour2 = stat[STAT_XP]-xpneededfornextlvl;
    if(pour1!=0) pourcents = pour2/pour1; //Calcul le pourcentage pour prochain niveau et évite une div. par zéro

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

//////////////////////Gestion des statistiques//////////////////////
int stat[NUMSTATS];

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

float kdratio = 0.f;
void calcratio() //Calcule le ratio pour l'afficher dans le menu (non sauvegardé)
{
    float fk = stat[STAT_KILLS], fm = stat[STAT_MORTS];
    kdratio = (fk/(fm == 0 ? 1 : fm)*1.f);
}

string statlogodir;
const char *getstatlogo(int statID) //Récupère le logo d'une statistique en particulier
{
    if(statID>NUMSTATS) return "media/texture/game/notexture.png";
    formatstring(statlogodir, "%s", statslist[statID].statlogo);
    return statlogodir;
}
ICOMMAND(getstatlogo, "i", (int *statID), result(getstatlogo(*statID)));

string tmp;
const char *getstatinfo(int statID, bool onlyvalue) //Récupère la description d'une statistique
{
    if(statID>NUMSTATS) return langage ? "Invalid ID" : "ID Invalide";
    switch(statID)
    {
        //Based on STAT_KILLS & STAT_MORTS, STAT_KDRATIO not saved.
        case STAT_KDRATIO:
        {
            calcratio();
            formatstring(tmp, "%s%s%.1f", onlyvalue ? "" : langage ? statslist[statID].statnicenameEN : statslist[statID].statnicenameFR, onlyvalue ? "" : " : ", kdratio);
            break;
        }
        //Easy secs to HH:MM:SS converter and displayer
        case STAT_TIMEPLAYED:
        {
            int tmpsec = stat[STAT_TIMEPLAYED], tmpmin = 0, tmphr = 0;
            while(tmpsec > 59) { tmpmin++ ; tmpsec-=60; }
            while(tmpmin > 59) { tmphr++ ; tmpmin-=60; }

            formatstring(tmp, "%s : %d %s%s %d %s%s %d %s%s",  langage ? statslist[statID].statnicenameEN : statslist[statID].statnicenameFR,
                        tmphr, langage ? "hour" : "heure", tmphr>1 ? "s" : "",
                        tmpmin, "min", tmpmin>1 ? "s" : "",
                        tmpsec, "sec", tmpsec>1 ? "s" : "");
        }
        break;
        //when we need to display the stat after description (rare cases)
        case STAT_DAMMAGERECORD:
        case STAT_MAXKILLDIST:
        case STAT_LEVEL:
        case STAT_BASEHACK:
            formatstring(tmp, "%s%s%d%s", onlyvalue ? "" : langage ? statslist[statID].statnicenameEN : statslist[statID].statnicenameFR, onlyvalue ? "" : " : ", stat[statID], statID!=STAT_MAXKILLDIST ? statID==STAT_BASEHACK ? langage ? " seconds" : " secondes" : "" : langage ? " meters" : " mètres");
            break;
        //otherwise, we put the stat value before the description by default
        default: formatstring(tmp, "%d%s%s", stat[statID], onlyvalue ? "" : " ", onlyvalue ? "" : langage ? statslist[statID].statnicenameEN : statslist[statID].statnicenameFR);
    }

    return tmp;
}
ICOMMAND(getstatinfo, "ii", (int *statID, bool *onlyvalue), result(getstatinfo(*statID, *onlyvalue)));

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
    if(!savefile) { conoutf(langage ? "\fcUnable to write your save file !" : "\fcUnable d'écrite votre fichier de sauvegarde !"); return; }

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
    cust[SMI_HAP] = cust[SMI_NOEL] = cust[CAPE_CUBE] = cust[TOM_MERDE] = cust[VOI_CORTEX] = rnd(99)+1;
}

void loadsave() //we read the poorly encrypted value for all stat
{
    givestarterkit(); //-> on donne toujours le kit de départ
    stream *savefile = openfile("config/stats.cfg", "r");
    if(!savefile) return;

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

string tempachname;
void unlockachievement(int achID) //Débloque le succès
{
    if(IS_ON_OFFICIAL_SERV && achievementlocked(achID)) //Ne débloque que si serveur officiel ET succès verrouillé
    {
        if(IS_USING_STEAM)
        {
            SteamUserStats()->SetAchievement(achievements[achID].achname); //Met le succès à jour côté steam
            SteamUserStats()->StoreStats();
        }

        succes[achID] = true; //Met le succès à jour côté client
        addxpandcc(25, 25);
        playsound(S_ACHIEVEMENTUNLOCKED);
        formatstring(tempachname, "%s", langage ? achievements[achID].achnicenameEN : achievements[achID].achnicenameFR);
        message[MSG_ACHUNLOCKED] = totalmillis;
    }
}

void getsteamachievements() //Récupère les succès enregistrés sur steam
{
    int achID = 0;
    loopi(NUMACHS)
    {
        SteamUserStats()->GetAchievement(achievements[achID].achname, &succes[achID]);
        achID++;
    }
}

string achcount;
const char *getachievementcount() //Récupère le nombre de succès déverrouillés
{
    int totalachunlocked = 0, achID = 0;

    loopi(NUMACHS)
    {
        if(!achievementlocked(achID)) totalachunlocked++;
        achID++;
    }

    formatstring(achcount, "%d/%d", totalachunlocked, NUMACHS);
    return achcount;
}
ICOMMAND(getachievementcount, "", (), result(getachievementcount()));

string logodir;
const char *getachievementslogo(int achID) //Récupère le logo d'un succès en particulier
{
    if(achID>NUMACHS) return "media/texture/game/notexture.png";
    formatstring(logodir, "media/interface/achievements/%s%s.jpg", achievements[achID].achname, achievementlocked(achID) ? "_no" : "_yes");
    return logodir;
}
ICOMMAND(getachievementslogo, "i", (int *achID), result(getachievementslogo(*achID)));

const char *getachievementname(int achID) //Récupère le nom d'un succès en particulier
{
    if(achID>NUMACHS) return langage ? "Invalid ID" : "ID Invalide";
    return langage ? achievements[achID].achnicenameEN : achievements[achID].achnicenameFR;
}
ICOMMAND(getachievementname, "i", (int *achID), result(getachievementname(*achID)));

const char *getachievementinfo(int achID) //Récupère la description d'un succès en particulier
{
    if(achID>NUMACHS) return langage ? "Invalid ID" : "ID Invalide";
    return langage ? achievements[achID].achdescEN : achievements[achID].achdescFR;
}
ICOMMAND(getachievementinfo, "i", (int *achID), result(getachievementinfo(*achID)));

const char *getachievementcolor(int achID) //Renvoie une couleur pour savoir si le succes est verrouillé ou non
{
    if(achID>NUMACHS) return "0x777777";
    return achievementlocked(achID) ? "0xD8AA88" : "0x99D899";
}
ICOMMAND(getachievementcolor, "i", (int *achID), result(getachievementcolor(*achID)));
