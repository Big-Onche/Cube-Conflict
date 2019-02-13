#include <climits>
#include <cstdlib>

using namespace std;

int ccxp = 0, cclvl = 1, needxp = 40, oldneed = 40, neededxp = 40;
float pourcents = -1;

int stat_kills, stat_morts, stat_killstreak;
enum {STAT_KILLS, STAT_MORTS, STAT_KILLSTREAK};

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

    f->printf("%s", encryptsave(tempformatstring("loadsave %d %d %d %d", ccxp, stat_kills, stat_morts, stat_killstreak)));
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

ICOMMAND(loadsave, "iiii", (int *save1, int *save2, int *save3, int *save4),
{
    ccxp = *save1;
    stat_kills = *save2;
    stat_morts = *save3;
    stat_killstreak = *save4;
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
    }
}

ICOMMAND(getmenustat, "i", (int *value),
{
    switch(*value)
    {
        case -1: conoutf("%d", ccxp); break;
        case 0: conoutf("%d", stat_kills); break;
        case 1: conoutf("%d", stat_morts); break;
        case 2: conoutf("%d", stat_killstreak); break;
    }
});
