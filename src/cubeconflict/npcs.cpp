// pimped old monster.cpp from sauerbraten: implements AI for single player monsters, currently client only (now with soft coded monsters!)
#include "gfx.h"
#include "stats.h"

#define MAXNPCS 64 //max amount of different NPCs & monsters for one map/game mode, I think it will be enough

extern int physsteps;
int gamesecs;

VAR(pissoffnpc, 0, 0, 1);

namespace game
{
    static vector<int> teleports;

    enum {M_KEVIN, M_DYLAN, M_YALIEN, M_B_ALIENK, M_ARMOR, M_NINJA, M_B_GIANT, M_CAMPER, M_ALIENS, M_UFO, M_PYRO, NUMMONSTERS};  // monsters for dmsp

    struct npc
    {   //all needed infos for npcs/monsters
        string nameen, namefr, mdlname, shieldname, hatname, capename, boost1name, boost2name;
        bool friendly;
        int npcclass, gun, speed, health, weight, bscale, painlag, trigdist, loyalty, respawn, dropval, spawnfreq;
        int hellosnd, painsnd, angrysnd, diesnd;
        // can be useful in the future to select difficulty and manipulate those vars
        int npctrigdist() const { return trigdist; }
        int npcrespawn() const { return respawn; }
        int npcspeed() const { return speed; }
        int npchealth() const { return health; }
        int npcpain() const { return painlag; }
    };

    npc npcs[MAXNPCS];
    //basic informations about npcs
    ICOMMAND(npcbase, "issiiiiii", (int *id, char *ne, char *nf, int *f, int *c, int *t, int *r, int *d, int *s),
        formatstring(npcs[*id].nameen, "%s", ne);
        formatstring(npcs[*id].namefr, "%s", nf);
        npcs[*id].friendly = *f;
        npcs[*id].npcclass = *c;
        npcs[*id].trigdist = *t;
        npcs[*id].respawn = *r;
        npcs[*id].dropval = *d;
        npcs[*id].spawnfreq = *s;
    );
    //main stats for npcs
    ICOMMAND(npcstats, "iiiiiiii", (int *id, int *g, int *s, int *h, int *w, int *b, int *p, int *l),
        npcs[*id].gun = *g;
        npcs[*id].speed = *s<=0 ? 1 : *s; //speed 0 causes crash
        npcs[*id].health = *h;
        npcs[*id].weight = *w;
        npcs[*id].bscale = *b;
        npcs[*id].painlag = *p;
        npcs[*id].loyalty = *l;
    );
    //sounds for npcs
    ICOMMAND(npcsounds, "iiiii", (int *id, int *h, int *p, int *a, int *d),
        npcs[*id].hellosnd = *h;
        npcs[*id].painsnd = *p;
        npcs[*id].angrysnd = *a;
        npcs[*id].diesnd = *d;
    );
    //models for npcs
    ICOMMAND(npcmodels, "issssss", (int *id, char *mn, char *sn, char *hn, char *cn, char *b1n, char *b2n),
        formatstring(npcs[*id].mdlname, "%s", mn);
        formatstring(npcs[*id].shieldname, "%s", sn);
        formatstring(npcs[*id].hatname, "%s", hn);
        formatstring(npcs[*id].capename, "%s", cn);
        formatstring(npcs[*id].boost1name, "%s", b1n);
        formatstring(npcs[*id].boost2name, "%s", b2n);
    );

    VAR(skill, 1, 10, 10);
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
            stackpos(0, 10, 0)
        {
            type = ENT_AI;
            respawn();
            if(_type>=MAXNPCS || _type < 0)
            {
                conoutf(CON_WARN, "warning: unknown monster in spawn: %d", _type);
                _type = 0;
            }
            mtype = _type;
            const npc &t = npcs[mtype];
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
            maxspeed = (float)t.npcspeed()*4;
            health = t.npchealth();
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

        const int positions[4][2] = { {25, 25}, {-25, -25}, {25, -25}, {-25, 25} };

        void monsteraction(int curtime)           // main AI thinking routine, called every frame for every monster
        {
            if(player1->state==CS_SPECTATOR || player1->state==CS_EDITING) return;
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

            if(dist < npcs[mtype].npctrigdist()*(friendly ? 4 : 2))
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
                    if(trigdist < npcs[mtype].npctrigdist()) targetpitch = asin((enemy->o.z - o.z) / dist) / RAD;            // if player1 is close to pnj, pnj look at the player
                    else targetpitch = 0;
            }

            if(mtype==M_UFO) if(timeinair<2000 && rnd(2)) jumping = true;

            if(blocked)                                                            // special case: if we run into scenery
            {
                blocked = false;
                if(!rnd(2500/npcs[mtype].npcspeed())) jumping = true;               // try to jump over obstackle (rare)
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
                        transition(M_ATTACKING, 1, 200, 200);
                    }
                    break;

                case M_NEUTRAL: if(trigger+10000<lastmillis) transition(M_FRIENDLY, 1, 100, 200);

                case M_FRIENDLY:
                    if(dist < npcs[mtype].npctrigdist())
                    {
                        targetyaw = enemyyaw;
                        if(pissoffnpc)
                        {
                            transition(M_ANGRY, 1, rnd(250), 10);
                            pissoffnpc = 0;
                        }
                    }
                    else targetyaw = spawnyaw;
                    break;

                case M_PAIN:
                case M_ATTACKING:
                case M_SEARCH:
                    if(player1->aptitude==APT_ESPION && player1->abilitymillis[ABILITY_2]) break;
                    if(trigger<lastmillis) transition(M_AGGRO, 1, 100, 200);
                    break;

                case M_SLEEP:                       // state classic sp monster start in, wait for visual contact
                {
                    if(editmode) break;

                    normalize_yaw(enemyyaw);
                    float angle = (float)fabs(enemyyaw-yaw);
                    int trigdist = dist*(player1->crouched() ? 2 : 1);                           // if player1 is crouched, minimal trigger distance is reduced by 2
                    if(trigdist < npcs[mtype].npctrigdist()/(friendly ? 2 : 4)                   // the better the angle to the player, the further the monster can see/hear
                    ||(trigdist < npcs[mtype].npctrigdist()/(friendly ? 1.5f : 3) && angle<135)
                    ||(trigdist < npcs[mtype].npctrigdist()/(friendly ? 1 : 2) && angle<90)
                    ||(trigdist < npcs[mtype].npctrigdist() && angle<45)
                    ||(monsterhurt && o.dist(player1->o) < npcs[mtype].npctrigdist()*(friendly ? 4 : 2)))
                    {
                        vec target;
                        if(raycubelos(o, enemy->o, target))
                        {
                            transition(friendly ? M_FRIENDLY : M_SEARCH, 1, 500, 200);
                            playsound(npcs[mtype].hellosnd, &o);
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
                        transition(M_ATTACKING, friendly ? 0 : 1, 600, 0);
                        if(friendly) transition(M_NEUTRAL, 1, 100, 200);
                    }
                    break;

                case M_RETREAT:
                    if(trigger<lastmillis)
                    {
                        targetyaw = -atan2(spawnpos.x - o.x, spawnpos.y - o.y)/RAD;
                        transition(M_RETREAT, 1, 0, 0);
                        if(o.dist(spawnpos)<10) {targetyaw = spawnyaw; transition(M_SLEEP, 0, 600, 0);}
                    }
                    break;

                case M_AGGRO:                        // monster has visual contact, heads straight for player and may want to shoot at any time
                    targetyaw = enemyyaw;

                    if(dist > npcs[mtype].npctrigdist()*2 && !m_dmsp) {transition(friendly ? M_NEUTRAL : M_RETREAT, 0, 600, 0); break;}
                    else if(player1->aptitude==APT_ESPION && player1->abilitymillis[ABILITY_2]) {transition(M_SEARCH, 0, 600, 0); break;}

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
                            switch(npcs[mtype].gun)
                            {
                                case GUN_CAC349: case GUN_CACFLEAU: case GUN_CACMARTEAU: case GUN_CACMASTER: case GUN_CACNINJA: melee = true; break;
                                case GUN_SV98: case GUN_SKS: case GUN_ARBALETE: case GUN_S_CAMPOUZE: longrange = true; break;
                            }
                            // the closer the monster is the more likely he wants to shoot,
                            if((!melee || dist<50) && !rnd(longrange ? (int)dist/12+1 : min((int)dist/12+1,6)) && enemy->state==CS_ALIVE)      // get ready to fire
                            {
                                attacktarget = target;
                                if(player1->aptitude==APT_ESPION && player1->abilitymillis[ABILITY_1]) attacktarget.add(vec(positions[player1->aptiseed][0], positions[player1->aptiseed][1], 0));
                                transition(M_AIMING, friendly ? 0 : 1, 1, 10);
                            }
                            else                                                        // track player some more
                            {
                                transition(M_AGGRO, 1, 0, 0);
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
                moveplayer(this, m_dmsp ? 1 : 10, true, curtime, 0, mtype==M_UFO ? 999999 : 0, mtype==M_UFO ? APT_PHYSICIEN : 0, 999999, false);        // use physics to move monster
            }
        }

        bool activetrigger = false;

        void checkmonsterstriggers()
        {
            if(npcs[mtype].friendly && player1->o.dist(this->o) < 40 && (this->monsterstate==M_FRIENDLY || this->monsterstate==M_NEUTRAL) && gfx::forcecampos==-1)
            {
                defformatstring(id, "npc_dial_%d", tag);
                if(identexists(id)) execute(id);
                activetrigger = true;
            }
            else if(activetrigger)
            {
                defformatstring(id, "npc_leave_%d", tag);
                if(identexists(id)) execute(id);
                activetrigger = false;
            }
        }

        void checkmonstersrespawns()
        {
            if(totalmillis - monsterlastdeath > npcs[mtype].npcrespawn()*1000)
            {
                targetyaw = spawnyaw;
                o = spawnpos;
                o.addz(4);
                state = CS_ALIVE;
                health = npcs[mtype].npchealth();
            }
        }

        void monsterpain(int damage, gameent *d, int atk)
        {
            playsound(npcs[mtype].painsnd, &o, 0, 0, 0, 250, -1, 500);

            if(d->type==ENT_AI && !friendly)     // a monster hit us
            {
                if(this!=d)            // guard for RL guys shooting themselves :)
                {
                    anger++;     // don't attack straight away, first get angry
                    int _anger = d->type==ENT_AI && mtype==((monster *)d)->mtype ? anger/2 : anger;
                    if(_anger >= npcs[mtype].loyalty)
                    {
                        transition(M_AGGRO, 1, 0, 200);
                        enemy = d;     // monster infight if very angry
                    }
                }
            }
            else if(d->type==ENT_PLAYER) // player hit us
            {
                if(friendly)
                {
                    if(this->monsterstate==M_FRIENDLY) transition(M_NEUTRAL, 1, 0, 200);      //if you mess with a friendly pnj, he gets neutral for a moment
                    else if(this->monsterstate==M_NEUTRAL)
                    {
                        transition(M_ANGRY, 1, rnd(250), 10);    //if you mess with a neutral pnj, he gets aggressive
                        playsound(npcs[mtype].angrysnd, &o, 0, 0, 0, 200, -1, 400);
                    }
                    return;
                }

                anger = 0;
                enemy = d;
                monsterhurt = true;
                monsterhurtpos = o;
            }
            damageeffect(damage, this, d, atk);
            if((health -= damage)<=0)
            {
                state = CS_DEAD;
                lastpain = lastmillis;
                playsound(npcs[mtype].diesnd, &o, 0, 0, 0, 200, -1, 400);
                monsterkilled(d);
                gibeffect(max(-health, 0), vel, this);
                monsterlastdeath = totalmillis;
                defformatstring(id, "monster_dead_%d", tag);
                if(identexists(id)) execute(id);
                if(m_dmsp && gamesecs<599) npcdrop(&monsterhurtpos, npcs[mtype].dropval);
                if(player1->aptitude==APT_FAUCHEUSE && player1->health<1500) player1->health = min(player1->health+50, 1500);
                switch(mtype)
                {
                    case M_KEVIN: case M_DYLAN: loopi(2+rnd(3)) spawnbouncer(o, vec(0, 0, 0), this, BNC_GIBS); break;
                    case M_UFO: loopi(25) spawnbouncer(o, vec(0, 0, 0), this, BNC_GRENADE, 5000+rnd(2000), true); break;
                    case M_ARMOR: loopi(10+rnd(5)) spawnbouncer(o, vec(0, 0, 0), this, BNC_ROBOT); break;
                }
            }
            else
            {
                transition(M_PAIN, 0, npcs[mtype].painlag, 200);      // in this state monster won't attack
                playsound(npcs[mtype].painsnd, &o, 0, 0, 0, 200, -1, 400);
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
        loopi(MAXNPCS)
        {
            conoutf("%d", npcs[i].spawnfreq);
            preloadmodel(npcs[i].mdlname);
            preloadmodel(npcs[i].hatname);
        }
    }

    vector<monster *> monsters;

    int nextmonster, spawnremain, numkilled, monstertotal, mtimestart, remain;

    int totmfreq()
    {
        int n = 0;
        loopi(MAXNPCS) {n += npcs[i].spawnfreq;}
        return n;
    }

    void spawnmonster(bool boss = false, int type = 0)     // spawn a random monster according to freq distribution in DMSP
    {
        if(boss) monsters.add(new monster(type, rnd(360), 0, 0, M_SEARCH, 1000, 1));
        else
        {
            int n = rnd(totmfreq()), type;
            conoutf("%d", totmfreq());
            for(int i = 0; ; i++) if((n -= npcs[i].spawnfreq)<0) { type = i; break; }
            monsters.add(new monster(type, rnd(360), 0, 0, M_SEARCH, 1000, 1));
        }
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
        gamesecs = 0;
        remain = 0;
        monsterhurt = false;
        if(m_dmsp)
        {
            nextmonster = mtimestart = lastmillis+5000;
            monstertotal = spawnremain = 999999999;
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

    void endsp()
    {
        loopv(monsters)
        {
            monsters[i]->lastpain = lastmillis;
            monsters[i]->state = CS_DEAD;
        }
        conoutf(CON_GAMEINFO, GAME_LANG ? "\f2You survived, well done!" : "\f2Tu as survécu, bravo !");
    }
    ICOMMAND(endsp, "", (), endsp());

    void monsterkilled(gameent *d)
    {
        if(d==player1)
        {
            numkilled++;
            player1->frags = numkilled;
            if(player1->frags>=200) unlockachievement(ACH_ELIMINATOR);
        }
        remain = monstertotal-numkilled;
    }

    int spawn = true;
    void updatemonsters(int curtime)
    {
        if(!isconnected()) return;
        if(m_dmsp && spawnremain && lastmillis>nextmonster && player1->state==CS_ALIVE)
        {
            if(spawnremain--==monstertotal) { hudmsg[MSG_PREMISSION]=lastmillis; playsound(S_INVASION); musicmanager(2+map_sel); execute("playmapsong"); }
            nextmonster = lastmillis+1000;
            gamesecs++;
            if(spawn && !intermission)
            {
                gamesecs > 270 && gamesecs < 330 ? spawnmonster(true, M_YALIEN) : spawnmonster();
                spawn = gamesecs > 450 ? true : false;
            }
            else spawn = true;

            switch(gamesecs)
            {
                case 60: case 180: loopi(gamesecs==180 ? 5 : 3) spawnmonster(true, M_NINJA); break;
                case 90: case 240: loopi(gamesecs==240 ? 5 : 3) spawnmonster(true, M_ARMOR); break;
                case 150: case 300:
                    spawnmonster(true, M_UFO);
                    loopi(5) spawnmonster(true, gamesecs== 300 ? M_B_ALIENK : M_ALIENS);
                    playsound(S_ALIEN_INVASION);
                    break;

                case 210: loopi(3) spawnmonster(true, M_B_GIANT); break;

                case 270: case 330:
                    loopi(3) spawnmonster(true, M_UFO);
                    loopi(15) spawnmonster(true, gamesecs==330 ? M_ALIENS : M_YALIEN);
                    playsound(S_ALIEN_INVASION);
                    break;
                case 360: loopi(10) spawnmonster(true, M_CAMPER); break;
                case 400: loopi(10) spawnmonster(true, 12+rnd(4)); break;
                case 480: loopi(10) spawnmonster(true, 12+rnd(4)); break;
                case 525: loopi(15) spawnmonster(true, 12+rnd(4)); break;
                case 550: loopi(5)  spawnmonster(true, M_UFO); playsound(S_ALIEN_INVASION);
                case 575: loopi(20) spawnmonster(true, 12+rnd(4)); break;
                case 600: endsp(); unlockachievement(ACH_SURVIVOR); break;
                case 610: trydisconnect(true); break;
                default:
                    if(gamesecs>450 && (gamesecs%4 == 0)) spawnmonster(true, M_PYRO);
            }
        }

        bool monsterwashurt = monsterhurt;

        loopv(monsters)
        {
            if(monsters[i]->state==CS_ALIVE)
            {
                monsters[i]->monsteraction(curtime);
                if(!m_dmsp) monsters[i]->checkmonsterstriggers();
            }
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

    void drawnpcs(gameent *d, float x, float y, float s)
    {
        float scale = calcradarscale();

        loopv(monsters)
        {
            monster *o = monsters[i];

            if(m_dmsp ? o->state==CS_ALIVE : npcs[o->mtype].friendly)
            {
                setbliptex(m_dmsp ? 2 : 1, "");
                gle::defvertex(2);
                gle::deftexcoord0();
                gle::begin(GL_QUADS);

                drawteammate(d, x, y, s, o, scale);
            }
        }
        gle::end();
    }

    const char *stnames[M_MAX] = {
        "none", "searching", "aggro", "retreat", "attacking", "in pain", "sleeping", "aiming", "friendly", "neutral", "angry"
    };

    VAR(dbgpnj, 0, 0, 1);

    void debugpnj(monster *m)
    {
        vec entpos = m->abovehead();
        vec campos = camera1->o;
        vec partpos = (entpos.add((campos.mul(vec(3, 3, 3))))).div(vec(4, 4, 4));

        defformatstring(s1, "health: %d - %s", m->health, m->health>0 ? "alive" : "dead");
        particle_textcopy(partpos.addz(1), s1, PART_TEXT, 1, 0xFFFFFF, 1);

        defformatstring(s2, "state: %s", stnames[m->monsterstate]);
        particle_textcopy(partpos.addz(1), s2, PART_TEXT, 1, 0xFFFFFF, 1);

        defformatstring(s3, "friendly? %s", m->friendly ? "yes" : "no");
        particle_textcopy(partpos.addz(1), s3, PART_TEXT, 1, 0xFFFFFF, 1);
    }

    static const int dirs[9] =
    {
        ANIM_LEFT,  ANIM_FORWARD,   ANIM_RIGHT,
        ANIM_LEFT,  0,              ANIM_RIGHT,
        ANIM_LEFT,  ANIM_BACKWARD,  ANIM_RIGHT
    };

    void rendermonsters()
    {
        loopv(monsters)
        {
            monster &m = *monsters[i];

            if(m.state!=CS_DEAD || lastmillis-m.lastpain<10000)
            {
                float yaw = m.yaw,
                      pitch = m.pitch;
                float fade = 1;
                vec o = m.feetpos();

                int anim = ANIM_IDLE|ANIM_LOOP; int basetime = 0;

                modelattach a[10];
                int ai = 0;

                if(m.state==CS_DEAD)
                {
                    fade -= clamp(float(lastmillis - (m.lastpain + 9000))/1000, 0.0f, 1.0f);
                    anim = ANIM_DYING;
                    basetime = m.lastpain;
                    if(lastmillis-basetime>1000) anim = ANIM_DEAD|ANIM_LOOP;
                }
                else
                {
                    //////////////////////////////////// Npc's anims ////////////////////////////////////////////////////////////////////////
                    if(!npcs[m.mtype].friendly)
                    {
                        if(m.inwater && m.physstate<=PHYS_FALL)
                        {
                            anim |= (((m.move || m.strafe) || m.vel.z + m.falling.z>0 ? ANIM_SWIM : ANIM_SINK)|ANIM_LOOP)<<ANIM_SECONDARY;
                            if(m.move && randomevent(0.16f*gfx::nbfps)) particle_splash(PART_WATER, 2, 120, m.o, 0x222222, 8.0f+rnd(5), 150, 15);
                        }
                        else
                        {
                            int dir = dirs[(m.move+1)*3 + (m.strafe+1)];
                            if(m.timeinair>50) anim |= ((ANIM_JUMP) | ANIM_END) << ANIM_SECONDARY;
                            else if(dir) anim |= (dir | ANIM_LOOP) << ANIM_SECONDARY;

                            if(m.move && m.physstate==PHYS_FLOOR && randomevent(0.16f*gfx::nbfps)) particle_splash(atmos && (lookupmaterial(m.feetpos())==MAT_WATER || map_atmo==4 || map_atmo==8) ? PART_WATER : PART_SMOKE, 3, 120, m.feetpos(), atmos && map_atmo==4 ? 0x131313 : 0x333022, 6.0f+rnd(5), 150, 15);
                        }
                    }

                    if((anim&ANIM_INDEX)==ANIM_IDLE && (anim>>ANIM_SECONDARY)&ANIM_INDEX) anim >>= ANIM_SECONDARY;
                    if(!((anim>>ANIM_SECONDARY)&ANIM_INDEX)) anim |= (ANIM_IDLE|ANIM_LOOP)<<ANIM_SECONDARY;

                    //////////////////////////////////// Weapon rendering ////////////////////////////////////////////////////////////////////////
                    int vanim = ANIM_VWEP_IDLE|ANIM_LOOP, vtime = 0;
                    if(m.lastaction && m.lastattack >= 0 && attacks[m.lastattack].gun==m.gunselect && lastmillis < m.lastaction+250)
                    {
                        vanim = ANIM_VWEP_SHOOT;
                        vtime = m.lastaction;
                    }

                    a[ai++] = modelattach("tag_weapon", guns[m.gunselect].vwep, vanim, vtime);

                    if(guns[m.gunselect].vwep)
                    {
                        m.muzzle = m.balles = vec(-1, -1, -1);
                        a[ai++] = modelattach("tag_muzzle", &m.muzzle);
                        a[ai++] = modelattach("tag_balles", &m.balles);
                    }

                    //////////////////////////////////// Other mdls rendering ////////////////////////////////////////////////////////////////////////
                    if(npcs[m.mtype].shieldname) a[ai++] = modelattach("tag_shield", npcs[m.mtype].shieldname, ANIM_VWEP_IDLE|ANIM_LOOP, 0);
                    if(npcs[m.mtype].hatname) a[ai++] = modelattach("tag_hat", npcs[m.mtype].hatname, ANIM_VWEP_IDLE|ANIM_LOOP, 0);
                    if(npcs[m.mtype].capename) a[ai++] = modelattach("tag_cape", npcs[m.mtype].capename, ANIM_VWEP_IDLE|ANIM_LOOP, 0);
                    if(npcs[m.mtype].boost1name) a[ai++] = modelattach("tag_boost1", npcs[m.mtype].boost1name, ANIM_VWEP_IDLE|ANIM_LOOP, 0);
                    if(npcs[m.mtype].boost2name) a[ai++] = modelattach("tag_boost2", npcs[m.mtype].boost2name, ANIM_VWEP_IDLE|ANIM_LOOP, 0);
                }

                rendermodel(npcs[m.mtype].mdlname, anim, o, yaw, pitch, 0, MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY, &m, a[0].tag ? a : NULL, basetime, 0, fade);
                if(dbgpnj) debugpnj(&m);
            }
        }
    }

    void suicidemonster(monster *m)
    {
        m->monsterpain(400, m, ATK_CAC349_SHOOT);
    }

    void hitmonster(int damage, monster *m, gameent *at, const vec &vel, int atk)
    {
        if(player1->boostmillis[B_ROIDS]) damage*=(player1->aptitude==APT_JUNKIE ? 3.f : 2.f);
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
