#include "gfx.h"
#include "customs.h"
#include "stats.h"

bool rndevent(int probability, int probabilityReduce) // adjust the probability (0-100) based on game speed and frames per second (60 as standard)
{
    float adjusted = max((((100 - probability) * 100) / game::gamespeed) * (curfps / 60.f), 1.f);
    if(probabilityReduce) adjusted *= probabilityReduce;
    return !game::ispaused() && !rnd((int)adjusted);
}

VARR(deadlylava, 0, 1, 1);

namespace game
{
    int oldapti;
    VARFP(player1_aptitude, 0, 0, NUMCLASSES-1,
    {
        if(player1->state != CS_DEAD && isconnected() && !intermission && !m_tutorial)
        {
            conoutf(CON_GAMEINFO, "\fc%s", readstr("Console_Game_UnableToChangeClass"));
            playSound(S_ERROR, vec(0, 0, 0), 0, 0, SND_UI);
            player1_aptitude = oldapti;
        }
        else
        {
            addmsg(N_SENDCLASS, "ri", player1_aptitude);
            player1->aptitude = player1_aptitude;
            oldapti = player1->aptitude;
            if(!islaunching) playSound(S_C_SOLDIER+player1_aptitude, vec(0, 0, 0), 0, 0, SND_UI);
            if(isconnected() && !intermission) unlockAchievement(ACH_UNDECIDED);
        }
    });

    bool intermission = false;
    int maptime = 0, maprealtime = 0, maplimit = -1;
    int lasthit = 0, lastspawnattempt = 0;
    int respawnent = -1;

    gameent *player1 = NULL;         // our client
    vector<gameent *> players;       // other clients

    VAR(teamscoreboardcolor, 0, 0, 1);
    ICOMMAND(getsbcolor, "", (), player1->team == 1 ? teamscoreboardcolor = 1 : teamscoreboardcolor = 0;);

    int following = -1;

    ICOMMAND(isteamone, "", (), intret(player1->team==1););

    VARFP(specmode, 0, 0, 2,
    {
        if(!specmode) stopfollowing();
        else if(following < 0) nextfollow();
    });

    gameent *followingplayer(gameent *fallback)
    {
        if(player1->state!=CS_SPECTATOR || following<0) return fallback;
        gameent *target = getclient(following);
        if(target && target->state!=CS_SPECTATOR) return target;
        return fallback;
    }

    int lasttimeupdate = 0, dmg = 0, dmgsecs[2];

    void dotime()
    {
        dmg = 0;
        loopi(4) dmg += avgdmg[i];
        if(totalmillis >= lasttimeupdate+1000) // 1 second interval
        {
            updateStat(1, STAT_TIMEPLAYED);
            lasttimeupdate = totalmillis;
            dmgsecs[0]==3 ? dmgsecs[0]=0 : dmgsecs[0]++;
            dmgsecs[1]==5 ? dmgsecs[1]=2 : dmgsecs[1]++;
            avgdmg[dmgsecs[1]-2] = 0;
        }
    }

    ICOMMAND(getCurrentDps, "", (), intret(dmg/4.f));

    ICOMMAND(getfollow, "", (),
    {
        gameent *f = followingplayer();
        intret(f ? f->clientnum : -1);
    });

    void stopfollowing()
    {
        if(following<0) return;
        following = -1;
    }

    void follow(char *arg)
    {
        int cn = -1;
        if(arg[0])
        {
            if(player1->state != CS_SPECTATOR) return;
            cn = parseplayer(arg);
            if(cn == player1->clientnum) cn = -1;
        }
        if(cn < 0 && (following < 0 || specmode)) return;
        following = cn;
    }
    COMMAND(follow, "s");

    void nextfollow(int dir)
    {
        if(player1->state!=CS_SPECTATOR) return;
        int cur = following >= 0 ? following : (dir > 0 ? clients.length() - 1 : 0);
        loopv(clients)
        {
            cur = (cur + dir + clients.length()) % clients.length();
            if(clients[cur] && clients[cur]->state!=CS_SPECTATOR)
            {
                following = cur;
                return;
            }
        }
        stopfollowing();
    }
    ICOMMAND(nextfollow, "i", (int *dir), nextfollow(*dir < 0 ? -1 : 1));

    void checkfollow()
    {
        if(player1->state != CS_SPECTATOR)
        {
            if(following >= 0) stopfollowing();
        }
        else
        {
            if(following >= 0)
            {
                gameent *d = clients.inrange(following) ? clients[following] : NULL;
                if(!d || d->state == CS_SPECTATOR) stopfollowing();
            }
            if(following < 0 && specmode) nextfollow();
        }
    }

    const char *getclientmap() { return clientmap; }

    void resetgamestate()
    {
        if(m_classicsp || m_tutorial)
        {
            clearmonsters();                 // all monsters back at their spawns for editing
            entities::resettriggers();
        }
        clearprojectiles();
        clearbouncers();
        clearEntsPos();
    }

    gameent *spawnstate(gameent *d)              // reset player state not persistent accross spawns
    {
        d->respawn();
        d->spawnstate(gamemode, d->aptitude);
        return d;
    }

    void respawnself()
    {
        if(ispaused()) return;
        if(m_mp(gamemode))
        {
            int seq = (player1->lifesequence<<16)|((lastmillis/1000)&0xFFFF);
            if(player1->respawned!=seq) { addmsg(N_TRYSPAWN, "rc", player1); player1->respawned = seq; }
        }
        else
        {
            spawnplayer(player1);
            showscores(false);
            lasthit = 0;
            if(cmode) cmode->respawned(player1);
        }
        postfx::updateMainFilter();
    }

    gameent *pointatplayer()
    {
        loopv(players) if(players[i] != player1 && intersect(players[i], player1->o, worldpos)) return players[i];
        return NULL;
    }

    gameent *hudplayer()
    {
        if((thirdperson && allowthirdperson()) || specmode > 1) return player1;
        return followingplayer(player1);
    }

    void setupcamera()
    {
        gameent *target = followingplayer();
        if(target)
        {
            player1->yaw = target->yaw;
            player1->pitch = target->state==CS_DEAD ? 0 : target->pitch;
            player1->o = target->o;
            player1->resetinterp();
        }
    }

    bool allowthirdperson(bool msg)
    {
        return player1->state==CS_SPECTATOR || player1->state==CS_EDITING || m_edit || !multiplayer(msg);
    }
    ICOMMAND(allowthirdperson, "b", (int *msg), intret(allowthirdperson(*msg!=0) ? 1 : 0));

    bool detachcamera()
    {
        gameent *d = followingplayer();
        if(d) return specmode > 1 || d->state == CS_DEAD;
        return player1->state == CS_DEAD;
    }

    bool collidecamera()
    {
        switch(player1->state)
        {
            case CS_EDITING: return false;
            case CS_SPECTATOR: return followingplayer()!=NULL;
        }
        return true;
    }

    VARP(smoothmove, 0, 75, 100);
    VARP(smoothdist, 0, 32, 64);

    void predictplayer(gameent *d, bool move)
    {
        d->o = d->newpos;
        d->yaw = d->newyaw;
        d->pitch = d->newpitch;
        d->roll = d->newroll;
        if(move)
        {
            moveplayer(d, 1, false);
            d->newpos = d->o;
        }
        float k = 1.0f - float(lastmillis - d->smoothmillis)/smoothmove;
        if(k>0)
        {
            d->o.add(vec(d->deltapos).mul(k));
            d->yaw += d->deltayaw*k;
            if(d->yaw<0) d->yaw += 360;
            else if(d->yaw>=360) d->yaw -= 360;
            d->pitch += d->deltapitch*k;
            d->roll += d->deltaroll*k;
        }
    }

    void getaptiweap()
    {
        switch(player1->aptitude)
        {
            case C_KAMIKAZE:
                if(player1->gunselect!=GUN_KAMIKAZE) gunselect(currentIdenticalWeapon, player1);
                break;
            case C_NINJA:
                if(player1->gunselect!=GUN_NINJA) gunselect(currentIdenticalWeapon, player1);
                break;
            default: gunselect(currentIdenticalWeapon, player1);
        }
    }

    bool hasAbilityEnabled(gameent *d, int numAbility)
    {
        if(d->abilitymillis[numAbility]) return true;
        return false;
    }

    bool hasboost(gameent *d)
    {
        loopi(NUMBOOSTS) { if(d->boostmillis[i]) return true; }
        return false;
    }

    bool isAttacking(gameent *d)
    {
        return d->lastaction && d->lastattack >= 0 && attacks[d->lastattack].gun == d->gunselect && lastmillis-d->lastaction<attacks[d->lastattack].attackdelay + 50;
    }

    void updatePlayersSounds(gameent *d)
    {
        bool isHudPlayer = (d == hudplayer());

        updateEntPos(d->entityId, isHudPlayer ? vec(0, 0, 0) : d->o);

        if(isHudPlayer) // underwater camera sound
        {
            bool inLiquid = ((lookupmaterial(camera1->o) & MATF_VOLUME) == MAT_WATER) || ((lookupmaterial(camera1->o) & MATF_VOLUME) == MAT_LAVA);
            static bool underwaterSound = false;
            if(inLiquid && !underwaterSound) { playSound(S_UNDERWATER, vec(0, 0, 0), 0, 0, SND_FIXEDPITCH|SND_LOOPED, hudplayer()->entityId, PL_UNDERWATER); underwaterSound = true; }
            else if(!inLiquid && underwaterSound) { stopLinkedSound(hudplayer()->entityId, PL_UNDERWATER); underwaterSound = false; }
        }

        if(d->state != CS_ALIVE) return;

        if(d->attacksound)
        {
            if((!isAttacking(d) || !(d->gunselect==GUN_PLASMA || d->gunselect==GUN_FLAMETHROWER || d->gunselect==GUN_S_GAU8)))
            {
                stopLinkedSound(d->entityId, PL_ATTACK);
                stopLinkedSound(d->entityId, PL_ATTACK_FAR);
                d->attacksound = 0;
            }

            bool hasShrooms = d->boostmillis[B_SHROOMS];

            float pitch = (hasShrooms ? 1.2f : 1.0f);

            if(d->aptitude == C_PRIEST && d->abilitymillis[ABILITY_3])
            {
                float abilityPitch = 1.5f - (d->abilitymillis[ABILITY_3] / 8000.0f);
                pitch *= abilityPitch;
            }

            updateSoundPitch(d->entityId, PL_ATTACK, pitch);
            updateSoundPitch(d->entityId, PL_ATTACK_FAR, pitch);

        }
        if(hasPowerArmor(d) && d->armour < 1000) // power armor alarm sound
        {
            if(!d->powerarmoursound)
            {
                playSound(S_ASSISTALARM, isHudPlayer ? vec(0, 0, 0) : d->o, 250, 100, SND_NOCULL|SND_FIXEDPITCH|SND_LOOPED, d->entityId, PL_POWERARMOR);
                d->powerarmoursound = true;
            }
        }
        else if(d->powerarmoursound) { stopLinkedSound(d->entityId, PL_POWERARMOR); d->powerarmoursound = false; }
    }

    void updatePlayersBoosts(int time, gameent *d)
    {
        if(hasboost(d))
        {
            bool isHudPlayer = (d == hudplayer());

            if(d->boostmillis[B_ROIDS] && (d->boostmillis[B_ROIDS] -= time)<=0)
            {
                d->boostmillis[B_ROIDS] = 0;
                playSound(S_ROIDS_PUPOUT, isHudPlayer ? vec(0, 0, 0) : d->o, 300, 50, NULL, d->entityId);
                if(d==player1) conoutf(CON_HUDCONSOLE, "\f8%s", readstr("GameMessage_RoidsEnded"));
            }

            if(d->boostmillis[B_EPO] && (d->boostmillis[B_EPO] -= time)<=0)
            {
                d->boostmillis[B_EPO] = 0;
                playSound(S_EPO_PUPOUT, isHudPlayer ? vec(0, 0, 0) : d->o, 300, 50, NULL, d->entityId);
                if(d==player1) conoutf(CON_HUDCONSOLE, "\f8%s", readstr("GameMessage_EpoEnded"));
            }

            if(d->boostmillis[B_JOINT] && (d->boostmillis[B_JOINT] -= time)<=0)
            {
                d->boostmillis[B_JOINT] = 0;
                if(d==player1) conoutf(CON_HUDCONSOLE, "\f8%s", readstr("GameMessage_JointEnded"));
            }

            static bool endingShrooms;
            if(isHudPlayer && d->boostmillis[B_SHROOMS] > 4000) endingShrooms = false;
            else if(!endingShrooms && isHudPlayer && d->boostmillis[B_SHROOMS] && d->boostmillis[B_SHROOMS] < 4000)
            {
                playSound(S_SHROOMS_PUPOUT);
                endingShrooms = true;
            }

            if(d->boostmillis[B_SHROOMS] && (d->boostmillis[B_SHROOMS] -= time)<=0)
            {
                d->boostmillis[B_SHROOMS] = 0;
                if(d==player1)
                {
                    conoutf(CON_HUDCONSOLE, "\f8%s", readstr("GameMessage_ShroomsEnded"));
                    postfx::clearShroomsEffect();
                }
            }

            if(d->boostmillis[B_RAGE] && (d->boostmillis[B_RAGE] -= time)<=0) d->boostmillis[B_RAGE] = 0;
        }

        if(d->afterburnmillis && (d->afterburnmillis -= time)<=0) d->afterburnmillis = 0;
    }

    void checkFire(gameent *d)
    {
        static int tick = 0;
        tick += curtime;

        if(tick < 500 && d->afterburnmillis) return;

        loopv(entities::ents)
        {
            gameentity &e = *(gameentity *)entities::ents[i];
            if(e.type == ET_PARTICLES && (e.attr1==1 || e.attr1==2) && d->o.dist(e.o) < e.attr2 / 25) addmsg(N_FIRETOUCH, "rc", d);
        }

        tick = 0;
    }

    void otherplayers(int curtime)
    {
        loopv(players)
        {
            gameent *d = players[i];

            if(isconnected()) updatePlayersSounds(d);

            if(d->state == CS_ALIVE && !intermission)
            {
                checkFire(d);

                if(!deadlylava && lookupmaterial(d->feetpos())&MAT_LAVA && lastmillis - d->lastlavatouch > 250)
                {
                    d->lastlavatouch = lastmillis;
                    addmsg(N_LAVATOUCH, "rc", d);
                    d->falling = vec(0, 0, 0);
                    d->physstate = PHYS_FALL;
                    d->timeinair = 1;
                    d->vel.z = max(d->vel.z, 300.f);
                }

                if(d!=player1)
                {
                    if(d->armourtype==A_POWERARMOR && !d->armour && d->ammo[GUN_POWERARMOR]) {gunselect(GUN_POWERARMOR, d, true); d->gunwait=0;}
                    if(lastmillis - d->lastaction >= d->gunwait) d->gunwait = 0;
                    updatePlayersBoosts(curtime, d);
                    updateAbilitiesSkills(curtime, d);
                }

                if((lookupmaterial(d->o) & MAT_GAMECLIP) == MAT_GAMECLIP)
                {
                    if(!d->isOutOfMap)
                    {
                        d->lastOutOfMap = totalmillis;
                        d->isOutOfMap = true;
                    }
                    if(d->lastOutOfMap < totalmillis - 9900)
                    {
                        suicide(d);
                        if(d==hudplayer())
                        {
                            playSound(S_ECLAIRPROCHE);
                            adddynlight(vec(d->abovehead()).addz(30), 500, vec(0.0f, 0.5f, 1.5f), 80, 40, L_NODYNSHADOW);
                            particle_flare(vec(d->o).add(vec(50 - rnd(101), 50 - rnd(101), 1500)), d->feetpos(), 175, PART_LIGHTNING, 0xFFFFFF, 40.0f);
                            particle_splash(PART_SMOKE,  15, 2000, d->o, 0x333333, 40.0f, 150, 500);
                        }
                    }
                }
                else d->isOutOfMap = false;
            }

            if(totalmillis - d->lastcurdamage > 500) d->curdamage = 0;

            if(d == player1 || d->ai) continue;
            if(d->state==CS_DEAD && d->ragdoll) moveGrave(d);

            const int lagtime = totalmillis-d->lastupdate;
            if(!lagtime || intermission) continue;
            else if(lagtime>1000 && d->state==CS_ALIVE)
            {
                d->state = CS_LAGGED;
                continue;
            }
            if(d->state==CS_ALIVE || d->state==CS_EDITING)
            {
                crouchplayer(d, 10, false);
                if(smoothmove && d->smoothmillis>0) predictplayer(d, true);
                else moveplayer(d, 1, false);
            }
            else if(d->state==CS_DEAD && !d->ragdoll && lastmillis-d->lastpain<2000) moveplayer(d, 1, true);
        }
    }

    VAR(canMove, 0, 1, 1);

    bool onFixedCamera(gameent *d)
    {
        return forcecampos >= 0 && d->state!=CS_SPECTATOR;
    }

    vec getPlayerVelocity()
    {
        static vec prevPos(0, 0, 0);
        static vec smoothVel(0, 0, 0);
        static int prevTime = 0;

        int currentTime = totalmillis;
        int timeElapsed = max(currentTime - prevTime, 1); // Time elapsed since the last frame in milliseconds

        vec velocity = vec(hudplayer()->o).sub(prevPos); // Calculate raw velocity

        if(timeElapsed > 0) velocity = velocity.mul(1000.0f / timeElapsed); // Convert milliseconds to seconds for velocity calculation

        float smoothingFactor = clamp(1.0f - (timeElapsed / 100.0f), 0.1f, 0.9f); // smoothing factor scaled with refresh rate
        smoothVel = smoothVel.mul(1.0f - smoothingFactor).add(velocity.mul(smoothingFactor)); // Smoothing the velocity using an exponential moving average (EMA)

        prevPos = hudplayer()->o;
        prevTime = currentTime;

        return smoothVel;
    }

    void updateworld()        // main game update loop
    {
        if(!maptime) { maptime = lastmillis; maprealtime = totalmillis; return; }
        if(!curtime) { gets2c(); if(player1->clientnum>=0) c2sinfo(); return; }

        physicsframe();
        ai::navigate();

        postfx::updateRadialBlur(getPlayerVelocity(), hudplayer()->boostmillis[B_SHROOMS]);

        if(player1->state==CS_ALIVE && !intermission)   // checking player1's shits
        {
            checkInventoryGuns();
            if(player1->armourtype==A_POWERARMOR && player1->ammo[GUN_POWERARMOR] && !player1->armour) {gunselect(GUN_POWERARMOR, player1, true); player1->gunwait=0;}
            else if(m_identique)
            {
                switch(player1->gunselect)
                {
                    case GUN_S_NUKE: case GUN_S_GAU8: case GUN_S_CAMPER: case GUN_S_ROCKETS: break;
                    default: getaptiweap();
                }
            }

            if(IS_ON_OFFICIAL_SERV) // checking for achievements
            {
                if(player1->health>=2000) unlockAchievement(ACH_SACAPV);
                if(player1->boostmillis[B_ROIDS] && player1->boostmillis[B_EPO] && player1->boostmillis[B_JOINT] && player1->boostmillis[B_SHROOMS]) unlockAchievement(ACH_DEFONCE);
                if(lookupmaterial(player1->o)==MAT_NOCLIP && !strcasecmp(getclientmap(), "moon")) unlockAchievement(ACH_SPAAACE);
                if(hasSuperWeapon(player1) && player1->boostmillis[B_ROIDS] && player1->armour && player1->armourtype==A_POWERARMOR) unlockAchievement(ACH_ABUS);
                if(player1->aptitude==C_KAMIKAZE && !player1->ammo[GUN_KAMIKAZE] && totalmillis-lastshoot>=500 && totalmillis-lastshoot<=750 && isconnected()) unlockAchievement(ACH_SUICIDEFAIL);
                if(player1->boostmillis[B_EPO] && player1->aptitude==C_JUNKIE) unlockAchievement(ACH_LANCEEPO);
            }

            updatePlayersBoosts(curtime, player1);
            updateAbilitiesSkills(curtime, player1);
        }

        updateweapons(curtime);
        otherplayers(curtime);
        ai::update();
        moveGraves();
        gets2c();
        updatemonsters(curtime);
        if(connected)
        {
            if(player1->state == CS_DEAD)
            {
                if(player1->ragdoll) moveGrave(player1);
                else
                {
                    player1->move = player1->strafe = 0;
                    moveplayer(player1, 10, true);
                }
            }
            else if(!intermission && !onFixedCamera(player1))
            {
                if(player1->ragdoll) cleanGrave(player1);
                crouchplayer(player1, 10, true);
                if(canMove || !m_tutorial)
                {
                    moveplayer(player1, 10, true);
                    swayhudgun(curtime);
                }
                entities::checkitems(player1);
                if(m_sp || m_classicsp || m_tutorial) entities::checktriggers();
                else if(cmode) cmode->checkitems(player1);
            }
        }
        if(player1->clientnum>=0) c2sinfo();   // do this last, to reduce the effective frame lag
    }

    float proximityscore(float x, float lower, float upper)
    {
        if(x <= lower) return 1.0f;
        if(x >= upper) return 0.0f;
        float a = x - lower, b = x - upper;
        return (b * b) / (a * a + b * b);
    }

    static inline float harmonicmean(float a, float b) { return a + b > 0 ? 2 * a * b / (a + b) : 0.0f; }

    // avoid spawning near other players
    float ratespawn(dynent *d, const extentity &e)
    {
        gameent *p = (gameent *)d;
        vec loc = vec(e.o).addz(p->eyeheight);
        float maxrange = !m_noitems ? 400.0f : (cmode ? 300.0f : 110.0f);
        float minplayerdist = maxrange;
        loopv(players)
        {
            const gameent *o = players[i];
            if(o == p)
            {
                if(m_noitems || (o->state != CS_ALIVE && lastmillis - o->lastpain > 3000)) continue;
            }
            else if(o->state != CS_ALIVE || isteam(o->team, p->team)) continue;

            vec dir = vec(o->o).sub(loc);
            float dist = dir.squaredlen();
            if(dist >= minplayerdist*minplayerdist) continue;
            dist = sqrtf(dist);
            dir.mul(1/dist);

            // scale actual distance if not in line of sight
            if(raycube(loc, dir, dist) < dist) dist *= 1.5f;
            minplayerdist = min(minplayerdist, dist);
        }
        float rating = 1.0f - proximityscore(minplayerdist, 80.0f, maxrange);
        return cmode ? harmonicmean(rating, cmode->ratespawn(p, e)) : rating;
    }

    void spawnplayer(gameent *d)   // place at random spawn
    {
        int ent = (m_classicsp || m_tutorial) && d == player1 && respawnent >= 0 ? respawnent : -1;

        if(cmode) cmode->pickspawn(d);
        else findplayerspawn(d, ent, m_teammode && !m_capture  ? d->team : 0);
        spawnstate(d);
        if(d==player1)
        {
            if(editmode) d->state = CS_EDITING;
            else if(d->state != CS_SPECTATOR) d->state = CS_ALIVE;
        }
        else d->state = CS_ALIVE;
        checkfollow();
    }

    VARP(spawnwait, 3000, 3000, 10000);

    void respawn()
    {
        if(player1->state==CS_DEAD)
        {
            player1->attacking = ACT_IDLE;
            int wait = cmode ? cmode->respawnwait(player1) : 0;
            if(wait>0)
            {
                lastspawnattempt = lastmillis;
                //conoutf(CON_GAMEINFO, "\f2you must wait %d second%s before respawn!", wait, wait!=1 ? "s" : "");
                return;
            }
            if(lastmillis < player1->lastpain + spawnwait) return;
            if(m_dmsp) { changemap(clientmap, gamemode); return; }    // if we die in SP we try the same map again
            respawnself();
        }
    }

    // inputs

    VARP(attackspawn, 0, 1, 1);

    void doaction(int act)
    {
        if(!connected || intermission || onFixedCamera(player1)) return;
        if((player1->attacking = act) && attackspawn) respawn();
    }

    ICOMMAND(shoot, "D", (int *down), doaction(*down ? ACT_SHOOT : ACT_IDLE));

    VARP(jumpspawn, 0, 1, 1);

    bool canjump()
    {
        if(!connected || intermission || onFixedCamera(player1)) return false;
        if(jumpspawn) respawn();
        return player1->state!=CS_DEAD;
    }

    bool cancrouch()
    {
        if(!connected || intermission || onFixedCamera(player1)) return false;
        return player1->state!=CS_DEAD;
    }

    VARFP(player1_taunt, 0, 0, 0, // sizeof(customsdance)/sizeof(customsdance[0])-1,
    {
        //if(!packtaunt) return;
        //if(cust[VOI_CORTEX+player1_danse]<= 0) {conoutf(CON_GAMEINFO, "\f3Vous ne possédez pas cette voix !"); //playsound(S_ERROR); player1_danse=0; return;}
        //addmsg(N_SENDDANSE, "ri", player1_danse);
        //stopsounds();
        //player1->customdanse = player1_danse;
        //playsound(S_CGCORTEX+(player1_danse));
    });

    void taunt()
    {
        //if(!packtaunt) return;
        //if(player1->state!=CS_ALIVE) return;
        //if(lastmillis-player1->lasttaunt<2000){conoutf(CON_GAMEINFO, "\faOn abuse pas des bonnes choses !"); return;}
        //player1->lasttaunt = lastmillis;
        //playsound(S_CGCORTEX+(player1->customdanse), forcecampos>=0 ? &player1->o : NULL, NULL, 0, -1, -1, -1, 400);
        //addmsg(N_TAUNT, "rc", player1);
    }
    COMMAND(taunt, "");

    VARP(hitsound, 0, 0, 1);

    void damaged(int damage, gameent *d, gameent *actor, bool local, int atk)
    {
        if((d->state!=CS_ALIVE && d->state != CS_LAGGED && d->state != CS_SPAWNING) || intermission) return;

        if(local) damage = d->dodamage(damage, d->aptitude==C_PHYSICIST && d->abilitymillis[ABILITY_1]);
        else if(actor==player1) return;

        gameent *h = hudplayer();
        if(h!=player1 && actor==h && d!=actor)
        {
            if(hitsound && lasthit != lastmillis) playSound(S_HIT, vec(0, 0, 0), 0, 0, SND_UI);
            lasthit = lastmillis;
        }

        if(d==h)
        {
            damageblend(damage);
            damagecompass(damage, actor->o);
        }

        if(!rnd(2))
        {
            if(d->aptitude==C_PHYSICIST && d->abilitymillis[ABILITY_1] && d->armour>0) playSound(S_PHY_1, d==h ? vec(0, 0, 0) : d->o, 200, 100, SND_LOWPRIORITY);
            else if(d->armour>0 && actor->gunselect!=GUN_FLAMETHROWER) playSound(S_IMPACTWOOD+d->armourtype, d==h ? vec(0, 0, 0) : d->o, 250, 50, SND_LOWPRIORITY);
        }

        damageeffect(damage, d, actor, atk);

        ai::damaged(d, actor);

        if(d->health<=0) {if(local) killed(d, actor, atk);}
    }

    VARP(deathscore, 0, 1, 1);

    void deathstate(gameent *d, bool restore)
    {
        d->state = CS_DEAD;
        d->lastpain = lastmillis;
        d->skeletonfade = 1.0f;
        d->tombepop = 0.0f;
        d->deaths++;
        d->killstreak = 0;
        d->afterburnmillis = 0;
        d->boostmillis[B_SHROOMS] = 0;
        d->isOutOfMap = false;
        loopi(NUMABILITIES) d->abilitymillis[i] = 0;

        // death gfx effects
        vec pos(d->o.x, d->o.y, d->o.z-9);
        gibeffect(4000, d->vel, d);
        particle_splash(PART_SMOKE,  8, 1500, pos, 0x333333, 12.0f,  125, 400);
        particle_splash(PART_SMOKE,  5, 900, pos, 0x440000, 10.0f,  125, 200);

        stopLinkedSound(d->entityId, 0, true);

        if(d==player1)
        {
            if(deathscore) showscores(true);
            disablezoom();
            cleardamagescreen();
            d->attacking = ACT_IDLE;
            postfx::clearShroomsEffect();
            postfx::updateMainFilter(postfx::FILTER_DEATH);
            d->roll = 0;
            playSound(S_DIE_P1, vec(0, 0, 0), 0, 0, SND_FIXEDPITCH);
            if(m_tutorial) execute("deathReset");
        }
        else
        {
            d->move = d->strafe = 0;
            d->resetinterp();
            d->smoothmillis = 0;
            playSound(S_DIE, d->o, 300, 50);
        }
    }

    VARP(teamcolorfrags, 0, 1, 1);

    string killername;
    ICOMMAND(getkillername, "", (), result(killername); );

    int killerclass, killerlevel, killerweapon;
    ICOMMAND(getkillerweapon, "", (), intret(killerweapon); );
    ICOMMAND(getkillerclass, "", (), intret(killerclass); );
    ICOMMAND(getkillerlevel, "", (), intret(killerlevel); );

    float killerdistance;
    ICOMMAND(getkilldistance, "", (), floatret(roundf(killerdistance * 10) / 10); );

    bool hassuicided = true;
    ICOMMAND(hassuicided, "", (), intret(hassuicided); );

    void killed(gameent *d, gameent *actor, int atk)
    {
        bool isHudPlayer = (actor == hudplayer());
        d->lastkiller = actor;

        if(d->state==CS_EDITING)
        {
            d->editstate = CS_DEAD;
            d->deaths++;
            if(d!=player1) d->resetinterp();
            return;
        }
        else if((d->state!=CS_ALIVE && d->state != CS_LAGGED && d->state != CS_SPAWNING) || intermission) return;

        ////////////////////////////// sounds //////////////////////////////
        int farKillerDistance = (camera1->o.dist(actor->o) > 250);

        switch(actor->killstreak) // killstreak
        {
            case 3:
                playSound(S_KS_X3, isHudPlayer ? vec(0, 0, 0) : actor->o, 300, 100);
                if(farKillerDistance) playSound(S_KS_X3_FAR, actor->o, 1500, 200);
                break;
            case 5:
                playSound(S_KS_X5, isHudPlayer ? vec(0, 0, 0) : actor->o, 300, 100);
                if(farKillerDistance) playSound(S_KS_X5_FAR, actor->o, 1500, 200);
                break;
            case 7:
            case 10:
            case 15:
                playSound(S_KS_X7, isHudPlayer ? vec(0, 0, 0) : actor->o, 300, 100);
                if(farKillerDistance) playSound(S_KS_X7_FAR, actor->o, 1500, 200);
                break;
        }

        ////////////////////////////// gfx //////////////////////////////
        if(actor->aptitude==C_REAPER)
        {
            if(camera1->o.dist(d->o) >= 250) playSound(S_ECLAIRLOIN, d->o, 1000, 250);
            else playSound(S_ECLAIRPROCHE, d==hudplayer() ? vec(0, 0, 0) : d->o, 300, 150);
            adddynlight(vec(d->abovehead()).addz(30), 500, vec(0.0f, 0.5f, 1.5f), 80, 40, L_NODYNSHADOW|DL_FLASH);
            particle_flare(vec(d->o).add(vec(50 - rnd(101), 50 - rnd(101), 1500)), d->feetpos(), 175, PART_LIGHTNING, 0xFFFFFF, 30.0f);
            particle_splash(PART_SMOKE,  15, 2000, d->o, 0x333333, 40.0f, 150, 500);
            if(actor==player1) { playSound(S_FAUCHEUSE); player1->vampiremillis=1500; }
        }

        ////////////////////////////// game messages //////////////////////////////
        gameent *h = followingplayer(player1);
        int contype = d==h || actor==h ? CON_FRAG_SELF : CON_FRAG_OTHER;

        defformatstring(dname, "%s", m_teammode && teamcolorfrags ? teamcolorname(d, readstr("GameMessage_You")) : colorname(d, NULL, readstr("GameMessage_You"), "\fc"));
        defformatstring(aname, "%s", m_teammode && teamcolorfrags ? teamcolorname(actor, readstr("GameMessage_You")) : colorname(actor, NULL, readstr("GameMessage_You"), "\fc"));

        if(d==actor) // suicide ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        {
            if(d==player1)
            {
                hassuicided = true;
                updateStat(1, STAT_MORTS); updateStat(1, STAT_SUICIDES);
                if(atk==ATK_M32) unlockAchievement(ACH_M32SUICIDE);
                conoutf(contype, "\fd%s %s", readstr("GameMessage_You"), readstr("Console_Game_Suicided"));
            }
            else conoutf(contype, "%s > \fl%s", dname, readstr("GameMessage_Suicide"));
        }
        else // Kill ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        {
            float killdistance = actor->o.dist(d->o)/18.f;
            if(actor==player1) //////////////////// you killed someone ////////////////////
            {
                playSound(S_KILL, vec(0, 0, 0), 0, 0, SND_FIXEDPITCH|SND_UI);
                defformatstring(victimname, "%s", dname);
                conoutf(CON_HUDCONSOLE, "%s \fc%s%s\f7! \f4(%.1fm)", readstr("GameMessage_YouKilled"), victimname, readstr("Misc_PrePuncSpace"), killdistance);
                conoutf(contype, "\fd%s\f7 > \fl%s\f7 > %s \fl(%.1fm)", player1->name, readstr(guns[atk].ident), victimname, killdistance);

                if(IS_ON_OFFICIAL_SERV && isteam(d->team, player1->team)) unlockAchievement(ACH_CPASBIEN);
                else if(IS_ON_OFFICIAL_SERV && !isteam(d->team, player1->team)) // now let's check for shittons of achievements if playing online
                {
                    if(killdistance>=100.f) unlockAchievement(ACH_BEAUTIR);
                    else if(killdistance<1.f && atk==ATK_MOSSBERG) unlockAchievement(ACH_TAKETHAT);
                    else if(killdistance>=69.f && killdistance<70.f ) unlockAchievement(ACH_NICE);
                    if(player1->state==CS_DEAD && player1->lastpain > 200) unlockAchievement(ACH_TUEURFANTOME);
                    if(player1->health<=10 && player1->state==CS_ALIVE) unlockAchievement(ACH_1HPKILL);

                    switch(atk)
                    {
                        case ATK_POWERARMOR: if(player1->armourtype==A_POWERARMOR) unlockAchievement(ACH_KILLASSIST); break;
                        case ATK_FLAMETHROWER: if(lookupmaterial(d->feetpos())==MAT_WATER) unlockAchievement(ACH_THUGPHYSIQUE); break;
                        case ATK_SV98: if(zoom==0) unlockAchievement(ACH_NOSCOPE); break;
                        case ATK_GLOCK: if(d->gunselect==GUN_S_NUKE || d->gunselect==GUN_S_CAMPER || d->gunselect==GUN_S_GAU8 || d->gunselect==GUN_S_ROCKETS) unlockAchievement(ACH_DAVIDGOLIATH); break;
                        case ATK_M_BUSTER: case ATK_M_FLAIL: case ATK_M_HAMMER: case ATK_M_MASTER: if(d->aptitude==C_NINJA) unlockAchievement(ACH_PASLOGIQUE); break;
                        case ATK_S_NUKE: case ATK_S_CAMPER: case ATK_S_GAU8: case ATK_S_ROCKETS: if(player1->aptitude==C_AMERICAN && player1->boostmillis[B_ROIDS]) unlockAchievement(ACH_JUSTEPOUR);
                    }

                    switch(player1->aptitude)
                    {
                        case C_AMERICAN: if(d->aptitude==C_SHOSHONE) unlockAchievement(ACH_FUCKYEAH); break;
                        case C_SPY: if(player1->abilitymillis[ABILITY_2]) unlockAchievement(ACH_ESPIONDEGUISE);
                    }

                    switch(player1->killstreak) {case 3: unlockAchievement(ACH_TRIPLETTE); break; case 5: unlockAchievement(ACH_PENTAPLETTE); break; case 10: unlockAchievement(ACH_DECAPLETTE);}

                    //now let's add player stats
                    updateStat(1, STAT_KILLS);
                    addReward(7+player1->killstreak-1, 3+player1->killstreak-1);
                    if(player1->killstreak > gameStat[STAT_KILLSTREAK]) updateStat(player1->killstreak, STAT_KILLSTREAK, true);
                    if(gameStat[STAT_MAXKILLDIST]<killdistance) updateStat(killdistance, STAT_MAXKILLDIST, true);
                }
            }
            else if(d==player1) //////////////////// you were killed ////////////////////
            {
                hassuicided = false;
                conoutf(contype, "%s\f7 > \fl%s\f7 > \fd%s \fl(%.1fm)", aname, readstr(guns[atk].ident), player1->name, killdistance);
                updateStat(1, STAT_MORTS);
                formatstring(killername, "%s", aname);
                killerweapon = atk;
                killerclass = actor->aptitude;
                killerlevel = actor->level;
                killerdistance = killdistance;
            }
            else //////////////////// someone killed someone ////////////////////
            {
                conoutf(contype, "%s\f7 > \fl%s\f7 > %s \fl(%.1fm)", aname, readstr(guns[atk].ident), dname, killdistance);
            }

            //////////////////// someone is killin' it ////////////////////
            defformatstring(verb, "%s", (actor==player1 ? readstr("GameMessage_Are") : readstr("GameMessage_Is") ));
            defformatstring(name, "%s", actor==player1 ? readstr("GameMessage_You")  : aname);

            if(actor->killstreak == 3 || actor->killstreak == 5 || actor->killstreak == 7 || actor->killstreak == 10 || actor->killstreak == 15)
            {
                defformatstring(KillStreakIdent, "GameMessage_Kill%s", actor->killstreak == 3 ? "X3" :
                                                                       actor->killstreak == 5 ? "X5" :
                                                                       actor->killstreak == 7 ? "X7" :
                                                                       actor->killstreak == 10 ? "X10" : "X15");

                conoutf(CON_HUDCONSOLE, "%s\f7%s %s", name, verb, readstr(KillStreakIdent));
            }
        }
        deathstate(d);
        ai::killed(d, actor);
    }

    vector<gameent *> bestplayers;
    vector<int> bestteams;

    VAR(hasnewdamagerecord, 0, 0, 1);
    VAR(isthebestplayer, 0, 0, 1);
    VAR(hasthebestteam, 0, 0, 1);

    void timeupdate(int secs)
    {
        server::timeupdate(secs);
        if(secs > 0)
        {
            maplimit = lastmillis + secs*1000;
        }
        else
        {
            intermission = true;
            player1->attacking = ACT_IDLE;
            if(cmode) cmode->gameover();
            int accuracy = (player1->totaldamage*100)/max(player1->totalshots, 1);
            if(m_sp) spsummary(accuracy);

            conoutf(CON_GAMEINFO, "\fa%s", readstr("Announcement_GameOver"));

            if(player1->frags>=10)
            {
                if(accuracy>=50) unlockAchievement(ACH_PRECIS);
                if(player1->deaths<=5) unlockAchievement(ACH_INCREVABLE);
                if(player1->frags>=30) unlockAchievement(ACH_KILLER);
            }

            if(player1->totaldamage/10 > 10000) unlockAchievement(ACH_DESTRUCTEUR);

            if(gameStat[STAT_DAMMAGERECORD] < player1->totaldamage/10)
            {
                updateStat(player1->totaldamage/10, STAT_DAMMAGERECORD, true);
                addReward(30, 15);
                hasnewdamagerecord = true;
            }
            else hasnewdamagerecord = false;

            if(player1->deaths <= player1->frags) addReward(40, 20);
            if(bestplayers.find(player1) >= 0) addReward(100, 50);

            bestteams.shrink(0);
            bestplayers.shrink(0);

            if(m_teammode)
            {
                getbestteams(bestteams);
                if(bestteams.htfind(player1->team) >= 0)
                {
                    hasthebestteam = true;
                    addReward(50, 25);
                }
                else hasthebestteam = false;
            }
            else hasthebestteam = false;

            getbestplayers(bestplayers);
            if(bestplayers.find(player1) >= 0)
            {
                isthebestplayer = true;
                addReward(100, 50);
            }
            else isthebestplayer = false;

            if(isthebestplayer || hasthebestteam)
            {
                unlockAchievement(ACH_WINNER);
                updateStat(1, STAT_WINS);
            }

            showscores(true);
            disablezoom();

            execident("intermission");
        }
    }

    ICOMMAND(getfrags, "", (), intret(player1->frags));
    ICOMMAND(getflags, "", (), intret(player1->flags));
    ICOMMAND(getdeaths, "", (), intret(player1->deaths));
    ICOMMAND(getaccuracy, "", (), intret((player1->totaldamage*100)/max(player1->totalshots, 1)));
    ICOMMAND(gettotaldamage, "", (), intret(player1->totaldamage/10));
    ICOMMAND(gettotalshots, "", (), intret(player1->totalshots));

    vector<gameent *> clients;

    gameent *newclient(int cn)   // ensure valid entity
    {
        if(cn < 0 || cn > max(0xFF, MAXCLIENTS + MAXBOTS))
        {
            neterr("clientnum", false);
            return NULL;
        }

        if(cn == player1->clientnum) return player1;

        while(cn >= clients.length()) clients.add(NULL);
        if(!clients[cn])
        {
            gameent *d = new gameent;
            d->clientnum = cn;
            clients[cn] = d;
            players.add(d);
        }
        return clients[cn];
    }

    gameent *getclient(int cn)   // ensure valid entity
    {
        if(cn == player1->clientnum) return player1;
        return clients.inrange(cn) ? clients[cn] : NULL;
    }

    void clientdisconnected(int cn, bool notify)
    {
        if(!clients.inrange(cn)) return;
        unignore(cn);
        gameent *d = clients[cn];
        if(d)
        {
            if(notify && d->name[0]) conoutf("\f7%s\f4 %s", colorname(d), readstr("GameMessage_Left"));
            d->isConnected = false;
            removeweapons(d);
            removetrackedparticles(d);
            removetrackeddynlights(d);
            stopLinkedSound(d->entityId, 0, true);
            removeEntityPos(d->entityId);
            if(cmode) cmode->removeplayer(d);
            removegroupedplayer(d);
            players.removeobj(d);
            DELETEP(clients[cn]);
            cleardynentcache();
        }
        if(following == cn)
        {
            if(specmode) nextfollow();
            else stopfollowing();
        }
    }

    void clearclients(bool notify)
    {
        loopv(clients) if(clients[i]) clientdisconnected(i, notify);
    }

    void initclient()
    {
        player1 = spawnstate(new gameent);
        filtertext(player1->name, readstr("Misc_BadUsername"), false, false, MAXNAMELEN);
        players.add(player1);
        player1->aptitude = player1_aptitude;
    }

    VARP(showmodeinfo, 0, 0, 1);

    void startgame()
    {
        clearmonsters();
        clearprojectiles();
        clearbouncers();
        clearEntsPos();
        clearGraves();
        clearteaminfo();

        // reset perma-state
        loopv(players) players[i]->startgame();

        setclientmode();
        intermission = false;

        maptime = maprealtime = 0;
        maplimit = -1;

        if(cmode)
        {
            cmode->preload();
            cmode->setup();
        }

        syncplayer();

        showscores(false);
        disablezoom();
        lasthit = 0;

        execident("mapstart");
    }

    void startmap(const char *name)   // called just after a map load
    {
        ai::savewaypoints();
        ai::clearwaypoints(true);

        respawnent = -1; // so we don't respawn at an old spot
        if(!m_mp(gamemode)) spawnplayer(player1);
        else findplayerspawn(player1, -1, m_teammode && !m_capture ? player1->team : 0);
        entities::resetspawns();
        copystring(clientmap, name ? name : "");

        sendmapinfo();
    }

    const char *getmapinfo()
    {
        return showmodeinfo && m_valid(gamemode) ? readstr(gamemodes[gamemode - STARTGAMEMODE].modeId) : NULL;
    }

    void physicstrigger(physent *d, bool local, int floorlevel, int waterlevel, int material)
    {
        bool isHudplayer = d==hudplayer();
        vec o = (isHudplayer && !isthirdperson()) ? d->feetpos() : d->o;
        gameent *pl = (gameent *)d;

        if(waterlevel>0)
        {
            if(material&MAT_WATER)
            {
                playSound(S_WATER, isHudplayer ? vec(0, 0, 0) : d->o, 200, 30);
                particle_splash(PART_WATER, 30, 120, o, 0x18181A, 10.0f+rnd(9), 500, -20);
                addmsg(N_WATERTOUCH, "rci", pl, false);
            }
        }
        else if(waterlevel<0)
        {
            if(material&MAT_WATER)
            {
                playSound(S_SPLASH, isHudplayer ? vec(0, 0, 0) : d->o, 300, 50);
                particle_splash(PART_WATER, 40, 150, o, 0x18181A, 10.f + rnd(12), 600, 30);
                addmsg(N_WATERTOUCH, "rci", pl, true);
                if(pl->afterburnmillis)
                {
                    playSound(S_FIRE_EXT, isHudplayer ? vec(0,0,0) : d->o, 200, 100, NULL, pl->entityId);
                    pl->afterburnmillis = 0;
                }
            }
            else if(material&MAT_LAVA)
            {
                playSound(S_LAVASPLASH, isHudplayer ? vec(0, 0, 0) : d->o, 300, 50);
                particle_splash(PART_SMOKE, 25, 100, o, 0x222222, (10.f + rnd(5)), 400, 20);
                particle_splash(PART_FIRE_BALL, 7, 120, o, 0xCC7744, (10.f + rnd(5)), 400, 300);
                loopi(2 + rnd(3)) particles::dirSplash(PART_FIRESPARK, 0xFFBB55, 750, 7, 300 + (rnd(500)), d->feetpos(), vec(0, 0, 1), 3.f+(rnd(30)/6.f), ((i + 1) * 125) + rnd(200), -1);
                loopi(4) particles::dirSplash(PART_SMOKE, 0x333333, 200, 4, 1500 + rnd(750), d->feetpos(), vec(0, 0, 1), 15.f + rnd(5), 50 + rnd(50), 5);
            }
        }
        if(floorlevel>0)
        {
            particle_splash(map_atmo==4 && atmos ? PART_WATER : PART_SMOKE, pl->armourtype==A_POWERARMOR ? 12 : 10, 100, d->feetpos(), map_atmo==4 && atmos ? 0x111111 : map_atmo==9 ? 0xFFFFFF : 0x666666, 7.0f+rnd(pl->armourtype==A_POWERARMOR ? 10 : 5), 400, 20);
            if(d==hudplayer()) startCameraAnimation(CAM_ANIM_JUMP, 600, vec(0, 0, 0), vec(0, 0, 0), vec(0, 0.2f, 0));
            if(d==player1 || d->type!=ENT_PLAYER || ((gameent *)d)->ai) msgsound(pl->armourtype==A_POWERARMOR && pl->armour ? S_JUMP_ASSIST : pl->aptitude==C_NINJA || (pl->aptitude==C_KAMIKAZE && pl->abilitymillis[ABILITY_2]) ? S_JUMP_NINJA : S_JUMP_BASIC, d);
        }
        else if(floorlevel<0)
        {
            particle_splash(map_atmo==4 && atmos ? PART_WATER : PART_SMOKE, pl->armourtype==A_POWERARMOR ? 20 : 15, 120, d->feetpos(), map_atmo==4 && atmos ? 0x131313 : map_atmo==9 ? 0xFFFFFF : 0x442211, 7.0f+rnd(pl->armourtype==A_POWERARMOR ? 10 : 5), 400, 20);
            if(d==hudplayer()) startCameraAnimation(CAM_ANIM_LAND, 400, vec(0, 0, -3), vec(0, 0, 0), vec(0, -0.4f, 0));
            if(d==player1 || d->type!=ENT_PLAYER || ((gameent *)d)->ai) msgsound(pl->armourtype==A_POWERARMOR && pl->armour ? S_LAND_ASSIST : S_LAND_BASIC, d);
        }
    }

    void footsteps(physent *d)
    {
        if(d->blocked) return;
        bool moving = d->move || d->strafe;
        bool isHudPlayer = d==hudplayer();
        gameent *pl = (gameent *)d;
        if(d->physstate>=PHYS_SLOPE && moving)
        {
            bool powerArmor = hasPowerArmor(pl);
            int snd = lookupmaterial(d->feetpos()) == MAT_WATER ? S_SWIM : (powerArmor ? S_FOOTSTEP_ASSIST : S_FOOTSTEP);

            int freq = (((100 - classes[pl->aptitude].speed) + 75) * 5) ;
            if(pl->crouched() || (pl->abilitymillis[ABILITY_2] && pl->aptitude==C_SPY)) freq *= 2;
            if(d->inwater) freq *= 1.5f;
            else if(powerArmor) freq *= 2;
            if(pl->boostmillis[B_EPO]) freq /= 2.f;

            if(lastmillis-pl->lastfootstep < freq) return;
            else
            {
                if(pl==hudplayer())
                {
                    static bool cameraSwayDir = false;
                    float swayValue = 0.12f * (classes[pl->aptitude].speed / 100.f);
                    if(pl->boostmillis[B_EPO]) swayValue *= 2.f;
                    startCameraAnimation(CAM_ANIM_SWAY, freq, vec(0, 0, 0), vec(0, 0, 0), vec(0, 0, (cameraSwayDir ? swayValue : -swayValue)));
                    cameraSwayDir = !cameraSwayDir;
                }
                playSound(snd, isHudPlayer ? vec(0, 0, 0) : d->o, powerArmor ? 300 : 150, 20, powerArmor ? NULL : SND_LOWPRIORITY, pl->entityId);

                if(pl->boostmillis[B_EPO] && !rnd(15))
                {
                    playSound(S_EPO_RUN, isHudPlayer ? vec(0, 0, 0) : d->o, 500, 100, NULL, pl->entityId);
                    if(camera1->o.dist(d->o) >= 400 && d!=player1) playSound(S_EPO_RUN_FAR, d->o, 1000, 500, NULL, pl->entityId);
                }
            }
        }
        pl->lastfootstep = lastmillis;
    }

    void dynentcollide(physent *d, physent *o, const vec &dir)
    {
        switch(d->type)
        {
            case ENT_AI: if(dir.z > 0) stackmonster((monster *)d, o); break;
        }
    }

    void msgsound(int n, physent *d)
    {
        if(!d || d==player1)
        {
            addmsg(N_SOUND, "ci", d, n);
            playSound(n);
        }
        else
        {
            if(d->type==ENT_PLAYER && ((gameent *)d)->ai) addmsg(N_SOUND, "ci", d, n);
            playSound(n, d->o, 150, 20);
        }
    }

    int numdynents() { return players.length()+monsters.length(); }

    dynent *iterdynents(int i)
    {
        if(i<players.length()) return players[i];
        i -= players.length();
        if(i<monsters.length()) return (dynent *)monsters[i];
        i -= monsters.length();
        return NULL;
    }

    bool duplicatename(gameent *d, const char *name = NULL, const char *alt = NULL)
    {
        if(!name) name = d->name;
        if(alt && d != player1 && !strcmp(name, alt)) return true;
        loopv(players) if(d!=players[i] && !strcmp(name, players[i]->name)) return true;
        return false;
    }

    const char *colorname(gameent *d, const char *name, const char * alt, const char *color)
    {
        if(!name) name = alt && d == player1 ? alt : d->name;
        bool dup = !name[0] || duplicatename(d, name, alt) || d->aitype != AI_NONE;
        if(dup || color[0])
        {
            if(dup)
            {
                if(d->aitype == AI_NONE) return tempformatstring("\fs%s%s \f5(%d)\fr", color, name, d->clientnum);
                else return tempformatstring("\fs%s[%s]%s\fr", color, readstr("Misc_Bot"), name);
            }
            return tempformatstring("\fs%s%s\fr", color, name);
        }
        return name;
    }

    const char *teamcolorname(gameent *d, const char *alt)
    {
        if(d->state == CS_SPECTATOR) return colorname(d, NULL, alt, "\f1"); // spectator (blue)
        if((isteam(hudplayer()->team, d->team) && m_teammode) || d==hudplayer()) return colorname(d, NULL, alt, "\fd"); // team mate (yellow)
        return colorname(d, NULL, alt, "\fc"); // enemies (red)
    }

    const char *teamcolor(int team)
    {
        return tempformatstring("\fs%s%s\fr", team!=player1->team ? teamtextcode[2] : teamtextcode[1], readstr("Team_Names", team));
    }

    void suicide(physent *d)
    {
        if(d==player1 || (d->type==ENT_PLAYER && ((gameent *)d)->ai))
        {
            if(d->state!=CS_ALIVE) return;
            gameent *pl = (gameent *)d;
            if(!m_mp(gamemode)) killed(pl, pl, 0);
            else
            {
                int seq = (pl->lifesequence<<16)|((lastmillis/1000)&0xFFFF);
                if(pl->suicided!=seq) { addmsg(N_SUICIDE, "rc", pl); pl->suicided = seq; }
            }
        }
        else if(d->type==ENT_AI) suicidemonster((monster *)d);
    }
    ICOMMAND(suicide, "", (), suicide(player1));

    bool needminimap() { return m_ctf || m_capture || m_tutorial || m_dmsp; }

    float clipconsole(float w, float h)
    {
        if(cmode) return cmode->clipconsole(w, h);
        return 0;
    }

    VARP(teamcrosshair, 0, 1, 1);
    VARP(hitcrosshair, 0, 200, 1000);

    const char *defaultcrosshair(int index)
    {
        switch(index)
        {
            case 2: return "media/interface/crosshair/default_hit.png";
            case 1: return "media/interface/crosshair/teammate.png";
            default: return "media/interface/crosshair/default.png";
        }
    }

    int selectcrosshair(vec &col)
    {
        gameent *d = hudplayer();

        if(d->state==CS_SPECTATOR || d->state==CS_DEAD || UI::uivisible("scoreboard")) return -1;

        if(d->state!=CS_ALIVE) return 0;

        int crosshair = 0;
        if(lasthit && lastmillis - lasthit < hitcrosshair) crosshair = 2;
        else if(teamcrosshair && m_teammode)
        {
            dynent *o = intersectclosest(d->o, worldpos, d);
            if(o && o->type==ENT_PLAYER && validteam(d->team) && ((gameent *)o)->team == d->team)
            {
                crosshair = 1;
                col = vec::hexcolor(teamtextcolor[player1->team!=d->team ? 2 : 1]);
            }
        }

        return crosshair;
    }

    const char *mastermodecolor(int n, const char *unknown)
    {
        return (n>=MM_START && size_t(n-MM_START)<sizeof(mastermodecolors)/sizeof(mastermodecolors[0])) ? mastermodecolors[n-MM_START] : unknown;
    }

    const char *mastermodeicon(int n, const char *unknown)
    {
        return (n>=MM_START && size_t(n-MM_START)<sizeof(mastermodeicons)/sizeof(mastermodeicons[0])) ? mastermodeicons[n-MM_START] : unknown;
    }

    ICOMMAND(servinfomode, "i", (int *i), GETSERVINFOATTR(*i, 0, mode, intret(mode)));
    ICOMMAND(servinfomodename, "i", (int *i),
        GETSERVINFOATTR(*i, 0, mode,
        {
            const char *name = server::modename(mode, NULL);
            if(name) result(name);
        }));
    ICOMMAND(servinfomastermode, "i", (int *i), GETSERVINFOATTR(*i, 2, mm, intret(mm)));
    ICOMMAND(servinfomastermodename, "i", (int *i),
        GETSERVINFOATTR(*i, 2, mm,
        {
            const char *name = server::mastermodename(mm, NULL);
            if(name) stringret(newconcatstring(mastermodecolor(mm, ""), name));
        }));
    ICOMMAND(servinfotime, "ii", (int *i, int *raw),
        GETSERVINFOATTR(*i, 1, secs,
        {
            secs = clamp(secs, 0, 59*60+59);
            if(*raw) intret(secs);
            else
            {
                int mins = secs/60;
                secs %= 60;
                result(tempformatstring("%d:%02d", mins, secs));
            }
        }));
    ICOMMAND(servinfoicon, "i", (int *i),
        GETSERVINFO(*i, si,
        {
            int mm = si->attr.inrange(2) ? si->attr[2] : MM_INVALID;
            result(si->maxplayers > 0 && si->numplayers >= si->maxplayers ? "serverfull" : mastermodeicon(mm, "serverunk"));
        }));

    // any data written into this vector will get saved with the map data. Must take care to do own versioning, and endianess if applicable. Will not get called when loading maps from other games, so provide defaults.
    void writegamedata(vector<char> &extras) {}
    void readgamedata(vector<char> &extras) {}

    const char *gameconfig() { return "config/game.cfg"; }
    const char *savedconfig() { return "config/settings.cfg"; }
    const char *autoexec() { return "config/autoexec.cfg"; }
    const char *savedservers() { return "config/servers.cfg"; }

    void loadconfigs()
    {
        execfile("config/auth.cfg", false);
    }

    bool clientoption(const char *arg) { return false; }
}

