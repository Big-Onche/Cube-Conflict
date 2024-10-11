//aptitudes.cpp: management of special abilities from certain classes.
//Classes specs (such as speed, damage, etc...) and abilities are declared in game.h, cuz game.h is always included where we need classes specs and abilities's data.

#include "gfx.h"
#include "stats.h"

bool disabledClasse[NUMCLASSES];

ICOMMAND(getclass, "i", (int *i), intret(disabledClasse[*i]) );

ICOMMAND(setclass, "i", (int *i),
    if(!validClass(*i))
    {
        conoutf(CON_ERROR, "\f3Invalid class ID! (must be 0 to %d)", NUMCLASSES);
        return;
    }
    if(*i == game::player1->aptitude)
    {
        conoutf(CON_ERROR, "\f3Cannot deactivate your current class!");
        playSound(S_ERROR, vec(0, 0, 0), 0, 0, SND_UI);
        return;
    }

    int maxClasses = NUMCLASSES;
    disabledClasse[*i] = !disabledClasse[*i];

    loopi(NUMCLASSES) maxClasses -= disabledClasse[i];
    if(!maxClasses)
    {
        disabledClasse[*i] = !disabledClasse[*i];
        conoutf(CON_ERROR, "\f3Cannot deactivate all classes!");
        playSound(S_ERROR, vec(0, 0, 0), 0, 0, SND_UI);
        return;
    }
);

ICOMMAND(setallclasses, "i", (int *enabled),
    loopi(NUMCLASSES)
    {
        if(!*enabled && i != game::player1->aptitude) disabledClasse[i] = true;
        else disabledClasse[i] = false;
    }
);

int getspyability;

namespace game
{
    void abilityEffect(gameent *d, int ability)
    {
        vec sndpos = d->o;
        bool noSoundPos = d==hudplayer();

        if(d->aptitude==C_SPY && ability==ABILITY_1) //for spy's ability 1, we play the sound at the decoy's position
        {
            const int positions[4][2] = { {25, 25}, {-25, -25}, {25, -25}, {-25, 25} };
            sndpos.add(vec(positions[d->seed][0], positions[d->seed][1], 0));
            noSoundPos = false; //even for player1
        }

        if(d==hudplayer()) playSound(S_SORTLANCE);
        playSound(classes[d->aptitude].abilities[ability].snd, noSoundPos ? vec(0, 0, 0) : sndpos, 250, 100, SND_NOCULL|SND_FIXEDPITCH, d->entityId, PL_ABI_1+ability);
        switch(ability) //here we play some one shot gfx effects
        {
            //case ABILITY_1:
            //break;

            case ABILITY_2:
                if(d->aptitude==C_SPY)
                {
                    loopi(1)particle_fireball(d->o,  50, PART_SHOCKWAVE, 300, 0xBBBBBB, 1.f);
                    particle_splash(PART_SMOKE, 7, 400, d->o, 0x666666, 15+rnd(5), 200, -10);
                }
                break;

            case ABILITY_3:
            {
                if(d->aptitude==C_SPY)
                {
                    particle_fireball(d->o, 1000, PART_RADAR, 1000, 0xAAAAAA, 20.f);
                    if(isteam(hudplayer()->team, d->team))
                    {
                        getspyability = totalmillis;
                        playSound(S_SPY_3, d==hudplayer() ? vec(0, 0, 0) : d->o, 3000, 50, SND_NOCULL|SND_FIXEDPITCH);
                    }
                }
            }
        }
    }

    bool canLaunchAbility(gameent *d, int ability)
    {
        if(d->state!=CS_ALIVE || !isconnected() || forcecampos>=0 || intermission || (ability<ABILITY_1 && ability>ABILITY_3)) return false;
        else return d->aptitude==C_WIZARD || d->aptitude==C_PHYSICIST || d->aptitude==C_SPY || d->aptitude==C_PRIEST || d->aptitude==C_SHOSHONE || d->aptitude==C_KAMIKAZE;
    }

    void launchAbility(gameent *d, int ability, bool request) //abilities commands
    {
        if(request) //player is requesting ability
        {
            if(!canLaunchAbility(d, ability)) return; //check for basic guards
            if(!d->abilityready[ability] || d->mana < classes[d->aptitude].abilities[ability].manacost) { if(d==player1) playSound(S_SORTIMPOSSIBLE); return; } //check for game vars (client sided)
            addmsg(N_REQABILITY, "rci", d, ability); //server sided game vars check
            return; //can stop after this, cuz server call this func with !request
        }
        //if all good, we let the ability begin
        d->abilityready[ability] = false;
        d->lastability[ability] = totalmillis;
        abilityEffect(d, ability);
        if(d==player1) updateStat(1, STAT_ABILITES);
    }

    ICOMMAND(aptitude, "i", (int *ability),  // player1 abilities commands
        if(player1->aptitude != C_KAMIKAZE) launchAbility(player1, *ability);
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
        if(d->vampiremillis && (d->vampiremillis -= time)<=0) d->vampiremillis = 0; // only for vampire screen gfx

        loopi(NUMABILITIES)
        {
            if(totalmillis-d->lastability[i] >= classes[d->aptitude].abilities[i].cooldown && !d->abilityready[i]) // ability rearm
            {
                if(d==hudplayer()) playSound(S_SORTPRET);
                d->abilityready[i] = true;
            }

            if(!hasAbilityEnabled(d, i)) continue; // no need to go further if ability no enabled
            if(d->abilitymillis[i] && (d->abilitymillis[i] -= time) <= 0) { d->abilitymillis[i] = 0; continue; } // end of an ability
            if(d->abilitymillis[i]) numEnabled++; // abilities used at the same time (only for achievement below)
        }

        if(d==player1 && player1->aptitude==C_SHOSHONE && numEnabled==3) unlockAchievement(ACH_WASHAKIE);
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
            case 0: val = classes[*apt].damage; break;
            case 1: val = classes[*apt].resistance; break;
            case 2: val = classes[*apt].accuracy; break;
            case 3: val = classes[*apt].speed;
        }
        intret(((val-100) / 5) +9);
    );
}
