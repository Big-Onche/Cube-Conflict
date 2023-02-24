//aptitudes.cpp: management of special abilities from certain classes.
//Classes specs (such as speed, damage, etc...) and abilities are declared in game.h, cuz game.h is always included where we need classes specs and abilities's data.

#include "gfx.h"
#include "stats.h"

int getspyability;

namespace game
{
    void abilityeffect(gameent *d, int ability)
    {
        vec sndpos = d->o;
        bool nullsndpos = d==hudplayer();

        if(d->aptitude==APT_ESPION && ability==ABILITY_1) //for spy's ability 1, we play the sound at the decoy's position
        {
            const int positions[4][2] = { {25, 25}, {-25, -25}, {25, -25}, {-25, 25} };
            sndpos.add(vec(positions[d->aptiseed][0], positions[d->aptiseed][1], 0));
            nullsndpos = false; //even for player1
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

    bool canlaunchability(gameent *d, int ability)
    {
        if(d->state!=CS_ALIVE || !isconnected() || gfx::forcecampos>=0 || intermission || premission || (ability<ABILITY_1 && ability>ABILITY_3)) return false;
        else return d->aptitude==APT_MAGICIEN || d->aptitude==APT_PHYSICIEN || d->aptitude==APT_ESPION || d->aptitude==APT_PRETRE || d->aptitude==APT_SHOSHONE || d->aptitude==APT_KAMIKAZE;
    }

    void aptitude(gameent *d, int ability, bool request) //abilities commands
    {
        if(request) //player is requesting ability
        {
            if(!canlaunchability(d, ability)) return; //check for basic guards
            if(!d->abilityready[ability] || d->mana < aptitudes[d->aptitude].abilities[ability].manacost) {if(d==hudplayer())playsound(S_SORTIMPOSSIBLE); return; } //check for game vars (client sided)
            addmsg(N_REQABILITY, "rci", d, ability); //server sided game vars check
            return; //can stop after this, cuz server call this func with !request
        }
        //if all good, we let the ability begin
        d->abilityready[ability] = false;
        d->lastability[ability] = totalmillis;
        abilityeffect(d, ability);
        if(d==player1) addstat(1, STAT_ABILITES);
    }

    void abilitycmd(int ability) //Player1 abilities commands
    {
        if(player1->aptitude!=APT_KAMIKAZE) aptitude(player1, ability);
        else ability==ABILITY_1 ? gunselect(GUN_KAMIKAZE, player1) : aptitude(player1, ability);
    }
    ICOMMAND(aptitude, "i", (int *ability), abilitycmd(*ability));

    void updateabilities(gameent *d) //abilities rearming after cooldown
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

    ICOMMAND(getclasslogo, "ii", (int *cn, int *numapt),
        if(*numapt==-1 && isconnected()) {gameent *d = getclient(*cn); *numapt = d->aptitude; }
        else if(*numapt==-2) *numapt = player1->aptitude;
        defformatstring(logodir, "media/interface/apt_logo/%d.png", *numapt);
        result(logodir);
    );

    ICOMMAND(getaptistatval, "ii", (int *apt, int *stat),
        int val = 0;
        switch(*stat)
        {
            case 0: val = aptitudes[*apt].apt_degats-100; break;
            case 1: val = aptitudes[*apt].apt_resistance-100; break;
            case 2: val = aptitudes[*apt].apt_precision-100; break;
            case 3: val = (aptitudes[*apt].apt_vitesse-1000)*-0.1f;
        }
        intret((val/5)+9)
    );
}
