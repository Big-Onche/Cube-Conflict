//stats.cpp: where we manage stats, achievements and local saves

#include "ccheader.h"
#include "steam_api.h"

using namespace std;

//////////////////////Gestion de l'xp et niveaux//////////////////////
int cclvl = 1, needxp = 40, oldneed = 40, neededxp = 40;
float pourcents = -1;

void genlvl() //Calcule le niveau du joueur
{
    while(stat[STAT_XP]>=needxp) //Ajoute un niveau et augmente l'xp demand� pour le niveau suivant
    {
        cclvl++;
        oldneed += 40;
        needxp += oldneed;
        neededxp += 40;
        if(isconnected()) {playsound(S_LEVELUP); message[MSG_LEVELUP]=totalmillis;}
    }

    float pour1 = neededxp, pour2 = stat[STAT_XP]-needxp;
    if(pour1!=0) pourcents = pour2/pour1; //Calcul le pourcentage pour prochain niveau et �vite une div. par z�ro

    switch(cclvl) //Regarde si il y a un succ�s � d�verrouiller
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
void addstat(int valeur, int statID, bool rewrite) // Ajoute la stat tr�s simplement
{
    if(!IS_ON_OFFICIAL_SERV) return;
    if(rewrite) stat[statID] = valeur;
    else stat[statID] += valeur;

    savejustincase++;
    if(savejustincase==100) {writesave(); savejustincase=0;} //Sauvegarde toutes les 100 stats ajout�s (�a monte vite avec les munitions tir�es), �a limite la casse en cas de crash
}

float kdratio = 0.f;
void calcratio() //Calcule le ratio pour l'afficher dans le menu (non sauvegard�)
{
    float fk = stat[STAT_KILLS], fm = stat[STAT_MORTS];
    kdratio = (fk/(fm == 0 ? 1 : fm)*1.f);
}

string statlogodir;
const char *getstatlogo(int statID) //R�cup�re le logo d'un succ�s en particulier
{
    if(statID>NUMSTATS) return "media/texture/game/notexture.png";
    formatstring(statlogodir, "%s", statslist[statID].statlogo);
    return statlogodir;
}
ICOMMAND(getstatlogo, "i", (int *statID), result(getstatlogo(*statID)));

string tmp;
const char *getstatinfo(int statID, bool onlyvalue) //R�cup�re la description d'une statistique
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
            formatstring(tmp, "%s%s%d%s", onlyvalue ? "" : langage ? statslist[statID].statnicenameEN : statslist[statID].statnicenameFR, onlyvalue ? "" : " : ", stat[statID], statID!=STAT_MAXKILLDIST ? "" : langage ? " meters" : " m�tres");
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
    if(!savefile) { conoutf(langage ? "\fcUnable to write your save file !" : "\fcUnable d'�crite votre fichier de sauvegarde !"); return; }

    int statID = 0;
    loopi(NUMSTATS)
    {
        savefile->printf("%s\n", encryptsave(tempformatstring("%d", stat[statID])));
        statID++;
    }

    savefile->close();
}

void givestarterkit() //Avant tout le joueur a les customisations de base et le niveau 1, m�me en cas de sauvegarde corrompue
{
    stat[STAT_LEVEL]++;
    stat[SMI_HAP] = stat[SMI_NOEL] = stat[CAPE_CUBE] = stat[TOM_MERDE] = stat[VOI_CORTEX] = rnd(99)+1;
}

void loadsave() //we read the poorly encrypted value for all stat
{
    stream *savefile = openfile("config/stats.cfg", "r");
    if(!savefile) {givestarterkit(); return;} //Si le fichier de stats n'existe pas -> on donne le kit de d�part

    char buf[50];
    int statID = 0;

    while(savefile->getline(buf, sizeof(buf)))
    {
        int i;
        //we decrypt the line
        for(i = 0; (i < 500 && buf[i] != '\0'); i++)
            buf[i] = buf[i] + SUPERCRYPTKEY;

        sscanf(buf, "%i", &stat[statID]);
        statID++;
    }
    savefile->close();

    genlvl(); //G�n�re le niveau apr�s le chargement des statistiques
}

//////////////////////Gestion des succ�s//////////////////////
bool succes[NUMACHS];
bool achievementlocked(int achID) {return !succes[achID];} //Succ�s verrouill� ? OUI = TRUE, NON = FALSE

VAR(totalachunlocked, 0, 0, 32);

string tempachname;
void unlockachievement(int achID) //D�bloque le succ�s
{
    if(IS_ON_OFFICIAL_SERV && achievementlocked(achID)) //Ne d�bloque que si serveur officiel ET succ�s verrouill�
    {
        if(IS_USING_STEAM)
        {
            SteamUserStats()->SetAchievement(achievements[achID].achname); //Met le succ�s � jour c�t� steam
            SteamUserStats()->StoreStats();
        }

        succes[achID] = true; //Met le succ�s � jour c�t� client
        addxpandcc(25, 25);
        playsound(S_ACHIEVEMENTUNLOCKED);
        formatstring(tempachname, "%s", langage ? achievements[achID].achnicenameEN : achievements[achID].achnicenameFR);
        message[MSG_ACHUNLOCKED] = totalmillis;
        totalachunlocked++;
    }
}

void getsteamachievements() //R�cup�re les succ�s enregistr�s sur steam
{
    int achID = 0;
    loopi(NUMACHS)
    {
        SteamUserStats()->GetAchievement(achievements[achID].achname, &succes[achID]);
        if(!achievementlocked(achID)) totalachunlocked++;
        achID++;
    }
}

string logodir;
const char *getachievementslogo(int achID) //R�cup�re le logo d'un succ�s en particulier
{
    if(achID>NUMACHS) return "media/texture/game/notexture.png";
    formatstring(logodir, "media/interface/achievements/%s%s.jpg", achievements[achID].achname, achievementlocked(achID) ? "_no" : "_yes");
    return logodir;
}
ICOMMAND(getachievementslogo, "i", (int *achID), result(getachievementslogo(*achID)));

const char *getachievementname(int achID) //R�cup�re le nom d'un succ�s en particulier
{
    if(achID>NUMACHS) return langage ? "Invalid ID" : "ID Invalide";
    return langage ? achievements[achID].achnicenameEN : achievements[achID].achnicenameFR;
}
ICOMMAND(getachievementname, "i", (int *achID), result(getachievementname(*achID)));

const char *getachievementinfo(int achID) //R�cup�re la description d'un succ�s en particulier
{
    if(achID>NUMACHS) return langage ? "Invalid ID" : "ID Invalide";
    return langage ? achievements[achID].achdescEN : achievements[achID].achdescFR;
}
ICOMMAND(getachievementinfo, "i", (int *achID), result(getachievementinfo(*achID)));

const char *getachievementcolor(int achID) //Renvoie une couleur pour savoir si le succes est verrouill� ou non
{
    if(achID>NUMACHS) return "0x777777";
    return achievementlocked(achID) ? "0xD8AA88" : "0x99D899";
}
ICOMMAND(getachievementcolor, "i", (int *achID), result(getachievementcolor(*achID)));
