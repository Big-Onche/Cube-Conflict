//aptitudes.cpp: management of special abilities from certain classes.
//Classes specs (such as speed, damage, etc...) and abilities are declared in game.h, cuz game.h is always included where we need classes specs and abilities's data.

#include "gfx.h"
#include "stats.h"

bool disabledClasse[NUMAPTS];

ICOMMAND(getclass, "i", (int *i), intret(disabledClasse[*i]) );

ICOMMAND(setclass, "i", (int *i),
    if(*i < 0 || *i > NUMAPTS-1) return;
    if(*i == game::player1->aptitude)
    {
        conoutf(CON_ERROR, "\f3Cannot deactivate your current classe!");
        playSound(S_ERROR);
        return;
    }

    int maxClasses = NUMAPTS;
    disabledClasse[*i] = !disabledClasse[*i];

    loopi(NUMAPTS) maxClasses -= disabledClasse[i];
    if(!maxClasses)
    {
        disabledClasse[*i] = !disabledClasse[*i];
        conoutf(CON_ERROR, "\f3Cannot deactivate all classes!");
        playSound(S_ERROR);
        return;
    }
);

ICOMMAND(setallclasses, "i", (int *enabled),
    loopi(NUMAPTS)
    {
        if(!*enabled && i != game::player1->aptitude) disabledClasse[i] = true;
        else disabledClasse[i] = false;
    }
);

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

        if(d==hudplayer()) playSound(S_SORTLANCE);
        playSound(aptitudes[d->aptitude].abilities[ability].snd, nullsndpos ? NULL : &sndpos, 250, 100, SND_NOCULL|SND_FIXEDPITCH, d->entityId, PL_ABI_1+ability);
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
                        playSound(S_SPY_3, d==hudplayer() ? NULL : &d->o, 3000, 50, SND_NOCULL|SND_FIXEDPITCH, d->entityId, PL_ABI_3);
                    }
                }
            }
        }
    }

    bool canlaunchability(gameent *d, int ability)
    {
        if(d->state!=CS_ALIVE || !isconnected() || gfx::forcecampos>=0 || intermission || (ability<ABILITY_1 && ability>ABILITY_3)) return false;
        else return d->aptitude==APT_MAGICIEN || d->aptitude==APT_PHYSICIEN || d->aptitude==APT_ESPION || d->aptitude==APT_PRETRE || d->aptitude==APT_SHOSHONE || d->aptitude==APT_KAMIKAZE;
    }

    void launchAbility(gameent *d, int ability, bool request) //abilities commands
    {
        if(request) //player is requesting ability
        {
            if(!canlaunchability(d, ability)) return; //check for basic guards
            if(!d->abilityready[ability] || d->mana < aptitudes[d->aptitude].abilities[ability].manacost) { if(d==player1) playSound(S_SORTIMPOSSIBLE); return; } //check for game vars (client sided)
            addmsg(N_REQABILITY, "rci", d, ability); //server sided game vars check
            return; //can stop after this, cuz server call this func with !request
        }
        //if all good, we let the ability begin
        d->abilityready[ability] = false;
        d->lastability[ability] = totalmillis;
        abilityeffect(d, ability);
        if(d==player1) updateStat(1, STAT_ABILITES);
    }

    ICOMMAND(aptitude, "i", (int *ability),  // player1 abilities commands
        if(player1->aptitude != APT_KAMIKAZE) launchAbility(player1, *ability);
        else *ability == ABILITY_1 ? gunselect(GUN_KAMIKAZE, player1) : launchAbility(player1, *ability);
    );

    char *getdisguisement(int seed) //spy's ability 2
    {
        static char dir[64];
        const char *name = getalias(tempformatstring("disguise_%d", seed));
        if(seed<0 || seed>3) name = "mapmodel/caisses/caissebois";
        formatstring(dir, "%s", name);
        return dir;
    }

    void updateAbilitiesSkills(int time, gameent *d)
    {
        int numEnabled = 0;
        if(d->vampimillis && (d->vampimillis -= time)<=0) d->vampimillis = 0; // only for vampire screen gfx

        loopi(NUMABILITIES)
        {
            if(totalmillis-d->lastability[i] >= aptitudes[d->aptitude].abilities[i].cooldown && !d->abilityready[i]) // ability rearm
            {
                if(d==hudplayer()) playSound(S_SORTPRET);
                d->abilityready[i] = true;
            }

            if(hasAbilityEnabled(d, i))
            {
                vec playerVel = d->vel;
                if(d!=hudplayer()) updateSoundPosition(d->entityId, d->o, playerVel.div(75), PL_ABI_1+i);
            }
            else continue; // no need to go further in the loop if ability no enabled

            if(d->abilitymillis[i] && (d->abilitymillis[i] -= time) <= 0) { d->abilitymillis[i] = 0; continue; } // end of an ability
            if(d->abilitymillis[i]) numEnabled++; // abilities used at the same time (only for achievement below)
        }

        if(d==player1 && player1->aptitude==APT_SHOSHONE && numEnabled==3) unlockAchievement(ACH_WASHAKIE);
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
            case 0: val = aptitudes[*apt].apt_degats; break;
            case 1: val = aptitudes[*apt].apt_resistance; break;
            case 2: val = aptitudes[*apt].apt_precision; break;
            case 3: val = aptitudes[*apt].apt_vitesse;
        }
        intret(((val-100) / 5) +9);
    );
}
