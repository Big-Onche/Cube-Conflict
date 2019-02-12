#include "game.h"

#include <climits>
#include <cstdlib>

using namespace std;

VAR(niveaumenu, 1, 1, 5000);
VAR(xpmenu, 0, 0, 999999999);

int ccxp = 0, lvl = 1, needxp = 40, oldneed = 40, neededxp = 40;
float pourcents = -1;

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

    f->printf("%s\n", encryptsave(tempformatstring("save_0 %d", ccxp)));
}

void genlvl() //Calcule le niveau du joueur
{
    while(ccxp>=needxp) //Ajoute un niveau et augmente l'xp demandé pour le niveau suivant
    {
        lvl++;
        oldneed += 40;
        needxp += oldneed;
        neededxp += 40;
    }
    float pour1 = neededxp, pour2 = ccxp-needxp;
    if(pour1!=0) pourcents = pour2/pour1; //Calcul le pourcentage
    niveaumenu = lvl;
    xpmenu = ccxp;
}

VARF(save_0, 0, 0, 999999999, ccxp = save_0; genlvl());

void addxp(int nbxp) // Ajoute l'xp très simplments
{
    //if(!con_serveurofficiel) return;
    ccxp += nbxp;
    genlvl(); //Recalcule le niveau
}
