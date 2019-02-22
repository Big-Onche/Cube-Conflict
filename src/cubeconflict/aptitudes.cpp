#include "game.h"
#include "engine.h"
#include "../cubeconflict/cubedef.h"

//Commandes d'aptitudes (touches 1, 2 et 3)
namespace game
{
    void aptitude_1(gameent *d) //Touche 1
    {
        if(d->state==CS_DEAD) return;

        if(d==player1)
        {
            switch(player1->aptitude)
            {
                case APT_MEDECIN: if(player1->ammo[GUN_MEDIGUN]>0) {player1->gunselect = GUN_MEDIGUN; playsound(S_WEAPLOAD, &player1->o); break; }
                case APT_KAMIKAZE: player1->gunselect = GUN_KAMIKAZE; playsound(S_WEAPLOAD, &player1->o); break;
            }
        }

        int neededdata = 0;

        switch(d->aptitude)
        {
            case APT_MAGICIEN:
            {
                //Sort de vitesse
                if(!d->sort1pret || d->mana<sorts[neededdata].mana1) {if(d==player1)playsound(S_SORTIMPOSSIBLE); break; }

                d->aptisort1 = 250;
                d->lastspecial1update = totalmillis;
                addmsg(N_SENDSORT1, "rci", d, d->aptisort1);
                d->mana -= sorts[neededdata].mana1;
                d->sort1pret = false;
                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 250);
                //if(con_serveurofficiel) stat_sortslances++;
                break;
            }
            case APT_PHYSICIEN:
            {
                //Sort de régénération de bouclier
                neededdata++;
                if(!d->sort1pret || d->mana<sorts[neededdata].mana1) {if(d==player1)playsound(S_SORTIMPOSSIBLE); break; }

                d->aptisort1 = 1500;
                d->lastspecial1update = totalmillis;
                addmsg(N_SENDSORT1, "rci", d, d->aptisort1);
                d->mana -= sorts[neededdata].mana1;
                d->sort1pret = false;
                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 250);
                playsound(S_SORTPHY1, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 250);
                //if(con_serveurofficiel) stat_sortslances++;
                break;
            }
            case APT_PRETRE:
            {
                //Sort qui double les objets pris
                neededdata+=2;
                if(!d->sort1pret || d->mana<sorts[neededdata].mana1) {if(d==player1)playsound(S_SORTIMPOSSIBLE); break; }

                d->aptisort1 = 4000;
                d->lastspecial1update = totalmillis;
                addmsg(N_SENDSORT1, "rci", d, d->aptisort1);
                d->mana -= sorts[neededdata].mana1;
                d->sort1pret = false;
                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 250);
                playsound(S_SORTPRETRE1, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 250);
                //if(con_serveurofficiel) stat_sortslances++;
                break;
            }
        }
    }
    ICOMMAND(aptitude_1, "", (), aptitude_1(player1));

    void aptitude_2(gameent *d)
    {
        if(d->state==CS_DEAD) return;

        int neededdata=0;

        switch(d->aptitude)
        {
            case APT_MAGICIEN:
            {
                // Sort précision et dégâts
                if(!d->sort2pret || d->mana<sorts[neededdata].mana2) {if(d==player1)playsound(S_SORTIMPOSSIBLE); break; }

                d->aptisort2 = 5000;
                d->lastspecial2update = totalmillis;
                addmsg(N_SENDSORT2, "rci", d, d->aptisort2);
                d->mana -= d->mana<sorts[neededdata].mana2;
                d->sort2pret = false;
                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 150);
                playsound(S_SORTMAGE2, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 150);
                //if(con_serveurofficiel) stat_sortslances++;
                break;
            }
            case APT_PHYSICIEN:
            {
                // Sort invisibilité
                neededdata++;
                if(!d->sort2pret || d->mana<sorts[neededdata].mana2) {if(d==player1)playsound(S_SORTIMPOSSIBLE); break; }

                d->aptisort2 = 4000;
                d->lastspecial2update = totalmillis;
                addmsg(N_SENDSORT2, "rci", d, d->aptisort2);
                d->mana -= d->mana<sorts[neededdata].mana2;
                d->sort2pret = false;
                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 150);
                playsound(S_SORTPHY2, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 150);
                //if(con_serveurofficiel) stat_sortslances++;
                break;
            }
            case APT_PRETRE:
            {
                //Sort qui régénere la santé
                neededdata+=2;
                if(!d->sort2pret || d->mana<sorts[neededdata].mana2) {if(d==player1)playsound(S_SORTIMPOSSIBLE); break; }

                d->aptisort2 = 5000;
                d->lastspecial2update = totalmillis;
                addmsg(N_SENDSORT2, "rci", d, d->aptisort2);
                d->mana -= d->mana<sorts[neededdata].mana2;
                d->sort2pret = false;
                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 250);
                playsound(S_SORTPRETRE2, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 250);
                //if(con_serveurofficiel) stat_sortslances++;
                break;
            }
        }
    }
    ICOMMAND(aptitude_2, "", (), aptitude_2(player1));

    void aptitude_3(gameent *d)
    {
        if(d->state==CS_DEAD) return;

        int neededdata=0;

        switch(d->aptitude)
        {
            case APT_MAGICIEN:
            {
                //Sort de résistance ultime
                if(!d->sort3pret || d->mana<sorts[neededdata].mana3) {if(d==player1)playsound(S_SORTIMPOSSIBLE); break; }

                d->aptisort3 = 3000;
                d->lastspecial3update = totalmillis;
                addmsg(N_SENDSORT3, "rci", d, d->aptisort3);
                d->mana -= sorts[neededdata].mana3;
                d->sort3pret = false;
                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 150);
                playsound(S_SORTMAGE3, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 150);
                //if(con_serveurofficiel) stat_sortslances++;
                break;
            }
            case APT_PHYSICIEN:
            {
                //Sort jet-pack
                neededdata++;
                if(!d->sort3pret || d->mana<sorts[neededdata].mana3) {if(d==player1)playsound(S_SORTIMPOSSIBLE); break; }

                d->aptisort3 = 6000;
                d->lastspecial3update = totalmillis;
                addmsg(N_SENDSORT3, "rci", d, d->aptisort3);
                d->mana -= sorts[neededdata].mana3;
                d->sort3pret = false;
                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 150);
                playsound(S_SORTPHY3, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 150);
                //if(con_serveurofficiel) stat_sortslances++;
                break;
            }
            case APT_PRETRE:
            {
                //Sort qui multiplie par 2.5x la cadence de tirs
                neededdata+=2;
                if(!d->sort3pret || d->mana<sorts[neededdata].mana3) {if(d==player1)playsound(S_SORTIMPOSSIBLE); break; }

                d->aptisort3 = 4000;
                d->lastspecial3update = totalmillis;
                addmsg(N_SENDSORT3, "rci", d, d->aptisort3);
                d->mana -= sorts[neededdata].mana3;
                d->sort3pret = false;
                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 150);
                playsound(S_SORTPRETRE3, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 150);
                //if(con_serveurofficiel) stat_sortslances++;
                break;
            }
        }
    }
    ICOMMAND(aptitude_3, "", (), aptitude_3(player1));

    void updatespecials(gameent *d) //Permet de réarmer les sorts en fonction de la durée de rechargement de ceux-ci
    {
        switch(d->aptitude)
        {
            case APT_MAGICIEN:
            {
                // Sort flash
                if(totalmillis-d->lastspecial1update >= 3000 && !d->sort1pret) {if(d==player1)playsound(S_SORTPRET); d->sort1pret = true; }
                // Sort précision
                if(totalmillis-d->lastspecial2update >= 9000 && !d->sort2pret) {if(d==player1)playsound(S_SORTPRET); d->sort2pret = true; }
                // Sort résistance
                if(totalmillis-d->lastspecial3update >= 6000 && !d->sort3pret) {if(d==player1)playsound(S_SORTPRET); d->sort3pret = true; }
                break;
            }
            case APT_PHYSICIEN:
            {
                // Sort régénération de bouclier
                if(totalmillis-d->lastspecial1update >= 3000 && !d->sort1pret) {if(d==player1)playsound(S_SORTPRET); d->sort1pret = true; }
                // Sort invisibilité
                if(totalmillis-d->lastspecial2update >= 5000 && !d->sort2pret) {if(d==player1)playsound(S_SORTPRET); d->sort2pret = true; }
                // Sort jet-pack
                if(totalmillis-d->lastspecial3update >= 9000 && !d->sort3pret) {if(d==player1)playsound(S_SORTPRET); d->sort3pret = true; }
                break;
            }
            case APT_PRETRE:
            {
                // Sort qui double les objets pris
                if(totalmillis-d->lastspecial1update >= 5000 && !d->sort1pret) {if(d==player1)playsound(S_SORTPRET); d->sort1pret = true; }
                // Sort retour de balles
                if(totalmillis-d->lastspecial2update >= 8000 && !d->sort2pret) {if(d==player1)playsound(S_SORTPRET); d->sort2pret = true; }
                // Sort d'immportalité
                if(totalmillis-d->lastspecial3update >= 10000 && !d->sort3pret) {if(d==player1)playsound(S_SORTPRET); d->sort3pret = true; }
                break;
            }
        }
    }
}
