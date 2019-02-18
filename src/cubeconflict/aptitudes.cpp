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

        switch(d->aptitude)
        {
            case APT_MAGICIEN:
            {
                //Sort de vitesse
                if(!d->sort1pret || d->mana<30) {if(d==player1)playsound(S_SORTIMPOSSIBLE); break; }

                d->aptisort1 = 250;
                d->lastspecial1update = totalmillis;
                d->mana -= 30;
                d->sort1pret = false;
                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 250);
                //if(con_serveurofficiel) stat_sortslances++;
                break;
            }
        }
    }
    ICOMMAND(aptitude_1, "", (), aptitude_1(player1));

    void aptitude_2(gameent *d)
    {
        if(d->state==CS_DEAD) return;

        switch(d->aptitude)
        {
            case APT_MAGICIEN:
            {
                if(!d->sort2pret || d->mana<40) {if(d==player1)playsound(S_SORTIMPOSSIBLE); break; }

                d->aptisort2 = 5000;
                d->lastspecial2update = totalmillis;
                d->mana -= 40;
                d->sort2pret = false;
                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 150);
                playsound(S_SORTMAGE2, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 150);
                //if(con_serveurofficiel) stat_sortslances++;
                break;
            }
        }
    }
    ICOMMAND(aptitude_2, "", (), aptitude_2(player1));

    void aptitude_3(gameent *d)
    {
        if(d->state==CS_DEAD) return;

        switch(d->aptitude)
        {
            case APT_MAGICIEN:
            {
                //Sort de résistance ultime
                if(!d->sort3pret || d->mana<60) {if(d==player1)playsound(S_SORTIMPOSSIBLE); break; }

                d->aptisort3 = 3000;
                d->lastspecial3update = totalmillis;
                addmsg(N_SENDSORT3, "rci", d, d->aptisort3);
                d->mana -= 60;
                d->sort3pret = false;
                d->sended3 = false;
                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 150);
                playsound(S_SORTMAGE3, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 150);
                //if(con_serveurofficiel) stat_sortslances++;
                break;
            }
        }
    }
    ICOMMAND(aptitude_3, "", (), aptitude_3(player1));

    void updatespecials(gameent *d)
    {
        switch(d->aptitude)
        {
            case APT_MAGICIEN:
            {
                // Sort flash
                if(totalmillis-d->lastspecial1update >= d->aptisort1)
                {
                    d->aptisort1=0;
                    if(totalmillis-d->lastspecial1update >= 3000 && !d->sort1pret) {if(d==player1)playsound(S_SORTPRET); d->sort1pret = true; }
                }

                // Sort précision
                if(totalmillis-d->lastspecial2update >= d->aptisort2)
                {
                    d->aptisort2=0;
                    if(totalmillis-d->lastspecial2update >= 7000 && !d->sort2pret) {if(d==player1)playsound(S_SORTPRET); d->sort2pret = true; }
                }
                // Sort résistance
                if(totalmillis-d->lastspecial3update >= 6000 && !d->sort3pret) {if(d==player1)playsound(S_SORTPRET); d->sort3pret = true; }
                if(totalmillis-d->lastspecial3update >= d->aptisort3 && !d->sended3)
                {
                    d->aptisort3=0;
                    addmsg(N_SENDSORT3, "rci", d, d->aptisort3);
                    d->sended3 = true;
                }
                break;
            }
        }
    }
}
