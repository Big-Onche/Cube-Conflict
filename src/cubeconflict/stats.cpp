#include <climits>
#include <cstdlib>
#include "cubedef.h"
#include "steam_api.h"

using namespace std;

//////////////////////Gestion de l'xp et niveaux//////////////////////
int ccxp = 0, cclvl = 1, needxp = 40, oldneed = 40, neededxp = 40;
float pourcents = -1;

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

void addxp(int nbxp) // Ajoute l'xp très simplment
{
    if(!conserveurofficiel) return;
    ccxp += nbxp;
    genlvl(); //Recalcule le niveau
}

//////////////////////Gestion des statistiques//////////////////////
int stat[NUMSTATS]; bool succes[NUMACHS];

void addstat(int valeur, int neededstat) // Ajoute la stat très simplement
{
    if(!conserveurofficiel) return;
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

//////////////////////Gestion des succès//////////////////////
bool achievementlocked(int achID) {return !succes[achID];} //Succès verrouillé ? OUI = TRUE, NON = FALSE

void unlockachievement(int achID) //Débloque le succès
{
    if(conserveurofficiel && achievementlocked(achID) && usesteam) //Ne débloque que si succès verrouillé ET Steam activé
    {
        SteamUserStats()->SetAchievement(achievements[achID].achname); //Met le succès à jour côté steam
        SteamUserStats()->StoreStats();
        succes[achID] = true; //Met le succès à jour côté client
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
    formatstring(logodir, "media/interface/achievements/%s%s.jpg", achievements[achID].achname, achievementlocked(achID) ? "_no" : "_yes");
    return logodir;
}
ICOMMAND(getachievementslogo, "i", (int *achID), result(getachievementslogo(*achID)));

const char *getachievementname(int achID) //Récupère le nom d'un succès en particulier
{
    return langage ? achievements[achID].achnicenameEN : achievements[achID].achnicenameFR;
}
ICOMMAND(getachievementname, "i", (int *achID), result(getachievementname(*achID)));

const char *getachievementinfo(int achID) //Récupère la description d'un succès en particulier
{
    return langage ? achievements[achID].achdescEN : achievements[achID].achdescFR;
}
ICOMMAND(getachievementinfo, "i", (int *achID), result(getachievementinfo(*achID)));

const char *getachievementcolor(int achID) //Renvoie une couleur pour savoir si le succes est verrouillé ou non
{
    return achievementlocked(achID) ? "0xD8AA88" : "0x99D899";
}
ICOMMAND(getachievementcolor, "i", (int *achID), result(getachievementcolor(*achID)));
