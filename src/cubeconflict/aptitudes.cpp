//aptitudes.cpp: management of special abilities from certain classes.
//Classes specs (such as speed, damage, etc...) and abilities are declared in game.h, cuz game.h is always included where we need classes specs and abilities's data.

#include "gfx.h"
#include "stats.h"

enum {WIZ_ABILITIES = 0, PHY_ABILITIES, PRI_ABILITIES, SPY_ABILITIES, SHO_ABILITIES, KAM_ABILITIES};
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

int getspyability;

namespace game
{

    ICOMMAND(getabi, "ii", (int *classe, int *ability), conoutf("Manacost: %d Duration: %d Cooldown: %d Classe: %s", aptitudes[*classe].abilities[*ability].manacost, aptitudes[*classe].abilities[*ability].duration, aptitudes[*classe].abilities[*ability].cooldown, aptitudes[*classe].apt_nomFR););

    bool canlaunchability(gameent *d, int ability)
    {
        if(d->state==CS_DEAD || !isconnected() || gfx::forcecampos>=0 || intermission || premission || (ability<ABILITY_1 && ability>ABILITY_3)) return false;
        else return d->aptitude==APT_MAGICIEN || d->aptitude==APT_PHYSICIEN || d->aptitude==APT_ESPION || d->aptitude==APT_PRETRE || d->aptitude==APT_SHOSHONE || d->aptitude==APT_KAMIKAZE;
    }

    void aptitude(gameent *d, int ability, bool request) //abilities commands
    {
        if(!canlaunchability(d, ability)) return; //first check

        if(request) //player is requesting ability
        {
            if(!d->abilityready[ability] || d->mana < aptitudes[d->aptitude].abilities[ability].manacost) {if(d==hudplayer())playsound(S_SORTIMPOSSIBLE); return; } //second check (client sided)
            addmsg(N_REQABILITY, "rci", d, ability); //third check (server sided)
            return; //can stop after this, cuz server call this func with !request
        }

        //if all good, we let the ability begin
        d->abilityready[ability] = false; d->lastability[ability] = totalmillis;
        playsound(S_SORTLANCE, d==hudplayer() ? NULL : &d->o, 0, 0, 0 , 100, -1, 250);
        if(d==player1) addstat(1, STAT_ABILITES);

        switch(ability)
        {
            case ABILITY_1:
            {
                vec soundpos = d->o;
                if(d->aptitude==APT_ESPION)
                {
                    int posx = 25, posy = 25;
                    switch(d->aptiseed)
                    {
                        case 1: posx=-25; posy=-25; break;
                        case 2: posx=25; posy=-25; break;
                        case 3: posx=-25; posy=25; break;
                    }
                    soundpos.add(vec(posx, posy, 0));
                }

                d->abi1chan = playsound(aptitudes[d->aptitude].abilities[ability].snd, &soundpos, NULL, 0, 0, 100, d->abi1chan, 225);
                d->abi1snd = aptitudes[d->aptitude].abilities[ability].snd;
            }
            break;

            case ABILITY_2:
            {
                d->abi2chan = playsound(aptitudes[d->aptitude].abilities[ability].snd, d==hudplayer() ? NULL : &d->o, NULL, 0, 0, 100, d->abi2chan, 275);
                d->abi2snd = aptitudes[d->aptitude].abilities[ability].snd;

                if(d->aptitude==APT_ESPION)
                {
                    loopi(1)particle_fireball(d->o,  50, PART_SHOCKWAVE, 300, 0xBBBBBB, 1.f);
                    particle_splash(PART_SMOKE, 7, 400, d->o, 0x666666, 15+rnd(5), 200, -10);
                }
            }
            break;

            case ABILITY_3:
            {
                d->abi3chan = playsound(aptitudes[d->aptitude].abilities[ability].snd, d==hudplayer() ? NULL : &d->o, NULL, 0, 0, 100, d->abi3chan, 275);
                d->abi3snd = aptitudes[d->aptitude].abilities[ability].snd;

                if(d->aptitude==APT_ESPION) particle_fireball(d->o, 1000, PART_RADAR, 1000, 0xAAAAAA, 20.f);

                if(isteam(player1->team, d->team) && d->aptitude==APT_ESPION && d!=player1)
                {
                    getspyability = totalmillis;
                    playsound(S_SPY_3);
                }
            }
        }
    }

    void player1aptitude(int ability) //Player1 abilities commands
    {
        if(player1->aptitude==APT_KAMIKAZE && ability==ABILITY_1) gunselect(GUN_KAMIKAZE, player1);
        else aptitude(player1, player1->aptitude==APT_KAMIKAZE ? ABILITY_2 : ability);
    }
    ICOMMAND(aptitude, "i", (int *ability), player1aptitude(*ability));

    void updatespecials(gameent *d) //Permet de réarmer les sorts en fonction de la durée de rechargement de ceux-ci
    {
        loopi(NUMABILITIES)
        {
            if(totalmillis-d->lastability[i] >= aptitudes[d->aptitude].abilities[i].cooldown && !d->abilityready[i])
            {
                if(d==hudplayer()) playsound(S_SORTPRET);
                d->abilityready[i] = true;
            }
        }
    }
}
