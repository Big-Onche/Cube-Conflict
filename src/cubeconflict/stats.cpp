#include <climits>
#include <cstdlib>
#include "cubedef.h"
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
    }

    float pour1 = neededxp, pour2 = stat[STAT_XP]-needxp;
    if(pour1!=0) pourcents = pour2/pour1; //Calcul le pourcentage pour prochain niveau

    switch(cclvl) //Regarde si il y a un succ�s � d�verouiller
    {
        case 5: unlockachievement(ACH_POSTULANT); break;
        case 10: unlockachievement(ACH_STAGIAIRE); break;
        case 20: unlockachievement(ACH_SOLDAT); break;
        case 50: unlockachievement(ACH_LIEUTENANT); break;
        case 100: unlockachievement(ACH_MAJOR);
    }

    game::player1->level = cclvl;
}

//////////////////////Gestion des statistiques//////////////////////
void addxpandcc(int nbxp, int cc) // Ajoute l'xp et/ou les CC
{
    if(!conserveurofficiel) return;
    stat[STAT_XP] += nbxp;
    genlvl(); //Recalcule le niveau

    stat[STAT_CC]+=cc;
}

int stat[NUMSTATS]; bool succes[NUMACHS];

void addstat(int valeur, int neededstat) // Ajoute la stat tr�s simplement
{
    if(!conserveurofficiel) return;
    stat[neededstat]+= valeur;
}

float menustat(int value) //R�cup�re les stats pour le menu (float utilis� pour le ratio)
{
    switch(value)
    {
        case -2:
        {
            float fk = stat[STAT_KILLS], fm = stat[STAT_MORTS];
            int ratiokd = (fk/(fk == 0 ? 1 : fm)*100);
            return ratiokd/100.f;
        }
        case -1: return cclvl; break;

        default: return stat[value];
    }
}

//////////////////////Gestion des sauvegardes//////////////////////
char *encryptsave(char savepart[20])
{
    int i;

    for(i = 0; (i < 100 && savepart[i] != '\0'); i++)
        savepart[i] = savepart[i] - 10;

    return savepart;
}

void writesave()
{
    stream *f = openutf8file("config/stats.cfg", "w");

    int statID = 0;
    loopi(NUMSTATS)
    {
        f->printf("%s\n", encryptsave(tempformatstring("%d", stat[statID])));
        statID++;
    }
}

void loadsave()
{
    stream *statlist = openfile("config/stats.cfg", "r");
    if(!statlist) return;

    char buf[50];
    int statID = 0;

    while(statlist->getline(buf, sizeof(buf)))
    {
        int i;

        for(i = 0; (i < 500 && buf[i] != '\0'); i++)
            buf[i] = buf[i] + 10;

        sscanf(buf, "%i", &stat[statID]);
        statID++;
    }
    statlist->close();

    genlvl();
}

//////////////////////Gestion des succ�s//////////////////////
bool achievementlocked(int achID) {return !succes[achID];} //Succ�s verrouill� ? OUI = TRUE, NON = FALSE

void unlockachievement(int achID) //D�bloque le succ�s
{
    if(conserveurofficiel && achievementlocked(achID) && usesteam) //Ne d�bloque que si serveur officiel ET succ�s verrouill� ET Steam activ�
    {
        SteamUserStats()->SetAchievement(achievements[achID].achname); //Met le succ�s � jour c�t� steam
        SteamUserStats()->StoreStats();
        succes[achID] = true; //Met le succ�s � jour c�t� client
        addxpandcc(25, 25);
    }
}

void getsteamachievements() //R�cup�re les succ�s enregistr�s sur steam
{
    int achID = 0;
    loopi(NUMACHS)
    {
        SteamUserStats()->GetAchievement(achievements[achID].achname, &succes[achID]);
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
