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

    f->printf("%s%s%s",
              encryptsave(tempformatstring("loadsavepart1 %d %d %d %d %d %d %d %d %d %d; ", ccxp, stat[0], stat[1], stat[2], stat[3], stat[4], stat[5], stat[6], stat[7], stat[8])),
              encryptsave(tempformatstring("loadsavepart2 %d %d %d %d %d %d %d %d %d %d; ", stat[9], stat[10], stat[11], stat[12], stat[13], stat[14], stat[15], stat[16], stat[17], stat[18])),
              encryptsave(tempformatstring("loadsavepart3 %d %d %d; ", stat[19], stat[20], stat[21]))
             );
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

ICOMMAND(loadsavepart1, "iiiiiiiiii", (int *asave1, int *asave2, int *asave3, int *asave4, int *asave5, int *asave6, int *asave7, int *asave8, int *asave9, int *asave10),
{
    ccxp = *asave1; //Xp donc level
    stat[0] = *asave2; stat[1] = *asave3; stat[2] = *asave4; //Kills, morts, killstrak donc ratio
    stat[3] = *asave5; stat[4] = *asave6; stat[5] = *asave7; stat[6] = *asave8; stat[7] = *asave9; stat[8] = *asave10; //Objets
    genlvl(); //Regénère le niveau afin d'avoir le bon niveau & pourcentage pour prochain niveau
});

ICOMMAND(loadsavepart2, "iiiiiiiiii", (int *bsave1, int *bsave2, int *bsave3, int *bsave4, int *bsave5, int *bsave6, int *bsave7, int *bsave8, int *bsave9, int *bsave10),
{
    stat[9] = *bsave1; stat[10] = *bsave2; stat[11] = *bsave3; stat[12] = *bsave4; stat[13] = *bsave5; //Boosts
    stat[14] = *bsave6; stat[15] = *bsave7; //Armes
    stat[16] = *bsave8; //Drapeaux pris
    stat[17] = *bsave9; //Parties gagnées
    stat[18] = *bsave10; //Armures assistée
});

ICOMMAND(loadsavepart3, "iii", (int *csave1, int *csave2, int *csave3),
{
    stat[19] = *csave1; stat[20] = *csave2; stat[21] = *csave3; //Tps de jeu (secondes, minutes, heures)
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
