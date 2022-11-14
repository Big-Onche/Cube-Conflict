// pimped old monster.h from sauerbraten: implements AI for single player monsters, currently client only
#include "game.h"
#include "ccheader.h"

extern int physsteps;

VAR(pissoffnpc, 0, 0, 1);

namespace game
{
    static vector<int> teleports;

    static const int TOTMFREQ = 14;
    static const int NUMMONSTERTYPES = 6;

    struct pnjtype      // see docs for how these values modify behaviour
    {
        short gun, speed, health, freq, lag, rate, pain, triggerdist, loyalty, bscale, weight, respawntime;
        bool friendly;
        short hellosound, painsound, angrysound, diesound;
        const char *namefr, *nameen, *mdlname, *shieldname, *hatname, *capename, *boost1modelname, *boost2modelname;
    };

    static const pnjtype pnjtypes[NUMMONSTERTYPES] =
    {   //weapon        sp. hea.  freq  lag  rate pain  trigdist. loy. size  weight  res. fri.   hellosnd.  painsnd.   angrysnd.  diesnd.    namefr          nameen            mdlldir            shielddir                      hatdir             capedir             boost1dir        boost2dir
        { GUN_S_NUKE,    1, 5000, 2,    30,  5,   100,  100,      5,   12,   85,     1,   true,  S_NULL,    S_NULL,    S_NULL,    S_NULL,    "Jean Onche",   "Jean Onche",     "smileys/hap",     "worldshield/or/100",          "hats/crown",      "capes/cape_elite", NULL,            NULL},
        { GUN_CACFLEAU,  1, 2500, 1,    30,  5,    50,  100,      5,   12,   90,     1,   true,  S_NULL,    S_NULL,    S_NULL,    S_NULL,    "Bjorn",        "Bjorn",          "npcs/bjorn",      "worldshield/bois/60",         NULL,              NULL,               NULL,            NULL},
        { GUN_SPOCKGUN,  1, 2000, 2,     5,  2,   150,  100,      1,   10,   70,     1,   true,  S_ALIEN_H, S_ALIEN_P, S_ALIEN_A, S_ALIEN_D, "le roi alien", "the alien king", "npcs/alien_king", "worldshield/magnetique/100",  "hats/crown/big",  NULL,               NULL,            NULL},
        { GUN_ARTIFICE,  1, 1500, 2,    75, 10,    50,  100,      5,   10,   60,     1,   true,  S_NULL,    S_NULL,    S_NULL,    S_NULL,    "Spike",        "Spike",          "smileys/content", NULL,                          NULL,              "boosts/steros",    "boosts/steros", "boosts/epo"},
        { GUN_ARBALETE,  1, 2000, 2,     5,  2,    25,  100,      5,   10,   70,     1,   true,  S_NULL,    S_NULL,    S_NULL,    S_NULL,    "Boing",        "Boing",          "smileys/fou",     NULL,                          "hats/ninja",      NULL,               NULL,            NULL},
        { GUN_GLOCK,     1,  750, 2,     5,  2,   150,  100,      1,   10,   70,     1,   false, S_ALIEN_H, S_ALIEN_P, S_ALIEN_A, S_ALIEN_D, "le roi alien", "the alien king", "npcs/kevin",      "worldshield/bois/20",         NULL,              NULL,               NULL,            NULL},
    };

    VAR(skill, 1, 3, 10);
    VAR(killsendsp, 0, 1, 1);

    bool monsterhurt;
    vec monsterhurtpos;

    struct monster : gameent
    {
        int monsterstate;                   // one of M_*, M_NONE means human

        int mtype, tag;                     // see monstertypes table
        gameent *enemy;                     // monster wants to kill this entity
        float targetyaw;                    // monster wants to look in this direction
        float targetpitch;
        int trigger;                        // millis at which transition to another monsterstate takes place
        vec attacktarget;                   // delayed attacks
        int anger;                          // how many times already hit by fellow monster
        bool friendly;

        physent *stacked;
        vec stackpos;
        vec spawnpos;
        int spawnyaw, monsterlastdeath;

        monster(int _type, int _yaw, int _pitch, int _tag, int _state, int _trigger, int _move) :
            monsterstate(_state), tag(_tag),
            stacked(NULL),
            stackpos(0, 0, 0)
        {
            type = ENT_AI;
            respawn();
            if(_type>=NUMMONSTERTYPES || _type < 0)
            {
                conoutf(CON_WARN, "warning: unknown monster in spawn: %d", _type);
                _type = 0;
            }
            mtype = _type;
            const pnjtype &t = pnjtypes[mtype];
            eyeheight = 8.0f;
            aboveeye = 7.0f;
            radius *= t.bscale/10.0f;
            xradius = yradius = radius;
            eyeheight *= t.bscale/10.0f;
            aboveeye *= t.bscale/10.0f;
            weight = t.weight;
            if(_state!=M_SLEEP) spawnplayer(this);
            trigger = lastmillis+_trigger;
            targetyaw = spawnyaw = yaw = (float)_yaw;
            targetpitch = pitch = (float)_pitch;
            move = _move;
            enemy = player1;
            gunselect = t.gun;
            maxspeed = (float)t.speed*4;
            health = t.health;
            armour = 0;
            loopi(NUMGUNS) ammo[i] = 10000;
            pitch = 0;
            roll = 0;
            state = CS_ALIVE;
            anger = 0;
            friendly = t.friendly;
            monsterlastdeath = 0;
            copystring(name, GAME_LANG ? t.nameen : t.namefr);
        }

        void normalize_yaw(float angle)
        {
            while(yaw<angle-180.0f) yaw += 360.0f;
            while(yaw>angle+180.0f) yaw -= 360.0f;
        }

        void normalize_pitch(float angle)
        {
            while(pitch<angle-180.0f) pitch += 360.0f;
            while(pitch>angle+180.0f) pitch -= 360.0f;
        }

        // monster AI is sequenced using transitions: they are in a particular state where
        // they execute a particular behaviour until the trigger time is hit, and then they
        // reevaluate their situation based on the current state, the environment etc., and
        // transition to the next state. Transition timeframes are parametrized by difficulty
        // level (skill), faster transitions means quicker decision making means tougher AI.

        void transition(int _state, int _moving, int n, int r) // n = at skill 0, n/2 = at skill 10, r = added random factor
        {
            monsterstate = _state;
            move = _moving;
            n = n*130/100;
            trigger = lastmillis+n-skill*(n/16)+rnd(r+1);
        }

        bool canmove()
        {
            switch(monsterstate)
            {
                case M_FRIENDLY:
                case M_NEUTRAL:
                case M_ANGRY:
                    return false;
                    break;
                default:
                    if(physstate == PHYS_FLOOR && friendly) return false;
                    else return true;
            }
        }

        void monsteraction(int curtime)           // main AI thinking routine, called every frame for every monster
        {
            if(enemy->state==CS_DEAD) { enemy = player1; anger = 0; }

            float dist = enemy->o.dist(o);

            normalize_yaw(targetyaw);
            if(targetyaw>yaw)             // slowly turn monster towards his target
            {
                yaw += curtime*(friendly ? 0.2f : 0.4f);
                if(targetyaw<yaw) yaw = targetyaw;
            }
            else
            {
                yaw -= curtime*(friendly ? 0.2f : 0.4f);
                if(targetyaw>yaw) yaw = targetyaw;
            }

            if(dist < pnjtypes[mtype].triggerdist*(friendly ? 4 : 2))
            {
                targetpitch < -45 ? targetpitch = -45 : targetpitch > 45 ? targetpitch = 45 : targetpitch;
                normalize_pitch(targetpitch);
                if(targetpitch>pitch)             // slowly turn monster towards his target
                {
                    pitch += curtime*0.3f;
                    if(targetpitch<pitch) pitch = targetpitch;
                }
                else
                {
                    pitch -= curtime*0.3f;
                    if(targetpitch>pitch) pitch = targetpitch;
                }
            }

            switch(monsterstate)
            {
                case M_SLEEP: targetpitch = 0; break;
                default:
                    int trigdist = dist*(player1->crouched() ? 2 : 1);
                    if(trigdist < pnjtypes[mtype].triggerdist) targetpitch = asin((enemy->o.z - o.z) / dist) / RAD;            // if player1 is close to pnj, pnj look at the player
                    else targetpitch = 0;
            }

            if(blocked)                                                            // special case: if we run into scenery
            {
                blocked = false;
                if(!rnd(2500/pnjtypes[mtype].speed)) jumping = true;               // try to jump over obstackle (rare)
                else if(trigger<lastmillis && (((monsterstate!=M_AGGRO) && (monsterstate!=M_RETREAT)) || !rnd(5)))    // search for a way around (common)
                {
                    targetyaw += 90+rnd(180);                                      // patented "random walk" AI pathfinding (tm) ;)
                    transition(M_SEARCH, 1, 100, 1000);
                }
            }

            float enemyyaw = -atan2(enemy->o.x - o.x, enemy->o.y - o.y)/RAD;

            switch(monsterstate)
            {
                case M_ANGRY:
                    {
                        targetyaw = enemyyaw;
                        transition(M_ATTACKING, 0, 200, 200);
                    }
                    break;

                case M_NEUTRAL: if(trigger+10000<lastmillis) transition(M_FRIENDLY, 1, 100, 200);

                case M_FRIENDLY:
                    if(dist < pnjtypes[mtype].triggerdist)
                    {
                        targetyaw = enemyyaw;
                        if(pissoffnpc)
                        {
                            transition(M_ANGRY, 1, pnjtypes[mtype].lag, 10);
                            pissoffnpc = 0;
                        }
                    }
                    else targetyaw = spawnyaw;
                    break;

                case M_PAIN:
                case M_ATTACKING:
                case M_SEARCH:
                    if(trigger<lastmillis) transition(M_AGGRO, 1, 100, 200);
                    break;

                case M_SLEEP:                       // state classic sp monster start in, wait for visual contact
                {
                    if(editmode) break;

                    normalize_yaw(enemyyaw);
                    float angle = (float)fabs(enemyyaw-yaw);
                    int trigdist = dist*(player1->crouched() ? 2 : 1);                           // if player1 is crouched, minimal trigger distance is reduced by 2
                    if(trigdist < pnjtypes[mtype].triggerdist/(friendly ? 2 : 4)                   // the better the angle to the player, the further the monster can see/hear
                    ||(trigdist < pnjtypes[mtype].triggerdist/(friendly ? 1.5f : 3) && angle<135)
                    ||(trigdist < pnjtypes[mtype].triggerdist/(friendly ? 1 : 2) && angle<90)
                    ||(trigdist < pnjtypes[mtype].triggerdist && angle<45)
                    ||(monsterhurt && o.dist(player1->o) < pnjtypes[mtype].triggerdist*(friendly ? 4 : 2)))
                    {
                        vec target;
                        if(raycubelos(o, enemy->o, target))
                        {
                            transition(friendly ? M_FRIENDLY : M_SEARCH, 1, 500, 200);
                            playsound(pnjtypes[mtype].hellosound, &o);
                            //transition(monsterhurt ? M_NEUTRAL : friendly ? M_FRIENDLY : M_SEARCH, 1, 500, 200);
                            //if(monsterhurt) break;
                            //if(this->state==M_FRIENDLY) playsound(pnjtypes[mtype].hellosound, &o, 0, 0, 0, 250, -1, 500);
                        }
                    }
                    break;
                }

                case M_AIMING:                      // this state is the delay between wanting to shoot and actually firing
                    if(trigger<lastmillis)
                    {
                        lastaction = 0;
                        attacking = true;
                        shoot(this, attacktarget);
                        transition(M_ATTACKING, 0, 600, 0);
                        if(friendly) transition(M_NEUTRAL, 1, 100, 200);
                    }
                    break;

                case M_RETREAT:
                    if(trigger<lastmillis)
                    {
                        targetyaw = -atan2(spawnpos.x - o.x, spawnpos.y - o.y)/RAD;
                        transition(M_RETREAT, 1, pnjtypes[mtype].rate, 0);
                        if(o.dist(spawnpos)<10) {targetyaw = spawnyaw; transition(M_SLEEP, 0, 600, 0);}
                    }
                    break;

                case M_AGGRO:                        // monster has visual contact, heads straight for player and may want to shoot at any time
                    targetyaw = enemyyaw;

                    if(dist > pnjtypes[mtype].triggerdist*2) {transition(friendly ? M_NEUTRAL : M_RETREAT, 0, 600, 0); break;}
                    if(trigger<lastmillis)
                    {
                        vec target;
                        if(!raycubelos(o, enemy->o, target))    // no visual contact anymore, let monster get as close as possible then search for player
                        {
                            transition(M_AGGRO, 1, 800, 500);
                        }
                        else
                        {
                            bool melee = false, longrange = false;
                            switch(pnjtypes[mtype].gun)
                            {
                                case GUN_CAC349: case GUN_CACFLEAU: case GUN_CACMARTEAU: case GUN_CACMASTER: case GUN_CACNINJA:
                                case GUN_SV98: case GUN_SKS: case GUN_ARBALETE: longrange = true; break;
                            }
                            // the closer the monster is the more likely he wants to shoot,
                            if((!melee || dist<20) && !rnd(longrange ? (int)dist/12+1 : min((int)dist/12+1,6)) && enemy->state==CS_ALIVE)      // get ready to fire
                            {
                                attacktarget = target;
                                transition(M_AIMING, 0, pnjtypes[mtype].lag, 10);
                            }
                            else                                                        // track player some more
                            {
                                transition(M_AGGRO, 1, pnjtypes[mtype].rate, 0);
                            }
                        }
                    }
                    break;
            }

            if((move || maymove() || (stacked && (stacked->state!=CS_ALIVE || stackpos != stacked->o))) && canmove())
            {
                vec pos = feetpos();
                loopv(teleports) // equivalent of player entity touch, but only teleports are used
                {
                    entity &e = *entities::ents[teleports[i]];
                    float dist = e.o.dist(pos);
                    if(dist<16) entities::teleport(teleports[i], this);
                }

                if(physsteps > 0) stacked = NULL;
                moveplayer(this, 10, true, curtime, 0, 0, 0, 0, false);        // use physics to move monster
            }
        }

        void checkmonstersrespawns()
        {
            if(totalmillis - monsterlastdeath > pnjtypes[this->type].respawntime)
            {
                targetyaw = spawnyaw;
                o = spawnpos;
                state = CS_ALIVE;
                health = pnjtypes[this->type].health;
            }
        }

        void monsterpain(int damage, gameent *d, int atk)
        {
            playsound(pnjtypes[mtype].painsound, &o, 0, 0, 0, 250, -1, 500);

            if(d->type==ENT_AI && !friendly)     // a monster hit us
            {
                if(this!=d)            // guard for RL guys shooting themselves :)
                {
                    anger++;     // don't attack straight away, first get angry
                    int _anger = d->type==ENT_AI && mtype==((monster *)d)->mtype ? anger/2 : anger;
                    if(_anger >= pnjtypes[mtype].loyalty)
                    {
                        transition(M_AGGRO, 1, pnjtypes[mtype].rate, 200);
                        enemy = d;     // monster infight if very angry
                    }
                }
            }
            else if(d->type==ENT_PLAYER) // player hit us
            {
                if(friendly)
                {
                    if(this->monsterstate==M_FRIENDLY) transition(M_NEUTRAL, 1, pnjtypes[mtype].rate, 200);      //if you mess with a friendly pnj, he gets neutral for a moment
                    else if(this->monsterstate==M_NEUTRAL)
                    {
                        transition(M_ANGRY, 1, pnjtypes[mtype].lag, 10);    //if you mess with a neutral pnj, he gets aggressive
                        playsound(pnjtypes[mtype].angrysound, &o);
                    }
                    return;
                }

                anger = 0;
                enemy = d;
                monsterhurt = true;
                monsterhurtpos = o;
            }
            damageeffect(damage, this, d, false, atk);
            if((health -= damage)<=0)
            {
                state = CS_DEAD;
                lastpain = lastmillis;
                playsound(pnjtypes[mtype].diesound, &o, 0, 0, 0, 250, -1, 500);
                monsterkilled();
                gibeffect(max(-health, 0), vel, this);
                monsterlastdeath = totalmillis;
                defformatstring(id, "monster_dead_%d", tag);
                if(identexists(id)) execute(id);
            }
            else
            {
                transition(M_PAIN, 0, pnjtypes[mtype].pain, 200);      // in this state monster won't attack
                playsound(pnjtypes[mtype].painsound, &o, 0, 0, 0, 250, -1, 500);
            }
        }
    };

    void stackmonster(monster *d, physent *o)
    {
        d->stacked = o;
        d->stackpos = o->o;
    }

    int nummonsters(int tag, int state)
    {
        int n = 0;
        loopv(monsters) if(monsters[i]->tag==tag && (monsters[i]->state==CS_ALIVE ? state!=1 : state>=1)) n++;
        return n;
    }
    ICOMMAND(nummonsters, "ii", (int *tag, int *state), intret(nummonsters(*tag, *state)));

    void preloadmonsters()
    {
        loopi(NUMMONSTERTYPES) preloadmodel(pnjtypes[i].mdlname);
    }

    vector<monster *> monsters;

    int nextmonster, spawnremain, numkilled, monstertotal, mtimestart, remain;

    void spawnmonster()     // spawn a random monster according to freq distribution in DMSP
    {
        int n = rnd(TOTMFREQ), type;
        for(int i = 0; ; i++) if((n -= pnjtypes[i].freq)<0) { type = i; break; }
        monsters.add(new monster(type, rnd(360), 0, 0, M_SEARCH, 1000, 1));
    }

    void clearmonsters()     // called after map start or when toggling edit mode to reset/spawn all monsters to initial state
    {
        removetrackedparticles();
        removetrackeddynlights();
        loopv(monsters) delete monsters[i];
        cleardynentcache();
        monsters.shrink(0);
        numkilled = 0;
        monstertotal = 0;
        spawnremain = 0;
        remain = 0;
        monsterhurt = false;
        if(m_dmsp)
        {
            nextmonster = mtimestart = lastmillis+2000;
            monstertotal = spawnremain = skill*10;
        }
        else if(m_classicsp || m_tutorial)
        {
            mtimestart = lastmillis;
            loopv(entities::ents)
            {
                extentity &e = *entities::ents[i];
                if(e.type!=MONSTER) continue;
                monster *m = new monster(e.attr1, e.attr2, e.attr3, e.attr4, M_SLEEP, 100, 0);
                monsters.add(m);
                m->o = e.o;
                m->spawnpos = e.o;
                entinmap(m);
                updatedynentcache(m);
                monstertotal++;
            }
        }
        teleports.setsize(0);
        if(m_dmsp || m_classicsp || m_tutorial)
        {
            loopv(entities::ents) if(entities::ents[i]->type==TELEPORT) teleports.add(i);
        }
    }

    void endsp(bool allkilled)
    {
        conoutf(CON_GAMEINFO, allkilled ? "\f2you have cleared the map!" : "\f2you reached the exit!");
        monstertotal = 0;
        game::addmsg(N_FORCEINTERMISSION, "r");
    }
    ICOMMAND(endsp, "", (), endsp(false));


    void monsterkilled()
    {
        numkilled++;
        player1->frags = numkilled;
        remain = monstertotal-numkilled;
    }

    void updatemonsters(int curtime)
    {
        if(m_dmsp && spawnremain && lastmillis>nextmonster)
        {
            if(spawnremain--==monstertotal) { conoutf(CON_GAMEINFO, "\f2The invasion has begun!"); playsound(S_DRAPEAUPRIS); }
            nextmonster = lastmillis+1000;
            spawnmonster();
        }

        //if(killsendsp && monstertotal && !spawnremain && numkilled==monstertotal) endsp(true);

        bool monsterwashurt = monsterhurt;

        loopv(monsters)
        {
            if(monsters[i]->state==CS_ALIVE) monsters[i]->monsteraction(curtime);
            else if(monsters[i]->state==CS_DEAD)
            {
                if(lastmillis-monsters[i]->lastpain<2000)
                {
                    monsters[i]->move = monsters[i]->strafe = 0;
                    moveplayer(monsters[i], 5, true, curtime, 0, 0, 0 , 0, false);
                }
                if(!m_dmsp) monsters[i]->checkmonstersrespawns();
            }
        }

        if(monsterwashurt) monsterhurt = false;
    }

    const char *stnames[M_MAX] = {
        "none", "searching", "aggro", "retreat", "attacking", "in pain", "sleeping", "aiming", "friendly", "neutral", "angry"
    };

    VAR(dbgpnj, 0, 0, 1);

    void debugpnj(monster *m)
    {
        vec pos = m->abovehead();

        pos.z += 2;
        defformatstring(s1, "health: %d - %s", m->health, m->health>0 ? "alive" : "dead");
        particle_textcopy(pos, s1, PART_TEXT, 1);

        pos.z += 2;
        defformatstring(s2, "state: %s", stnames[m->monsterstate]);
        particle_textcopy(pos, s2, PART_TEXT, 1);

        pos.z += 2;
        defformatstring(s3, "friendly? %s", m->friendly ? "yes" : "no");
        particle_textcopy(pos, s3, PART_TEXT, 1);
    }

    void rendermonsters()
    {
        loopv(monsters)
        {
            monster &m = *monsters[i];
            if(m.state!=CS_DEAD || lastmillis-m.lastpain<10000)
            {
                float yaw = m.yaw,
                      pitch = m.pitch;
                vec o = m.feetpos();

                float fade = 1;
                if(m.state==CS_DEAD) fade -= clamp(float(lastmillis - (m.lastpain + 9000))/1000, 0.0f, 1.0f);

                modelattach a[10];
                int ai = 0;

                int vanim = ANIM_VWEP_IDLE|ANIM_LOOP, vtime = 0;
                if(m.lastaction && m.lastattack >= 0 && attacks[m.lastattack].gun==m.gunselect && lastmillis < m.lastaction+250)
                {
                    vanim = attacks[m.lastattack].vwepanim;
                    vtime = m.lastaction;
                }

                a[ai++] = modelattach("tag_weapon", guns[m.gunselect].vwep, vanim, vtime);

                if(guns[m.gunselect].vwep)
                {
                    m.muzzle = vec(-1, -1, -1);
                    a[ai++] = modelattach("tag_muzzle", &m.muzzle);
                    a[ai++] = modelattach("tag_balles", &m.balles);
                }

                if(pnjtypes[m.mtype].shieldname) a[ai++] = modelattach("tag_shield", pnjtypes[m.mtype].shieldname, ANIM_VWEP_IDLE|ANIM_LOOP, 0);
                if(pnjtypes[m.mtype].hatname) a[ai++] = modelattach("tag_hat", pnjtypes[m.mtype].hatname, ANIM_VWEP_IDLE|ANIM_LOOP, 0);
                if(pnjtypes[m.mtype].capename) a[ai++] = modelattach("tag_cape", pnjtypes[m.mtype].capename, ANIM_VWEP_IDLE|ANIM_LOOP, 0);
                if(pnjtypes[m.mtype].boost1modelname) a[ai++] = modelattach("tag_boost1", pnjtypes[m.mtype].boost1modelname, ANIM_VWEP_IDLE|ANIM_LOOP, 0);
                if(pnjtypes[m.mtype].boost2modelname) a[ai++] = modelattach("tag_boost2", pnjtypes[m.mtype].boost2modelname, ANIM_VWEP_IDLE|ANIM_LOOP, 0);

                rendermodel(pnjtypes[m.mtype].mdlname, ANIM_SHOOT, o, yaw, pitch, 0, MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY, &m, a[0].tag ? a : NULL, 0, 0, fade);
                if(dbgpnj) debugpnj(&m);
            }
        }
    }

    void suicidemonster(monster *m)
    {
        m->monsterpain(400, player1, ATK_CAC349_SHOOT);
    }

    void hitmonster(int damage, monster *m, gameent *at, const vec &vel, int atk)
    {
        m->monsterpain(damage, at, atk);
    }

    void spsummary(int accuracy)
    {
        conoutf(CON_GAMEINFO, "\f2--- single player time score: ---");
        int pen, score = 0;
        pen = ((lastmillis-maptime)*100)/(1000*getvar("gamespeed")); score += pen; if(pen) conoutf(CON_GAMEINFO, "\f2time taken: %d seconds (%d simulated seconds)", pen, (lastmillis-maptime)/1000);
        pen = player1->deaths*60; score += pen; if(pen) conoutf(CON_GAMEINFO, "\f2time penalty for %d deaths (1 minute each): %d seconds", player1->deaths, pen);
        pen = remain*10;          score += pen; if(pen) conoutf(CON_GAMEINFO, "\f2time penalty for %d monsters remaining (10 seconds each): %d seconds", remain, pen);
        pen = (10-skill)*20;      score += pen; if(pen) conoutf(CON_GAMEINFO, "\f2time penalty for lower skill level (20 seconds each): %d seconds", pen);
        pen = 100-accuracy;       score += pen; if(pen) conoutf(CON_GAMEINFO, "\f2time penalty for missed shots (1 second each %%): %d seconds", pen);
        //defformatstring(aname)("bestscore_%s", getclientmap());
        //const char *bestsc = getalias(aname);
        //int bestscore = *bestsc ? parseint(bestsc) : score;
        //if(score<bestscore) bestscore = score;
        //defformatstring(nscore)("%d", bestscore);
       // alias(aname, nscore);
        //conoutf(CON_GAMEINFO, "\f2TOTAL SCORE (time + time penalties): %d seconds (best so far: %d seconds)", score, bestscore);
    }
}
