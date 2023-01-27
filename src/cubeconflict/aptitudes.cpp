//aptitudes.cpp: management of special abilities from certain classes.
//Classes specs (such as speed, damage, etc...) and abilities are declared in game.h, cuz game.h is always included where we need classes specs and abilities's data.

#include "gfx.h"
#include "stats.h"

int getspyability;

namespace game
{
    bool canlaunchability(gameent *d, int ability)
    {
        if(d->state==CS_DEAD || !isconnected() || gfx::forcecampos>=0 || intermission || premission || (ability<ABILITY_1 && ability>ABILITY_3)) return false;
        else return d->aptitude==APT_MAGICIEN || d->aptitude==APT_PHYSICIEN || d->aptitude==APT_ESPION || d->aptitude==APT_PRETRE || d->aptitude==APT_SHOSHONE || d->aptitude==APT_KAMIKAZE;
    }

    void abilityeffect(gameent *d, int ability)
    {
        switch(ability)
        {
            //case ABILITY_1:
            //break;

            case ABILITY_2:
                if(d->aptitude==APT_ESPION)
                {
                    loopi(1)particle_fireball(d->o,  50, PART_SHOCKWAVE, 300, 0xBBBBBB, 1.f);
                    particle_splash(PART_SMOKE, 7, 400, d->o, 0x666666, 15+rnd(5), 200, -10);
                }
                break;

            case ABILITY_3:
            {
                if(d->aptitude==APT_ESPION) particle_fireball(d->o, 1000, PART_RADAR, 1000, 0xAAAAAA, 20.f);

                if(isteam(player1->team, d->team) && d->aptitude==APT_ESPION && d!=player1)
                {
                    getspyability = totalmillis;
                    playsound(S_SPY_3);
                }
            }
        }
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

        d->abichan[ability] = playsound(aptitudes[d->aptitude].abilities[ability].snd, d==hudplayer() ? NULL : &d->o, NULL, 0, 0, 100, d->abichan[ability], 225);
        d->abisnd[ability] = aptitudes[d->aptitude].abilities[ability].snd;

        abilityeffect(d, ability);
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
