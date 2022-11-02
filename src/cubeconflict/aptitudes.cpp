//aptitudes.cpp: management of special abilities from certain classes.
//Classes specs (such as speed, damage, etc...) and abilities are declared in game.h, cuz game.h is always included where we need classes specs and abilities's data.

#include "game.h"
#include "stats.h"

int getspyability;

int abilitydata(int aptitude)
{
    switch(aptitude)
    {
        case APT_PHYSICIEN: return 1;
        case APT_PRETRE:    return 2;
        case APT_SHOSHONE:  return 3;
        case APT_ESPION:    return 4;
        case APT_KAMIKAZE:  return 5;
        default: return 0; //APT_MAGICIEN
    }
}

namespace game
{
    void aptitude(gameent *d, int ability, bool send) //Commandes d'aptitudes
    {
        if(d->state==CS_DEAD && !isconnected()) return;

        switch(ability)
        {
            case 1:
                if(send) //We send the shit to the server: client send to the server, then the server checks if it's valid or not
                {
                    if(!d->sort1pret || d->mana < sorts[abilitydata(d->aptitude)].mana1) {if(d==player1)playsound(S_SORTIMPOSSIBLE); return; }
                    d->aptisort1 = sorts[abilitydata(d->aptitude)].duree1;
                    d->lastspecial1update = totalmillis;
                    addmsg(N_SENDABILITY, "rcii", d, ability, d->aptisort1);
                    d->mana -= sorts[abilitydata(d->aptitude)].mana1;
                    d->sort1pret = false;
                    if(d==player1) stat[STAT_ABILITES]++;
                }
                //We recieve some shit from the server
                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 250);
                if(d->aptitude!=APT_ESPION) d->sortchan = playsound(sorts[abilitydata(d->aptitude)].sound1, d==hudplayer() ? NULL : &d->o, NULL, 0, 0, 100, d->sortchan, 300);
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

                    d->sortchan = playsound(sorts[abilitydata(d->aptitude)].sound1, &doublepos, NULL, 0, 0, 100, d->sortchan, 225);
                }
                return;

            case 2:
                if(send)
                {
                    if(!d->sort2pret || d->mana<sorts[abilitydata(d->aptitude)].mana2) {if(d==player1)playsound(S_SORTIMPOSSIBLE); return; }
                    d->aptisort2 = sorts[abilitydata(d->aptitude)].duree2;
                    d->lastspecial2update = totalmillis;
                    addmsg(N_SENDABILITY, "rcii", d, ability, d->aptisort2);
                    d->mana -= sorts[abilitydata(d->aptitude)].mana2;
                    d->sort2pret = false;
                    if(d==player1) stat[STAT_ABILITES]++;
                }

                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0, 100, -1, 250);
                d->sortchan = playsound(sorts[abilitydata(d->aptitude)].sound2, d==hudplayer() ? NULL : &d->o, NULL, 0, 0, 100, d->sortchan, 275);

                if(d->aptitude==APT_ESPION)
                {
                    loopi(1)particle_fireball(d->o,  50, PART_ONDECHOC, 300, 0xBBBBBB, 1.f);
                    particle_splash(PART_SMOKE, 7, 400, d->o, 0x666666, 15+rnd(5), 200, -10);
                }
                return;

            case 3:
                if(send)
                {
                    if(!d->sort3pret || d->mana<sorts[abilitydata(d->aptitude)].mana3) {if(d==player1)playsound(S_SORTIMPOSSIBLE); return; }
                    d->aptisort3 = sorts[abilitydata(d->aptitude)].duree3;
                    d->lastspecial3update = totalmillis;
                    addmsg(N_SENDABILITY, "rcii", d, ability, d->aptisort3);
                    d->mana -= sorts[abilitydata(d->aptitude)].mana3;
                    d->sort3pret = false;
                    if(d==player1) stat[STAT_ABILITES]++;
                }

                playsound(S_SORTLANCE, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 250);
                d->sortchan = playsound(sorts[abilitydata(d->aptitude)].sound3, d==hudplayer() ? NULL : &d->o, NULL, 0, 0, 100, d->sortchan, 275);

                if(d->aptitude==APT_ESPION) particle_fireball(d->o, 1000, PART_RADAR, 1000, 0xAAAAAA, 20.f);

                if(isteam(player1->team, d->team) && d->aptitude==APT_ESPION && d!=player1)
                {
                    getspyability = totalmillis;
                    playsound(S_SPY_3);
                }
                return;
        }
    }

    void player1aptitude(int ability) //Player1 abilities commands
    {
        switch(ability)
        {
            case 1:
                switch(player1->aptitude)
                {
                    case APT_KAMIKAZE: player1->gunselect = GUN_KAMIKAZE; playsound(S_WPLOADFASTWOOSH); return;
                    case APT_PRETRE: case APT_PHYSICIEN: case APT_MAGICIEN: case APT_SHOSHONE: case APT_ESPION: aptitude(player1, ability);
                    default: return;
                }
                break;
            default:
                if(player1->aptitude==APT_PRETRE || player1->aptitude==APT_PHYSICIEN || player1->aptitude==APT_MAGICIEN || player1->aptitude==APT_SHOSHONE || player1->aptitude==APT_ESPION || player1->aptitude==APT_KAMIKAZE) aptitude(player1, player1->aptitude==APT_KAMIKAZE ? 2 : ability);
        }
    }
    ICOMMAND(aptitude, "i", (int *ability), if(isconnected()) player1aptitude(*ability));

    void updatespecials(gameent *d) //Permet de réarmer les sorts en fonction de la durée de rechargement de ceux-ci
    {
        if(totalmillis-d->lastspecial1update >= sorts[abilitydata(d->aptitude)].reload1 && !d->sort1pret) {if(d==player1)playsound(S_SORTPRET); d->sort1pret = true; }
        if(totalmillis-d->lastspecial2update >= sorts[abilitydata(d->aptitude)].reload2 && !d->sort2pret) {if(d==player1)playsound(S_SORTPRET); d->sort2pret = true; }
        if(totalmillis-d->lastspecial3update >= sorts[abilitydata(d->aptitude)].reload3 && !d->sort3pret) {if(d==player1)playsound(S_SORTPRET); d->sort3pret = true; }
    }
}
