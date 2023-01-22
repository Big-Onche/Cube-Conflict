
#include "gfx.h"
#include "customs.h"
#include "stats.h"

float killdistance = 0;
int oldapti;
bool hassuicided;

bool randomevent(int probability)
{
    if(game::ispaused()) return false;
    else if(probability <= 1) return true;
    else return rnd(probability)==0;
}

VARP(map_sel, 0, 1, 99);

namespace game
{
    VARFP(player1_aptitude, 0, 0, sizeof(aptitudes)/sizeof(aptitudes[0])-1,
    {
        if(player1->state != CS_DEAD && isconnected() && !m_tutorial)
        {
            conoutf(CON_GAMEINFO, GAME_LANG ? "\fcCannot change class while alive!" : "\fcImpossible de changer d'aptitude en étant vivant !");
            playsound(S_ERROR);
            player1_aptitude = oldapti;
        }
        else
        {
            addmsg(N_SENDAPTITUDE, "ri", player1_aptitude);
            player1->aptitude = player1_aptitude;
            oldapti = player1->aptitude;
            if(!isconnected())stopsounds();
            playsound(S_APT_SOLDAT+player1_aptitude);
        }
    });

    bool intermission = false;
    bool premission = false;
    int maptime = 0, maprealtime = 0, maplimit = -1;
    int lasthit = 0, lastspawnattempt = 0;
    int respawnent = -1;

    gameent *player1 = NULL;         // our client
    vector<gameent *> players;       // other clients

    VAR(teamscoreboardcolor, 0, 0, 1);
    ICOMMAND(getsbcolor, "", (), player1->team == 1 ? teamscoreboardcolor = 1 : teamscoreboardcolor = 0;);

    int following = -1;

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

    int lasttimeupdate = 0;

    void dotime()
    {
        if(totalmillis >= lasttimeupdate+1000) //1 second interval
        {
            addstat(1, STAT_TIMEPLAYED);
            lasttimeupdate = totalmillis;
        }
    }

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
    }

    gameent *pointatplayer()
    {
        loopv(players) if(players[i] != player1 && intersect(players[i], player1->o, worldpos)) return players[i];
        return NULL;
    }

    gameent *hudplayer()
    {
        if(thirdperson || specmode > 1) return player1;
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
                if(player1->gunselect!=GUN_KAMIKAZE) gunselect(cnidentiquearme, player1);
                break;
            case APT_NINJA:
                if(player1->gunselect!=GUN_CACNINJA) gunselect(cnidentiquearme, player1);
                break;
            default: gunselect(cnidentiquearme, player1);
        }
    }

    bool hasabilityon(gameent *d)
    {
        loopi(NUMABILITIES) { if(d->abilitymillis[i]) return true; }
        return false;
    }

    bool hasboost(gameent *d)
    {
        loopi(NUMBOOSTS) { if(d->boostmillis[i]) return true; }
        return false;
    }

    void otherplayers(int curtime)
    {
        loopv(players)
        {
            gameent *d = players[i];

            if(d!=player1 && d->state==CS_ALIVE && !intermission && !premission)
            {
                if(d->armourtype==A_ASSIST && d->ammo[GUN_ASSISTXPL]>0 && d->armour==0 && d->state==CS_ALIVE) {gunselect(GUN_ASSISTXPL, d, true); d->gunwait=0;}

                if(curtime>0 && d->ragemillis) d->ragemillis = max(d->ragemillis-curtime, 0);

                if(lastmillis - d->lastaction >= d->gunwait) d->gunwait = 0;
                if(hasboost(d)) entities::checkboosts(curtime, d);
                if(d->ragemillis || d->vampimillis || hasabilityon(d)) entities::checkaptiskill(curtime, d);
                if(d->aptitude==APT_MAGICIEN || d->aptitude==APT_PHYSICIEN || d->aptitude==APT_PRETRE || d->aptitude==APT_SHOSHONE || d->aptitude==APT_ESPION) updatespecials(d);
            }

            if(d==hudplayer() && d->state==CS_ALIVE && isconnected())
            {
                if(d->armour<=1000 && d->armourtype==A_ASSIST && d->armour>0) d->alarmchan = playsound(S_ASSISTALARM, NULL, NULL, 0, -1, 1000, d->alarmchan);
                else d->stoppowerarmorsound();
            }

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

    VAR(isalive, 0, 1, 1);

    void updateworld()        // main game update loop
    {
        loopi(gfx::nbfps > 250 ? 1 : 250/gfx::nbfps)
        {
            if(gfx::zoom)
            {
                gfx::weapposside = min(gfx::weapposside + 2, guns[player1->gunselect].maxweapposside);
                if(gfx::weapposup>1) gfx::weapposup -= 1;
            }
            else
            {
                gfx::weapposup = min(gfx::weapposup + 1, guns[player1->gunselect].maxweapposup);
                if(gfx::weapposside>1) gfx::weapposside -= 2;
            }
        }

        if(!maptime) { maptime = lastmillis; maprealtime = totalmillis; return; }
        if(!curtime) { gets2c(); if(player1->clientnum>=0) c2sinfo(); return; }

        physicsframe();
        ai::navigate();

        if(player1->state==CS_ALIVE && !intermission && !premission)   // checking player1's shits
        {
            isalive = 1;

            if(player1->armourtype==A_ASSIST && player1->ammo[GUN_ASSISTXPL]>0 && player1->armour==0) {gunselect(GUN_ASSISTXPL, player1, true); player1->gunwait=0;}
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
                if(player1->health>=2000) unlockachievement(ACH_SACAPV);
                if(player1->boostmillis[B_ROIDS] && player1->boostmillis[B_EPO] && player1->boostmillis[B_JOINT] && player1->boostmillis[B_SHROOMS]) unlockachievement(ACH_DEFONCE);
                if(lookupmaterial(player1->o)==MAT_NOCLIP && map_sel==3) unlockachievement(ACH_SPAAACE);
                if(p1hassuperweapon && player1->boostmillis[B_ROIDS] && player1->armour>0 && player1->armourtype==A_ASSIST) unlockachievement(ACH_ABUS);
                if(player1->aptitude==APT_KAMIKAZE && player1->ammo[GUN_KAMIKAZE]<=0 && totalmillis-lastshoot>=500 && totalmillis-lastshoot<=750 && isconnected()) unlockachievement(ACH_SUICIDEFAIL);
                if(player1->boostmillis[B_EPO] && player1->aptitude==APT_JUNKIE) unlockachievement(ACH_LANCEEPO);
            }

            if(hasboost(player1)) entities::checkboosts(curtime, player1);
            if(player1->ragemillis || player1->vampimillis || hasabilityon(player1)) entities::checkaptiskill(curtime, player1);
            if(player1->aptitude==APT_MAGICIEN || player1->aptitude==APT_PHYSICIEN || player1->aptitude==APT_PRETRE || player1->aptitude==APT_SHOSHONE || player1->aptitude==APT_ESPION) updatespecials(player1);
        }
        else if (player1->state == CS_DEAD) isalive = 0;

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
                moveplayer(player1, 10, true, player1->boostmillis[B_EPO], player1->boostmillis[B_JOINT], player1->aptitude, player1->aptitude==APT_MAGICIEN ? player1->abilitymillis[ABILITY_1] : player1->aptitude==APT_SHOSHONE || player1->aptitude==APT_ESPION || player1->aptitude==APT_KAMIKAZE ? player1->abilitymillis[ABILITY_2] : player1->abilitymillis[ABILITY_3], player1->armourtype==A_ASSIST && player1->armour>0 ? true : false);
                swayhudgun(curtime);
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

    VARP(spawnwait, 2000, 2000, 10000);

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

    bool allowmove(physent *d)
    {
        return true;
        //return !((gameent *)d)->lasttaunt || lastmillis-((gameent *)d)->lasttaunt>=5000;
    }

    VARFP(player1_danse, 0, 0, sizeof(customsdance)/sizeof(customsdance[0])-1,
    {
        if(cust[VOI_CORTEX+player1_danse]<= 0) {conoutf(CON_GAMEINFO, "\f3Vous ne possédez pas cette voix !"); playsound(S_ERROR); player1_danse=0; return;}
        addmsg(N_SENDDANSE, "ri", player1_danse);
        stopsounds();
        player1->customdanse = player1_danse;
        playsound(S_CGCORTEX+(player1_danse));
    });

    void taunt()
    {
        if(GAME_LANG) return;
        if(player1->state!=CS_ALIVE) return;
        if(lastmillis-player1->lasttaunt<2000){conoutf(CON_GAMEINFO, "\faOn abuse pas des bonnes choses !"); return;}
        player1->lasttaunt = lastmillis;
        playsound(S_CGCORTEX+(player1->customdanse));
        addmsg(N_TAUNT, "rc", player1);
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
            if(hitsound && lasthit != lastmillis) playsound(S_HIT);
            lasthit = lastmillis;
        }

        if(d==h)
        {
            damageblend(damage);
            damagecompass(damage, actor->o);
        }

        if(randomevent(2))
        {
            if(d->aptitude==APT_PHYSICIEN && d->abilitymillis[ABILITY_1] && d->armour>0)
            {
                playsound(S_PHY_1, d==h ? NULL : &d->o, 0, 0, 0 , 100, -1, 200);
            }
            else if(d->armour>0 && actor->gunselect!=GUN_LANCEFLAMMES)
                playsound(d->armourtype == A_BLUE ? S_IMPACTWOOD :
                          d->armourtype == A_GREEN ? S_IMPACTIRON :
                          d->armourtype == A_YELLOW ? S_IMPACTGOLD :
                          d->armourtype == A_ASSIST ? S_IMPACTPOWERARMOR : S_IMPACTMAGNET,
                          d==h ? NULL : &d->o, 0, 0, 0 , 100, -1, 200);
        }

        damageeffect(damage, d, actor, d!=h, atk);

        ai::damaged(d, actor);

        if(d->health<=0) {if(local) killed(d, actor, atk);}
    }

    VARP(deathscore, 0, 1, 1);

    void deathstate(gameent *d, gameent *actor, bool restore)
    {
        d->state = CS_DEAD;
        d->lastpain = lastmillis;
        d->skeletonfade = 1.0f;
        d->tombepop = 0.0f;

        d->deaths++;
        d->killstreak = 0;
        loopi(NUMABILITIES) d->abilityready[i] = true;

        //Effet graphique de mort
        vec pos(d->o.x, d->o.y, d->o.z-9);
        gibeffect(4000, d->vel, d);
        particle_splash(PART_SMOKE,  8, 1500, pos, 0x333333, 12.0f,  125, 400);
        particle_splash(PART_SMOKE,  5, 900, pos, 0x440000, 10.0f,  125, 200);

        if(d==player1)
        {
            if(deathscore) showscores(true);
            disablezoom();
            d->attacking = ACT_IDLE;
            gfx::resetshroomsgfx();
            d->roll = 0;
            playsound(S_DIE_P1);
        }
        else
        {
            d->move = d->strafe = 0;
            d->resetinterp();
            d->smoothmillis = 0;
            playsound(S_DIE, &d->o, 0, 0, 0 , 100, -1, 300);
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
                playsound(S_KS_X3, actor==player1 ? NULL : &actor->o, 0, 0, 0 , 100, -1, 300);
                if(camera1->o.dist(actor->o) >= 250) playsound(S_KS_X3_FAR, &actor->o, NULL, 0, 0 , 200, -1, 2000);
                break;
            case 5:
                playsound(S_KS_X5, actor==player1 ? NULL : &actor->o, 0, 0, 0 , 100, -1, 300);
                if(camera1->o.dist(actor->o) >= 250) playsound(S_KS_X5_FAR, &actor->o, NULL, 0, 0 , 200, -1, 750);
                break;
            case 7:
            case 10:
            case 15:
                playsound(S_KS_X7, actor==player1 ? NULL : &actor->o, 0, 0, 0 , 100, -1, 300);
                if(camera1->o.dist(actor->o) >= 250) playsound(S_KS_X7_FAR, &actor->o, NULL, 0, 0 , 200, -1, 1500);
                break;
        }

        //////////////////////////////GRAPHISMES//////////////////////////////
        if(actor->aptitude==7) //Eclair aptitude faucheuse
        {
            if(camera1->o.dist(d->o) >= 250) playsound(S_ECLAIRLOIN, &d->o, NULL, 0, 0, 100, -1, 1000);
            else playsound(S_ECLAIRPROCHE, &d->o, NULL, 0, 0, 100, -1, 300);
            adddynlight(d->o.add(vec(0, 0, 20)), 5000, vec(1.5f, 1.5f, 1.5f), 80, 40);
            vec pos(d->o.x, d->o.y, d->o.z-50);
            particle_flare(vec(0, rnd(15000)+rnd(-30000), 20000+rnd(20000)), pos, 175, PART_LIGHTNING, 0xFFFFFF, 40.0f);
            particle_splash(PART_SMOKE,  15, 2000, d->o, 0x333333, 40.0f,  150,   500);
            if(actor==player1) {playsound(S_FAUCHEUSE);}
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

        if(d==actor || atk==-1) // Suicide ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        {
            conoutf(contype, "%s%s %s%s%s", d==player1 ? "\fd" : "", dname, GAME_LANG ? "" : d==player1 ? "t'es " : "s'est ", GAME_LANG ? partmessageEN[rnd(2)].partsuicide : partmessageFR[rnd(5)].partsuicide, d==player1 ? " !" : ".");
            if(d==player1)
            {
                copystring(str_armetueur, GAME_LANG ? partmessageEN[rnd(5)].parttroll : partmessageFR[rnd(9)].parttroll);
                hassuicided = true;
                addstat(1, STAT_MORTS); addstat(1, STAT_SUICIDES);
                if(atk==ATK_M32_SHOOT)unlockachievement(ACH_M32SUICIDE);
            }
        }
        else if(isteam(d->team, actor->team)) // Tir allié /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        {
            contype |= CON_TEAMKILL;
            if(d==player1) //TU as été tué par un allié
            {
                copystring(str_pseudotueur, aname); n_aptitudetueur = actor->aptitude;
                copystring(str_armetueur, GAME_LANG ? guns[atk].armedescEN : guns[atk].armedescFR);
                conoutf(contype, "\f6%s %s \fd(%s)", dname, GAME_LANG ? "got fragged by a teammate" : "as été tué par un allié", aname);
                addstat(1, STAT_MORTS);
            }
            else
            {
                conoutf(contype, "\f2%s %s%s \fd(%s)", aname, GAME_LANG ? "" : actor==player1 ? "as " : "a ", GAME_LANG ? "fragged a teammate" : "tué un allié" , dname); //Quelqu'un a ou TU as tué un de ses alliés
                if(actor==player1) {addstat(1, STAT_ALLIESTUES); unlockachievement(ACH_CPASBIEN);}
            }
        }
        else // Kill ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        {
            float distance = actor->o.dist(d->o)/18.f;
            if(actor==player1) ////////////////////TU as tué quelqu'un////////////////////
            {
                conoutf(contype, "\fd%s \f7%s%s \fc%s \f7%s %s (%.1fm)",
                    aname,
                    GAME_LANG ? "" : "as ",
                    GAME_LANG ? partmessageEN[rnd(7)].partverb : partmessageFR[rnd(15)].partverb,
                    dname,
                    GAME_LANG ? "with" : "avec",
                    GAME_LANG ? guns[atk].armedescEN : guns[atk].armedescFR,
                    distance);
                playsound(S_KILL);
                hudmsg[MSG_YOUKILLED] = totalmillis; hudmsg[MSG_OWNKILLSTREAK] = totalmillis; copystring(str_pseudovictime, dname); n_aptitudevictime = d->aptitude; killdistance = distance;

                if(IS_ON_OFFICIAL_SERV) //now let's check for shittons of achievements if playing online
                {
                    if(distance>=100.f) unlockachievement(ACH_BEAUTIR);
                    if(player1->state==CS_DEAD) unlockachievement(ACH_TUEURFANTOME);
                    if(player1->health<=100 && player1->state==CS_ALIVE) unlockachievement(ACH_1HPKILL);

                    switch(atk)
                    {
                        case ATK_ASSISTXPL_SHOOT: if(player1->armourtype==A_ASSIST) unlockachievement(ACH_KILLASSIST); break;
                        case ATK_LANCEFLAMMES_SHOOT: if(lookupmaterial(d->feetpos())==MAT_WATER) unlockachievement(ACH_THUGPHYSIQUE); break;
                        case ATK_SV98_SHOOT: if(gfx::zoom==0) unlockachievement(ACH_NOSCOPE); break;
                        case ATK_GLOCK_SHOOT: if(d->gunselect==GUN_S_NUKE || d->gunselect==GUN_S_CAMPOUZE || d->gunselect==GUN_S_GAU8 || d->gunselect==GUN_S_ROQUETTES) unlockachievement(ACH_DAVIDGOLIATH); break;
                        case ATK_CAC349_SHOOT: case ATK_CACFLEAU_SHOOT: case ATK_CACMARTEAU_SHOOT: case ATK_CACMASTER_SHOOT: if(d->aptitude==APT_NINJA) unlockachievement(ACH_PASLOGIQUE); break;
                        case ATK_NUKE_SHOOT: case ATK_CAMPOUZE_SHOOT: case ATK_GAU8_SHOOT: case ATK_ROQUETTES_SHOOT: if(player1->aptitude==APT_AMERICAIN && player1->boostmillis[B_ROIDS]) unlockachievement(ACH_JUSTEPOUR);
                    }

                    switch(player1->aptitude)
                    {
                        case APT_AMERICAIN: if(d->aptitude==APT_SHOSHONE) unlockachievement(ACH_FUCKYEAH); break;
                        case APT_ESPION: if(player1->abilitymillis[ABILITY_2]) unlockachievement(ACH_ESPIONDEGUISE);
                    }

                    switch(player1->killstreak) {case 3: unlockachievement(ACH_TRIPLETTE); break; case 5: unlockachievement(ACH_PENTAPLETTE); break; case 10: unlockachievement(ACH_DECAPLETTE);}

                    //now let's add player stats
                    addstat(1, STAT_KILLS);
                    addxpandcc(7+player1->killstreak-1, 3+player1->killstreak-1);
                    if(player1->killstreak > stat[STAT_KILLSTREAK]) addstat(player1->killstreak, STAT_KILLSTREAK, true);
                    if(stat[STAT_MAXKILLDIST]<distance) addstat(distance, STAT_MAXKILLDIST, true);
                }
            }
            else if(d==player1) ////////////////////TU as été tué////////////////////
            {
                conoutf(contype, "\fd%s \f7%s %s %s \fc%s \f7%s %s (%.1fm)",
                    dname,
                    GAME_LANG ? "got" : "as été",
                    GAME_LANG ? partmessageEN[rnd(7)].partverb : partmessageFR[rnd(15)].partverb,
                    GAME_LANG ? "by" : "par",
                    aname,
                    GAME_LANG ? "with" : "avec",
                    GAME_LANG ? guns[atk].armedescEN : guns[atk].armedescFR,
                    distance);
                copystring(str_pseudotueur, aname); n_aptitudetueur = actor->aptitude;
                copystring(str_armetueur, GAME_LANG ? guns[atk].armedescEN : guns[atk].armedescFR);
                hassuicided = false;
                addstat(1, STAT_MORTS);
            }
            else ////////////////////Quelqu'un a tué quelqu'un////////////////////
            {
                conoutf(contype, "%s \f7%s%s %s \f7%s %s (%.1fm)",
                    aname,
                    GAME_LANG ? "" : "a ",
                    GAME_LANG ? partmessageEN[rnd(7)].partverb : partmessageFR[rnd(15)].partverb,
                    dname,
                    GAME_LANG ? "with" : "avec",
                    GAME_LANG ? guns[atk].armedescEN : guns[atk].armedescFR, distance);
            }

            if(actor!=player1) ////////////////////Informe que quelqu'un est chaud////////////////////
            {
                if(actor->killstreak==3 || actor->killstreak==5 || actor->killstreak==10)
                {
                    copystring(str_pseudoacteur, aname);
                    n_killstreakacteur = actor->killstreak;
                    hudmsg[MSG_OTHERKILLSTREAK] = totalmillis;
                }
            }
        }
        deathstate(d, actor);
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
                if(accuracy>=50) unlockachievement(ACH_PRECIS);
                if(player1->deaths<=5) unlockachievement(ACH_INCREVABLE);
                if(player1->frags>=30) unlockachievement(ACH_KILLER);
            }
            if(player1->totaldamage/10 > 10000) unlockachievement(ACH_DESTRUCTEUR);

            defformatstring(flags, "%d", player1->flags);
            conoutf(CON_GAMEINFO, GAME_LANG ? "\faGAME OVER !" : "\faFIN DE LA PARTIE !");
            if(GAME_LANG) conoutf(CON_GAMEINFO, "\f2Kills : %d | Deaths : %d | Total damage : %d | Accuracy : %d%% %s %s", player1->frags, player1->deaths, player1->totaldamage/10, accuracy, m_ctf ? "| Flags :" : "", m_ctf ? flags : "");
            else conoutf(CON_GAMEINFO, "\f2Éliminations : %d | Morts : %d | Dégats infligés : %d | Précision : %d%% %s %s", player1->frags, player1->deaths, player1->totaldamage/10, accuracy, m_ctf ? "| Drapeaux :" : "", m_ctf ? flags : "");
            if(stat[STAT_DAMMAGERECORD] < player1->totaldamage/10) addstat(player1->totaldamage/10, STAT_DAMMAGERECORD, true);
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
            if(notify && d->name[0]) conoutf("\f7%s\f4 vient de quitter la partie", colorname(d));
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
        genpseudo(3);
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

        conoutf(CON_GAMEINFO, GAME_LANG ? "\f2Gamemode is : %s": "\f2Le mode de jeu est : %s", server::modeprettyname(gamemode));

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
        if(GAME_LANG) return showmodeinfo && m_valid(gamemode) ? gamemodes[gamemode - STARTGAMEMODE].nameEN : NULL;
        else return showmodeinfo && m_valid(gamemode) ? gamemodes[gamemode - STARTGAMEMODE].nameFR : NULL;
    }

    vector<char *> astuces_en;
    ICOMMAND(astuce_en, "s", (char *astuce_en), { astuces_en.add(newstring(astuce_en)); });
    vector<char *> astuces_fr;
    ICOMMAND(astuce_fr, "s", (char *astuce_fr), { astuces_fr.add(newstring(astuce_fr)); });

    const char *getastuce()
    {
        static char astuce[1000];
        astuce[0] = '\0';
        if((GAME_LANG) && !astuces_en.empty()) strcat(astuce, astuces_en[rnd(astuces_en.length())]);
        else if (!astuces_fr.empty()) strcat(astuce, astuces_fr[rnd(astuces_fr.length())]);

        return astuce;
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
                playsound(S_WATER, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 350);
                particle_splash(PART_EAU, 30, 120, o, 0x18181A, 10.0f+rnd(9), 500, -20);
            }
        }
        else if(waterlevel<0)
        {
            if(material&MAT_WATER)
            {
                playsound(S_SPLASH, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 350);
                particle_splash(PART_EAU, 40, 150, o, 0x18181A, 10.0f+rnd(12), 600, 30);
            }
            else if(material&MAT_LAVA)
            {
                playsound(S_SPLASH_LAVA, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 350);
                particle_splash(PART_SMOKE, 25, 100, o, 0x222222, 10.0f+rnd(5), 400, 20);
                particle_splash(PART_FLAME1+rnd(2), 7, 120, o, 0xCC7744, 10.00f+rnd(5), 400, 300);
                loopi(5)regularsplash(PART_FIRESPARK, 0xFFBB55, 500, 10, 500+(rnd(500)), d->o, 1.5f+(rnd(18)/5.f), -10, true);
            }
        }
        if (floorlevel>0)
        {
            particle_splash(map_atmo==4 && randomambience  ? PART_EAU : PART_SMOKE, pl->armourtype==A_ASSIST ? 12 : 10, 100, d->feetpos(), map_atmo==4 && randomambience ? 0x111111 : map_atmo==9 ? 0xFFFFFF : 0x666666, 7.0f+rnd(pl->armourtype==A_ASSIST ? 10 : 5), 400, 20);
            if(d==player1 || d->type!=ENT_PLAYER || ((gameent *)d)->ai) msgsound(pl->armourtype==A_ASSIST && pl->armour>0 ? S_JUMP_ASSIST : pl->aptitude==APT_NINJA || (pl->aptitude==APT_KAMIKAZE && pl->abilitymillis[ABILITY_2]) ? S_JUMP_NINJA : S_JUMP_BASIC, d);
        }
        else if(floorlevel<0)
        {
            particle_splash(map_atmo==4 && randomambience ? PART_EAU : PART_SMOKE, pl->armourtype==A_ASSIST ? 20 : 15, 120, d->feetpos(), map_atmo==4 && randomambience ? 0x131313 : map_atmo==9 ? 0xFFFFFF : 0x442211, 7.0f+rnd(pl->armourtype==A_ASSIST ? 10 : 5), 400, 20);
            if(d==player1 || d->type!=ENT_PLAYER || ((gameent *)d)->ai) msgsound(pl->armourtype==A_ASSIST && pl->armour>0 ? S_LAND_ASSIST : S_LAND_BASIC, d);
        }
    }

    void footsteps(physent *d)
    {
        if(d->blocked) return;
        bool moving = d->move || d->strafe;
        gameent *pl = (gameent *)d;
        if(d->physstate>=PHYS_SLOPE && moving)
        {
            int snd = pl->armourtype==A_ASSIST && pl->armour> 0 ? S_FOOTSTEP_ASSIST : S_FOOTSTEP;
            if(lookupmaterial(d->feetpos())==MAT_WATER) snd = S_SWIM;
            if(lastmillis-pl->lastfootstep < (d->vel.magnitude()*(aptitudes[pl->aptitude].apt_vitesse*0.35f)*(pl->crouched() || (pl->abilitymillis[ABILITY_2] && pl->aptitude==APT_ESPION) ? 2 : 1)*(d->inwater ? 2 : 1)*(pl->armourtype==A_ASSIST && pl->armour> 0 ? 2.f : 1)/d->vel.magnitude())) return;
            else {playsound(snd, d==hudplayer() ? NULL : &d->o, 0, 0, 0 , pl->armourtype==A_ASSIST ? 300 : 150, -1, pl->armourtype==A_ASSIST ? 600 : 300); if(pl->boostmillis[B_EPO]) if(randomevent(4)) playsound(S_EPO_RUN, d==hudplayer() ? NULL : &d->o, 0, 0, 0 , 500, -1, 1000);}
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
            playsound(n);
        }
        else
        {
            if(d->type==ENT_PLAYER && ((gameent *)d)->ai)
                addmsg(N_SOUND, "ci", d, n);
            playsound(n, &d->o, 0, 0, 0 , 100, -1, 350);
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
            if(!m_mp(gamemode)) killed(pl, pl, -1);
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

    float abovegameplayhud(int w, int h)
    {
        switch(hudplayer()->state)
        {
            case CS_EDITING:
            case CS_SPECTATOR:
                return 1;
            default:
                return 1650.0f/1800.0f;
        }
    }

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

        drawmessages(player1->killstreak, str_pseudovictime, n_aptitudevictime, str_pseudoacteur, n_killstreakacteur, killdistance);

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

        if(d->gunwait) col.mul(0.5f);

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
            const char *name = server::modeprettyname(mode, NULL);
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
    const char *restoreconfig() { return "config/restore.cfg"; }
    const char *autoexec() { return "config/autoexec.cfg"; }
    const char *savedservers() { return "config/servers.cfg"; }

    void loadconfigs()
    {
        execfile("config/auth.cfg", false);
    }

    bool clientoption(const char *arg) { return false; }
}

