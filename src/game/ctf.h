#ifndef PARSEMESSAGES

#ifdef SERVMODE
VAR(ctftkpenalty, 0, 1, 1);

struct ctfservmode : servmode
#else
struct ctfclientmode : clientmode
#endif
{
    static const int MAXFLAGS = 20;
    static const int FLAGRADIUS = 20;
    static const int FLAGLIMIT = 10;
    static const int RESPAWNSECS = 7;

    struct flag
    {
        int id, version;
        vec droploc, spawnloc;
        int team, droptime, owntime;
#ifdef SERVMODE
        int owner, dropcount, dropper;
#else
        gameent *owner;
        float dropangle, spawnangle;
        vec interploc;
        float interpangle;
        int interptime;
#endif

        flag() : id(-1) { reset(); }

        void reset()
        {
            version = 0;
            droploc = spawnloc = vec(0, 0, 0);
#ifdef SERVMODE
            dropcount = 0;
            owner = dropper = -1;
            owntime = 0;
#else
            if(id >= 0) loopv(players) players[i]->flagpickup &= ~(1<<id);
            owner = NULL;
            dropangle = spawnangle = 0;
            interploc = vec(0, 0, 0);
            interpangle = 0;
            interptime = 0;
#endif
            team = 0;
            droptime = owntime = 0;
        }

#ifndef SERVMODE
        vec pos() const
        {
            if(owner) return vec(owner->o).sub(owner->eyeheight);
            if(droptime) return droploc;
            return spawnloc;
        }
#endif
    };

    vector<flag> flags;
    int scores[MAXTEAMS];

    void resetflags()
    {
        flags.shrink(0);
        loopk(MAXTEAMS) scores[k] = 0;
    }

#ifdef SERVMODE
    bool addflag(int i, const vec &o, int team)
#else
    bool addflag(int i, const vec &o, int team)
#endif
    {
        if(i<0 || i>=MAXFLAGS) return false;
        while(flags.length()<=i) flags.add();
        flag &f = flags[i];
        f.id = i;
        f.reset();
        f.team = team;
        f.spawnloc = o;
        return true;
    }

#ifdef SERVMODE
    void ownflag(int i, int owner, int owntime)
#else
    void ownflag(int i, gameent *owner, int owntime)
#endif
    {
        flag &f = flags[i];
        f.owner = owner;
        f.owntime = owntime;
#ifdef SERVMODE
        if(owner == f.dropper) { if(f.dropcount < INT_MAX) f.dropcount++; }
        else f.dropcount = 0;
        f.dropper = -1;
#else
        loopv(players) players[i]->flagpickup &= ~(1<<f.id);
#endif
    }

#ifdef SERVMODE
    void dropflag(int i, const vec &o, int droptime, int dropper = -1, bool penalty = false)
#else
    void dropflag(int i, const vec &o, float yaw, int droptime)
#endif
    {
        flag &f = flags[i];
        f.droploc = o;
        f.droptime = droptime;
#ifdef SERVMODE
        if(dropper < 0) f.dropcount = 0;
        else if(penalty) f.dropcount = INT_MAX;
        f.dropper = dropper;
        f.owner = -1;
#else
        loopv(players) players[i]->flagpickup &= ~(1<<f.id);
        f.owner = NULL;
        f.dropangle = yaw;
#endif
    }

#ifdef SERVMODE
    void returnflag(int i)
#else
    void returnflag(int i)
#endif
    {
        flag &f = flags[i];
        f.droptime = 0;
#ifdef SERVMODE
        f.dropcount = 0;
        f.owner = f.dropper = -1;
#else
        loopv(players) players[i]->flagpickup &= ~(1<<f.id);
        f.owner = NULL;
#endif
    }

    int totalscore(int team)
    {
        return validteam(team) ? scores[team-1] : 0;
    }

    int setscore(int team, int score)
    {
        if(validteam(team)) return scores[team-1] = score;
        return 0;
    }

    int addscore(int team, int score)
    {
        if(validteam(team)) return scores[team-1] += score;
        return 0;
    }

    bool hidefrags() { return true; }

    int getteamscore(int team)
    {
        return totalscore(team);
    }

    void getteamscores(vector<teamscore> &tscores)
    {
        loopk(MAXTEAMS) if(scores[k]) tscores.add(teamscore(k+1, scores[k]));
    }

#ifdef SERVMODE
    static const int RESETFLAGTIME = 10000;

    bool notgotflags;

    ctfservmode() : notgotflags(false) {}

    void reset(bool empty)
    {
        resetflags();
        notgotflags = !empty;
    }

    void cleanup()
    {
        reset(false);
    }

    void setup()
    {
        reset(false);
        if(notgotitems || ments.empty()) return;
        loopv(ments)
        {
            entity &e = ments[i];
            if(e.type != FLAG || !validteam(e.attr2)) continue;
            if(!addflag(flags.length(), e.o, e.attr2)) break;
        }
        notgotflags = false;
    }

    void newmap()
    {
        reset(true);
    }

    void dropflag(clientinfo *ci, clientinfo *dropper = NULL)
    {
        if(notgotflags) return;
        loopv(flags) if(flags[i].owner==ci->clientnum)
        {
            flag &f = flags[i];
            ivec o(vec(ci->state.o).mul(DMF));
            sendf(-1, 1, "ri7", N_DROPFLAG, ci->clientnum, i, ++f.version, o.x, o.y, o.z);
            dropflag(i, vec(o).div(DMF), lastmillis, dropper ? dropper->clientnum : ci->clientnum, dropper && dropper!=ci);
        }
    }

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
        dropflag(ci);
        loopv(flags) if(flags[i].dropper == ci->clientnum) { flags[i].dropper = -1; flags[i].dropcount = 0; }
    }

    void died(clientinfo *ci, clientinfo *actor)
    {
        dropflag(ci, ctftkpenalty && actor && actor != ci && isteam(actor->team, ci->team) ? actor : NULL);
        loopv(flags) if(flags[i].dropper == ci->clientnum) { flags[i].dropper = -1; flags[i].dropcount = 0; }
    }

    bool canspawn(clientinfo *ci, bool connecting)
    {
        return connecting || !ci->state.lastdeath || gamemillis+curtime-ci->state.lastdeath >= RESPAWNSECS*1000;
    }

    bool canchangeteam(clientinfo *ci, int oldteam, int newteam)
    {
        return true;
    }

    void changeteam(clientinfo *ci, int oldteam, int newteam)
    {
        dropflag(ci);
    }

    void scoreflag(clientinfo *ci, int goal, int relay = -1)
    {
        returnflag(relay >= 0 ? relay : goal);
        ci->state.flags++;
        int team = ci->team, score = addscore(team, 1);
        sendf(-1, 1, "ri9", N_SCOREFLAG, ci->clientnum, relay, relay >= 0 ? ++flags[relay].version : -1, goal, ++flags[goal].version, team, score, ci->state.flags);
        if(score >= FLAGLIMIT) startintermission();
    }

    void takeflag(clientinfo *ci, int i, int version)
    {
        if(notgotflags || !flags.inrange(i) || ci->state.state!=CS_ALIVE || !ci->team) return;
        flag &f = flags[i];
        if(!validteam(f.team) || f.owner>=0 || f.version != version || (f.droptime && f.dropper == ci->clientnum && f.dropcount >= 3)) return;
        if(f.team!=ci->team)
        {
            loopvj(flags) if(flags[j].owner==ci->clientnum) return;
            ownflag(i, ci->clientnum, lastmillis);
            sendf(-1, 1, "ri4", N_TAKEFLAG, ci->clientnum, i, ++f.version);
        }
        else if(f.droptime)
        {
            returnflag(i);
            sendf(-1, 1, "ri4", N_RETURNFLAG, ci->clientnum, i, ++f.version);
        }
        else
        {
            loopvj(flags) if(flags[j].owner==ci->clientnum) { scoreflag(ci, i, j); break; }
        }
    }

    void update()
    {
        if(gamemillis>=gamelimit || notgotflags) return;
        loopv(flags)
        {
            flag &f = flags[i];
            if(f.owner<0 && f.droptime && lastmillis - f.droptime >= RESETFLAGTIME)
            {
                returnflag(i);
                sendf(-1, 1, "ri3", N_RESETFLAG, i, ++f.version);
            }
        }
    }

    void initclient(clientinfo *ci, packetbuf &p, bool connecting)
    {
        putint(p, N_INITFLAGS);
        loopk(2) putint(p, scores[k]);
        putint(p, flags.length());
        loopv(flags)
        {
            flag &f = flags[i];
            putint(p, f.version);
            putint(p, f.owner);
            if(f.owner<0)
            {
                putint(p, f.droptime ? 1 : 0);
                if(f.droptime)
                {
                    putint(p, int(f.droploc.x*DMF));
                    putint(p, int(f.droploc.y*DMF));
                    putint(p, int(f.droploc.z*DMF));
                }
            }
        }
    }

    void parseflags(ucharbuf &p, bool commit)
    {
        int numflags = getint(p);
        loopi(numflags)
        {
            int team = getint(p);
            vec o;
            loopk(3) o[k] = max(getint(p)/DMF, 0.0f);
            if(p.overread()) break;
            if(commit && notgotflags)
            {
                addflag(i, o, team);
            }
        }
        if(commit && notgotflags)
        {
            notgotflags = false;
        }
    }
};
#else
    #define FLAGCENTER 3.5f
    #define FLAGFLOAT 7

    void preload()
    {
        preloadmodel("drapeau/jaune");
        preloadmodel("drapeau/rouge");
        for(int i = S_DRAPEAUPRIS; i <= S_DRAPEAUTOMBE; i++) preloadsound(i);
    }

    void drawblip(gameent *d, float x, float y, float s, const vec &pos, bool flagblip)
    {
        float scale = calcradarscale();
        vec dir = d->o;
        dir.sub(pos).div(scale);
        float size = flagblip ? 0.1f : 0.05f,
              xoffset = flagblip ? -2*(3/32.0f)*size : -size,
              yoffset = flagblip ? -2*(1 - 3/32.0f)*size : -size,
              dist = dir.magnitude2(), maxdist = 1 - 0.05f - 0.05f;
        if(dist >= maxdist) dir.mul(maxdist/dist);
        dir.rotate_around_z(camera1->yaw*-RAD);
        drawradar(x + s*0.5f*(1.0f + dir.x + xoffset), y + s*0.5f*(1.0f + dir.y + yoffset), size*s);
    }

    void drawblip(gameent *d, float x, float y, float s, int i, bool flagblip)
    {
        flag &f = flags[i];
        setbliptex(player1->team!=f.team ? 2 : 1, flagblip ? "_flag" : "");
        drawblip(d, x, y, s, flagblip ? (f.owner ? f.owner->o : (f.droptime ? f.droploc : f.spawnloc)) : f.spawnloc, flagblip);
    }

    float clipconsole(float w, float h)
    {
        return (h*(1 + 1 + 10))/(4*10);
    }

    void drawhud(gameent *d, int w, int h)
    {
        loopv(flags)
        {
            if(flags[i].owner == player1) {settexture("media/interface/hud/drapeau_ennemi.png"); bgquad(w-130, h-260, 115, 115);}
        }

        pushhudscale(h/1800.0f);
        pushhudscale(2);
        pophudmatrix();
        resethudshader();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        int s = 1800/4, x = 1800*w/h - s - s/10, y = s/10;

        gle::colorf(1, 1, 1, minimapalpha);
        if(minimapalpha >= 1) glDisable(GL_BLEND);
        bindminimap();
        drawminimap(d, x, y, s);
        if(minimapalpha >= 1) glEnable(GL_BLEND);
        gle::colorf(1, 1, 1);
        float margin = 0.04f, roffset = s*margin, rsize = s + 2*roffset;
        setradartex();
        drawradar(x - roffset, y - roffset, rsize);
        settexture("media/interface/hud/boussole.png", 3);
        pushhudmatrix();
        hudmatrix.translate(x - roffset + 0.5f*rsize, y - roffset + 0.5f*rsize, 0);
        hudmatrix.rotate_around_z((camera1->yaw + 180)*-RAD);
        flushhudmatrix();
        drawradar(-0.5f*rsize, -0.5f*rsize, rsize);
        pophudmatrix();
        drawplayerblip(d, x, y, s, 1.5f);
        loopv(flags)
        {
            flag &f = flags[i];
            if(!validteam(f.team)) continue;
            if(f.owner)
            {
                if(lastmillis%1000 >= 500) continue;
            }
            else if(f.droptime && (f.droploc.x < 0 || lastmillis%300 >= 150)) continue;
            drawblip(d, x, y, s, i, true);
        }
        drawteammates(d, x, y, s);
    }

    void removeplayer(gameent *d)
    {
        loopv(flags) if(flags[i].owner == d)
        {
            flag &f = flags[i];
            f.interploc.x = -1;
            f.interptime = 0;
            dropflag(i, f.owner->o, f.owner->yaw, 1);
        }
    }

    vec interpflagpos(flag &f, float &angle)
    {
        vec pos = f.owner ? vec(f.owner->abovehead()).addz(1) : (f.droptime ? f.droploc : f.spawnloc);
        if(f.owner) angle = f.owner->yaw;
        else { angle = f.droptime ? f.dropangle : f.spawnangle; pos.addz(FLAGFLOAT); }
        if(pos.x < 0) return pos;
        pos.addz(FLAGCENTER);
        if(f.interptime && f.interploc.x >= 0)
        {
            float t = min((lastmillis - f.interptime)/500.0f, 1.0f);
            pos.lerp(f.interploc, pos, t);
            angle += (1-t)*(f.interpangle - angle);
        }
        return pos;
    }

    vec interpflagpos(flag &f) { float angle; return interpflagpos(f, angle); }

    void rendergame()
    {
        loopv(flags)
        {
            flag &f = flags[i];
            if(!f.owner && f.droptime && f.droploc.x < 0) continue;
            const char *flagname = player1->team==f.team ? "drapeau/jaune" : "drapeau/rouge";
            float angle;
            vec pos = interpflagpos(f, angle);
            rendermodel(flagname, ANIM_MAPMODEL|ANIM_LOOP,
                        pos, angle, 0, 0,
                        MDL_CULL_VFC | MDL_CULL_OCCLUDED);
        }
    }

    void setup()
    {
        resetflags();
        loopv(entities::ents)
        {
            extentity *e = entities::ents[i];
            if(e->type!=FLAG || !validteam(e->attr2)) continue;
            int index = flags.length();
            if(!addflag(index, e->o, e->attr2)) continue;
            flags[index].spawnangle = e->attr1;
        }
    }

    void senditems(packetbuf &p)
    {
        putint(p, N_INITFLAGS);
        putint(p, flags.length());
        loopv(flags)
        {
            flag &f = flags[i];
            putint(p, f.team);
            loopk(3) putint(p, int(f.spawnloc[k]*DMF));
        }
    }

    void parseflags(ucharbuf &p, bool commit)
    {
        loopk(2)
        {
            int score = getint(p);
            if(commit) scores[k] = score;
        }
        int numflags = getint(p);
        loopi(numflags)
        {
            int version = getint(p), owner = getint(p), dropped = 0;
            vec droploc(0, 0, 0);
            if(owner<0)
            {
                dropped = getint(p);
                if(dropped) loopk(3) droploc[k] = getint(p)/DMF;
            }
            if(p.overread()) break;
            if(commit && flags.inrange(i))
            {
                flag &f = flags[i];
                f.version = version;
                f.owner = owner>=0 ? (owner==player1->clientnum ? player1 : newclient(owner)) : NULL;
                f.owntime = owner>=0 ? lastmillis : 0;
                f.droptime = dropped ? lastmillis : 0;
                f.droploc = dropped ? droploc : f.spawnloc;
                f.interptime = 0;

                if(dropped && !droptofloor(f.droploc.addz(4), 4, 0)) f.droploc = vec(-1, -1, -1);
            }
        }
    }

    void trydropflag()
    {
        if(!m_ctf) return;
        loopv(flags) if(flags[i].owner == player1)
        {
            addmsg(N_TRYDROPFLAG, "rc", player1);
            return;
        }
    }

    const char *teamcolorflag(flag &f)
    {
        return teamcolor(f.team);
    }

    void dropflag(gameent *d, int i, int version, const vec &droploc)
    {
        if(!flags.inrange(i)) return;
        flag &f = flags[i];
        f.version = version;
        f.interploc = interpflagpos(f, f.interpangle);
        f.interptime = lastmillis;
        dropflag(i, droploc, d->yaw, lastmillis);
        d->flagpickup |= 1<<f.id;
        if(!droptofloor(f.droploc.addz(4), 4, 0))
        {
            f.droploc = vec(-1, -1, -1);
            f.interptime = 0;
        }
        conoutf(CON_GAMEINFO, "%s\f7 %s perdu le drapeau %s", teamcolorname(d), d==player1 ? "as" : "a", teamcolorflag(f));
        playsound(S_DRAPEAUTOMBE);
    }

    void flagexplosion(int i, int team, const vec &loc)
    {
        int fcolor;
        vec color;
        if(player1->team==team) { fcolor = 0xFF0000; color = vec(0.25f, 0.25f, 1); }
        else { fcolor = 0xFFFF22; color = vec(1, 0.25f, 0.25f); }
        particle_fireball(loc, 30, PART_EXPLOSION, -1, fcolor, 4.8f);
        adddynlight(loc, 35, color, 900, 100);
        particle_splash(PART_SPARK, 150, 300, loc, fcolor, 0.24f);
    }

    void flageffect(int i, int team, const vec &from, const vec &to)
    {
        if(from.x >= 0)
            flagexplosion(i, team, from);
        if(from==to) return;
        if(to.x >= 0)
            flagexplosion(i, team, to);
        if(from.x >= 0 && to.x >= 0)
            particle_flare(from, to, 600, PART_LIGHTNING, player1->team==team ? 0xFF2222 : 0xFFFF00, 1.0f);
    }

    void returnflag(gameent *d, int i, int version)
    {
        if(!flags.inrange(i)) return;
        flag &f = flags[i];
        f.version = version;
        flageffect(i, f.team, interpflagpos(f), vec(f.spawnloc).addz(FLAGFLOAT+FLAGCENTER));
        f.interptime = 0;
        returnflag(i);
        conoutf(CON_GAMEINFO, "%s\f7 %s r�cup�r� le drapeau %s", teamcolorname(d), d==player1 ? "as" : "a", teamcolorflag(f));
        d->team==player1->team ? ctfmessage3=totalmillis : ctfmessage4=totalmillis;
        if(d==player1) addxpandcc(10, 3);
        playsound(S_DRAPEAURESET);
    }

    void resetflag(int i, int version)
    {
        if(!flags.inrange(i)) return;
        flag &f = flags[i];
        f.version = version;
        flageffect(i, f.team, interpflagpos(f), vec(f.spawnloc).addz(FLAGFLOAT+FLAGCENTER));
        f.interptime = 0;
        returnflag(i);
        conoutf(CON_GAMEINFO, "Le drapeau %s\f7 a �t� replac�.", teamcolorflag(f));
        playsound(S_DRAPEAURESET);
    }

    void scoreflag(gameent *d, int relay, int relayversion, int goal, int goalversion, int team, int score, int dflags)
    {
        setscore(team, score);
        if(flags.inrange(goal))
        {
            flag &f = flags[goal];
            f.version = goalversion;
            if(relay >= 0)
            {
                flags[relay].version = relayversion;
                flageffect(goal, team, vec(f.spawnloc).addz(FLAGFLOAT+FLAGCENTER), vec(flags[relay].spawnloc).addz(FLAGFLOAT+FLAGCENTER));
            }
            else flageffect(goal, team, interpflagpos(f), vec(f.spawnloc).addz(FLAGFLOAT+FLAGCENTER));
            f.interptime = 0;
            returnflag(relay >= 0 ? relay : goal);
            d->flagpickup &= ~(1<<f.id);
            if(d->feetpos().dist(f.spawnloc) < FLAGRADIUS) d->flagpickup |= 1<<f.id;
        }
        if(d!=player1) particle_textcopy(d->abovehead(), tempformatstring("%d", score), PART_TEXT, 2000, 0x32FF64, 4.0f, -8);
        d->flags = dflags;
        conoutf(CON_GAMEINFO, "%s\f7 %s marqu� un point pour l'�quipe %s !", teamcolorname(d), d==player1 ? "as" : "a", teamcolor(team));
        team==player1->team ? ctfmessage1=totalmillis : ctfmessage2=totalmillis;
        if(d==player1) addxpandcc(20, 10);

        playsound(team==player1->team ? S_DRAPEAUSCORE : S_DRAPEAUTOMBE);

        if(score >= FLAGLIMIT) conoutf(CON_GAMEINFO, "%s\f7 a gagn� la partie !", teamcolor(team));
    }

    void takeflag(gameent *d, int i, int version)
    {
        if(!flags.inrange(i)) return;
        flag &f = flags[i];
        f.version = version;
        f.interploc = interpflagpos(f, f.interpangle);
        f.interptime = lastmillis;
        conoutf(CON_GAMEINFO, "%s\f7 %s vol� le drapeau %s", teamcolorname(d), d==player1 ? "as" : "a", teamcolorflag(f));
        d->team==player1->team ? ctfmessage5=totalmillis : ctfmessage6=totalmillis;
        if(d==player1) {addstat(1, STAT_DRAPEAUX); addxpandcc(5, 2);}
        ownflag(i, d, lastmillis);
        playsound(S_DRAPEAUPRIS);
    }

    void checkitems(gameent *d)
    {
        if(d->state!=CS_ALIVE) return;
        vec o = d->feetpos();
        bool tookflag = false;
        loopv(flags)
        {
            flag &f = flags[i];
            if(!validteam(f.team) || f.team==player1->team || f.owner || (f.droptime && f.droploc.x<0)) continue;
            const vec &loc = f.droptime ? f.droploc : f.spawnloc;
            if(o.dist(loc) < FLAGRADIUS)
            {
                if(d->flagpickup&(1<<f.id)) continue;
                if((lookupmaterial(o)&MATF_CLIP) != MAT_GAMECLIP && (lookupmaterial(loc)&MATF_CLIP) != MAT_GAMECLIP)
                {
                    tookflag = true;
                    addmsg(N_TAKEFLAG, "rcii", d, i, f.version);
                }
                d->flagpickup |= 1<<f.id;
            }
            else d->flagpickup &= ~(1<<f.id);
        }
        loopv(flags)
        {
            flag &f = flags[i];
            if(!validteam(f.team) || f.team!=player1->team || f.owner || (f.droptime && f.droploc.x<0)) continue;
            const vec &loc = f.droptime ? f.droploc : f.spawnloc;
            if(o.dist(loc) < FLAGRADIUS)
            {
                if(!tookflag && d->flagpickup&(1<<f.id)) continue;
                if((lookupmaterial(o)&MATF_CLIP) != MAT_GAMECLIP && (lookupmaterial(loc)&MATF_CLIP) != MAT_GAMECLIP)
                    addmsg(N_TAKEFLAG, "rcii", d, i, f.version);
                d->flagpickup |= 1<<f.id;
            }
            else d->flagpickup &= ~(1<<f.id);
       }
    }

    void respawned(gameent *d)
    {
        vec o = d->feetpos();
        d->flagpickup = 0;
        loopv(flags)
        {
            flag &f = flags[i];
            if(!validteam(f.team) || f.owner || (f.droptime && f.droploc.x<0)) continue;
            if(o.dist(f.droptime ? f.droploc : f.spawnloc) < FLAGRADIUS) d->flagpickup |= 1<<f.id;
       }
    }

    int respawnwait(gameent *d)
    {
        return max(0, RESPAWNSECS-(lastmillis-d->lastpain)/1000);
    }

	bool aihomerun(gameent *d, ai::aistate &b)
	{
        vec pos = d->feetpos();
        loopk(2)
        {
            int goal = 2000;
            loopv(flags)
            {
                flag &g = flags[i];
                if(g.team == d->team && (k || (!g.owner && !g.droptime)) &&
                    (!flags.inrange(goal) || g.pos().squaredist(pos) < flags[goal].pos().squaredist(pos)))
                {
                    goal = i;
                }
            }
            if(flags.inrange(goal) && ai::makeroute(d, b, flags[goal].pos()))
            {
                d->ai->switchstate(b, ai::AI_S_PURSUE, ai::AI_T_AFFINITY, goal);
                return true;
            }
        }
	    if(b.type == ai::AI_S_INTEREST && b.targtype == ai::AI_T_NODE) return true; // we already did this..
		if(randomnode(d, b, ai::SIGHTMIN, 1e16f))
		{
            d->ai->switchstate(b, ai::AI_S_INTEREST, ai::AI_T_NODE, d->ai->route[0]);
            return true;
		}
		return false;
	}

	bool aicheck(gameent *d, ai::aistate &b)
	{
        static vector<int> takenflags;
        takenflags.setsize(0);
        loopv(flags)
        {
            flag &g = flags[i];
            if(g.owner == d) return aihomerun(d, b);
            else if(g.team == d->team && ((g.owner && g.team != g.owner->team) || g.droptime))
                takenflags.add(i);
        }
        if(!ai::badhealth(d) && !takenflags.empty())
        {
            int flag = takenflags.length() > 2 ? rnd(takenflags.length()) : 0;
            d->ai->switchstate(b, ai::AI_S_PURSUE, ai::AI_T_AFFINITY, takenflags[flag]);
            return true;
        }
		return false;
	}

	void aifind(gameent *d, ai::aistate &b, vector<ai::interest> &interests)
	{
		vec pos = d->feetpos();
		loopvj(flags)
		{
			flag &f = flags[j];
			if(f.owner != d)
			{
				static vector<int> targets; // build a list of others who are interested in this
				targets.setsize(0);
				bool home = f.team == d->team;
				ai::checkothers(targets, d, home ? ai::AI_S_PURSUE : ai::AI_S_INTEREST, ai::AI_T_AFFINITY, j, true);
				gameent *e = NULL;

				loopi(numdynents()) if((e = (gameent *)iterdynents(i)) && !e->ai && e->state == CS_ALIVE && isteam(d->team, e->team))
				{ // try to guess what non ai are doing
					vec ep = e->feetpos();
					if(targets.find(e->clientnum) < 0 && (ep.squaredist(f.pos()) <= (FLAGRADIUS*FLAGRADIUS*4) || f.owner == e))
						targets.add(e->clientnum);
				}


                    ai::interest &n = interests.add();
                    n.state = ai::AI_S_PURSUE;
                    n.node = ai::closestwaypoint(f.pos(), ai::SIGHTMIN, true);
                    n.target = j;
                    n.targtype = ai::AI_T_AFFINITY;
                    n.score = pos.squaredist(f.pos());

			}
		}
	}

	bool aidefend(gameent *d, ai::aistate &b)
	{
        loopv(flags)
        {
            flag &g = flags[i];
            if(g.owner == d) return aihomerun(d, b);
        }
		if(flags.inrange(b.target))
		{
			flag &f = flags[b.target];
			if(f.droptime) return ai::makeroute(d, b, f.pos());
			if(f.owner) return ai::violence(d, b, f.owner, 4);
			int walk = 0;
			if(lastmillis-b.millis >= (201-d->skill)*11)
			{
				static vector<int> targets; // build a list of others who are interested in this
				targets.setsize(0);
				ai::checkothers(targets, d, ai::AI_S_DEFEND, ai::AI_T_AFFINITY, b.target, true);
				gameent *e = NULL;
				loopi(numdynents()) if((e = (gameent *)iterdynents(i)) && !e->ai && e->state == CS_ALIVE && isteam(d->team, e->team))
				{ // try to guess what non ai are doing
					vec ep = e->feetpos();
					if(targets.find(e->clientnum) < 0 && (ep.squaredist(f.pos()) <= (FLAGRADIUS*FLAGRADIUS*4) || f.owner == e))
						targets.add(e->clientnum);
				}
				if(!targets.empty())
				{
					d->ai->trywipe = true; // re-evaluate so as not to herd
					return true;
				}
				else
				{
					walk = 2;
					b.millis = lastmillis;
				}
			}
			vec pos = d->feetpos();
			float mindist = float(FLAGRADIUS*FLAGRADIUS*8);
			loopv(flags)
			{ // get out of the way of the returnee!
				flag &g = flags[i];
				if(pos.squaredist(g.pos()) <= mindist)
				{
					if(g.owner && g.owner->team == d->team) walk = 1;
					if(g.droptime && ai::makeroute(d, b, g.pos())) return true;
				}
			}
			return ai::defend(d, b, f.pos(), float(FLAGRADIUS*2), float(FLAGRADIUS*(2+(walk*2))), walk);
		}
		return false;
	}

	bool aipursue(gameent *d, ai::aistate &b)
	{
		if(flags.inrange(b.target))
		{
			flag &f = flags[b.target];
            if(f.owner == d) return aihomerun(d, b);
			if(f.team == d->team)
			{
				if(f.droptime) return ai::makeroute(d, b, f.pos());
				if(f.owner) return ai::violence(d, b, f.owner, 4);
                loopv(flags)
                {
                    flag &g = flags[i];
                    if(g.owner == d) return ai::makeroute(d, b, f.pos());
                }
			}
			else
			{
				if(f.owner) return ai::violence(d, b, f.owner, 4);
				return ai::makeroute(d, b, f.pos());
			}
		}
		return false;
	}
};

extern ctfclientmode ctfmode;
ICOMMAND(dropflag, "", (), { ctfmode.trydropflag(); });

#endif

#elif SERVMODE

case N_TRYDROPFLAG:
{
    if((ci->state.state!=CS_SPECTATOR || ci->local || ci->privilege) && cq && smode==&ctfmode) ctfmode.dropflag(cq);
    break;
}

case N_TAKEFLAG:
{
    int flag = getint(p), version = getint(p);
    if((ci->state.state!=CS_SPECTATOR || ci->local || ci->privilege) && cq && smode==&ctfmode) ctfmode.takeflag(cq, flag, version);
    break;
}

case N_INITFLAGS:
    if(smode==&ctfmode) ctfmode.parseflags(p, (ci->state.state!=CS_SPECTATOR || ci->privilege || ci->local) && !strcmp(ci->clientmap, smapname));
    break;

#else

case N_INITFLAGS:
{
    ctfmode.parseflags(p, m_ctf);
    break;
}

case N_DROPFLAG:
{
    int ocn = getint(p), flag = getint(p), version = getint(p);
    vec droploc;
    loopk(3) droploc[k] = getint(p)/DMF;
    gameent *o = ocn==player1->clientnum ? player1 : newclient(ocn);
    if(o && m_ctf) ctfmode.dropflag(o, flag, version, droploc);
    break;
}

case N_SCOREFLAG:
{
    int ocn = getint(p), relayflag = getint(p), relayversion = getint(p), goalflag = getint(p), goalversion = getint(p), team = getint(p), score = getint(p), oflags = getint(p);
    gameent *o = ocn==player1->clientnum ? player1 : newclient(ocn);
    if(o && m_ctf) ctfmode.scoreflag(o, relayflag, relayversion, goalflag, goalversion, team, score, oflags);
    break;
}

case N_RETURNFLAG:
{
    int ocn = getint(p), flag = getint(p), version = getint(p);
    gameent *o = ocn==player1->clientnum ? player1 : newclient(ocn);
    if(o && m_ctf) ctfmode.returnflag(o, flag, version);
    break;
}

case N_TAKEFLAG:
{
    int ocn = getint(p), flag = getint(p), version = getint(p);
    gameent *o = ocn==player1->clientnum ? player1 : newclient(ocn);
    if(o && m_ctf) ctfmode.takeflag(o, flag, version);
    break;
}

case N_RESETFLAG:
{
    int flag = getint(p), version = getint(p);
    if(m_ctf) ctfmode.resetflag(flag, version);
    break;
}

#endif

