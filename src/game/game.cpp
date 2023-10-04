#include "gfx.h"
#include "customs.h"
#include "stats.h"

bool randomevent(int probability)
{
    if(game::ispaused()) return false;
    return probability <= 1 ? true : rnd(probability)==0;
}

namespace game
{
    int oldapti;
    VARFP(player1_aptitude, 0, 0, NUMAPTS-1,
    {
        if(player1->state != CS_DEAD && isconnected() && !premission && !intermission && !m_tutorial)
        {
            conoutf(CON_GAMEINFO, GAME_LANG ? "\fcCannot change class while alive!" : "\fcImpossible de changer d'aptitude en étant vivant !");
            playSound(S_ERROR);
            player1_aptitude = oldapti;
        }
        else
        {
            addmsg(N_SENDAPTITUDE, "ri", player1_aptitude);
            player1->aptitude = player1_aptitude;
            oldapti = player1->aptitude;
            if(!islaunching) playSound(S_APT_SOLDAT+player1_aptitude, NULL, 0, 0, SND_FIXEDPITCH|SND_NOTIFICATION);
            if(isconnected() && !premission && !intermission) unlockAchievement(ACH_UNDECIDED);
        }
    });

    bool intermission = false, premission = false;
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
        loopi(4) dmg+=avgdmg[i];
        if(totalmillis >= lasttimeupdate+1000) //1 second interval
        {
            updateStat(1, STAT_TIMEPLAYED);
            lasttimeupdate = totalmillis;
            dmgsecs[0]==3 ? dmgsecs[0]=0 : dmgsecs[0]++;
            dmgsecs[1]==5 ? dmgsecs[1]=2 : dmgsecs[1]++;
            avgdmg[dmgsecs[1]-2] = 0;
        }
    }

    ICOMMAND(getdps, "", (), intret(dmg/4.f));

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
        gfx::resetpostfx();
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
            moveplayer(d, 1, false, d->boostmillis[B_EPO], d->boostmillis[B_JOINT], d->aptitude, d->aptitude==APT_MAGICIEN ? d->abilitymillis[ABILITY_1] : d->aptitude==APT_SHOSHONE || d->aptitude==APT_ESPION || d->aptitude==APT_KAMIKAZE ? d->abilitymillis[ABILITY_2] : d->abilitymillis[ABILITY_3], d->armourtype==A_ASSIST && d->armour>0 ? true : false);
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
            case APT_KAMIKAZE:
                if(player1->gunselect!=GUN_KAMIKAZE) gunselect(cncurweapon, player1);
                break;
            case APT_NINJA:
                if(player1->gunselect!=GUN_CACNINJA) gunselect(cncurweapon, player1);
                break;
            default: gunselect(cncurweapon, player1);
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

    bool isattacking(gameent *d)
    {
        return d->lastaction && d->lastattack >= 0 && attacks[d->lastattack].gun == d->gunselect && lastmillis-d->lastaction<attacks[d->lastattack].attackdelay + 50;
    }

    bool uwSoundPlaying = false;

    void updatePlayersSounds(gameent *d)
    {
        if(lookupmaterial(camera1->o)==MAT_WATER && !uwSoundPlaying) // underwater camera sound
        {
            playSound(S_UNDERWATER, NULL, 0, 0, SND_FIXEDPITCH|SND_LOOPED, hudplayer()->entityId, PL_UNDERWATER_SND);
            uwSoundPlaying = true;
        }
        if(lookupmaterial(camera1->o)!=MAT_WATER) { stopLinkedSound(hudplayer()->entityId, PL_UNDERWATER_SND); uwSoundPlaying = false; }

        vec playerVel = d->vel;

        if(isattacking(d) && d!=hudplayer()) updateSoundPosition(d->entityId, d->muzzle, playerVel.div(vec(75, 75, 75)), PL_ATTACK_SND); // looped weapon sounds
        if(!isattacking(d)) { stopLinkedSound(d->entityId, PL_ATTACK_SND); d->attacksound = false; }

        if(d->armourtype==A_ASSIST && d->armour && d->armour<1000 && d->state==CS_ALIVE) // power armor alarm sound
        {
            if(!d->powerarmoursound && d->armour)
            {
                playSound(S_ASSISTALARM, d==hudplayer() ? NULL : &d->o, 250, 100, SND_FIXEDPITCH|SND_LOOPED, d->entityId, PL_POWERARMOR_SND);
                d->powerarmoursound = true;
            }
            else if(d!=hudplayer()) updateSoundPosition(d->entityId, d->o, playerVel.div(vec(75, 75, 75)), PL_POWERARMOR_SND);
        }
        else if (d->powerarmoursound) { stopLinkedSound(d->entityId, PL_POWERARMOR_SND); d->powerarmoursound = false; }
    }

    void otherplayers(int curtime)
    {
        loopv(players)
        {
            gameent *d = players[i];

            if(isconnected()) updatePlayersSounds(d);

            if(d!=player1 && d->state==CS_ALIVE && !intermission && !premission)
            {
                if(d->armourtype==A_ASSIST && d->ammo[GUN_ASSISTXPL] && !d->armour) {gunselect(GUN_ASSISTXPL, d, true); d->gunwait=0;}
                if(lastmillis - d->lastaction >= d->gunwait) d->gunwait = 0;
                if(hasboost(d)) entities::checkboosts(curtime, d);
                updateAbilitiesSkills(curtime, d);
            }

            if(totalmillis - d->lastcurdamage > 500) d->curdamage = 0;

            if(d == player1 || d->ai) continue;
            if(d->state==CS_DEAD && d->ragdoll) moveragdoll(d);

            const int lagtime = totalmillis-d->lastupdate;
            if(!lagtime || intermission || premission) continue;
            else if(lagtime>1000 && d->state==CS_ALIVE)
            {
                d->state = CS_LAGGED;
                continue;
            }
            if(d->state==CS_ALIVE || d->state==CS_EDITING)
            {
                crouchplayer(d, 10, false);
                if(smoothmove && d->smoothmillis>0) predictplayer(d, true);
                else moveplayer(d, 1, false, d->boostmillis[B_EPO], d->boostmillis[B_JOINT], d->aptitude, d->aptitude==APT_MAGICIEN ? d->abilitymillis[ABILITY_1] : d->aptitude==APT_SHOSHONE || d->aptitude==APT_ESPION || d->aptitude==APT_KAMIKAZE ? d->abilitymillis[ABILITY_2] : d->abilitymillis[ABILITY_3], d->armourtype==A_ASSIST && d->armour>0 ? true : false);
            }
            else if(d->state==CS_DEAD && !d->ragdoll && lastmillis-d->lastpain<2000) moveplayer(d, 1, true, d->boostmillis[B_EPO], d->boostmillis[B_JOINT], d->aptitude, d->aptitude==APT_MAGICIEN ? d->abilitymillis[ABILITY_1] : d->aptitude==APT_SHOSHONE || d->aptitude==APT_ESPION  || d->aptitude==APT_KAMIKAZE ? d->abilitymillis[ABILITY_2] : d->abilitymillis[ABILITY_3], false);
        }
    }

    VAR(canmove, 0, 1, 1);

    void updateworld()        // main game update loop
    {
        int delta = 250 / max(gfx::nbfps, 1);
        int horizontaltrans = gfx::zoom ? 2 : -2;
        int verticaltrans = gfx::zoom ? -1 : 1;

        gfx::weapposside = clamp(gfx::weapposside + horizontaltrans * delta, 1, guns[player1->gunselect].maxweapposside);
        gfx::weapposup = clamp(gfx::weapposup + verticaltrans * delta, 1, guns[player1->gunselect].maxweapposup);

        if(!maptime) { maptime = lastmillis; maprealtime = totalmillis; return; }
        if(!curtime) { gets2c(); if(player1->clientnum>=0) c2sinfo(); return; }

        physicsframe();
        ai::navigate();

        if(player1->state==CS_ALIVE && !intermission && !premission)   // checking player1's shits
        {
            if(player1->armourtype==A_ASSIST && player1->ammo[GUN_ASSISTXPL] && !player1->armour) {gunselect(GUN_ASSISTXPL, player1, true); player1->gunwait=0;}
            else if(m_identique)
            {
                switch(player1->gunselect)
                {
                    case GUN_S_NUKE: case GUN_S_GAU8: case GUN_S_CAMPOUZE: case GUN_S_ROQUETTES: break;
                    default: getaptiweap();
                }
            }

            if(IS_ON_OFFICIAL_SERV) // checking for achievements
            {
                bool p1hassuperweapon = false;
                loopi(4) if(player1->ammo[GUN_S_NUKE+i]>0) p1hassuperweapon = true;
                if(player1->health>=2000) unlockAchievement(ACH_SACAPV);
                if(player1->boostmillis[B_ROIDS] && player1->boostmillis[B_EPO] && player1->boostmillis[B_JOINT] && player1->boostmillis[B_SHROOMS]) unlockAchievement(ACH_DEFONCE);
                if(lookupmaterial(player1->o)==MAT_NOCLIP && !strcasecmp(maptitle_en, "Moon")) unlockAchievement(ACH_SPAAACE);
                if(p1hassuperweapon && player1->boostmillis[B_ROIDS] && player1->armour>0 && player1->armourtype==A_ASSIST) unlockAchievement(ACH_ABUS);
                if(player1->aptitude==APT_KAMIKAZE && player1->ammo[GUN_KAMIKAZE]<=0 && totalmillis-lastshoot>=500 && totalmillis-lastshoot<=750 && isconnected()) unlockAchievement(ACH_SUICIDEFAIL);
                if(player1->boostmillis[B_EPO] && player1->aptitude==APT_JUNKIE) unlockAchievement(ACH_LANCEEPO);
            }

            if(hasboost(player1)) entities::checkboosts(curtime, player1);
            updateAbilitiesSkills(curtime, player1);
        }

        updateweapons(curtime);
        otherplayers(curtime);
        ai::update();
        moveragdolls();
        gets2c();
        updatemonsters(curtime);
        if(connected)
        {
            if(player1->state == CS_DEAD)
            {
                if(player1->ragdoll) moveragdoll(player1);
                else
                {
                    player1->move = player1->strafe = 0;
                    moveplayer(player1, 10, true, 0, 0, player1->aptitude, 0, false);
                }
            }
            else if(!intermission && !premission && gfx::forcecampos<0)
            {
                if(player1->ragdoll) cleanragdoll(player1);
                crouchplayer(player1, 10, true);
                if(canmove || !m_tutorial)
                {
                    moveplayer(player1, 10, true, player1->boostmillis[B_EPO], player1->boostmillis[B_JOINT], player1->aptitude, player1->aptitude==APT_MAGICIEN ? player1->abilitymillis[ABILITY_1] : player1->aptitude==APT_SHOSHONE || player1->aptitude==APT_ESPION || player1->aptitude==APT_KAMIKAZE ? player1->abilitymillis[ABILITY_2] : player1->abilitymillis[ABILITY_3], player1->armourtype==A_ASSIST && player1->armour>0 ? true : false);
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
        if(!connected || intermission || premission || gfx::forcecampos>=0) return;
        if((player1->attacking = act) && attackspawn) respawn();
    }

    ICOMMAND(shoot, "D", (int *down), doaction(*down ? ACT_SHOOT : ACT_IDLE));

    VARP(jumpspawn, 0, 1, 1);

    bool canjump()
    {
        if(!connected || intermission || premission || gfx::forcecampos>=0) return false;
        if(jumpspawn) respawn();
        return player1->state!=CS_DEAD;
    }

    bool cancrouch()
    {
        if(!connected || intermission || premission || gfx::forcecampos>=0) return false;
        return player1->state!=CS_DEAD;
    }

    VARFP(player1_taunt, 0, 0, sizeof(customsdance)/sizeof(customsdance[0])-1,
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
        //playsound(S_CGCORTEX+(player1->customdanse), gfx::forcecampos>=0 ? &player1->o : NULL, NULL, 0, -1, -1, -1, 400);
        //addmsg(N_TAUNT, "rc", player1);
    }
    COMMAND(taunt, "");

    //void showvoice()
    //{
    //    if(GAME_LANG) return;
    //    playsound(S_CGCORTEX+UI_voix);
    //}
    //COMMAND(showvoice, "");

    VARP(hitsound, 0, 0, 1);

    void damaged(int damage, gameent *d, gameent *actor, bool local, int atk)
    {
        if((d->state!=CS_ALIVE && d->state != CS_LAGGED && d->state != CS_SPAWNING) || intermission || premission) return;

        if(local) damage = d->dodamage(damage, d->aptitude, d->abilitymillis[ABILITY_1]);
        else if(actor==player1) return;

        gameent *h = hudplayer();
        if(h!=player1 && actor==h && d!=actor)
        {
            if(hitsound && lasthit != lastmillis) playSound(S_HIT, NULL, 0, 0, SND_NOTIFICATION);
            lasthit = lastmillis;
        }

        if(d==h)
        {
            damageblend(damage);
            damagecompass(damage, actor->o);
        }

        if(randomevent(2))
        {
            if(d->aptitude==APT_PHYSICIEN && d->abilitymillis[ABILITY_1] && d->armour>0) playSound(S_PHY_1, d==h ? NULL : &d->o, 200, 100, SND_LOWPRIORITY);
            else if(d->armour>0 && actor->gunselect!=GUN_LANCEFLAMMES) playSound(S_IMPACTWOOD+d->armourtype, d==h ? NULL : &d->o, 250, 50, SND_LOWPRIORITY);
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
        loopi(NUMABILITIES) d->abilityready[i] = true;

        // death gfx effects
        vec pos(d->o.x, d->o.y, d->o.z-9);
        gibeffect(4000, d->vel, d);
        particle_splash(PART_SMOKE,  8, 1500, pos, 0x333333, 12.0f,  125, 400);
        particle_splash(PART_SMOKE,  5, 900, pos, 0x440000, 10.0f,  125, 200);

        loopi(NUMABILITIES)
        {
            if(hasAbilityEnabled(d, i)) { stopLinkedSound(d->entityId, PL_ABI_SND_1+i); } // stopping ability(ties) sound(s)
        }

        if(d==player1)
        {
            if(deathscore) showscores(true);
            disablezoom();
            cleardamagescreen();
            d->boostmillis[B_SHROOMS] = 0;
            d->attacking = ACT_IDLE;
            gfx::resetpostfx();
            if(!cbcompensation) addpostfx("deathscreen", 1, 1, 1, 1, vec4(1, 1, 1, 1));
            d->roll = 0;
            playSound(S_DIE_P1, NULL, 0, 0, SND_FIXEDPITCH);
            if(m_tutorial) execute("reset_needed_triggers");
        }
        else
        {
            d->move = d->strafe = 0;
            d->resetinterp();
            d->smoothmillis = 0;
            playSound(S_DIE, &d->o, 300, 50);
        }
    }

    VARP(teamcolorfrags, 0, 1, 1);

    static const struct partFR { const char *partverb, *parttroll, *partsuicide; } partmessageFR[] =
    {
        {"explosé", "ta bêtise.", "suicidé : Darwin Award"},
        {"owned", "ton incompétence.", "tué tout seul"},
        {"niqué", "ta débilité !", "niqué comme un con"},
        {"tué", "ton manque de chance.", "annihilé bêtement"},
        {"butté", "la meilleure volonté du monde.", "exterminé sans ennemis"},
        {"troué", "succès."},
        {"dézingué", "détermination."},
        {"annihilé", "une précision sans précédent."},
        {"brisé", "d'atroces souffrances."},
        {"neutralisé"},
        {"pulverisé"},
        {"exterminé"},
        {"achevé"},
        {"détruit"},
        {"vaporisé"},
    };

    static const struct partEN { const char *partverb, *parttroll, *partsuicide; } partmessageEN[] =
    {
        {"killed", "stupidity.", "suicided: Darwin Award"},
        {"slayed", "determination.", "committed suicide"},
        {"finished", "success!"},
        {"deleted", "excruciating pain."},
        {"murdered", "regrets."},
        {"destroyed"},
        {"annihilated"},
    };

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
        d->killstreak = 0;

        if(d->state==CS_EDITING)
        {
            d->editstate = CS_DEAD;
            d->deaths++;
            if(d!=player1) d->resetinterp();
            return;
        }
        else if((d->state!=CS_ALIVE && d->state != CS_LAGGED && d->state != CS_SPAWNING) || intermission || premission) return;

        //////////////////////////////SONS//////////////////////////////
        switch(actor->killstreak) //Sons Killstreak
        {
            case 3:
                playSound(S_KS_X3, actor==player1 ? NULL : &actor->o, 300, 100);
                if(camera1->o.dist(actor->o) >= 250) playSound(S_KS_X3_FAR, &actor->o, 1500, 200);
                break;
            case 5:
                playSound(S_KS_X5, actor==player1 ? NULL : &actor->o, 300, 100);
                if(camera1->o.dist(actor->o) >= 250) playSound(S_KS_X5_FAR, &actor->o, 1500, 200);
                break;
            case 7:
            case 10:
            case 15:
                playSound(S_KS_X7, actor==player1 ? NULL : &actor->o, 300, 100);
                if(camera1->o.dist(actor->o) >= 250) playSound(S_KS_X7_FAR, &actor->o, 1500, 200);
                break;
        }

        //////////////////////////////GRAPHISMES//////////////////////////////
        if(actor->aptitude==APT_FAUCHEUSE) //Eclair aptitude faucheuse
        {
            if(camera1->o.dist(d->o) >= 250) playSound(S_ECLAIRLOIN, &d->o, 1000, 100);
            else playSound(S_ECLAIRPROCHE, &d->o, 300, 100);
            adddynlight(d->o.add(vec(0, 0, 20)), 5000, vec(1.5f, 1.5f, 1.5f), 80, 40);
            vec pos(d->o.x, d->o.y, d->o.z-50);
            particle_flare(vec(0, rnd(15000)+rnd(-30000), 20000+rnd(20000)), pos, 175, PART_LIGHTNING, 0xFFFFFF, 40.0f);
            particle_splash(PART_SMOKE,  15, 2000, d->o, 0x333333, 40.0f,  150,   500);
            if(actor==player1) { playSound(S_FAUCHEUSE); player1->vampimillis=1500; }
        }

        //////////////////////////////MESSAGES//////////////////////////////
        gameent *h = followingplayer(player1);
        int contype = d==h || actor==h ? CON_FRAG_SELF : CON_FRAG_OTHER;
        const char *dname = "", *aname = "";

        if(m_teammode && teamcolorfrags)
        {
            dname = teamcolorname(d, GAME_LANG ? "You" : "Tu");
            aname = teamcolorname(actor, GAME_LANG ? "You" : "Tu");
        }
        else
        {
            dname = colorname(d, NULL, GAME_LANG ? "\fdYou" : "\fdTu", "\fc");
            aname = colorname(actor, NULL, GAME_LANG ? "\fdYou" : "\fdTu", "\fc");
        }

        if(d==actor) // Suicide ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        {
            conoutf(contype, "%s%s %s%s%s", d==player1 ? "\fd" : "", dname, GAME_LANG ? "" : d==player1 ? "t'es " : "s'est ", GAME_LANG ? partmessageEN[rnd(2)].partsuicide : partmessageFR[rnd(5)].partsuicide, d==player1 ? " !" : ".");
            if(d==player1)
            {
                hassuicided = true;
                updateStat(1, STAT_MORTS); updateStat(1, STAT_SUICIDES);
                if(atk==ATK_M32_SHOOT)unlockAchievement(ACH_M32SUICIDE);
            }
        }
        else // Kill ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        {
            float killdistance = actor->o.dist(d->o)/18.f;
            if(actor==player1) ////////////////////TU as tué quelqu'un////////////////////
            {
                playSound(S_KILL, NULL, 0, 0, SND_FIXEDPITCH|SND_NOTIFICATION);
                conoutf(CON_HUDCONSOLE, "%s \fc%s \f7! \f4(%.1fm)", GAME_LANG ? "You killed" : "Tu as tué", dname, killdistance);
                conoutf(contype, "\fd%s\f7 > \fl%s\f7 > %s \fl(%.1fm)", player1->name, readstr(guns[atk].ident), dname, killdistance);

                if(IS_ON_OFFICIAL_SERV) //now let's check for shittons of achievements if playing online
                {
                    if(killdistance>=100.f) unlockAchievement(ACH_BEAUTIR);
                    else if(killdistance<1.f && atk==ATK_MOSSBERG_SHOOT) unlockAchievement(ACH_TAKETHAT);
                    else if(killdistance>=69.f && killdistance<70.f ) unlockAchievement(ACH_NICE);
                    if(player1->state==CS_DEAD && player1->lastpain > 200) unlockAchievement(ACH_TUEURFANTOME);
                    if(player1->health<=10 && player1->state==CS_ALIVE) unlockAchievement(ACH_1HPKILL);
                    if(isteam(d->team, player1->team)) {updateStat(1, STAT_ALLIESTUES); unlockAchievement(ACH_CPASBIEN);}

                    switch(atk)
                    {
                        case ATK_ASSISTXPL_SHOOT: if(player1->armourtype==A_ASSIST) unlockAchievement(ACH_KILLASSIST); break;
                        case ATK_LANCEFLAMMES_SHOOT: if(lookupmaterial(d->feetpos())==MAT_WATER) unlockAchievement(ACH_THUGPHYSIQUE); break;
                        case ATK_SV98_SHOOT: if(gfx::zoom==0) unlockAchievement(ACH_NOSCOPE); break;
                        case ATK_GLOCK_SHOOT: if(d->gunselect==GUN_S_NUKE || d->gunselect==GUN_S_CAMPOUZE || d->gunselect==GUN_S_GAU8 || d->gunselect==GUN_S_ROQUETTES) unlockAchievement(ACH_DAVIDGOLIATH); break;
                        case ATK_CAC349_SHOOT: case ATK_CACFLEAU_SHOOT: case ATK_CACMARTEAU_SHOOT: case ATK_CACMASTER_SHOOT: if(d->aptitude==APT_NINJA) unlockAchievement(ACH_PASLOGIQUE); break;
                        case ATK_NUKE_SHOOT: case ATK_CAMPOUZE_SHOOT: case ATK_GAU8_SHOOT: case ATK_ROQUETTES_SHOOT: if(player1->aptitude==APT_AMERICAIN && player1->boostmillis[B_ROIDS]) unlockAchievement(ACH_JUSTEPOUR);
                    }

                    switch(player1->aptitude)
                    {
                        case APT_AMERICAIN: if(d->aptitude==APT_SHOSHONE) unlockAchievement(ACH_FUCKYEAH); break;
                        case APT_ESPION: if(player1->abilitymillis[ABILITY_2]) unlockAchievement(ACH_ESPIONDEGUISE);
                    }

                    switch(player1->killstreak) {case 3: unlockAchievement(ACH_TRIPLETTE); break; case 5: unlockAchievement(ACH_PENTAPLETTE); break; case 10: unlockAchievement(ACH_DECAPLETTE);}

                    //now let's add player stats
                    updateStat(1, STAT_KILLS);
                    addReward(7+player1->killstreak-1, 3+player1->killstreak-1);
                    if(player1->killstreak > stat[STAT_KILLSTREAK]) updateStat(player1->killstreak, STAT_KILLSTREAK, true);
                    if(stat[STAT_MAXKILLDIST]<killdistance) updateStat(killdistance, STAT_MAXKILLDIST, true);
                }
            }
            else if(d==player1) ////////////////////TU as été tué////////////////////
            {
                conoutf(contype, "%s\f7 > \fl%s\f7 > \fd%s \fl(%.1fm)", aname, readstr(guns[atk].ident), player1->name, killdistance);
                updateStat(1, STAT_MORTS);
                formatstring(killername, "%s", aname);
                killerweapon = atk;
                killerclass = actor->aptitude;
                killerlevel = actor->level;
                killerdistance = killdistance;
                hassuicided = false;
            }
            else ////////////////////Quelqu'un a tué quelqu'un////////////////////
            {
                conoutf(contype, "%s\f7 > \fl%s\f7 > %s \fl(%.1fm)", aname, readstr(guns[atk].ident), dname, killdistance);
            }

            ////////////////////Informe que quelqu'un est chaud////////////////////
            defformatstring(verb, "%s", GAME_LANG ? (actor==player1 ? "are" : "is") : (actor==player1 ? "es" : "est"));
            defformatstring(name, "%s", actor==player1 ? "Tu" : aname);

            switch(actor->killstreak)
            {
                case 3: conoutf(CON_HUDCONSOLE, "%s\f7 %s %s", name, verb, GAME_LANG ? "hot! (Triple kill)" : "chaud ! (Triplette)"); break;
                case 5: conoutf(CON_HUDCONSOLE, "%s\f7 %s %s", name, verb, GAME_LANG ? "killing it! (Pentakill)" : "dominant ! (Pentaplette)"); break;
                case 7: conoutf(CON_HUDCONSOLE, "%s\f7 %s %s", name, verb, GAME_LANG ? "instoppable! (Heptakill!)" : "inarrêtable ! (Heptaplette)"); break;
                case 10: conoutf(CON_HUDCONSOLE, "%s\f7 %s %s", name, verb, GAME_LANG ? "invincible! (Decakill!)" : "invincible ! (Décaplette)"); break;
                case 15: conoutf(CON_HUDCONSOLE, "%s\f7 %s %s", name, verb, GAME_LANG ? "as god! (Pentakaidecakill!)" : "un dieu ! (Pentakaidecaplette)"); break;
            }
        }
        deathstate(d);
        ai::killed(d, actor);
    }

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

            if(player1->frags>=10)
            {
                if(accuracy>=50) unlockAchievement(ACH_PRECIS);
                if(player1->deaths<=5) unlockAchievement(ACH_INCREVABLE);
                if(player1->frags>=30) unlockAchievement(ACH_KILLER);
            }
            if(player1->totaldamage/10 > 10000) unlockAchievement(ACH_DESTRUCTEUR);

            defformatstring(flags, "%d", player1->flags);
            conoutf(CON_GAMEINFO, GAME_LANG ? "\faGAME OVER!" : "\faFIN DE LA PARTIE !");
            if(GAME_LANG) conoutf(CON_GAMEINFO, "\f2Kills : %d | Deaths : %d | Total damage : %d | Accuracy : %d%% %s %s", player1->frags, player1->deaths, player1->totaldamage/10, accuracy, m_ctf ? "| Flags :" : "", m_ctf ? flags : "");
            else conoutf(CON_GAMEINFO, "\f2Éliminations : %d | Morts : %d | Dégats infligés : %d | Précision : %d%% %s %s", player1->frags, player1->deaths, player1->totaldamage/10, accuracy, m_ctf ? "| Drapeaux :" : "", m_ctf ? flags : "");
            if(stat[STAT_DAMMAGERECORD] < player1->totaldamage/10) updateStat(player1->totaldamage/10, STAT_DAMMAGERECORD, true);
            showscores(true);
            disablezoom();

            execident("intermission");
        }
    }

    ICOMMAND(getfrags, "", (), intret(player1->frags));
    ICOMMAND(getflags, "", (), intret(player1->flags));
    ICOMMAND(getdeaths, "", (), intret(player1->deaths));
    ICOMMAND(getaccuracy, "", (), intret((player1->totaldamage*100)/max(player1->totalshots, 1)));
    ICOMMAND(gettotaldamage, "", (), intret(player1->totaldamage));
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
            if(notify && d->name[0]) conoutf("\f7%s\f4 %s", colorname(d), GAME_LANG ? "left the game" : "a quitté la partie");
            removeweapons(d);
            removetrackedparticles(d);
            removetrackeddynlights(d);
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
        filtertext(player1->name, GAME_LANG ? "BadUsername" : "PseudoPourri", false, false, MAXNAMELEN);
        players.add(player1);
        player1->aptitude = player1_aptitude;
    }

    VARP(showmodeinfo, 0, 0, 1);

    void startgame()
    {
        clearmonsters();
        clearprojectiles();
        clearbouncers();
        clearragdolls();
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

        if(premission) conoutf(CON_HUDCONSOLE, GAME_LANG ? "\fcThe game is about to begin" : "\fcLa partie va commencer !");
        conoutf(CON_HUDCONSOLE, GAME_LANG ? "\f2Gamemode is: %s": "\f2Le mode de jeu est : %s", server::modename(gamemode));

        const char *info = m_valid(gamemode) ? GAME_LANG ? gamemodes[gamemode - STARTGAMEMODE].nameEN : gamemodes[gamemode - STARTGAMEMODE].nameFR : NULL;
        if(showmodeinfo && info) conoutf(CON_GAMEINFO, "\f0%s", info);

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
        return showmodeinfo && m_valid(gamemode) ? (GAME_LANG ? gamemodes[gamemode - STARTGAMEMODE].nameEN : gamemodes[gamemode - STARTGAMEMODE].nameFR): NULL;
    }

    const char *getscreenshotinfo()
    {
        return server::modename(gamemode, NULL);
    }

    void physicstrigger(physent *d, bool local, int floorlevel, int waterlevel, int material)
    {
        vec o = (d==hudplayer() && !isthirdperson()) ? d->feetpos() : d->o;
        gameent *pl = (gameent *)d;
        if(waterlevel>0)
        {
            if(material&MAT_WATER)
            {
                playSound(S_WATER, d==hudplayer() ? NULL : &d->o, 200, 30);
                particle_splash(PART_WATER, 30, 120, o, 0x18181A, 10.0f+rnd(9), 500, -20);
            }
        }
        else if(waterlevel<0)
        {
            if(material&MAT_WATER)
            {
                playSound(S_SPLASH, d==hudplayer() ? NULL : &d->o, 300, 50);
                particle_splash(PART_WATER, 40, 150, o, 0x18181A, 10.0f+rnd(12), 600, 30);
            }
            else if(material&MAT_LAVA)
            {
                playSound(S_LAVASPLASH, d==hudplayer() ? NULL : &d->o, 300, 50);
                particle_splash(PART_SMOKE, 25, 100, o, 0x222222, 10.0f+rnd(5), 400, 20);
                particle_splash(PART_FIRE_BALL, 7, 120, o, 0xCC7744, 10.00f+rnd(5), 400, 300);
                loopi(5)regularsplash(PART_FIRESPARK, 0xFFBB55, 500, 10, 500+(rnd(500)), d->o, 1.5f+(rnd(18)/5.f), -10, true);
            }
        }
        if (floorlevel>0)
        {
            particle_splash(map_atmo==4 && atmos ? PART_WATER : PART_SMOKE, pl->armourtype==A_ASSIST ? 12 : 10, 100, d->feetpos(), map_atmo==4 && atmos ? 0x111111 : map_atmo==9 ? 0xFFFFFF : 0x666666, 7.0f+rnd(pl->armourtype==A_ASSIST ? 10 : 5), 400, 20);
            if(d==player1 || d->type!=ENT_PLAYER || ((gameent *)d)->ai) msgsound(pl->armourtype==A_ASSIST && pl->armour ? S_JUMP_ASSIST : pl->aptitude==APT_NINJA || (pl->aptitude==APT_KAMIKAZE && pl->abilitymillis[ABILITY_2]) ? S_JUMP_NINJA : S_JUMP_BASIC, d);
        }
        else if(floorlevel<0)
        {
            particle_splash(map_atmo==4 && atmos ? PART_WATER : PART_SMOKE, pl->armourtype==A_ASSIST ? 20 : 15, 120, d->feetpos(), map_atmo==4 && atmos ? 0x131313 : map_atmo==9 ? 0xFFFFFF : 0x442211, 7.0f+rnd(pl->armourtype==A_ASSIST ? 10 : 5), 400, 20);
            if(d==player1 || d->type!=ENT_PLAYER || ((gameent *)d)->ai) msgsound(pl->armourtype==A_ASSIST && pl->armour ? S_LAND_ASSIST : S_LAND_BASIC, d);
        }
    }

    void footsteps(physent *d)
    {
        if(d->blocked) return;
        bool moving = d->move || d->strafe;
        gameent *pl = (gameent *)d;
        if(d->physstate>=PHYS_SLOPE && moving)
        {
            bool haspowerarmour = pl->armourtype == A_ASSIST;
            int snd = haspowerarmour && pl->armour ? S_FOOTSTEP_ASSIST : S_FOOTSTEP;
            if(lookupmaterial(d->feetpos())==MAT_WATER) snd = S_SWIM;
            if(lastmillis-pl->lastfootstep < (d->vel.magnitude()*(aptitudes[pl->aptitude].apt_vitesse*0.35f)*(pl->crouched() || (pl->abilitymillis[ABILITY_2] && pl->aptitude==APT_ESPION) ? 2 : 1)*(d->inwater ? 2 : 1)*(pl->armourtype==A_ASSIST && pl->armour> 0 ? 2.f : 1)/d->vel.magnitude())) return;
            else
            {
                playSound(snd, d==hudplayer() ? NULL : &d->o, haspowerarmour ? 300 : 150, 20, haspowerarmour ? NULL : SND_LOWPRIORITY);
                if(pl->boostmillis[B_EPO]) if(randomevent(4)) playSound(S_EPO_RUN, d==hudplayer() ? NULL : &d->o, 1000, 500);
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
            playSound(n, &d->o, 150, 20);
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
            if(dup) return tempformatstring(d->aitype == AI_NONE ? "\fs%s%s \f5(%d)\fr" : GAME_LANG ? "\fs%s[AI]%s" : "\fs%s[IA]%s", color, name, d->clientnum);
            return tempformatstring("\fs%s%s\fr", color, name);
        }
        return name;
    }

    VARP(teamcolortext, 0, 1, 1);

    const char *teamcolorname(gameent *d, const char *alt)
    {
        if(!teamcolortext || !m_teammode || !validteam(d->team) || d->state == CS_SPECTATOR) return colorname(d, NULL, alt);
        return colorname(d, NULL, alt, d->team!=player1->team ? teamtextcode[2] : teamtextcode[1]);
    }

    const char *teamcolor(int team)
    {
        return tempformatstring("\fs%s%s\fr", team!=player1->team ? teamtextcode[2] : teamtextcode[1], GAME_LANG ? teamnames_EN[team] : teamnames_FR[team]);
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
    const char *savedconfig() { return "config/saved.cfg"; }
    const char *autoexec() { return "config/autoexec.cfg"; }
    const char *savedservers() { return "config/servers.cfg"; }

    void loadconfigs()
    {
        execfile("config/auth.cfg", false);
    }

    bool clientoption(const char *arg) { return false; }
}

