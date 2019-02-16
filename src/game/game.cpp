#include "game.h"
#include "../cubeconflict/cubedef.h"

string str_pseudovictime, str_pseudotueur, str_armetueur, str_pseudoacteur;
int n_aptitudetueur, n_aptitudevictime, n_killstreakacteur;
bool suicided;

namespace game
{
    VARFP(player1_aptitude, 0, 0, 13,
        {
        addmsg(N_SENDAPTITUDE, "ri", player1_aptitude);
        player1->aptitude = player1_aptitude;
    });

    bool intermission = false;
    int maptime = 0, maprealtime = 0, maplimit = -1;
    int lasthit = 0, lastspawnattempt = 0;

    gameent *player1 = NULL;         // our client
    vector<gameent *> players;       // other clients

    int following = -1;

    VARFP(specmode, 0, 0, 2,
    {
        if(!specmode) stopfollowing();
        else if(following < 0) nextfollow();
    });

    gameent *followingplayer()
    {
        if(player1->state!=CS_SPECTATOR || following<0) return NULL;
        gameent *target = getclient(following);
        if(target && target->state!=CS_SPECTATOR) return target;
        return NULL;
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
        int cur = following >= 0 ? following : (dir < 0 ? clients.length() - 1 : 0);
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
        gameent *target = followingplayer();
        return target ? target : player1;
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
            moveplayer(d, 1, false, d->epomillis, d->jointmillis, d->aptitude);
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

    int battlevivants;

    void otherplayers(int curtime)
    {
        battlevivants=0;
        loopv(players)
        {
            gameent *d = players[i];

            //if(d->state==CS_ALIVE && m_battle) battlevivants++;

            //if(d->ragemillis>0 && d->state==CS_ALIVE && d==hudplayer()) playsound(S_RAGE, NULL, NULL, 0, -1, 50, d->ragechan);
            //else d->stopragesound();
            if(curtime>0 && d->ragemillis && d!=player1) d->ragemillis = max(d->ragemillis-curtime, 0);
            if(d==hudplayer())
            {
                if(d->health<=15 && d->state==CS_ALIVE)
                {
                    d->hurtchan = playsound(S_HEARTBEAT, NULL, NULL, 0, -1, 1000, d->hurtchan);
                }
                else
                {
                    d->stopheartbeat();
                }
            }
            if(d == player1 || d->ai) continue;
            if(d->state==CS_DEAD && d->ragdoll) moveragdoll(d); //if() RAGRAG
            else if(!intermission)
            {
                if(lastmillis - d->lastaction >= d->gunwait) d->gunwait = 0;
                if(d->steromillis) entities::checkstero(curtime, d);
                if(d->epomillis) entities::checkepo(curtime, d);
                if(d->jointmillis) entities::checkjoint(curtime, d);
                if(d->champimillis) entities::checkchampi(curtime, d);
                if(d->ragemillis) entities::checkrage(curtime, d);
            }

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
                else moveplayer(d, 1, false, d->epomillis, d->jointmillis, d->aptitude);
            }
            else if(d->state==CS_DEAD && !d->ragdoll && lastmillis-d->lastpain<2000) moveplayer(d, 1, true, d->epomillis, d->jointmillis, d->aptitude);
        }
    }

    void updateworld()        // main game update loop
    {
        //CubeConflict
        //Bon ce code fonctionne mais il est � chier on est d'accord, am�lioration � faire � l'avenir.
        //(Toujours mieux cod� que Fallout 76)
        int nbmove = 2;
        if(nbfps<30) nbmove = 6; //Permet d'�viter que l'animation ne soit trop lente en cas de framerate bas, y'a surement moyen de faire mieux mais �a fait le taff pour l'instant
        else if(nbfps<60) nbmove = 4;
        else if(nbfps<120) nbmove = 3;

        loopi(nbmove)
        {
            if(zoom==1)
            {
                if(weapposside<guns[player1->gunselect].maxweapposside) weapposside += 2;
                else if(weapposside>guns[player1->gunselect].maxweapposside) weapposside -= 1;
                if(shieldside>maxshieldside) shieldside -= 1;
                else if(shieldside<maxshieldside) shieldside += 1;
                if(weapposup>1) weapposup -= 1;
            }
            else
            {
                if(weapposup<guns[player1->gunselect].maxweapposup) weapposup += 1;
                else if(weapposup>guns[player1->gunselect].maxweapposup) weapposup -= 1;
                if(maxshieldside<-7)maxshieldside=-7;
                if(weapposside>1) weapposside -= 1;
                if(shieldside<1) shieldside += 1;
            }
        }

        if(!maptime) { maptime = lastmillis; maprealtime = totalmillis; return; }
        if(!curtime) { gets2c(); if(player1->clientnum>=0) c2sinfo(); return; }

        physicsframe();
        ai::navigate();

        if(player1->state != CS_DEAD && !intermission)
        {
            if(player1->steromillis) entities::checkstero(curtime, player1);
            if(player1->epomillis) entities::checkepo(curtime, player1);
            if(player1->jointmillis) entities::checkjoint(curtime, player1);
            if(player1->champimillis) entities::checkchampi(curtime, player1);
            if(player1->ragemillis) entities::checkrage(curtime, player1);
        }
        updateweapons(curtime);
        otherplayers(curtime);
        ai::update();
        moveragdolls();
        gets2c();
        if(connected)
        {
            if(player1->state == CS_DEAD)
            {
                if(player1->ragdoll) moveragdoll(player1);
                else
                {
                    player1->move = player1->strafe = 0;
                    moveplayer(player1, 10, true, player1->epomillis, player1->jointmillis, player1->aptitude);
                }
            }
            else if(!intermission)
            {
                if(player1->ragdoll) cleanragdoll(player1);
                crouchplayer(player1, 10, true);
                moveplayer(player1, 10, true, player1->epomillis, player1->jointmillis, player1->aptitude); //
                swayhudgun(curtime);
                entities::checkitems(player1);
                if(cmode) cmode->checkitems(player1);
            }
        }
        if(player1->clientnum>=0) c2sinfo();   // do this last, to reduce the effective frame lag

    }

    void spawnplayer(gameent *d)   // place at random spawn
    {
        if(cmode) cmode->pickspawn(d);
        else findplayerspawn(d, -1, m_teammode ? d->team : 0);
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
            respawnself();
        }
    }

    // inputs

    void doaction(int act)
    {
        if(!connected || intermission) return;
        if((player1->attacking = act)) respawn();
    }

    ICOMMAND(shoot, "D", (int *down), doaction(*down ? ACT_SHOOT : ACT_IDLE));

    bool canjump()
    {
        if(!connected || intermission) return false;
        respawn();
        return player1->state!=CS_DEAD;
    }

    bool cancrouch()
    {
        if(!connected || intermission) return false;
        return player1->state!=CS_DEAD;
    }

    bool allowmove(physent *d)
    {
        if(d->type!=ENT_PLAYER) return true;
        return !((gameent *)d)->lasttaunt || lastmillis-((gameent *)d)->lasttaunt>=1000;
    }

    void taunt()
    {
        if(player1->state!=CS_ALIVE || player1->physstate<PHYS_SLOPE) return;
        if(lastmillis-player1->lasttaunt<1000) return;
        player1->lasttaunt = lastmillis;
        addmsg(N_TAUNT, "rc", player1);
    }
    COMMAND(taunt, "");

    VARP(hitsound, 0, 0, 1);

    void regened(int damage, gameent *d, gameent *actor, bool local)
    {
        if((d->state!=CS_ALIVE && d->state != CS_LAGGED && d->state != CS_SPAWNING) || intermission) return;
        //if(local) damage = m_sp ? d->dodamage(damage/3.0f) : d->dodamage(damage);
        if(local && actor->aptitude==4) damage = actor->doregen(damage);
    }

    void damaged(int damage, gameent *d, gameent *actor, bool local, int atk)
    {
        if((d->state!=CS_ALIVE && d->state != CS_LAGGED && d->state != CS_SPAWNING) || intermission) return;

        if(local) damage = d->dodamage(damage);
        else if(actor==player1) return;

        gameent *h = hudplayer();
        if(h!=player1 && actor==h && d!=actor)
        {
            if(hitsound && lasthit != lastmillis) playsound(S_HIT);
            lasthit = lastmillis;
        }
        if(d==h)
        {
            if(atk==ATK_MEDIGUN_SHOOT)
            {
                regenblend(damage);
                regencompass(damage, actor->o);
                playsound(S_REGENMEDIGUN);
            }
            else
            {
                damageblend(damage);
                damagecompass(damage, actor->o);
                playsound(S_BALLECORPS);
                switch(rnd(2)) {case 0: if(d->armour>0)playsound(S_BALLEBOUCLIER); break; }
            }
        }
        else switch(rnd(2)) {case 0: if(d->armour>0)playsound(S_BALLEBOUCLIERENT, &d->o, 0, 0, 0 , 100, -1, 200); break; }

        damageeffect(damage, d, actor, d!=h, atk);

        ai::damaged(d, actor);

        if(d->health<=0) {if(local) killed(d, actor);}
    }

    VARP(deathscore, 0, 1, 1);

    void deathstate(gameent *d, bool restore)
    {
        d->state = CS_DEAD;
        d->lastpain = lastmillis;

        d->deaths++;
        d->killstreak = 0;
        vec pos(d->o.x, d->o.y, d->o.z-9);
        gibeffect(10000, d->vel, d);
        particle_splash(PART_SMOKE,  8, 1500, pos, 0x333333, 12.0f,  125, 400);
        particle_splash(PART_SMOKE,  5, 900, pos, 0x440000, 10.0f,  125, 200);
        //MORT

        if(d==player1)
        {
            if(deathscore) showscores(true);
            disablezoom();
            d->attacking = ACT_IDLE;
            d->roll = 0;
            playsound(S_DIE2);
        }
        else
        {
            d->move = d->strafe = 0;
            d->resetinterp();
            d->smoothmillis = 0;
            playsound(S_DIE1, &d->o, 0, 0, 0 , 100, -1, 300);
        }
    }

    VARP(teamcolorfrags, 0, 1, 1);

    static const struct part { const char *partverb, *parttroll, *partsuicide; } partmessage[] =
    {
        {"explos�", "ta b�tise.", "suicid� : Darwin Award"},
        {"owned", "ton incomp�tence.", "tu� tout seul"},
        {"niqu�", "ta d�bilit� !", "niqu� comme un con"},
        {"tu�", "ton manque de chance.", "annihil� b�tement"},
        {"butt�", "avec la meilleure volont� du monde.", "extermin� sans ennemis"},
        {"trou�", "succ�s."},
        {"d�zingu�", "d�termination."},
        {"annihil�", "une pr�cision sans pr�c�dent."},
        {"bris�", "d'atroces souffrances."},
        {"neutralis�"},
        {"pulveris�"},
        {"extermin�"},
        {"achev�"},
        {"d�truit"},
        {"vaporis�"},
    };

    void killed(gameent *d, gameent *actor)
    {
        d->killstreak = 0;
        //////////////////////////////GESTION DE ET STATISTIQUES//////////////////////////////
        if(actor==player1 && d!=player1)
        {
            addxp(5+player1->killstreak-1);
            addstat(1, STAT_KILLS);
            if(player1->killstreak>stat_killstreak) addstat(player1->killstreak, STAT_KILLSTREAK);
        }
        else if (d==player1) addstat(1, STAT_MORTS);

        if(d->state==CS_EDITING)
        {
            d->editstate = CS_DEAD;
            d->deaths++;
            if(d!=player1) d->resetinterp();
            return;
        }
        else if((d->state!=CS_ALIVE && d->state != CS_LAGGED && d->state != CS_SPAWNING) || intermission) return;

        //////////////////////////////SONS//////////////////////////////
        switch (actor->killstreak) //Sons Risitas Killstreak
        {
            case 3:
                playsound(S_RISIKILL, actor==player1 ? NULL : &actor->o, 0, 0, 0 , 100, -1, 300);
                if(camera1->o.dist(actor->o) >= 250) playsound(S_RISIKILLLOIN, &actor->o, NULL, 0, 0 , 200, -1, 2000);
                break;
            case 5: case 7:
                playsound(S_BIGRISIKILL, actor==player1 ? NULL : &actor->o, 0, 0, 0 , 100, -1, 300);
                if(camera1->o.dist(actor->o) >= 250) playsound(S_BIGRISIKILLLOIN, &actor->o, NULL, 0, 0 , 200, -1, 750);
                break;
            default:
                if(actor->killstreak>=10)
                {
                    playsound(S_GIGARISIKILL, actor==player1 ? NULL : &actor->o, 0, 0, 0 , 100, -1, 300);
                    if(camera1->o.dist(actor->o) >= 250) playsound(S_GIGARISIKILLLOIN, &actor->o, NULL, 0, 0 , 200, -1, 1500);
                }
        }

        //if(m_battle && actor==player1) playsound(S_BATTLEKILL);  //Sons de kills pour certaines armes

        switch(actor->gunselect)
        {
            case GUN_UZI: playsound(S_BLOHBLOH, actor==player1 ? NULL : &actor->o, 0, 0, 0 , 100, -1, 300); break;
            case GUN_FAMAS: playsound(S_FAMASLOL, actor==player1 ? NULL : &actor->o, 0, 0, 0 , 100, -1, 300); break;
            case GUN_SMAW: playsound(S_BOOBARL, actor==player1 ? NULL : &actor->o, 0, 0, 0 , 100, -1, 300); break;
            case GUN_AK47: playsound(S_KALASHLOL, actor==player1 ? NULL : &actor->o, 0, 0, 0 , 100, -1, 300); break;
            case GUN_ARTIFICE: playsound(S_ARTIFICELOL, actor==player1 ? NULL : &actor->o, 0, 0, 0 , 100, -1, 300); break;
            case GUN_M32: playsound(S_GRENADELOL, actor==player1 ? NULL : &actor->o, 0, 0, 0 , 100, -1, 300); break;
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
        gameent *h = followingplayer();
        if(!h) h = player1;
        int contype = d==h || actor==h ? CON_FRAG_SELF : CON_FRAG_OTHER;
        const char *dname = "", *aname = "";
        if(m_teammode && teamcolorfrags)
        {
            dname = teamcolorname(d, "Tu");
            aname = teamcolorname(actor, "Tu");
        }
        else
        {
            dname = colorname(d, NULL, "\fdTu", "\fc");
            aname = colorname(actor, NULL, "\fdTu", "\fc");
        }

        if(d==actor) // Suicide ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        {
            conoutf(contype, "%s%s %s %s%s", d==player1 ? "\fd" : "", dname, d==player1 ? "t'es" : "s'est", partmessage[rnd(5)].partsuicide, d==player1 ? " !" : ".");
            if(d==player1) {player1->killstreak=0; copystring(str_armetueur, partmessage[rnd(9)].parttroll); suicided = true;}
        }
        else if(isteam(d->team, actor->team)) // Tir alli� /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        {
            contype |= CON_TEAMKILL;
            if(actor==player1) conoutf(contype, "\f6%s fragged a teammate (%s)", aname, dname); //TU as tu� un alli�
            else if(d==player1) conoutf(contype, "\f6%s got fragged by a teammate (%s)", dname, aname); //TU as �t� tu� par un alli�
            else conoutf(contype, "\f2%s fragged a teammate (%s)", aname, dname); //Quelqu'un a tu� un de ses alli�s
        }
        else // Kill ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        {
            if(actor==player1) { conoutf(contype, "\fd%s \f7as %s \fc%s \f7avec %s", aname, partmessage[rnd(15)].partverb, dname, guns[actor->gunselect].armedesc); playsound(S_KILL); message1 = totalmillis; message2 = totalmillis; copystring(str_pseudovictime, dname); n_aptitudevictime = d->aptitude;} //TU as tu� quelqu'un
            else if(d==player1) { conoutf(contype, "\fd%s \f7as �t� %s par \fc%s \f7avec %s", dname, partmessage[rnd(15)].partverb, aname, guns[actor->gunselect].armedesc);  player1->killstreak=0; copystring(str_pseudotueur, aname); n_aptitudetueur = actor->aptitude; copystring(str_armetueur, guns[actor->gunselect].armedesc);  suicided = false;} //TU as �t� tu�
            else conoutf(contype, "%s \f7a %s %s \f7avec %s", aname, partmessage[rnd(15)].partverb, dname, guns[actor->gunselect].armedesc); //Quelqu'un a tu� quelqu'un

            if(actor!=player1) // Informe que quelqu'un est chaud  /////////////////////////////////////////////////////////////////////////////////////////////////////////
            {
                if(actor->killstreak==3 || actor->killstreak==5 || actor->killstreak==10)
                {
                    copystring(str_pseudoacteur, aname);
                    n_killstreakacteur = actor->killstreak;
                    message3 = totalmillis;
                }

            }
        }

        deathstate(d);
        ai::killed(d, actor);
    }

    void timeupdate(int secs)
    {
        if(secs > 0)
        {
            maplimit = lastmillis + secs*1000;
        }
        else
        {
            intermission = true;
            player1->attacking = ACT_IDLE;
            if(cmode) cmode->gameover();
            conoutf(CON_GAMEINFO, "\f2FIN DE LA PARTIE !");
            //if(m_ctf) conoutf(CON_GAMEINFO, "\f2player frags: %d, flags: %d, deaths: %d", player1->frags, player1->flags, player1->deaths);
            //else conoutf(CON_GAMEINFO, "\f2player frags: %d, deaths: %d", player1->frags, player1->deaths);
            //int accuracy = (player1->totaldamage*100)/max(player1->totalshots, 1);
            //conoutf(CON_GAMEINFO, "\f2player total damage dealt: %d, damage wasted: %d, accuracy(%%): %d", player1->totaldamage, player1->totalshots-player1->totaldamage, accuracy);

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
        filtertext(player1->name, "Invitay", false, false, MAXNAMELEN);
        players.add(player1);
        player1->aptitude = player1_aptitude;
    }

    VARP(showmodeinfo, 0, 0, 1);

    void startgame()
    {
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

        conoutf(CON_GAMEINFO, "\f2Le mode de jeu est : %s", server::modeprettyname(gamemode));

        const char *info = m_valid(gamemode) ? gamemodes[gamemode - STARTGAMEMODE].info : NULL;
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

        if(!m_mp(gamemode)) spawnplayer(player1);
        else findplayerspawn(player1, -1, m_teammode ? player1->team : 0);
        entities::resetspawns();
        copystring(clientmap, name ? name : "");

        sendmapinfo();
    }

    const char *getmapinfo()
    {
        return showmodeinfo && m_valid(gamemode) ? gamemodes[gamemode - STARTGAMEMODE].info : NULL;
    }

    const char *getscreenshotinfo()
    {
        return server::modename(gamemode, NULL);
    }

    void physicstrigger(physent *d, bool local, int floorlevel, int waterlevel, int material)
    {
        if     (waterlevel>0) { if(material!=MAT_LAVA) playsound(S_SPLASH1, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 350); }
        else if(waterlevel<0) playsound(material==MAT_LAVA ? S_BURN : S_SPLASH2, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 350);
        if     (floorlevel>0) { if(d==player1 || d->type!=ENT_PLAYER || ((gameent *)d)->ai) msgsound(S_JUMP, d); }
        else if(floorlevel<0) { if(d==player1 || d->type!=ENT_PLAYER || ((gameent *)d)->ai) msgsound(S_LAND, d); }
    }

    void dynentcollide(physent *d, physent *o, const vec &dir)
    {
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

    int numdynents() { return players.length(); }

    dynent *iterdynents(int i)
    {
        if(i<players.length()) return players[i];
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
            if(dup) return tempformatstring(d->aitype == AI_NONE ? "\fs%s%s \f5(%d)\fr" : "\fs%s[BOT]%s", color, name, d->clientnum);
            return tempformatstring("\fs%s%s\fr", color, name);
        }
        return name;
    }

    VARP(teamcolortext, 0, 1, 1);

    const char *teamcolorname(gameent *d, const char *alt)
    {
        if(!teamcolortext || !m_teammode || !validteam(d->team)) return colorname(d, NULL, alt);
        return colorname(d, NULL, alt, teamtextcode[d->team]);
    }

    const char *teamcolor(const char *prefix, const char *suffix, int team, const char *alt)
    {
        if(!teamcolortext || !m_teammode || !validteam(team)) return alt;
        return tempformatstring("\fs%s%s%s%s\fr", teamtextcode[team], prefix, teamnames[team], suffix);
    }

    void suicide(physent *d)
    {
        if(d==player1 || (d->type==ENT_PLAYER && ((gameent *)d)->ai))
        {
            if(d->state!=CS_ALIVE) return;
            gameent *pl = (gameent *)d;
            if(!m_mp(gamemode)) killed(pl, pl);
            else
            {
                int seq = (pl->lifesequence<<16)|((lastmillis/1000)&0xFFFF);
                if(pl->suicided!=seq) { addmsg(N_SUICIDE, "rc", pl); pl->suicided = seq; }
            }
        }
    }
    ICOMMAND(suicide, "", (), suicide(player1));

    bool needminimap() { return m_ctf; }

    void drawicon(int icon, float x, float y, float sz)
    {
        settexture("media/interface/hud/items.png");
        float tsz = 0.25f, tx = tsz*(icon%4), ty = tsz*(icon/4);
        gle::defvertex(2);
        gle::deftexcoord0();
        gle::begin(GL_TRIANGLE_STRIP);
        gle::attribf(x,    y);    gle::attribf(tx,     ty);
        gle::attribf(x+sz, y);    gle::attribf(tx+tsz, ty);
        gle::attribf(x,    y+sz); gle::attribf(tx,     ty+tsz);
        gle::attribf(x+sz, y+sz); gle::attribf(tx+tsz, ty+tsz);
        gle::end();
    }

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
    VARP(hitcrosshair, 0, 425, 1000);

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

        drawmessages(player1->killstreak, str_pseudovictime, n_aptitudevictime, str_pseudoacteur, n_killstreakacteur);

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

                col = vec::hexcolor(teamtextcolor[d->team]);
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
    const char *defaultconfig() { return "config/default.cfg"; }
    const char *autoexec() { return "config/autoexec.cfg"; }
    const char *savedservers() { return "config/servers.cfg"; }

    void loadconfigs()
    {
        execfile("config/auth.cfg", false);
    }

    bool clientoption(const char *arg) { return false; }
}

