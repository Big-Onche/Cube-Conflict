#include <climits>
#include <cstdlib>
#include "engine.h"
#include "cubedef.h"

using namespace std;

int ccxp = 0, cclvl = 1, needxp = 40, oldneed = 40, neededxp = 40;
float pourcents = -1;

int stat[NUMSTATS];

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

    f->printf("%s%s", encryptsave(tempformatstring("loadsavepart1 %d %d %d %d %d %d %d %d %d %d; ", ccxp, stat[0], stat[1], stat[2], stat[3], stat[4], stat[5], stat[6], stat[7], stat[8])), encryptsave(tempformatstring("loadsavepart2 %d %d %d %d %d", stat[9], stat[10], stat[11], stat[12], stat[13])));
    //f->printf("%s\n", );
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

ICOMMAND(loadsavepart1, "iiiiiiiiii", (int *save1, int *save2, int *save3, int *save4, int *save5, int *save6, int *save7, int *save8, int *save9, int *save10),
{
    ccxp = *save1; //Xp donc level
    stat[0] = *save2; stat[1] = *save3; stat[2] = *save4; //Kills, morts, killstrak donc ratio
    stat[3] = *save5; stat[4] = *save6; stat[5] = *save7; stat[6] = *save8; stat[7] = *save9; stat[8] = *save10; //Objets
    genlvl(); //Regénère le niveau afin d'avoir le bon niveau & pourcentage pour prochain niveau
});

ICOMMAND(loadsavepart2, "iiiiiiiiiiii", (int *save1, int *save2, int *save3, int *save4, int *save5),
{
    stat[9] = *save1; stat[10] = *save2; stat[11] = *save3; stat[12] = *save4; stat[13] = *save5; //Boosts
});

void addxp(int nbxp) // Ajoute l'xp très simplment
{
    //if(!con_serveurofficiel) return;
    ccxp += nbxp;
    genlvl(); //Recalcule le niveau
}

void addstat(int valeur, int neededstat) // Ajoute la stat très simplement
{
    //if(!con_serveurofficiel) return;
    stat[neededstat]+= valeur;
}

float menustat(int value)
{
    if(value<0)
    {
        switch(value)
        {
            case -3:
            {
                float float_kills = stat[0], float_morts = stat[1];
                int ratiokd = (float_kills/(float_morts == 0 ? 1 : float_morts))*100;
                float ratiokdmenu = ratiokd;
                return ratiokdmenu/100;
            }
            case -2: return cclvl;
            case -1: return ccxp;
            default: return 0;
        }
    }
    else return stat[value];

}
