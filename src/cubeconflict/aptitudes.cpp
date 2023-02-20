//aptitudes.cpp: management of special abilities from certain classes.
//Classes specs (such as speed, damage, etc...) and abilities are declared in game.h, cuz game.h is always included where we need classes specs and abilities's data.

#include "gfx.h"
#include "stats.h"

int getspyability;

namespace game
{
    bool canlaunchability(gameent *d, int ability)
    {
        if(d->state!=CS_ALIVE || !isconnected() || gfx::forcecampos>=0 || intermission || premission || (ability<ABILITY_1 && ability>ABILITY_3)) return false;
        else return d->aptitude==APT_MAGICIEN || d->aptitude==APT_PHYSICIEN || d->aptitude==APT_ESPION || d->aptitude==APT_PRETRE || d->aptitude==APT_SHOSHONE || d->aptitude==APT_KAMIKAZE;
    }

    void abilityeffect(gameent *d, int ability)
    {
        vec sndpos = d->o;
        bool nullsndpos = d==hudplayer();

        if(d->aptitude==APT_ESPION && ability==ABILITY_1) //for spy's ability 1, we play the sound at the decoy's position
        {
            const int positions[4][2] = { {25, 25}, {-25, -25}, {25, -25}, {-25, 25} };
            sndpos.add(vec(positions[d->aptiseed][0], positions[d->aptiseed][1], 0));
            nullsndpos = false;
        }

        playsound(S_SORTLANCE, d==hudplayer() ? NULL : &d->o, 0, 0, 0 , 100, -1, 250);
        d->abichan[ability] = playsound(aptitudes[d->aptitude].abilities[ability].snd, nullsndpos ? NULL : &sndpos, NULL, 0, 0, 100, d->abichan[ability], 225);
        d->abisnd[ability] = aptitudes[d->aptitude].abilities[ability].snd;

        switch(ability) //here we play some one shot gfx effects
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
                if(d->aptitude==APT_ESPION)
                {
                    particle_fireball(d->o, 1000, PART_RADAR, 1000, 0xAAAAAA, 20.f);
                    if(isteam(hudplayer()->team, d->team))
                    {
                        getspyability = totalmillis;
                        playsound(S_SPY_3, d==hudplayer() ? NULL : &d->o, NULL, 0, 0, 0, -1, 4000);
                    }
                }
            }
        }
    }

    void aptitude(gameent *d, int ability, bool request) //abilities commands
    {
        if(!canlaunchability(d, ability)) return; //first check

        if(request) //player is requesting ability (client sided)
        {
            if(!d->abilityready[ability] || d->mana < aptitudes[d->aptitude].abilities[ability].manacost) {if(d==hudplayer())playsound(S_SORTIMPOSSIBLE); return; } //second check (client sided)
            addmsg(N_REQABILITY, "rci", d, ability); //third check (server sided)
            return; //can stop after this, cuz server call this func with !request
        }

        //if all good, we let the ability begin
        d->abilityready[ability] = false;
        d->lastability[ability] = totalmillis;
        abilityeffect(d, ability);
        if(d==player1) addstat(1, STAT_ABILITES);
    }

    void player1aptitude(int ability) //Player1 abilities commands
    {
        if(player1->aptitude!=APT_KAMIKAZE) aptitude(player1, ability);
        else ability==ABILITY_1 ? gunselect(GUN_KAMIKAZE, player1) : aptitude(player1, ability);
    }
    ICOMMAND(aptitude, "i", (int *ability), player1aptitude(*ability));

    void updatespecials(gameent *d) //Permet de r�armer les sorts en fonction de la dur�e de rechargement de ceux-ci
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
