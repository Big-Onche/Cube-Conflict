#include <climits>
#include <cstdlib>
#include "engine.h"
#include "../cubeconflict/cubedef.h"

using namespace std;

int ccxp = 0, cclvl = 1, needxp = 40, oldneed = 40, neededxp = 40;
float pourcents = -1;

int stat_kills, stat_morts, stat_killstreak, stat_bouclierbois, stat_bouclierfer, stat_bouclieror, stat_boucliermagnetique;

//////////////////////Gestion des sauvegardes//////////////////////
char *encryptsave(char savepart[100])
{
    int i;

    for(i = 0; (i < 100 && savepart[i] != '\0'); i++)
        savepart[i] = savepart[i] + 10;

    return savepart;
}

void writesave()
{
    stream *f = openutf8file("config/sauvegarde.cfg", "w");

    f->printf("%s", encryptsave(tempformatstring("loadsave %d %d %d %d %d %d %d %d", ccxp, stat_kills, stat_morts, stat_killstreak, stat_bouclierbois, stat_bouclierfer, stat_bouclieror, stat_boucliermagnetique)));
}

//////////////////////Gestion de l'xp et autres statistiques//////////////////////
void genlvl() //Calcule le niveau du joueur
{
    while(ccxp>=needxp) //Ajoute un niveau et augmente l'xp demandé pour le niveau suivant
    {
        cclvl++;
        oldneed += 40;
        needxp += oldneed;
        neededxp += 40;
    }
    float pour1 = neededxp, pour2 = ccxp-needxp;
    if(pour1!=0) pourcents = pour2/pour1; //Calcul le pourcentage pour prochain niveau
}

ICOMMAND(loadsave, "iiiiiiii", (int *save1, int *save2, int *save3, int *save4, int *save5, int *save6, int *save7, int *save8),
{
    ccxp = *save1;
    stat_kills = *save2;
    stat_morts = *save3;
    stat_killstreak = *save4;
    stat_bouclierbois = *save5;
    stat_bouclierfer = *save6;
    stat_bouclieror = *save7;
    stat_boucliermagnetique = *save8;
    genlvl(); //Regénère le niveau afin d'avoir le bon niveau & pourcentage pour prochain niveau
});

void addxp(int nbxp) // Ajoute l'xp très simplment
{
    //if(!con_serveurofficiel) return;
    ccxp += nbxp;
    genlvl(); //Recalcule le niveau
}

void addstat(int valeur, int stat) // Ajoute la stat très simplement
{
    //if(!con_serveurofficiel) return;
    switch(stat)
    {
        case STAT_KILLS: stat_kills += valeur; break;
        case STAT_MORTS: stat_morts += valeur; break;
        case STAT_KILLSTREAK: stat_killstreak += valeur; break;
        case STAT_BOUCLIERBOIS: stat_bouclierbois += valeur; break;
        case STAT_BOUCLIERFER: stat_bouclierfer += valeur; break;
        case STAT_BOUCLIEROR: stat_bouclieror += valeur; break;
        case STAT_BOUCLIERMAGNETIQUE: stat_boucliermagnetique += valeur; break;
    }
}

float menustat(int value)
{
    switch(value)
    {
        case -3:
        {
            float float_kills = stat_kills, float_morts = stat_morts;
            int ratiokd = (float_kills/(float_morts == 0 ? 1 : float_morts))*100;
            float ratiokdmenu = ratiokd;
            return ratiokdmenu/100;
        }
        case -2: return cclvl;
        case -1: return ccxp;
        case 0: return stat_kills;
        case 1: return stat_morts;
        case 2: return stat_killstreak;
        case 3: return stat_bouclierbois;
        case 4: return stat_bouclierfer;
        case 5: return stat_bouclieror;
        case 6: return stat_boucliermagnetique;
        default: return 0;
    }
}
