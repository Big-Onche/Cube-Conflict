//aptitudes.cpp: management of special abilities from certain classes

#include "game.h"
#include "ccheader.h"

namespace game
{
    void aptitude(gameent *d, int skill, bool send) //Commandes d'aptitudes
    {
        if(d->state==CS_DEAD) return;

        int neededdata = 0;
        switch(d->aptitude) {case APT_PHYSICIEN: neededdata++; break; case APT_PRETRE: neededdata+=2; break; case APT_SHOSHONE: neededdata+=3; break; case APT_ESPION: neededdata+=4; break; case APT_KAMIKAZE: neededdata+=5;}

        switch(skill)
        {
            case 1:
                if(send) //We send the shit to the server
                {
                    if(!d->sort1pret || d->mana<sorts[neededdata].mana1) {if(d==player1)playsound(S_SORTIMPOSSIBLE); break; }
                    d->aptisort1 = sorts[neededdata].duree1;
                    d->lastspecial1update = totalmillis;
                    addmsg(N_SENDSORT1, "rci", d, d->aptisort1);
                    d->mana -= sorts[neededdata].mana1;
                    d->sort1pret = false;
                }
                //We recieve some shit from the server
                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 250);
                if(d->aptitude!=APT_ESPION) d->sortchan = playsound(sorts[neededdata].sound1, d==hudplayer() ? NULL : &d->o, NULL, 0, 0, 100, d->sortchan, 300);
                else
                {
                    vec doublepos = d->o;
                    int posx = 25, posy = 25;
                    switch(d->aptiseed)
                    {
                        case 1: posx=-25; posy=-25; break;
                        case 2: posx=25; posy=-25; break;
                        case 3: posx=-25; posy=25; break;
                    }
                    doublepos.add(vec(posx, posy, 0));

                    d->sortchan = playsound(sorts[neededdata].sound1, &doublepos, NULL, 0, 0, 100, d->sortchan, 225);
                }
                return;

            case 2:
                if(send)
                {
                    if(!d->sort2pret || d->mana<sorts[neededdata].mana2) {if(d==player1)playsound(S_SORTIMPOSSIBLE); break; }
                    d->aptisort2 = sorts[neededdata].duree2;
                    d->lastspecial2update = totalmillis;
                    addmsg(N_SENDSORT2, "rci", d, d->aptisort2);
                    d->mana -= sorts[neededdata].mana2;
                    d->sort2pret = false;
                }

                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 250);
                d->sortchan = playsound(sorts[neededdata].sound2, d==hudplayer() ? NULL : &d->o, NULL, 0, 0, 100, d->sortchan, 275);

                if(d->aptitude==APT_ESPION)
                {
                    loopi(1)particle_fireball(d->o,  50, PART_ONDECHOC, 300, 0xBBBBBB, 1.f);
                    particle_splash(PART_SMOKE, 7, 400, d->o, 0x666666, 15+rnd(5), 200, -10);
                }
                return;

            case 3:
                if(send)
                {
                    if(!d->sort3pret || d->mana<sorts[neededdata].mana3) {if(d==player1)playsound(S_SORTIMPOSSIBLE); break; }
                    d->aptisort3 = sorts[neededdata].duree3;
                    d->lastspecial3update = totalmillis;
                    addmsg(N_SENDSORT3, "rci", d, d->aptisort3);
                    d->mana -= sorts[neededdata].mana3;
                    d->sort3pret = false;
                }

                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 250);
                d->sortchan = playsound(sorts[neededdata].sound3, d==hudplayer() ? NULL : &d->o, NULL, 0, 0, 100, d->sortchan, 275);

                if(d->aptitude==APT_ESPION) particle_fireball(d->o, 1000, PART_RADAR, 1000, 0xAAAAAA, 20.f);

                if(isteam(player1->team, d->team) && d->aptitude==APT_ESPION && d!=player1)
                {
                    getsort = totalmillis;
                    playsound(S_SORTESP3);
                }
                return;
        }
    }

    void player1aptitude(int ability)
    {
        switch(ability)
        {
            case 1:
                switch(player1->aptitude)
                {
                    case APT_KAMIKAZE: player1->gunselect = GUN_KAMIKAZE; playsound(S_WEAPLOAD); return;
                    case APT_PRETRE: case APT_PHYSICIEN: case APT_MAGICIEN: case APT_SHOSHONE: case APT_ESPION: aptitude(player1, ability);
                    default: return;
                }
                break;
            default:
                if(player1->aptitude==APT_PRETRE || player1->aptitude==APT_PHYSICIEN || player1->aptitude==APT_MAGICIEN || player1->aptitude==APT_SHOSHONE || player1->aptitude==APT_ESPION || player1->aptitude==APT_KAMIKAZE) aptitude(player1, player1->aptitude==APT_KAMIKAZE ? 2 : ability);
        }
    }

    ICOMMAND(aptitude, "i", (int *ability), player1aptitude(*ability));

    void updatespecials(gameent *d) //Permet de réarmer les sorts en fonction de la durée de rechargement de ceux-ci
    {
        int neededdata = 0;
        switch(d->aptitude) {case APT_PHYSICIEN: neededdata++; break; case APT_PRETRE: neededdata+=2; break; case APT_SHOSHONE: neededdata+=3; break; case APT_ESPION: neededdata+=4;}

        if(totalmillis-d->lastspecial1update >= sorts[neededdata].reload1 && !d->sort1pret) {if(d==player1)playsound(S_SORTPRET); d->sort1pret = true; }
        if(totalmillis-d->lastspecial2update >= sorts[neededdata].reload2 && !d->sort2pret) {if(d==player1)playsound(S_SORTPRET); d->sort2pret = true; }
        if(totalmillis-d->lastspecial3update >= sorts[neededdata].reload3 && !d->sort3pret) {if(d==player1)playsound(S_SORTPRET); d->sort3pret = true; }
    }
}
