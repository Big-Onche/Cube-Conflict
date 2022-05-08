#include "game.h"
#include "cubedef.h"

extern int fog;

namespace ai
{
    using namespace game;

    string ia;
    char choosenname[32];

    char *rndname()
    {
        char buf[50];
        int names[3];

        stream *namelist = openfile("config/pseudonymes/pseudos_bots.cfg", "r");
        while(namelist->getline(buf, sizeof(buf)))
        {
            if(sscanf(buf, "NOMBREPSEUDOS: %i", &names[0])==1)
            {
                names[1] = rnd(names[0]);
            }
            else
            {
                sscanf(buf, "NOM %i: %s", &names[2], choosenname);
                if(names[2] == names[1])
                {
                    namelist->close();
                    return choosenname;
                }
            }
        }
        namelist->close();
        copystring(ia, "bot");
        return ia;
    }

    avoidset obstacles;
    int updatemillis = 0, iteration = 0, itermillis = 0, forcegun = -1;
    vec aitarget(0, 0, 0);

    VAR(aidebug, 0, 0, 6);
    VAR(aiforcegun, -1, -1, NUMGUNS-1);
    VAR(IA_number, 0, 5, 49);

    int IA_rndlvl;
    VARF(IA_lvl, 0, 2, 4,
        switch(IA_lvl)
        {
            case 0: IA_rndlvl = 39; return;
            case 1: IA_rndlvl = 51; return;
            case 2: IA_rndlvl = 63; return;
            case 3: IA_rndlvl = 75; return;
            case 4: IA_rndlvl = 88; return;
        }
    );

    ICOMMAND(addbots, "s", (char *s), loopi(IA_number){addmsg(N_ADDBOT, "ri", IA_rndlvl+rnd(7));} );
    ICOMMAND(delbot, "", (), addmsg(N_DELBOT, "r"));
    ICOMMAND(botlimit, "i", (int *n), addmsg(N_BOTLIMIT, "ri", *n));
    ICOMMAND(botbalance, "i", (int *n), addmsg(N_BOTBALANCE, "ri", *n));

    void bottaunt(gameent *d)
    {
        if(langage) return;
        if(d->state!=CS_ALIVE || d->physstate<PHYS_SLOPE) return;
        if(lastmillis-d->lasttaunt<7500) return;
        d->lasttaunt = lastmillis;
        d->dansechan = playsound(S_CGCORTEX+(d->customdanse), d==hudplayer() ? NULL : &d->o, NULL, 0, 0, 150, d->dansechan, 400);
        addmsg(N_TAUNT, "rc", d);
    }

    float viewdist(int x)
    {
        return x <= 100 ? clamp((SIGHTMIN+(SIGHTMAX-SIGHTMIN))/100.f*float(x), float(SIGHTMIN), float(fog)) : float(fog);
    }

    float viewfieldx(int x)
    {
        return x <= 100 ? clamp((VIEWMIN+(VIEWMAX-VIEWMIN))/100.f*float(x), float(VIEWMIN), float(VIEWMAX)) : float(VIEWMAX);
    }

    float viewfieldy(int x)
    {
        return viewfieldx(x)*3.f/4.f;
    }

    bool canmove(gameent *d)
    {
        return d->state != CS_DEAD && !intermission;
    }

    float attackmindist(int atk)
    {
        return max(int(attacks[atk].exprad/3), 2);
    }

    float attackmaxdist(int atk)
    {
        return attacks[atk].range*1.1f;
    }

    bool attackrange(gameent *d, int atk, float dist)
    {
        float mindist = attackmindist(atk), maxdist = attackmaxdist(atk);
        return dist >= mindist*mindist && dist <= maxdist*maxdist;
    }

    bool targetable(gameent *d, gameent *e)
    {
        if((e->aptitude==APT_ESPION && e->aptisort2 && e->attacking==ACT_IDLE && e->physstate!=PHYS_FALL)||(e->aptitude==APT_PHYSICIEN && e->aptisort2)) return false;
        if(d == e) return false;
        return e->state == CS_ALIVE && !isteam(d->team, e->team);
    }

    bool getsight(vec &o, float yaw, float pitch, vec &q, vec &v, float mdist, float fovx, float fovy)
    {
        float dist = o.dist(q);

        if(dist<=mdist)
        {
            float x = fmod(fabs((dist > 0 ? asin((q.z-o.z)/dist)/RAD : 0) - pitch), 360);
            float y = fmod(fabs(-atan2(q.x-o.x, q.y-o.y)/RAD-yaw), 360);
            if(min(x, 360-x) <= fovx && min(y, 360-y) <= fovy) return raycubelos(o, q, v);
        }
        return false;
    }

    bool cansee(gameent *d, vec &x, vec &y, vec &targ)
    {
        if(d->aptitude==APT_ESPION && d->aptisort3) return true;
        aistate &b = d->ai->getstate();
        if(canmove(d) && b.type != AI_S_WAIT)
            return getsight(x, d->yaw, d->pitch, y, targ, d->ai->views[2], d->ai->views[0], d->ai->views[1]);
        return false;
    }

    extern bool hasrange(gameent *d, gameent *e, int weap);

    bool canshoot(gameent *d, int atk, gameent *e)
    {
        if(attackrange(d, atk, e->o.squaredist(d->o)) && targetable(d, e))
            return d->ammo[attacks[atk].gun] > 0 && lastmillis - d->lastaction + 1000 >= d->gunwait + d->gunaccel*(d->gunselect==GUN_PULSE ? 50 : d->gunselect== GUN_S_ROQUETTES ? 150 : 8) + attacks[d->gunselect].attackdelay; //

        return false;
        //return !d->ai->becareful && d->ammo[attacks[atk].gun] > 0 && lastmillis - d->lastaction >= d->gunwait;
    }

    bool hastarget(gameent *d, int atk, aistate &b, gameent *e, float yaw, float pitch, float dist)
	{ // add margins of error
        if(attackrange(d, atk, dist) || (d->skill <= 100 && !rnd(d->skill)))
        {
            float skew = clamp(float(lastmillis-d->ai->enemymillis)/float((d->skill*attacks[atk].attackdelay/200.f)), 0.f, attacks[atk].projspeed ? 0.25f : 1e16f),
                offy = yaw-d->yaw, offp = pitch-d->pitch;

            if(offy > 180) offy -= 360;
            else if(offy < -180) offy += 360;
            if(fabs(offy) <= d->ai->views[0]*skew && fabs(offp) <= d->ai->views[1]*skew) return true;
        }
        return false;
	}

    vec getaimpos(gameent *d, int atk, gameent *e)
    {
        vec o = e->o;

        if(e->aptitude==APT_ESPION && e->aptisort1)
        {
            int posx = 25, posy = 25;
            switch(e->aptiseed)
            {
                 case 1: posx=-25; posy=-25; break;
                 case 2: posx=25; posy=-25; break;
                 case 3: posx=-25; posy=25; break;
            }
            o.add(vec(posx, posy, 0));
        }

        switch(atk)
        {
            case ATK_PULSE_SHOOT: case ATK_GRAP1_SHOOT: o.z += (e->aboveeye*0.2f)-(0.8f*d->eyeheight); break;
            case ATK_SMAW_SHOOT: case ATK_NUKE_SHOOT: case ATK_ROQUETTES_SHOOT: case ATK_ARTIFICE_SHOOT: o.z += (e->aboveeye*0.2f)-(0.9f*d->eyeheight); break;
            default: o.z += (e->aboveeye-e->eyeheight)*0.5f;
        }

        if(d->skill <= 100)
        {
            if(lastmillis >= d->ai->lastaimrnd)
            {
                const int aiskew[NUMGUNS] = { 150, 1, 1, 30, 30, 1, 1, 15, 40, 1, 1, 3, 25, 10, 25, 1, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

                #define rndaioffset(r) ((rnd(int(r*aiskew[d->gunselect]*2)+1)-(r*aiskew[d->gunselect]))*(1.f/float(max(d->skill, 1))))
                loopk(3) d->ai->aimrnd[k] = rndaioffset(e->radius);
                int dur = (d->skill+10)*10;
                d->ai->lastaimrnd = lastmillis+dur+rnd(dur);
            }
            loopk(3) o[k] += d->ai->aimrnd[k];
        }
        return o;
    }

    void create(gameent *d)
    {
        if(!d->ai) d->ai = new aiinfo;
    }

    void destroy(gameent *d)
    {
        if(d->ai) DELETEP(d->ai);
    }

    void init(gameent *d, int at, int ocn, int apti, int cape, int tombe, int danse, int sk, int bn, int pm, int col, const char *name, int team)
    {
        gameent *o = newclient(ocn);

        d->aitype = at;

        bool resetthisguy = false;
        copystring(d->name, rndname(), MAXNAMELEN+1);

        if(!d->name[0])
        {
            if(aidebug) conoutf(CON_DEBUG, "%s assigned to %s at skill %d", colorname(d, name), o ? colorname(o) : "?", sk);
            else conoutf("\f7%s\f4 vient de rejoindre la partie", colorname(d, name));
            resetthisguy = true;
        }
        else
        {
            if(d->ownernum != ocn)
            {
                if(aidebug) conoutf(CON_DEBUG, "%s reassigned to %s", colorname(d, name), o ? colorname(o) : "?");
                resetthisguy = true;
            }
            if(d->skill != sk && aidebug) conoutf(CON_DEBUG, "%s changed skill to %d", colorname(d, name), sk);
        }

        d->team = validteam(team) ? team : 0;
        d->ownernum = ocn;
        d->plag = 0;
        d->skill = d->level = sk;
        d->playermodel = chooserandomplayermodel(pm);
        d->playercolor = col;

        d->aptitude = apti;
        d->customcape = cape;
        d->customtombe = tombe;
        d->customdanse = danse;

        d->tombepop = 0;
        d->skeletonfade = 1.0f;

        if(resetthisguy) removeweapons(d);
        if(d->ownernum >= 0 && player1->clientnum == d->ownernum)
        {
            create(d);
            if(d->ai)
            {
                d->ai->views[0] = viewfieldx(d->skill*3.6f);
                d->ai->views[1] = viewfieldy(d->skill*3.6f);
                d->ai->views[2] = viewdist(d->skill*3.f);
            }
        }
        else if(d->ai) destroy(d);
    }

    void update()
    {
        if(intermission) { loopv(players) if(players[i]->ai) players[i]->stopmoving(); }
        else // fixed rate logic done out-of-sequence at 1 frame per second for each ai
        {
            if(totalmillis-updatemillis > 1000)
            {
                avoid();
                forcegun = multiplayer(false) ? -1 : aiforcegun;
                updatemillis = totalmillis;
            }
            if(!iteration && totalmillis-itermillis > 1000)
            {
                iteration = 1;
                itermillis = totalmillis;
            }
            int count = 0;
            loopv(players) if(players[i]->ai) think(players[i], ++count == iteration ? true : false);
            if(++iteration > count) iteration = 0;
        }
    }

    bool checkothers(vector<int> &targets, gameent *d, int state, int targtype, int target, bool teams, int *members)
    { // checks the states of other ai for a match
        targets.setsize(0);
        loopv(players)
        {
            gameent *e = players[i];
            if(targets.find(e->clientnum) >= 0) continue;
            if(teams && d && !isteam(d->team, e->team)) continue;
            if(members) (*members)++;
            if(e == d || !e->ai || e->state != CS_ALIVE) continue;
            aistate &b = e->ai->getstate();
            if(state >= 0 && b.type != state) continue;
            if(target >= 0 && b.target != target) continue;
            if(targtype >=0 && b.targtype != targtype) continue;
            targets.add(e->clientnum);
        }
        return !targets.empty();
    }

    bool makeroute(gameent *d, aistate &b, int node, bool changed, int retries)
    {
        if(!iswaypoint(d->lastnode)) return false;
        if(changed && d->ai->route.length() > 1 && d->ai->route[0] == node) return true;
        if(route(d, d->lastnode, node, d->ai->route, obstacles, retries))
        {
            b.override = false;
            return true;
        }
        // retry fails: 0 = first attempt, 1 = try ignoring obstacles, 2 = try ignoring prevnodes too
		if(retries <= 1) return makeroute(d, b, node, false, retries+1);
		return false;
    }

    bool makeroute(gameent *d, aistate &b, const vec &pos, bool changed, int retries)
    {
        int node = closestwaypoint(pos, SIGHTMIN, true);
        return makeroute(d, b, node, changed, retries);
    }

    bool randomnode(gameent *d, aistate &b, const vec &pos, float guard, float wander)
    {
        static vector<int> candidates;
        candidates.setsize(0);
        findwaypointswithin(pos, guard, wander, candidates);

        while(!candidates.empty())
        {
            int w = rnd(candidates.length()), n = candidates.removeunordered(w);
            if(n != d->lastnode && !d->ai->hasprevnode(n) && !obstacles.find(n, d) && makeroute(d, b, n)) return true;
        }
        return false;
    }

    bool randomnode(gameent *d, aistate &b, float guard, float wander)
    {
        return randomnode(d, b, d->feetpos(), guard, wander);
    }

    bool badhealth(gameent *d)
    {
        if(d->health<85 && d->health-d->skill<60) return true;
        else return false;
    }

    bool needmana(gameent *d)
    {
        switch(d->aptitude)
        {
            case APT_ESPION:
            case APT_MAGICIEN:
            case APT_SHOSHONE:
            case APT_PRETRE:
            case APT_PHYSICIEN:
                if(d->mana<100) return true;
                else return false;
            break;
            default:
                return false;
        }
    }

    bool enemy(gameent *d, aistate &b, const vec &pos, float guard = SIGHTMIN, int pursue = 0)
    {
        gameent *t = NULL;
        vec dp = d->headpos();
        float mindist = guard*guard, bestdist = 1e16f;
        int atk = guns[d->gunselect].attacks[ACT_SHOOT];

        loopv(players)
        {
            gameent *e = players[i];
            if(e == d || !targetable(d, e)) continue;
            vec ep = getaimpos(d, atk, e);
            float dist = ep.squaredist(dp);
            if(dist < bestdist && (cansee(d, dp, ep) || dist <= mindist))
            {
                t = e;
                bestdist = dist;
            }
        }
        if(t && violence(d, b, t, pursue)) return true;
        return false;
    }

    bool patrol(gameent *d, aistate &b, const vec &pos, float guard, float wander, int walk, bool retry)
    {
        vec feet = d->feetpos();
        if(walk == 2 || b.override || (walk && feet.squaredist(pos) <= guard*guard) || !makeroute(d, b, pos))
        { // run away and back to keep ourselves busy
            if(!b.override && randomnode(d, b, pos, guard, wander))
            {
                b.override = true;
                return true;
            }
            else if(d->ai->route.empty())
            {
                if(!retry)
                {
                    b.override = false;
                    return patrol(d, b, pos, guard, wander, walk, true);
                }
                b.override = false;
                return false;
            }
        }
        b.override = false;
        return true;
    }

    bool defend(gameent *d, aistate &b, const vec &pos, float guard, float wander, int walk)
    {
        bool hasenemy = enemy(d, b, pos, wander);
        if(!walk)
        {
            if(d->feetpos().squaredist(pos) <= guard*guard)
            {
                b.idle = hasenemy ? 2 : 1;
                return true;
            }
            walk++;
        }
        return patrol(d, b, pos, guard, wander, walk);
    }

    bool violence(gameent *d, aistate &b, gameent *e, int pursue)
    {
        if(e && targetable(d, e))
        {
            if(pursue)
            {
                if((b.targtype != AI_T_AFFINITY || !(pursue%2)) && makeroute(d, b, e->lastnode))
                    d->ai->switchstate(b, AI_S_PURSUE, AI_T_PLAYER, e->clientnum);
                else if(pursue >= 3) return false; // can't pursue
            }
            if(d->ai->enemy != e->clientnum)
            {
                d->ai->enemyseen = d->ai->enemymillis = lastmillis;
                d->ai->enemy = e->clientnum;
            }
            return true;
        }
        return false;
    }

    bool target(gameent *d, aistate &b, int pursue = 0, bool force = false, float mindist = 0.f)
    {
        static vector<gameent *> hastried; hastried.setsize(0);
        vec dp = d->headpos();
        while(true)
        {
            float dist = 1e16f;
            gameent *t = NULL;
            int atk = guns[d->gunselect].attacks[ACT_SHOOT];
            loopv(players)
            {
                gameent *e = players[i];
                if(e == d || hastried.find(e) >= 0 || !targetable(d, e)) continue;
                vec ep = getaimpos(d, atk, e);
                float v = ep.squaredist(dp);
                if((!t || v < dist) && (mindist <= 0 || v <= mindist) && (force || cansee(d, dp, ep)))
                {
                    t = e;
                    dist = v;
                }
            }
            if(t)
            {
                if(violence(d, b, t, pursue)) return true;
                hastried.add(t);
            }
            else break;
        }
        return false;
    }

    int isgoodammo(int gun) { return gun >= GUN_RAIL && gun <= GUN_S_CAMPOUZE; } //Toutes les armes sont bonnes à prendre :/

    bool hasgoodammo(gameent *d)
    {
        static const int goodguns[] = { GUN_S_CAMPOUZE, GUN_S_NUKE, GUN_S_GAU8, GUN_S_ROQUETTES, GUN_CACNINJA, GUN_KAMIKAZE, GUN_PULSE, GUN_RAIL, GUN_FAMAS, GUN_SMAW, GUN_MINIGUN, GUN_SPOCKGUN, GUN_AK47, GUN_SV98, GUN_SKS, GUN_ARTIFICE, GUN_ARBALETE, GUN_LANCEFLAMMES };
        loopi(sizeof(goodguns)/sizeof(goodguns[0])) if(d->hasammo(goodguns[0])) return true;
        if(d->ammo[GUN_M32] > 5) return true;
        return false;
    }

    void assist(gameent *d, aistate &b, vector<interest> &interests, bool all, bool force)
    {
        loopv(players)
        {
            gameent *e = players[i];
            if(e == d || (!all && e->aitype != AI_NONE) || !isteam(d->team, e->team)) continue;
            interest &n = interests.add();
            n.state = AI_S_DEFEND;
            n.node = e->lastnode;
            n.target = e->clientnum;
            n.targtype = AI_T_PLAYER;
            n.score = e->o.squaredist(d->o)/(hasgoodammo(d) ? 1e8f : (force ? 1e4f : 1e2f));
        }
    }

    static void tryitem(gameent *d, extentity &e, int id, aistate &b, vector<interest> &interests, bool force = false)
    {
        if(d->aptitude==APT_KAMIKAZE && d->aptisort2) return; //Le kamikaze s'en fout de chopper un item une fois la bombe activée
        float score = 0;
        switch(e.type)
        {
            case I_SUPERARME:
                score = 1e9f;
                break;
            case I_BOOSTDEGATS: case I_BOOSTGRAVITE: case I_BOOSTPRECISION: case I_BOOSTPV: case I_BOOSTVITESSE:
                d->aptitude==APT_JUNKIE ? score = 1e9f : score = 1e7f;
                break;
            case I_SANTE:
                if(d->health<800){score = d->health < 500 ? 1e5f : 1e3f; }
                if(d->mana>40 && d->aptitude==APT_PRETRE && d->health<=300) aptitude(d, 1);
                break;
            case I_MANA:
                if(d->mana < 101 && d->aptitude!=APT_VAMPIRE) score = 1e5f;
                else if (d->aptitude==APT_VAMPIRE) score = d->health < 500 ? 1e7f : d->health < 800 ? 1e5f : 1e3f;
                break;
            case I_BOUCLIERBOIS:
                if(d->armourtype==A_ASSIST && d->armour<1500) score = d->armour<1500 ? 1e6f : 1e4f;
                else if(d->armour<600) score = 1e5f;
                break;
            case I_BOUCLIERFER:
                if(d->armourtype==A_ASSIST && d->armour<1500) score = d->armour<1500 ? 1e6f : 1e5f;
                else if(d->armour<900) score = 1e5f;
                break;
            case I_BOUCLIEROR: case I_BOUCLIERMAGNETIQUE:
                if(d->armourtype==A_ASSIST && d->armour<1500) score = d->armour<1500 ? 1e6f : 1e5f;
                if(d->armour <= 1250) score = 1e6f;
            case I_ARMUREASSISTEE:
                if(d->armourtype!=A_ASSIST) score = 1e9f;
                break;
            default:
            {
                if(e.type >= I_RAIL && e.type <= I_GLOCK && !d->hasmaxammo(e.type))
                {
                    int gun = e.type - I_RAIL + GUN_RAIL;
                    if(isgoodammo(gun)) score = hasgoodammo(d) ? 1e2f : 1e3f;
                }
                break;
            }
        }
        if(score != 0)
        {
            interest &n = interests.add();
            n.state = AI_S_INTEREST;
            n.node = closestwaypoint(e.o, SIGHTMIN, true);
            n.target = id;
            n.targtype = AI_T_ENTITY;
            n.score = d->feetpos().squaredist(e.o)/(force ? -1 : score);
        }
    }

    void items(gameent *d, aistate &b, vector<interest> &interests, bool force = false)
    {
        loopv(entities::ents)
        {
            extentity &e = *(extentity *)entities::ents[i];

            if(!e.spawned() || e.nopickup() || !d->canpickup(e.type, d->aptitude, d->armourtype)) continue;
            tryitem(d, e, i, b, interests, force);
        }
    }

    static vector<int> targets;

    bool parseinterests(gameent *d, aistate &b, vector<interest> &interests, bool override, bool ignore)
    {
        while(!interests.empty())
        {
            int q = interests.length()-1;
            loopi(interests.length()-1) if(interests[i].score < interests[q].score) q = i;
            interest n = interests.removeunordered(q);
            bool proceed = true;
            if(!ignore) switch(n.state)
            {
                case AI_S_DEFEND: // don't get into herds
                {
                    int members = 0;
                    proceed = !checkothers(targets, d, n.state, n.targtype, n.target, true, &members) && members > 1;
                    break;
                }
                default: break;
            }
            if(proceed && makeroute(d, b, n.node))
            {
                d->ai->switchstate(b, n.state, n.targtype, n.target);
                return true;
            }
        }
        return false;
    }

    bool find(gameent *d, aistate &b, bool override = false)
    {
        static vector<interest> interests;
        interests.setsize(0);

        if(!hasgoodammo(d) || d->health < 750)
            items(d, b, interests);
        else
        {
            static vector<int> nearby;
            nearby.setsize(0);
            findents(I_RAIL, I_MANA, false, d->feetpos(), vec(32, 32, 24), nearby);
            loopv(nearby)
            {
                int id = nearby[i];
                extentity &e = *(extentity *)entities::ents[id];
                if(d->canpickup(e.type, d->aptitude, d->armourtype)) tryitem(d, e, id, b, interests);
            }
        }
        if(cmode) cmode->aifind(d, b, interests);
        if(m_teammode) assist(d, b, interests);
        return parseinterests(d, b, interests, override);
    }

    bool findassist(gameent *d, aistate &b, bool override = false)
    {
        static vector<interest> interests;
        interests.setsize(0);
        assist(d, b, interests);
        while(!interests.empty())
        {
            int q = interests.length()-1;
            loopi(interests.length()-1) if(interests[i].score < interests[q].score) q = i;
            interest n = interests.removeunordered(q);
            bool proceed = true;

            if(proceed && makeroute(d, b, n.node))
            {
                d->ai->switchstate(b, n.state, n.targtype, n.target);
                return true;
            }
        }
        return false;
    }

    void damaged(gameent *d, gameent *e)
    {
        if(d->ai && canmove(d) && targetable(d, e)) // see if this ai is interested in a grudge
        {
            aistate &b = d->ai->getstate();
            if(violence(d, b, e)) return;
        }
        if(checkothers(targets, d, AI_S_DEFEND, AI_T_PLAYER, d->clientnum, true))
        {
            loopv(targets)
            {
                gameent *t = getclient(targets[i]);
                if(!t->ai || !canmove(t) || !targetable(t, e)) continue;
                aistate &c = t->ai->getstate();
                if(violence(t, c, e)) return;
            }
        }
    }

    void findorientation(vec &o, float yaw, float pitch, vec &pos)
    {
        vec dir;
        vecfromyawpitch(yaw, pitch, 1, 0, dir);
        if(raycubepos(o, dir, pos, 0, RAY_CLIPMAT|RAY_SKIPFIRST) == -1)
            pos = dir.mul(2*getworldsize()).add(o); //otherwise 3dgui won't work when outside of map
    }

    void setup(gameent *d)
    {
        d->ai->clearsetup();
        d->ai->reset(true);
        d->ai->lastrun = lastmillis;

        if(forcegun >= 0 && forcegun < NUMGUNS) d->ai->weappref = forcegun;

        switch(d->aptitude)
        {
            case APT_KAMIKAZE: d->ai->weappref = GUN_KAMIKAZE; break;
            case APT_NINJA: d->ai->weappref = GUN_CACNINJA; break;
            default:
            {
                if(m_identique) d->ai->weappref = cnidentiquearme;
                else d->ai->weappref = rnd(GUN_GLOCK-GUN_RAIL+1)+GUN_RAIL;
            }
        }

        vec dp = d->headpos();
        findorientation(dp, d->yaw, d->pitch, d->ai->target);
    }

    void spawned(gameent *d)
    {
        if(d->ai) setup(d);
    }

    void killed(gameent *d, gameent *e)
    {
        if(d->ai) d->ai->reset();
    }

    void itemspawned(int ent)
    {
        if(!entities::ents.inrange(ent)) return;
        extentity &e = *entities::ents[ent];
        if(e.type >= I_RAIL && e.type <= I_MANA)
        {
            loopv(players) if(players[i] && players[i]->ai && players[i]->aitype == AI_BOT && players[i]->canpickup(e.type, players[i]->aptitude, player1->armourtype))
            {
                gameent *d = players[i];
                if(d->aptitude==APT_KAMIKAZE && d->aptisort2) return;  //Le kamikaze s'en fout de chopper un objet juste avant de mourrir
                bool wantsitem = false;
                switch(e.type)
                {
                    case I_SANTE: wantsitem = badhealth(d); break;
                    case I_MANA:
                        if(d->aptitude==APT_VAMPIRE) wantsitem = badhealth(d);
                        else wantsitem = needmana(d);
                        break;
                    default:
                        wantsitem = true;
                }
                if(wantsitem)
                {
                    aistate &b = d->ai->getstate();
                    if(b.targtype == AI_T_AFFINITY) continue;
                    if(b.type == AI_S_INTEREST && b.targtype == AI_T_ENTITY)
                    {
                        if(entities::ents.inrange(b.target))
                        {
                            if(d->o.squaredist(entities::ents[ent]->o) < d->o.squaredist(entities::ents[b.target]->o))
                                d->ai->switchstate(b, AI_S_INTEREST, AI_T_ENTITY, ent);
                        }
                        continue;
                    }
                    d->ai->switchstate(b, AI_S_INTEREST, AI_T_ENTITY, ent);
                }
            }
        }
    }

    bool check(gameent *d, aistate &b)
    {
        if(cmode && cmode->aicheck(d, b)) return true;
        return false;
    }

    int dowait(gameent *d, aistate &b)
    {
        d->ai->clear(true); // ensure they're clean
        if(check(d, b) || find(d, b)) return 1;
        if(target(d, b, 4, false)) return 1;
        if(target(d, b, 4, true)) return 1;
        if(randomnode(d, b, SIGHTMIN, 1e16f))
        {
            d->ai->switchstate(b, AI_S_INTEREST, AI_T_NODE, d->ai->route[0]);
            return 1;
        }
        return 0; // but don't pop the state
    }

    int dodefend(gameent *d, aistate &b)
    {
        if(d->state == CS_ALIVE)
        {
            switch(b.targtype)
            {
                case AI_T_NODE:
                    if(check(d, b)) return 1;
                    if(iswaypoint(b.target)) return defend(d, b, waypoints[b.target].o) ? 1 : 0;
                    break;
                case AI_T_ENTITY:
                    if(check(d, b)) return 1;
                    if(entities::ents.inrange(b.target)) return defend(d, b, entities::ents[b.target]->o) ? 1 : 0;
                    break;
                case AI_T_AFFINITY:
                    if(cmode) return cmode->aidefend(d, b) ? 1 : 0;
                    break;
                case AI_T_PLAYER:
                {
                    if(check(d, b)) return 1;
                    gameent *e = getclient(b.target);
                    if(e && e->state == CS_ALIVE) return defend(d, b, e->feetpos()) ? 1 : 0;
                    break;
                }
                default: break;
            }
        }
        return 0;
    }

    int dointerest(gameent *d, aistate &b)
    {
        if(d->state != CS_ALIVE) return 0;
        switch(b.targtype)
        {
            case AI_T_NODE: // this is like a wait state without sitting still..
                if(check(d, b) || find(d, b)) return 1;
                if(target(d, b, 4, true)) return 1;
                if(iswaypoint(b.target) && vec(waypoints[b.target].o).sub(d->feetpos()).magnitude() > CLOSEDIST)
                    return makeroute(d, b, waypoints[b.target].o) ? 1 : 0;
                break;
            case AI_T_ENTITY:
                if(entities::ents.inrange(b.target))
                {
                    extentity &e = *(extentity *)entities::ents[b.target];
                    if(!e.spawned() || e.type < I_RAIL || e.type > I_MANA || d->hasmaxammo(e.type)) return 0;
                    return makeroute(d, b, e.o) ? 1 : 0;
                }
                break;
        }
        return 0;
    }

    int dopursue(gameent *d, aistate &b)
    {
        if(d->state == CS_ALIVE)
        {
            switch(b.targtype)
            {
                case AI_T_NODE:
                {
                    if(check(d, b)) return 1;
                    if(iswaypoint(b.target))
                        return defend(d, b, waypoints[b.target].o) ? 1 : 0;
                    break;
                }

                case AI_T_AFFINITY:
                {
                    if(cmode) return cmode->aipursue(d, b) ? 1 : 0;
                    break;
                }

                case AI_T_PLAYER:
                {
                    //if(check(d, b)) return 1;
                    gameent *e = getclient(b.target);
                    if(e && e->state == CS_ALIVE)
                    {
                        switch(d->aptitude)
                        {
                            case APT_MAGICIEN:
                                if(d->mana>60 && d->o.dist(e->o)<500) aptitude(d, 1);
                                else if (d->mana>=100 && d->o.dist(e->o)>500) aptitude(d, 2);
                                break;
                            case APT_ESPION:
                                switch(rnd(2))
                                {
                                    case 0: if(d->mana>=40 && d->o.dist(e->o)<700 && !d->aptisort2) aptitude(d, 1); break;
                                    case 1: if(d->mana>=50 && d->o.dist(e->o)<700 && !d->aptisort1) aptitude(d, 2);
                                }

                        }

                        int atk = guns[d->gunselect].attacks[ACT_SHOOT];
                        float guard = SIGHTMIN, wander = attacks[atk].range;
                        return patrol(d, b, e->feetpos(), guard, wander) ? 1 : 0;
                    }
                    break;
                }
                default: break;
            }
        }
        return 0;
    }

    int closenode(gameent *d)
    {
        vec pos = d->feetpos();
        int node1 = -1, node2 = -1;
        float mindist1 = CLOSEDIST*CLOSEDIST, mindist2 = CLOSEDIST*CLOSEDIST;
        loopv(d->ai->route) if(iswaypoint(d->ai->route[i]))
        {
            vec epos = waypoints[d->ai->route[i]].o;
            float dist = epos.squaredist(pos);
            if(dist > FARDIST*FARDIST) continue;
            int entid = obstacles.remap(d, d->ai->route[i], epos);
            if(entid >= 0)
            {
                if(entid != i) dist = epos.squaredist(pos);
                if(dist < mindist1) { node1 = i; mindist1 = dist; }
            }
            else if(dist < mindist2) { node2 = i; mindist2 = dist; }
        }
        return node1 >= 0 ? node1 : node2;
    }

    int wpspot(gameent *d, int n, bool check = false)
    {
        if(iswaypoint(n)) loopk(2)
        {
            vec epos = waypoints[n].o;
            int entid = obstacles.remap(d, n, epos, k!=0);
            if(iswaypoint(entid))
            {
                d->ai->spot = epos;
                d->ai->targnode = entid;
                return !check || d->feetpos().squaredist(epos) > MINWPDIST*MINWPDIST ? 1 : 2;
            }
        }
        return 0;
    }

    int randomlink(gameent *d, int n)
    {
        if(iswaypoint(n) && waypoints[n].haslinks())
        {
            waypoint &w = waypoints[n];
            static vector<int> linkmap; linkmap.setsize(0);
            loopi(MAXWAYPOINTLINKS)
            {
                if(!w.links[i]) break;
                if(iswaypoint(w.links[i]) && !d->ai->hasprevnode(w.links[i]) && d->ai->route.find(w.links[i]) < 0)
                    linkmap.add(w.links[i]);
            }
            if(!linkmap.empty()) return linkmap[rnd(linkmap.length())];
        }
        return -1;
    }

    bool anynode(gameent *d, aistate &b, int len = NUMPREVNODES)
    {
        if(iswaypoint(d->lastnode)) loopk(2)
        {
            d->ai->clear(k ? true : false);
            int n = randomlink(d, d->lastnode);
            if(wpspot(d, n))
            {
                d->ai->route.add(n);
                d->ai->route.add(d->lastnode);
                loopi(len)
                {
                    n = randomlink(d, n);
                    if(iswaypoint(n)) d->ai->route.insert(0, n);
                    else break;
                }
                return true;
            }
        }
        return false;
    }

    bool checkroute(gameent *d, int n)
    {
        if(d->ai->route.empty() || !d->ai->route.inrange(n)) return false;
        int last = d->ai->lastcheck ? lastmillis-d->ai->lastcheck : 0;
        if(last < 500 || n < 3) return false; // route length is too short
        d->ai->lastcheck = lastmillis;
        int w = iswaypoint(d->lastnode) ? d->lastnode : d->ai->route[n], c = min(n-1, NUMPREVNODES);
        loopj(c) // check ahead to see if we need to go around something
        {
            int p = n-j-1, v = d->ai->route[p];
            if(d->ai->hasprevnode(v) || obstacles.find(v, d)) // something is in the way, try to remap around it
            {
                int m = p-1;
                if(m < 3) return false; // route length is too short from this point
                loopirev(m)
                {
                    int t = d->ai->route[i];
                    if(!d->ai->hasprevnode(t) && !obstacles.find(t, d))
                    {
                        static vector<int> remap; remap.setsize(0);
                        if(route(d, w, t, remap, obstacles))
                        { // kill what we don't want and put the remap in
                            while(d->ai->route.length() > i) d->ai->route.pop();
                            loopvk(remap) d->ai->route.add(remap[k]);
                            return true;
                        }
                        return false; // we failed
                    }
                }
                return false;
            }
        }
        return false;
    }

    bool hunt(gameent *d, aistate &b)
    {
        if(!d->ai->route.empty())
        {
            int n = closenode(d);
            if(d->ai->route.inrange(n) && checkroute(d, n)) n = closenode(d);
            if(d->ai->route.inrange(n))
            {
                if(!n)
                {
                    switch(wpspot(d, d->ai->route[n], true))
                    {
                        case 2: d->ai->clear(false);
                        case 1: return true; // not close enough to pop it yet
                        case 0: default: break;
                    }
                }
                else
                {
                    while(d->ai->route.length() > n+1) d->ai->route.pop(); // waka-waka-waka-waka
                    int m = n-1; // next, please!
                    if(d->ai->route.inrange(m) && wpspot(d, d->ai->route[m])) return true;
                }
            }
        }
        b.override = false;
        return anynode(d, b);
    }

    void jumpto(gameent *d, aistate &b, const vec &pos)
    {
        if(d->aptitude==APT_ESPION && d->aptisort2) return; // Ne saute pas si déguisé pour ne pas se faire repérer

		vec off = vec(pos).sub(d->feetpos()), dir(off.x, off.y, 0);
        bool sequenced = d->ai->blockseq || d->ai->targseq, offground = d->timeinair && !d->inwater,
            jump = !offground && lastmillis >= d->ai->jumpseed && (sequenced || off.z >= JUMPMIN || lastmillis >= d->ai->jumprand);
		if(jump)
		{
			vec old = d->o;
			d->o = vec(pos).addz(d->eyeheight);
			if(collide(d, vec(0, 0, 1))) jump = false;
			d->o = old;
			if(jump)
			{
				float radius = 18*18;
				loopv(entities::ents) if(entities::ents[i]->type == JUMPPAD)
				{
					gameentity &e = *(gameentity *)entities::ents[i];
					if(e.o.squaredist(pos) <= radius) { jump = false; break; }
				}
			}
		}
		if(jump)
		{
			d->jumping = true;
			int seed = (111-d->skill)*(d->inwater ? 3 : 5);
			d->ai->jumpseed = lastmillis+seed+rnd(seed);
			seed *= b.idle ? 50 : 25;
			d->ai->jumprand = lastmillis+seed+rnd(seed);
		}
    }

    void fixfullrange(float &yaw, float &pitch, float &roll, bool full)
    {
        if(full)
        {
            if(pitch < -180.0f) pitch = 180.0f - fmodf(-180.0f - pitch, 360.0f);
            else if(pitch >= 180.0f) pitch = fmodf(pitch + 180.0f, 360.0f) - 180.0f;
            if(roll < -180.0f) roll = 180.0f - fmodf(-180.0f - roll, 360.0f);
            else if(roll >= 180.0f) roll = fmodf(roll + 180.0f, 360.0f) - 180.0f;
        }
        else
        {
            if(pitch > 89.9f) pitch = 89.9f;
            else if(pitch < -89.9f) pitch = -89.9f;
            if(roll > 89.9f) roll = 89.9f;
            else if(roll < -89.9f) roll = -89.9f;
        }
        if(yaw < 0.0f) yaw = 360.0f - fmodf(-yaw, 360.0f);
        else if(yaw >= 360.0f) yaw = fmodf(yaw, 360.0f);
    }

    void fixrange(float &yaw, float &pitch)
    {
        float r = 0.f;
        fixfullrange(yaw, pitch, r, false);
    }

    void getyawpitch(const vec &from, const vec &pos, float &yaw, float &pitch)
    {
        float dist = from.dist(pos);
        yaw = -atan2(pos.x-from.x, pos.y-from.y)/RAD;
        pitch = dist > 0 ? asin((pos.z-from.z)/dist)/RAD : 0;
    }

    void scaleyawpitch(float &yaw, float &pitch, float targyaw, float targpitch, float frame, float scale)
    {
        if(yaw < targyaw-180.0f) yaw += 360.0f;
        if(yaw > targyaw+180.0f) yaw -= 360.0f;
        float offyaw = fabs(targyaw-yaw)*frame, offpitch = fabs(targpitch-pitch)*frame*scale;
        if(targyaw > yaw)
        {
            yaw += offyaw;
            if(targyaw < yaw) yaw = targyaw;
        }
        else if(targyaw < yaw)
        {
            yaw -= offyaw;
            if(targyaw > yaw) yaw = targyaw;
        }
        if(targpitch > pitch)
        {
            pitch += offpitch;
            if(targpitch < pitch) pitch = targpitch;
        }
        else if(targpitch < pitch)
        {
            pitch -= offpitch;
            if(targpitch > pitch) pitch = targpitch;
        }
        fixrange(yaw, pitch);
    }

    bool lockon(gameent *d, int atk, gameent *e, float maxdist)
    {
        if(!d->blocked && !d->timeinair)
        {
            vec dir = vec(e->o).sub(d->o);
            float xydist = dir.x*dir.x+dir.y*dir.y, zdist = dir.z*dir.z, mdist = maxdist*maxdist, ddist = d->radius*d->radius+e->radius*e->radius;
            if(zdist <= ddist && xydist >= ddist+4 && xydist <= mdist+ddist) return true;
        }
        return false;
    }

    int process(gameent *d, aistate &b)
    {
        int result = 0, stupify = d->skill <= 10+rnd(15) ? rnd(d->skill*1000) : 0, skmod = 101-d->skill;
        float frame = d->skill <= 100 ? float(lastmillis-d->ai->lastrun)/float(max(skmod,1)*10) : 1;
        vec dp = d->headpos();

        bool idle = b.idle == 1 || (stupify && stupify <= skmod);
        d->ai->dontmove = false;
        if(idle)
        {
            d->ai->lastaction = d->ai->lasthunt = lastmillis;
            d->ai->dontmove = true;
            d->ai->spot = vec(0, 0, 0);
        }
        else if(hunt(d, b))
        {
            getyawpitch(dp, vec(d->ai->spot).addz(d->eyeheight), d->ai->targyaw, d->ai->targpitch);
            d->ai->lasthunt = lastmillis;
        }
        else
        {
            idle = d->ai->dontmove = true;
            d->ai->spot = vec(0, 0, 0);
            d->crouching = -1;
        }

        if(d->aptisort3 && d->aptitude==APT_PHYSICIEN) switch(rnd(30)) {case 1: d->jumping = true;}
        else if(d->gunselect==GUN_ARTIFICE || d->gunselect==GUN_SMAW || d->gunselect==GUN_S_NUKE || d->gunselect==GUN_S_ROQUETTES || d->jointmillis) d->jumping = true;
        else if(d->aptitude==APT_NINJA) switch(rnd(30)) {case 1: d->jumping = true;}
		else if(!d->ai->dontmove) jumpto(d, b, d->ai->spot);

        gameent *e = getclient(d->ai->enemy);
        bool enemyok = e && targetable(d, e);

        if(enemyok && (d->aptitude==APT_KAMIKAZE || d->aptitude==APT_NINJA))
        {
            makeroute(d, b, e->o);
            if(d->aptisort2<2250 && d->aptisort2 && d->aptitude==APT_KAMIKAZE) d->gunselect=GUN_KAMIKAZE;
        }

        if(!enemyok || d->skill >= 30)
        {
            gameent *f = (gameent *)intersectclosest(dp, d->ai->target, d);
            if(f)
            {
                if(targetable(d, f))
                {
                    if(!enemyok) violence(d, b, f);

                    switch(d->aptitude)
                    {
                        case APT_PRETRE: case APT_SHOSHONE: if(d->mana>70 && d->o.dist(f->o)<750) aptitude(d, 3); break;
                        case APT_KAMIKAZE:
                            if(d->o.dist(f->o)<500) aptitude(d, 2);
                            break;
                        case APT_ESPION:
                            switch(rnd(2))
                            {
                                case 0: if(d->mana>=40 && d->o.dist(f->o)<700 && !d->aptisort2) aptitude(d, 1); break;
                                case 1: if(d->mana>=50 && d->o.dist(f->o)<700 && !d->aptisort1) aptitude(d, 2);
                            }
                            break;
                        case APT_NINJA: makeroute(d, b, f->o); break;
                    }
                    enemyok = true;
                    e = f;
                }
                else enemyok = false;
            }
            else if(!enemyok && target(d, b, 0, false, SIGHTMIN))
                enemyok = (e = getclient(d->ai->enemy)) != NULL;
        }
        if(enemyok)
        {
            int atk = guns[d->gunselect].attacks[ACT_SHOOT];
            vec ep = getaimpos(d, atk, e);
            float yaw, pitch;
            getyawpitch(dp, ep, yaw, pitch);
            fixrange(yaw, pitch);
            bool insight = cansee(d, dp, ep), hasseen = d->ai->enemyseen && lastmillis-d->ai->enemyseen <= (d->skill*150),
                quick = d->ai->enemyseen && lastmillis-d->ai->enemyseen <= skmod+30;
            if(insight) d->ai->enemyseen = lastmillis;
            if(idle || insight || hasseen || quick)
            {
                float sskew = insight || d->skill > 100 ? 1.5f : (hasseen ? 1.f : 0.5f);
                if(insight && lockon(d, atk, e, 16))
                {
                    d->ai->targyaw = yaw;
                    d->ai->targpitch = pitch;
                    if(!idle) frame *= 2;
                    d->ai->becareful = false;
                }
                scaleyawpitch(d->yaw, d->pitch, yaw, pitch, frame, sskew);
                if(insight || quick)
                {
                    if(canshoot(d, atk, e) && hastarget(d, atk, b, e, yaw, pitch, dp.squaredist(ep)))
                    {
                        d->attacking = attacks[atk].action;
                        d->ai->lastaction = lastmillis;
                        result = 3;
                    }
                    else result = 2;
                }
                else result = 1;
            }
            else
            {
                if(!d->ai->enemyseen || lastmillis-d->ai->enemyseen > (d->skill*50)+3000)
                {
                    d->ai->enemy = -1;
                    d->ai->enemyseen = d->ai->enemymillis = 0;
                }
                enemyok = false;
                result = 0;
            }
        }
        else
        {
            if(!enemyok)
            {
                d->ai->enemy = -1;
                d->ai->enemyseen = d->ai->enemymillis = 0;
            }
            enemyok = false;
            result = 0;
        }

        fixrange(d->ai->targyaw, d->ai->targpitch);
        if(!result) scaleyawpitch(d->yaw, d->pitch, d->ai->targyaw, d->ai->targpitch, frame*0.25f, 1.f);

        if(d->ai->becareful && d->physstate == PHYS_FALL)
        {
            float offyaw, offpitch;
            vec v = vec(d->vel).normalize();
            vectoyawpitch(v, offyaw, offpitch);
            offyaw -= d->yaw; offpitch -= d->pitch;
            if(fabs(offyaw)+fabs(offpitch) >= 135) d->ai->becareful = false;
            else if(d->ai->becareful) {d->crouching = -1 ; d->ai->dontmove = true;}
        }
        else d->ai->becareful = false;

        if(d->ai->dontmove) d->move = d->strafe = 0;
        else
        { // our guys move one way.. but turn another?! :)
            const struct aimdir { int move, strafe, offset; } aimdirs[8] =
            {
                {  1,  0,   0 },
                {  1,  -1,  45 },
                {  0,  -1,  90 },
                { -1,  -1, 135 },
                { -1,  0, 180 },
                { -1, 1, 225 },
                {  0, 1, 270 },
                {  1, 1, 315 }
            };
            float yaw = d->ai->targyaw-d->yaw;
            if(yaw < 0.0f) yaw = 360.0f - fmodf(-yaw, 360.0f);
            else if(yaw >= 360.0f) yaw = fmodf(yaw, 360.0f);
            int r = clamp(((int)floor((yaw+22.5f)/45.0f))&7, 0, 7);
            const aimdir &ad = aimdirs[r];
            d->move = ad.move;
            d->strafe = ad.strafe;
        }
        findorientation(dp, d->yaw, d->pitch, d->ai->target);
        return result;
    }

    bool hasrange(gameent *d, gameent *e, int weap)
    {
        if(!e) return true;
        if(targetable(d, e))
        {
            int atk = guns[weap].attacks[ACT_SHOOT];
            vec ep = getaimpos(d, atk, e);
            float dist = ep.squaredist(d->headpos());
            if(attackrange(d, atk, dist)) return true;
        }
        return false;
    }

    bool request(gameent *d, aistate &b)
    {
        gameent *e = getclient(d->ai->enemy);
        loopi(4) if(hasrange(d, e, GUN_S_NUKE+i) && d->hasammo(GUN_S_NUKE+i)){gunselect(GUN_S_NUKE+i, d); return process(d, b) >= 2;} //Super armes sélectionnées en priorité
        if(d->aptitude==APT_KAMIKAZE && hasrange(d, e, GUN_KAMIKAZE)) {gunselect(GUN_KAMIKAZE, d); return process(d, b) >= 2;}

        if(m_identique)
        {
            switch(d->aptitude)
            {
                case APT_KAMIKAZE:
                    if(hasrange(d, e, GUN_KAMIKAZE)) gunselect(GUN_KAMIKAZE, d);
                    else gunselect(cnidentiquearme, d);
                    break;
                case APT_NINJA:
                    if(hasrange(d, e, GUN_CACNINJA)) gunselect(GUN_CACNINJA, d);
                    else gunselect(cnidentiquearme, d);
                    break;
                default:
                    gunselect(cnidentiquearme, d);
            }
            return process(d, b) >= 2;
        }

        if(!d->hasammo(d->gunselect) || !hasrange(d, e, d->gunselect) || (d->gunselect != d->ai->weappref && (!isgoodammo(d->gunselect) || d->hasammo(d->ai->weappref))))
        {
            static const int gunprefs[] = {GUN_S_NUKE, GUN_S_CAMPOUZE, GUN_S_GAU8, GUN_S_ROQUETTES, GUN_KAMIKAZE, GUN_CACNINJA, GUN_CACFLEAU, GUN_CACMARTEAU, GUN_CACMASTER, GUN_CAC349, GUN_MINIGUN, GUN_PULSE, GUN_RAIL, GUN_LANCEFLAMMES, GUN_HYDRA, GUN_SV98, GUN_SMAW, GUN_ARBALETE, GUN_ARTIFICE, GUN_MOSSBERG, GUN_FAMAS, GUN_M32, GUN_SKS, GUN_SPOCKGUN, GUN_UZI };

            int gun = -1;
            if(d->hasammo(d->ai->weappref) && hasrange(d, e, d->ai->weappref)) gun = d->ai->weappref;
            else
            {
                loopi(sizeof(gunprefs)/sizeof(gunprefs[0])) if(d->hasammo(gunprefs[i]) && hasrange(d, e, gunprefs[i]))
                {
                    gun = gunprefs[i];
                    break;
                }
            }
            if(gun >= 0 && gun != d->gunselect) gunselect(gun, d);
        }
        return process(d, b) >= 2;
    }

	void timeouts(gameent *d, aistate &b)
	{
        if(d->blocked)
        {
            d->ai->blocktime += lastmillis-d->ai->lastrun;
            if(d->ai->blocktime > (d->ai->blockseq+1)*1000)
            {
                d->ai->blockseq++;
                switch(d->ai->blockseq)
                {
                    case 1:
                        d->ai->reset(true); break;
                        d->crouching = -1;
                        break;
                    case 2: case 3:
                        if(entities::ents.inrange(d->ai->targnode)) d->ai->addprevnode(d->ai->targnode);
                        d->ai->clear(false);
                        d->crouching = -1;
                        break;
                    case 4: d->ai->reset(true); break;
                    case 5:
                        d->ai->reset(false);
                        d->crouching = -1;
                        break;
                    case 6: default: suicide(d); return; break; // this is our last resort..
                }
            }
        }
        else d->ai->blocktime = d->ai->blockseq = 0;

        if(d->ai->targnode == d->ai->targlast)
        {
            d->ai->targtime += lastmillis-d->ai->lastrun;
            if(d->ai->targtime > (d->ai->targseq+1)*1000)
            {
                d->ai->targseq++;
                switch(d->ai->targseq)
                {
                    case 1: case 2: case 3:
                        if(entities::ents.inrange(d->ai->targnode)) d->ai->addprevnode(d->ai->targnode);
                        d->ai->clear(false);
                        break;
                    case 4: d->ai->reset(true); break;
                    case 5: d->ai->reset(false); break;
                    case 6: default: suicide(d); return; break; // this is our last resort..
                }
            }
        }
        else
        {
            d->ai->targtime = d->ai->targseq = 0;
            d->ai->targlast = d->ai->targnode;
        }

        if(d->ai->lasthunt)
        {
            int millis = lastmillis-d->ai->lasthunt;
            if(millis <= 1000) { d->ai->tryreset = false; d->ai->huntseq = 0; }
            else if(millis > (d->ai->huntseq+1)*1000)
            {
                d->ai->huntseq++;
                switch(d->ai->huntseq)
                {
                    case 1: d->ai->reset(true); break;
                    case 2: d->ai->reset(false); break;
                    case 3: default: suicide(d); return; break; // this is our last resort..
                }
            }
        }
	}

    void logic(gameent *d, aistate &b, bool run)
    {
        bool allowmove = canmove(d) && b.type != AI_S_WAIT;
        if(d->state != CS_ALIVE || !allowmove) d->stopmoving();
        if(d->state == CS_ALIVE)
        {
            if(allowmove)
            {
                if(!request(d, b)) target(d, b, 0, b.idle ? true : false);
                shoot(d, d->ai->target);
            }
            if(!intermission)
            {
                if(d->ragdoll) cleanragdoll(d); // RAGRAG
                moveplayer(d, 10, true, d->epomillis, d->jointmillis, d->aptitude, d->aptitude==APT_MAGICIEN ? d->aptisort1 : d->aptitude==APT_SHOSHONE || d->aptitude==APT_ESPION || d->aptitude==APT_KAMIKAZE ? d->aptisort2 : d->aptisort3, d->armourtype==A_ASSIST && d->armour>0 ? true : false);
                if(allowmove && !b.idle) timeouts(d, b);
				entities::checkitems(d);
				if(cmode) cmode->checkitems(d);
            }
        }
        else if(d->state == CS_DEAD)
        {
            if(d->ragdoll) moveragdoll(d); // RAGRAG
            else
            {
                d->move = d->strafe = 0;
                moveplayer(d, 10, false, 0, 0, 0, 0);
            }
        }
        d->attacking = d->jumping = false;
    }

	void avoid()
    {
        // guess as to the radius of ai and other critters relying on the avoid set for now
        float guessradius = player1->radius;
        obstacles.clear();
        loopv(players)
        {
            dynent *d = players[i];
            if(d->state != CS_ALIVE) continue;
            obstacles.avoidnear(d, d->o.z + d->aboveeye + 1, d->feetpos(), guessradius + d->radius);
        }
		extern avoidset wpavoid;
		obstacles.add(wpavoid);
		avoidweapons(obstacles, guessradius);
    }

    void think(gameent *d, bool run)
    {
        // the state stack works like a chain of commands, certain commands simply replace each other
        // others spawn new commands to the stack the ai reads the top command from the stack and executes
        // it or pops the stack and goes back along the history until it finds a suitable command to execute
        bool cleannext = false;
        if(d->ai->state.empty()) d->ai->addstate(AI_T_NODE);
        loopvrev(d->ai->state)
        {
            aistate &c = d->ai->state[i];

            if(cleannext)
            {
                c.millis = lastmillis;
                c.override = false;
                cleannext = false;
            }
            if(d->state == CS_DEAD && d->respawned!=d->lifesequence && (!cmode || cmode->respawnwait(d) <= 0) && lastmillis - d->lastpain >= 2500+rnd(1000))
            {
                addmsg(N_TRYSPAWN, "rc", d);
                d->respawned = d->lifesequence;
            }
            else if(d->state == CS_ALIVE && run)
            {
                int result = 0;
                c.idle = 0;

                switch(d->aptitude)
                {
                    case APT_MAGICIEN: if(d->health<250+d->skill*2 && d->mana>=60) aptitude(d, 3); break;
                    case APT_PRETRE: if(d->mana>30 && d->health<200+d->skill) aptitude(d, 2); break;
                    case APT_SHOSHONE: if(d->mana>=100) aptitude(d, 2); break;
                    case APT_PHYSICIEN:
                        switch(rnd(100)) {case 0: if(d->mana>70) aptitude(d, 3);}
                        if(d->health<400+d->skill && d->mana>=50) aptitude(d, 2);
                        if(d->health<700 && d->armour<50 && d->mana>=25) aptitude(d, 1);
                        break;
                    case APT_ESPION:
                        if(d->mana>100) aptitude(d, 3);
                }

                switch(rnd(150)){case 0: bottaunt(d);}

                switch(c.type)
                {
                    case AI_S_WAIT:
                        result = dowait(d, c);
                        break;
                    //case AI_S_DEFEND: result = dodefend(d, c);
                    //{
                    //    if(d->aptitude==APT_SHOSHONE && d->mana>99) aptitude(d, 2);
                    //    break;
                    //}
                    case AI_S_PURSUE: result = dopursue(d, c); break;
                    case AI_S_INTEREST: result = dointerest(d, c); break;
                    default: result = 0; break;
                }
                if(result <= 0)
                {
                    if(c.type != AI_S_WAIT)
                    {
                        switch(result)
                        {
                            case 0: default: d->ai->removestate(i); cleannext = true; break;
                            case -1: i = d->ai->state.length()-1; break;
                        }
                        continue; // shouldn't interfere
                    }
                }
            }
            logic(d, c, run);
            break;
        }
        if(d->ai->trywipe) d->ai->wipe();
        d->ai->lastrun = lastmillis;
    }

    void drawroute(gameent *d, float amt = 1.f)
    {
        int last = -1;
        loopvrev(d->ai->route)
        {
            if(d->ai->route.inrange(last))
            {
                int index = d->ai->route[i], prev = d->ai->route[last];
                if(iswaypoint(index) && iswaypoint(prev))
                {
                    waypoint &e = waypoints[index], &f = waypoints[prev];
                    vec fr = f.o, dr = e.o;
                    fr.z += amt; dr.z += amt;
                    particle_flare(fr, dr, 1, PART_STREAK, 0xFFFFFF);
                }
            }
            last = i;
        }
        if(aidebug >= 5)
        {
            vec pos = d->feetpos();
            if(d->ai->spot != vec(0, 0, 0)) particle_flare(pos, d->ai->spot, 1, PART_LIGHTNING, 0x00FFFF);
            if(iswaypoint(d->ai->targnode))
                particle_flare(pos, waypoints[d->ai->targnode].o, 1, PART_LIGHTNING, 0xFF00FF);
            if(iswaypoint(d->lastnode))
                particle_flare(pos, waypoints[d->lastnode].o, 1, PART_LIGHTNING, 0xFFFF00);
            loopi(NUMPREVNODES) if(iswaypoint(d->ai->prevnodes[i]))
            {
                particle_flare(pos, waypoints[d->ai->prevnodes[i]].o, 1, PART_LIGHTNING, 0x884400);
                pos = waypoints[d->ai->prevnodes[i]].o;
            }
        }
    }

    VAR(showwaypoints, 0, 0, 1);
    VAR(showwaypointsradius, 0, 200, 10000);

    const char *stnames[AI_S_MAX] = {
        "wait", "defend", "pursue", "interest"
    }, *sttypes[AI_T_MAX+1] = {
        "none", "node", "player", "affinity", "entity"
    };
    void render()
    {
        if(aidebug > 1)
        {
            int total = 0, alive = 0;
            loopv(players) if(players[i]->ai) total++;
            loopv(players) if(players[i]->state == CS_ALIVE && players[i]->ai)
            {
                gameent *d = players[i];
                vec pos = d->abovehead();
                pos.z += 3;
                alive++;
                if(aidebug >= 4) drawroute(d, 4.f*(float(alive)/float(total)));
                if(aidebug >= 3)
                {
                    defformatstring(q, "node: %d route: %d (%d)",
                        d->lastnode,
                        !d->ai->route.empty() ? d->ai->route[0] : -1,
                        d->ai->route.length()
                    );
                    particle_textcopy(pos, q, PART_TEXT, 1);
                    pos.z += 2;
                }
                bool top = true;
                loopvrev(d->ai->state)
                {
                    aistate &b = d->ai->state[i];
                    defformatstring(s, "%s%s (%d ms) %s:%d",
                        top ? "\fg" : "\fy",
                        stnames[b.type],
                        lastmillis-b.millis,
                        sttypes[b.targtype+1], b.target
                    );
                    particle_textcopy(pos, s, PART_TEXT, 1);
                    pos.z += 2;
                    if(top)
                    {
                        if(aidebug >= 3) top = false;
                        else break;
                    }
                }
                if(aidebug >= 3)
                {
                    if(d->ai->weappref >= 0 && d->ai->weappref < NUMGUNS)
                    {
                        particle_textcopy(pos, guns[d->ai->weappref].name, PART_TEXT, 1);
                        pos.z += 2;
                    }
                    gameent *e = getclient(d->ai->enemy);
                    if(e)
                    {
                        particle_textcopy(pos, colorname(e), PART_TEXT, 1);
                        pos.z += 2;
                    }
                }
            }
            if(aidebug >= 4)
            {
                int cur = 0;
                loopv(obstacles.obstacles)
                {
                    const avoidset::obstacle &ob = obstacles.obstacles[i];
                    int next = cur + ob.numwaypoints;
                    for(; cur < next; cur++)
                    {
                        int ent = obstacles.waypoints[cur];
                        if(iswaypoint(ent))
                            regular_particle_splash(PART_EDIT, 2, 40, waypoints[ent].o, 0xFF6600, 1.5f);
                    }
                    cur = next;
                }
            }
        }
        if(showwaypoints || aidebug >= 6)
        {
            vector<int> close;
            int len = waypoints.length();
            if(showwaypointsradius)
            {
                findwaypointswithin(camera1->o, 0, showwaypointsradius, close);
                len = close.length();
            }
            loopi(len)
            {
                waypoint &w = waypoints[showwaypointsradius ? close[i] : i];
                loopj(MAXWAYPOINTLINKS)
                {
                     int link = w.links[j];
                     if(!link) break;
                     particle_flare(w.o, waypoints[link].o, 1, PART_STREAK, 0x0000FF);
                }
            }

        }
    }
}
