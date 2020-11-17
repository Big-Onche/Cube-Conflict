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
    while(stat[STAT_XP]>=needxp) //Ajoute un niveau et augmente l'xp demandé pour le niveau suivant
    {
        cclvl++;
        oldneed += 40;
        needxp += oldneed;
        neededxp += 40;
    }

    float pour1 = neededxp, pour2 = stat[STAT_XP]-needxp;
    if(pour1!=0) pourcents = pour2/pour1; //Calcul le pourcentage pour prochain niveau

    switch(cclvl) //Regarde si il y a un succès à déverouiller
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

void addstat(int valeur, int neededstat) // Ajoute la stat très simplement
{
    if(!conserveurofficiel) return;
    stat[neededstat]+= valeur;
}

float menustat(int value) //Récupère les stats pour le menu (float utilisé pour le ratio)
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

//////////////////////Gestion des succès//////////////////////
bool achievementlocked(int achID) {return !succes[achID];} //Succès verrouillé ? OUI = TRUE, NON = FALSE

void unlockachievement(int achID) //Débloque le succès
{
    if(conserveurofficiel && achievementlocked(achID) && usesteam) //Ne débloque que si serveur officiel ET succès verrouillé ET Steam activé
    {
        SteamUserStats()->SetAchievement(achievements[achID].achname); //Met le succès à jour côté steam
        SteamUserStats()->StoreStats();
        succes[achID] = true; //Met le succès à jour côté client
        addxpandcc(25, 25);
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
