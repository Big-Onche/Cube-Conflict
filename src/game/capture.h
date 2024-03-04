// capture.h: client and server state for capture gamemode
#ifndef PARSEMESSAGES

#ifdef SERVMODE
struct captureservmode : servmode
#else
VARP(capturetether, 0, 1, 1);
VARP(autorepammo, 0, 1, 1);
VARP(basenumbers, 0, 0, 1);

struct captureclientmode : clientmode
#endif
{
    static const int CAPTURERADIUS = 128;
    static const int CAPTUREHEIGHT = 96;
    static const int OCCUPYBONUS = 1;
    static const int OCCUPYPOINTS = 1;
    static const int OCCUPYENEMYLIMIT = 16;
    static const int OCCUPYNEUTRALLIMIT = 8;
    static const int SCORESECS = 10;
    static const int AMMOSECS = 15;
    static const int REGENSECS = 1;
    static const int REGENHEALTH = 100;
    static const int REGENARMOUR = 100;
    static const int REGENAMMO = 20;
    static const int MAXAMMO = 5;
    static const int REPAMMODIST = 32;
    static const int RESPAWNSECS = 5;
    static const int MAXBASES = 100;

    struct baseinfo
    {
        vec o;
        int owner, enemy;
#ifndef SERVMODE
        vec ammopos;
        string name, info;
#endif
        int ammogroup, ammotype, tag, ammo, owners, enemies, converted, capturetime;

        baseinfo() { reset(); }

        void noenemy()
        {
            enemy = 0;
            enemies = 0;
            converted = 0;
        }

        void reset()
        {
            noenemy();
            owner = 0;
            capturetime = -1;
            ammogroup = 0;
            ammotype = 0;
            tag = -1;
            ammo = 0;
            owners = 0;
        }

        bool enter(int team)
        {
            if(team==owner)
            {
                owners++;
                return false;
            }
            if(!enemies)
            {
                if(enemy!=team)
                {
                    converted = 0;
                    enemy = team;
                }
                enemies++;
                return true;
            }
            else if(enemy!=team) return false;
            else enemies++;
            return false;
        }

        bool steal(int team)
        {
            return !enemies && owner!=team;
        }

        bool leave(int team)
        {
            if(team==owner && owners > 0)
            {
                owners--;
                return false;
            }
            if(enemy!=team || enemies <= 0) return false;
            enemies--;
            return !enemies;
        }

        int occupy(int team, int units)
        {
            if(enemy!=team) return -1;
            converted += units;
            if(units<0)
            {
                if(converted<=0) noenemy();
                return -1;
            }
            else if(converted<(owner ? int(OCCUPYENEMYLIMIT) : int(OCCUPYNEUTRALLIMIT))) return -1;
            if(owner) { owner = 0; converted = 0; enemy = team; return 0; }
            else { owner = team; ammo = 0; capturetime = 0; owners = enemies; noenemy(); return 1; }
        }

        bool addammo(int i)
        {
            if(ammo>=MAXAMMO) return false;
            ammo = min(ammo+i, int(MAXAMMO));
            return true;
        }

        bool takeammo(int team)
        {
            if(owner!=team || ammo<=0) return false;
            ammo--;
            return true;
        }
    };

    vector<baseinfo> bases;

    struct score
    {
        int team, total;
    };

    vector<score> scores;

    int captures;

    void resetbases()
    {
        bases.shrink(0);
        scores.shrink(0);
        captures = 0;
    }

    bool hidefrags() { return true; }

    int getteamscore(int team)
    {
        loopv(scores)
        {
            score &cs = scores[i];
            if(cs.team==team) return cs.total;
        }
        return 0;
    }

    void getteamscores(vector<teamscore> &teamscores)
    {
        loopv(scores) teamscores.add(teamscore(scores[i].team, scores[i].total));
    }

    score &findscore(int team)
    {
        loopv(scores)
        {
            score &cs = scores[i];
            if(cs.team==team) return cs;
        }
        score &cs = scores.add();
        cs.team = team;
        cs.total = 0;
        return cs;
    }

    void addbase(int ammotype, const vec &o)
    {
        if(bases.length() >= MAXBASES) return;
        baseinfo &b = bases.add();
        b.ammogroup = min(ammotype, 0);
        b.ammotype = rnd(17);

        b.o = o;
        if(b.ammogroup)
        {
            loopi(bases.length()-1) if(b.ammogroup == bases[i].ammogroup)
            {
                b.ammotype = bases[i].ammotype;
                return;
            }
            int uses[I_ARTIFICE-I_RAIL+1];
            memset(uses, 0, sizeof(uses));
            loopi(bases.length()-1) if(bases[i].ammogroup)
            {
                loopj(i) if(bases[j].ammogroup == bases[i].ammogroup) goto nextbase;
                uses[bases[i].ammotype-1]++;
                nextbase:;
            }
            int mintype = 0;
            loopi(I_ARTIFICE-I_RAIL+1) if(uses[i] < uses[mintype]) mintype = i;
            int numavail = 0, avail[I_ARTIFICE-I_RAIL+1];
            loopi(I_ARTIFICE-I_RAIL+1) if(uses[i] == uses[mintype]) avail[numavail++] = i+1;
            b.ammotype = avail[rnd(numavail)];
        }
    }

    void initbase(int i, int ammotype, int owner, int enemy, int converted, int ammo)
    {
        if(!bases.inrange(i)) return;
        baseinfo &b = bases[i];
        b.ammotype = ammotype;
        b.owner = owner;
        b.enemy = enemy;
        b.converted = converted;
        b.ammo = ammo;
    }

    bool hasbases(int team)
    {
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.owner && b.owner==team) return true;
        }
        return false;
    }

    bool insidebase(const baseinfo &b, const vec &o, bool isalive)
    {
        if(!isalive || game::intermission) return false;
        float dx = (b.o.x-o.x), dy = (b.o.y-o.y), dz = (b.o.z-o.z);
        return dx*dx + dy*dy <= CAPTURERADIUS*CAPTURERADIUS && fabs(dz) <= CAPTUREHEIGHT;
    }

#ifndef SERVMODE
    static const int AMMOHEIGHT = 5;

    captureclientmode() : captures(0)
    {
    }

    void respawned(gameent *d)
    {
    }

    void replenishammo()
    {
        if(!m_capture || m_regencapture) return;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(insidebase(b, player1->feetpos(), player1->state==CS_ALIVE) && player1->hasmaxammo(b.ammotype-1+I_RAIL)) return;
        }
        addmsg(N_REPAMMO, "rc", player1);
    }

    void receiveammo(gameent *d, int type)
    {
        type += I_RAIL-1;
        if(type<I_RAIL || type>I_ARTIFICE) return;
        entities::repammo(d, type, d==player1);
    }

    void checkitems(gameent *d)
    {
        if(m_regencapture || !autorepammo || d!=player1 || d->state!=CS_ALIVE) return;
        vec o = d->feetpos();
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(insidebase(b, d->feetpos(), d->state==CS_ALIVE) && b.owner==d->team && b.o.dist(o) < 12)
            {
                if(d->lastrepammo!=i)
                {
                    if(b.ammo > 0 && !player1->hasmaxammo(b.ammotype-1+I_RAIL)) addmsg(N_REPAMMO, "rc", d);
                    d->lastrepammo = i;
                }
                return;
            }
        }
        d->lastrepammo = -1;
    }

    int insidebasetimer = 0;

    void rendertether(gameent *d)
    {
        int oldbase = d->lastbase;
        d->lastbase = -1;
        vec pos(d->o.x, d->o.y, d->o.z + (d->aboveeye - d->eyeheight)/2);
        if(d->state==CS_ALIVE)
        {
            loopv(bases)
            {
                baseinfo &b = bases[i];

                if(insidebase(bases[i], player1->feetpos(), player1->state==CS_ALIVE) && d==player1 && totalmillis >= insidebasetimer+1000 && (b.converted || !b.owner))
                {
                    addReward(1, 1);
                    updateStat(1, STAT_BASEHACK);
                    insidebasetimer = totalmillis;
                }

                if(!insidebase(b, d->feetpos(), d->state==CS_ALIVE) || (b.owner!=d->team && b.enemy!=d->team)) continue;
                if(d->lastbase < 0 && (lookupmaterial(d->feetpos())&MATF_CLIP) == MAT_GAMECLIP) break;

                vec basepos(b.o);
                basepos.add(vec(-10+rnd(20), -10+rnd(20), -10+rnd(20)));
                basepos.sub(pos);
                basepos.normalize().mul(1300.0f);

                if(rndevent(93)) particle_flying_flare(pos, basepos, 500, rnd(2) ? PART_ZERO : PART_ONE, isteam(player1->team, d->team) ? 0xFFFF00 : 0xFF0000, 0.7f+(rnd(5)/10.f), 100);

                if(oldbase < 0)
                {
                    if(b.owner!=d->team && b.owner) playSound(S_TERMINAL_ALARM, b.o, 300, 50, SND_FIXEDPITCH);
                    playSound(S_TERMINAL_ENTER, d==hudplayer() ? vec(0, 0, 0) : pos, 150, 50);
                    particle_splash(PART_ZERO, 12, 250, pos, isteam(player1->team, d->team) ? 0xFFFF00 : 0xFF0000, 0.8f, 200, 50);
                    particle_splash(PART_ONE, 12, 250, pos, isteam(player1->team, d->team) ? 0xFFFF00 : 0xFF0000, 0.8f, 200, 50);
                }
                d->lastbase = i;
            }
        }
        if(d->lastbase < 0 && oldbase >= 0)
        {
            playSound(S_TERMINAL_ENTER, d==hudplayer() ? vec(0, 0, 0) : pos, 150, 50);
            particle_splash(PART_ZERO, 12, 200, pos, isteam(player1->team, d->team) ? 0xFFFF00 : 0xFF0000, 0.8f, 200, 50);
            particle_splash(PART_ONE, 12, 200, pos, isteam(player1->team, d->team) ? 0xFFFF00 : 0xFF0000, 0.8f, 200, 50);
        }
    }

    void preload()
    {
        static const char * const basemodels[3] = { "base/neutral", "base/red", "base/yellow" };
        loopi(3) preloadmodel(basemodels[i]);
    }

    void rendergame()
    {
        if(capturetether && canEmitParticles())
        {
            loopv(players)
            {
                gameent *d = players[i];
                if(d) rendertether(d);
            }
            rendertether(player1);
        }
        loopv(bases)
        {
            baseinfo &b = bases[i];
            const char *basename = b.owner ? (b.owner!=hudplayer()->team ? "base/red" : "base/yellow") : "base/neutral";
            rendermodel(basename, ANIM_MAPMODEL|ANIM_LOOP, b.o, 0, 0, 0, MDL_CULL_VFC | MDL_CULL_OCCLUDED);

            const char *ammoname = entities::entmdlname(I_RAIL+b.ammotype);
            if(m_regencapture)
            {
                vec height(0, 0, 0);
                abovemodel(height, ammoname);
                vec ammopos(b.ammopos);
                ammopos.z -= height.z/2 + sinf(lastmillis/100.0f)/20;
                rendermodel(ammoname, ANIM_MAPMODEL|ANIM_LOOP, ammopos, lastmillis/10.0f, 0, 0, MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_FORCESHADOW);
            }

            int tcolor = 0x888888, mtype = -1, mcolor = 0xFFFFFF, mcolor2 = 0;

            if(b.owner)
            {
                defformatstring(baseteam, "%s", b.converted ? readstr("Capture_Contested") : (b.owner==hudplayer()->team ? readstr("Capture_Allied") : readstr("Capture_Enemy")));
                bool isowner = b.owner==hudplayer()->team;
                if(b.enemy) { mtype = PART_METER_VS; mcolor = 0xFF0000; mcolor2 = 0xFFFF00; if(!isowner) swap(mcolor, mcolor2); }
                if(!b.name[0]) formatstring(b.info, "%s %d - %s", readstr("Capture_Terminal"), b.tag, baseteam);
                else if(basenumbers) formatstring(b.info, "%s (%d) - %s", b.name, b.tag, baseteam);
                else formatstring(b.info, "%s - %s", b.name, baseteam);
                tcolor = isowner ? 0xFFFF00 : 0xFF0000;

                if(b.owner==hudplayer()->team && hudplayer()->state==CS_ALIVE)
                {
                    vec bpos = b.o;
                    particle_hud(PART_BLIP, bpos.add(vec(0, 0, 10)), (totalmillis % 1001 < 500) && b.converted ? 0xFF0000 : 0xFFFF00);
                }
            }
            else if(b.enemy)
            {
                if(!b.name[0]) formatstring(b.info, "%s %d - %s", readstr("Capture_Terminal"), b.tag, readstr("Capture_Hacking"));
                else if(basenumbers) formatstring(b.info, "%s (%d) - %s", b.name, b.tag, readstr("Capture_Hacking"));
                else formatstring(b.info, "%s - %s", b.name, readstr("Capture_Hacking"));
                if(b.enemy!=hudplayer()->team) { tcolor = 0xFF0000; mtype = PART_METER; mcolor = 0xFF0000; }
                else { tcolor = 0xFFFF00; mtype = PART_METER; mcolor = 0xFFFF00; }
            }
            else if(!b.name[0]) formatstring(b.info, "%s %d", readstr("Capture_Terminal"), b.tag);
            else if(basenumbers) formatstring(b.info, "%s (%d)", b.name, b.tag);
            else copystring(b.info, b.name);

            vec above(b.ammopos);
            above.z += AMMOHEIGHT;
            if(b.info[0]) particle_text(above, b.info, PART_TEXT, 1, tcolor, 3.0f);
            if(mtype>=0)
            {
                above.z += 3.5f;
                particle_meter(above, b.converted/float((b.owner ? int(OCCUPYENEMYLIMIT) : int(OCCUPYNEUTRALLIMIT))), mtype, 1, mcolor, mcolor2, 3.0f);
            }
        }
    }

    void drawblips(gameent *d, float blipsize, int fw, int fh, int type, bool skipenemy = false)
    {
        float scale = calcradarscale();
        int blips = 0;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(skipenemy && b.enemy) continue;
            switch(type)
            {
                case 1: if(!b.owner || b.owner!=hudplayer()->team) continue; break;
                case 0: if(b.owner) continue; break;
                case -1: if(!b.owner || b.owner==hudplayer()->team) continue; break;
                case -2: if(!b.enemy || b.enemy==hudplayer()->team) continue; break;
            }
            vec dir(d->o);
            dir.sub(b.o).div(scale);
            float dist = dir.magnitude2(), maxdist = 1 - 0.05f - blipsize;
            if(dist >= maxdist) dir.mul(maxdist/dist);
            dir.rotate_around_z(-camera1->yaw*RAD);
            if(basenumbers)
            {
                static string blip;
                formatstring(blip, "%d", b.tag);
                int tw, th;
                text_bounds(blip, tw, th);
                draw_text(blip, int(0.5f*(dir.x*fw/blipsize - tw)), int(0.5f*(dir.y*fh/blipsize - th)));
            }
            else
            {
                if(!blips) { gle::defvertex(2); gle::deftexcoord0(); gle::begin(GL_QUADS); }
                float x = 0.5f*(dir.x*fw/blipsize - fw), y = 0.5f*(dir.y*fh/blipsize - fh);
                gle::attribf(x,    y);    gle::attribf(0, 0);
                gle::attribf(x+fw, y);    gle::attribf(1, 0);
                gle::attribf(x+fw, y+fh); gle::attribf(1, 1);
                gle::attribf(x,    y+fh); gle::attribf(0, 1);
            }
            blips++;
        }
        if(blips && !basenumbers) gle::end();
    }

    int respawnwait(gameent *d, int delay = 0)
    {
        return d->respawnwait(RESPAWNSECS, delay);
    }

    int clipconsole(int w, int h)
    {
        return (h*(1 + 1 + 10))/(4*10);
    }

    void drawhud(gameent *d, int w, int h)
    {
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
        bool showenemies = lastmillis%1000 >= 500;
        int fw = 1, fh = 1;
        if(basenumbers)
        {
            pushfont();
            setfont("digit_blue");
            text_bounds(" ", fw, fh);
        }
        else settexture("media/interface/hud/blip_blue.png", 3);
        float blipsize = basenumbers ? 0.1f : 0.05f;
        pushhudmatrix();
        hudmatrix.translate(x + 0.5f*s, y + 0.5f*s, 0);
        hudmatrix.scale((s*blipsize)/fw, (s*blipsize)/fh, 1.0f);
        flushhudmatrix();
        drawblips(d, blipsize, fw, fh, 1, showenemies);
        if(basenumbers) setfont("digit_grey");
        else settexture("media/interface/hud/blip_grey.png", 3);
        drawblips(d, blipsize, fw, fh, 0, showenemies);
        if(basenumbers) setfont("digit_red");
        else settexture("media/interface/hud/blip_red.png", 3);
        drawblips(d, blipsize, fw, fh, -1, showenemies);
        if(showenemies) drawblips(d, blipsize, fw, fh, -2);
        pophudmatrix();
        if(basenumbers) popfont();
        drawteammates(d, x, y, s);
    }

    void setup()
    {
        resetbases();
        loopv(entities::ents)
        {
            extentity *e = entities::ents[i];
            if(e->type!=BASE) continue;
            baseinfo &b = bases.add();
            b.o = e->o;
            b.ammopos = b.o;
            abovemodel(b.ammopos, "base/neutral");
            b.ammopos.z += AMMOHEIGHT-2;
            b.ammotype = e->attr1;
            defformatstring(alias, "base_%s_%d", readstr("languages", language), e->attr2);
            const char *name = getalias(alias);
            copystring(b.name, name);
            b.tag = e->attr2>0 ? e->attr2 : bases.length();
        }
    }

    void senditems(packetbuf &p)
    {
        putint(p, N_BASES);
        putint(p, bases.length());
        loopv(bases)
        {
            baseinfo &b = bases[i];
            putint(p, b.ammotype);
            putint(p, int(b.o.x*DMF));
            putint(p, int(b.o.y*DMF));
            putint(p, int(b.o.z*DMF));
        }
    }

    void updatebase(int i, int owner, int enemy, int converted, int ammo)
    {
        if(!bases.inrange(i)) return;
        baseinfo &b = bases[i];
        if(owner)
        {
            if(b.owner!=owner)
            {
                if(!b.name[0])
                {
                    conoutf(CON_GAMEINFO, "%s %s %s \"\fe%d\f7\".", readstr("Misc_Team"), teamcolor(owner), readstr("Console_Game_Capture_Hack"), b.tag);
                    if(owner==hudplayer()->team) conoutf(CON_HUDCONSOLE, "\f9%s \"\fe%d\f9\".", readstr("Console_Game_Capture_WeHacked"), b.tag);
                }
                else
                {
                    conoutf(CON_GAMEINFO, "%s %s %s \"\fe%s\f7\".", readstr("Misc_Team"), teamcolor(owner), readstr("Console_Game_Capture_Hack"), b.name);
                    if(owner==hudplayer()->team) conoutf(CON_HUDCONSOLE, "\f9%s \"\fe%s\f9\".", readstr("Console_Game_Capture_WeHacked"), b.name);
                }
                playSound(owner==hudplayer()->team ? S_TERMINAL_HACKED : S_TERMINAL_HACKED_E, b.o, 2500, 200, SND_FIXEDPITCH);
            }
        }
        else if(b.owner)
        {
            if(!b.name[0])
            {
                conoutf(CON_GAMEINFO, "%s %s %s \"\fe%d\f7\".", readstr("Misc_Team"), teamcolor(b.owner), readstr("Console_Game_Capture_Lost"), b.tag);
                if(b.owner==hudplayer()->team) conoutf(CON_HUDCONSOLE, "\fc%s \"\fe%d\f3\".", readstr("Console_Game_Capture_WeLost"), b.tag);
            }
            else
            {
                conoutf(CON_GAMEINFO, "%s %s %s \"\fe%s\f7\".", readstr("Misc_Team"), teamcolor(b.owner), readstr("Console_Game_Capture_Lost"), b.name);
                if(b.owner==hudplayer()->team) conoutf(CON_HUDCONSOLE, "\fc%s \"\fe%s\f3\".", readstr("Console_Game_Capture_WeLost"), b.name);
            }
            playSound(owner==hudplayer()->team ? S_TERMINAL_LOST : S_TERMINAL_LOST_E, b.o, 2500, 200, SND_FIXEDPITCH);
        }
        if(b.owner!=owner)
        {
            loopi(2) particle_splash(PART_ZERO+i, 12, 500, b.ammopos, owner ? (owner==hudplayer()->team ? 0xFFFF00 : 0xFF0000) : 0x777777, 1.f, 400, -50);
        }

        b.owner = owner;
        b.enemy = enemy;
        b.converted = converted;
        if(ammo>b.ammo) playSound(S_ITEMSPAWN, b.o, 250, 50);
        b.ammo = ammo;
    }

    void setscore(int base, int team, int total)
    {
        findscore(team).total = total;
        if(total>=10000) conoutf(CON_GAMEINFO, "%s %s %s", readstr("Misc_Team"), teamcolor(team), readstr("Console_Game_Capture_Won"));
        else if(bases.inrange(base))
        {
            baseinfo &b = bases[base];
            if(b.owner==team)
            {
                defformatstring(msg, "%d", total);
                vec above(b.ammopos);
                above.z += AMMOHEIGHT+1.0f;
                particle_textcopy(above, msg, PART_TEXT, 1500, isteam(team, player1->team) ? 0xFFFF00 : 0xFF0000, 5.0f, -5);
            }
        }
    }

    void cnbasescore(gameent *d, int score)
    {
        d->flags = score;
    }

    // prefer spawning near friendly base
    float ratespawn(gameent *d, const extentity &e)
    {
        float minbasedist = 1e16f;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(!b.owner || b.owner!=d->team) continue;
            minbasedist = min(minbasedist, e.o.dist(b.o));
        }
        return minbasedist < 1e16f ? proximityscore(minbasedist, 128.0f, 512.0f) : 1.0f;
    }

	bool aicheck(gameent *d, ai::aistate &b)
	{
		return false;
	}

	void aifind(gameent *d, ai::aistate &b, vector<ai::interest> &interests)
	{
		vec pos = d->feetpos();
		loopvj(bases)
		{
			baseinfo &f = bases[j];
			static vector<int> targets; // build a list of others who are interested in this
			targets.setsize(0);
			ai::checkothers(targets, d, ai::AI_S_DEFEND, ai::AI_T_AFFINITY, j, true);
			gameent *e = NULL;
			int regen = !m_regencapture || d->health >= 750 || d->armour >= 1000+d->skill*10 ? 0 : 1;
			if(m_regencapture)
			{
			    if(d->armour < 1000+d->skill*10) regen = 2;
				if(!d->hasmaxammo(f.ammotype-1+I_RAIL)/2.f) regen = 4;
			}
			loopi(numdynents()) if((e = (gameent *)iterdynents(i)) && !e->ai && e->state == CS_ALIVE && isteam(d->team, e->team))
			{ // try to guess what non ai are doing
				vec ep = e->feetpos();
				if(targets.find(e->clientnum) < 0 && ep.squaredist(f.o) <= (CAPTURERADIUS*CAPTURERADIUS))
					targets.add(e->clientnum);
			}
			if((regen) || (targets.empty() && (!f.owner || f.owner!=d->team || f.enemy)))
			{
				ai::interest &n = interests.add();
				n.state = ai::AI_S_DEFEND;
				n.node = ai::closestwaypoint(f.o, ai::SIGHTMIN, false);
				n.target = j;
				n.targtype = ai::AI_T_AFFINITY;
				n.score = pos.squaredist(f.o)/(regen ? float(100*regen) : 1.f);
			}
		}
	}

	bool aidefend(gameent *d, ai::aistate &b)
	{
        if(!bases.inrange(b.target)) return false;
        baseinfo &f = bases[b.target];
		bool regen = !m_regencapture || d->health >= 750 || d->armour >= 1000+d->skill*10 ? false : true;
		if(!regen && m_regencapture)
		{
		    if(d->armour < 1000+d->skill*10) regen = true;
			int gun = f.ammotype-1+I_RAIL;
			if(f.ammo > 0 && !d->hasmaxammo(gun))
				regen = true;
		}
		int walk = 0;
		if(!regen && !f.enemy && f.owner && f.owner==d->team)
		{
			static vector<int> targets; // build a list of others who are interested in this
			targets.setsize(0);
			ai::checkothers(targets, d, ai::AI_S_DEFEND, ai::AI_T_AFFINITY, b.target, true);
			gameent *e = NULL;
			loopi(numdynents()) if((e = (gameent *)iterdynents(i)) && !e->ai && e->state == CS_ALIVE && isteam(d->team, e->team))
			{ // try to guess what non ai are doing
				vec ep = e->feetpos();
				if(targets.find(e->clientnum) < 0 && (ep.squaredist(f.o) <= (CAPTURERADIUS*CAPTURERADIUS*4)))
					targets.add(e->clientnum);
			}
			if(!targets.empty())
			{
				if(lastmillis-b.millis >= (201-d->skill)*33)
				{
					d->ai->trywipe = true; // re-evaluate so as not to herd
					return true;
				}
				else walk = 2;
			}
			else walk = 1;
			b.millis = lastmillis;
		}
		return ai::defend(d, b, f.o, float(CAPTURERADIUS), float(CAPTURERADIUS*(2+(walk*2))), walk); // less wander than ctf
	}

	bool aipursue(gameent *d, ai::aistate &b)
	{
		b.type = ai::AI_S_DEFEND;
		return aidefend(d, b);
	}
};

extern captureclientmode capturemode;
ICOMMAND(repammo, "", (), capturemode.replenishammo());
ICOMMAND(insidebases, "", (),
{
    vector<char> buf;
    if(m_capture && player1->state == CS_ALIVE) loopv(capturemode.bases)
    {
        captureclientmode::baseinfo &b = capturemode.bases[i];
        if(capturemode.insidebase(b, player1->feetpos(), player1->state==CS_ALIVE))
        {
            if(buf.length()) buf.add(' ');
            defformatstring(basenum, "%d", b.tag);
            buf.put(basenum, strlen(basenum));
        }
    }
    buf.add('\0');
    result(buf.getbuf());
});


ICOMMAND(hudbasesstats, "i", (int *team),
    if(m_capture)
    {
        int totalbases = 0;
        int baseteam[3];
        loopi(3) baseteam[i] = 0; //cannot initialize baseteam[] with "{0, 0, 0}" in ICOMMAND macro
        loopv(capturemode.bases)
        {
            captureclientmode::baseinfo &b = capturemode.bases[i];
            totalbases++;
            b.owner ? (b.owner!=hudplayer()->team ? baseteam[2]++ : baseteam[1]++) : baseteam[0]++;
        }
        float pcts[3];
        loopi(3) pcts[i] = ((float)baseteam[i] / totalbases) * 100;
        floatret(pcts[*team]);
    }
);

#else
    bool notgotbases;

    captureservmode() : captures(0), notgotbases(false) {}

    void reset(bool empty)
    {
        resetbases();
        notgotbases = !empty;
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
            if(e.type != BASE) continue;
            int ammotype = e.attr1;
            addbase(ammotype, e.o);
        }
        notgotbases = false;
        sendbases();
        loopv(clients) if(clients[i]->state.state==CS_ALIVE) entergame(clients[i]);
    }

    void newmap()
    {
        reset(true);
    }

    void stealbase(int n, int team)
    {
        baseinfo &b = bases[n];
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->state.state==CS_ALIVE && ci->team>0 && ci->team<=2 && ci->team==team && insidebase(b, ci->state.o, ci->state.state==CS_ALIVE))
                b.enter(ci->team);
        }
        sendbaseinfo(n);
    }

    void replenishammo(clientinfo *ci)
    {
        if(notgotbases || ci->state.state!=CS_ALIVE) return;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(insidebase(b, ci->state.o, ci->state.state==CS_ALIVE) && !ci->state.hasmaxammo(b.ammotype-1+I_RAIL) && b.takeammo(ci->team))
            {
                sendbaseinfo(i);
                sendf(-1, 1, "riii", N_REPAMMO, ci->clientnum, b.ammotype);
                ci->state.addammo(b.ammotype);
                break;
            }
        }
    }

    void movebases(int team, const vec &oldpos, bool oldclip, const vec &newpos, bool newclip, bool isalive)
    {
        if(gamemillis>=gamelimit || team<1 || team>2) return;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            bool leave = !oldclip && insidebase(b, oldpos, isalive),
                 enter = !newclip && insidebase(b, newpos, isalive);
            if(leave && !enter && b.leave(team)) sendbaseinfo(i);
            else if(enter && !leave && b.enter(team)) sendbaseinfo(i);
            else if(leave && enter && b.steal(team)) stealbase(i, team);
        }
    }

    void leavebases(int team, const vec &o, bool isalive)
    {
        movebases(team, o, false, vec(-1e10f, -1e10f, -1e10f), true, isalive);
    }

    void enterbases(int team, const vec &o, bool isalive)
    {
        movebases(team, vec(-1e10f, -1e10f, -1e10f), true, o, false, isalive);
    }

    void addscore(int base, int team, int n)
    {
        if(!n) return;
        score &cs = findscore(team);
        cs.total += n;
        sendf(-1, 1, "ri4", N_BASESCORE, base, team, cs.total);
    }

    void addciscore(baseinfo &b)
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i];

            if(ci->state.state==CS_ALIVE && (ci->team==1 || ci->team==2) && insidebase(b, ci->state.o, ci->state.state==CS_ALIVE))
            {
                if(b.owner!=ci->team) ci->state.flags+=1;
                sendf(-1, 1, "ri3", N_SCOREBASE, ci->clientnum, ci->state.flags);
            }
        }
    }

    void regenowners(baseinfo &b, int ticks)
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->state.state==CS_ALIVE && b.owner==ci->team && insidebase(b, ci->state.o, ci->state.state==CS_ALIVE) && (ci->team==1 || ci->team==2))
            {
                bool notify = false;
                if(ci->state.health < ci->state.maxhealth)
                {
                    ci->state.health = min(ci->state.health + ticks*REGENHEALTH, ci->state.maxhealth);
                    notify = true;
                }

                if(ci->state.mana < 150)
                {
                    ci->state.mana = min(ci->state.mana + ticks*REGENHEALTH/10, 150);
                    notify = true;
                }

                if(ci->state.armour<3000)
                {
                    if(!ci->state.armour) ci->state.armourtype = A_WOOD;
                    ci->state.armour = min(ci->state.armour + ticks*REGENARMOUR, 3000);
                    notify = true;
                }

                switch(ci->state.armourtype)
                {
                    case A_WOOD: if(ci->state.armour>750) ci->state.armourtype = A_IRON; break;

                    case A_IRON: if(ci->state.armour>1250) ci->state.armourtype = A_GOLD; break;

                    case A_GOLD: if(ci->state.armour>2000) {ci->state.armourtype = A_ASSIST; if(!ci->state.ammo[GUN_ASSISTXPL]) ci->state.ammo[GUN_ASSISTXPL] = 1;}
                }

                if(!ci->state.hasmaxammo(b.ammotype))
                {
                    ci->state.addammo(b.ammotype, ticks*REGENAMMO, 100);
                    notify = true;
                }

                if(notify) sendf(-1, 1, "ri7", N_BASEREGEN, ci->clientnum, ci->state.health, ci->state.armour, ci->state.mana, b.ammotype, ci->state.ammo[b.ammotype]);
            }
        }
    }

    void update()
    {
        endcheck();
        if(gamemillis>=gamelimit) return;
        int t = gamemillis/1000 - (gamemillis-curtime)/1000;
        if(t<1) return;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.enemy)
            {
                if(!b.owners || !b.enemies) b.occupy(b.enemy, OCCUPYBONUS*(b.enemies ? 1 : -1) + OCCUPYPOINTS*(b.enemies ? b.enemies : -(1+b.owners))*t);
                sendbaseinfo(i);
            }
            else if(b.owner)
            {
                b.capturetime += t;

                int score = b.capturetime/SCORESECS - (b.capturetime-t)/SCORESECS;
                if(score) addscore(i, b.owner, score);

                if(m_regencapture)
                {
                    int regen = b.capturetime/REGENSECS - (b.capturetime-t)/REGENSECS;
                    if(regen) regenowners(b, regen);
                }
            }
            addciscore(b);
        }
    }

    void sendbaseinfo(int i)
    {
        baseinfo &b = bases[i];
        sendf(-1, 1, "ri6", N_BASEINFO, i, b.owner, b.enemy, b.enemy ? b.converted : 0, b.owner ? b.ammo : 0);
    }

    void sendbases()
    {
        packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
        initclient(NULL, p, false);
        sendpacket(-1, 1, p.finalize());
    }

    void initclient(clientinfo *ci, packetbuf &p, bool connecting)
    {
        if(connecting)
        {
            loopv(scores)
            {
                score &cs = scores[i];
                putint(p, N_BASESCORE);
                putint(p, -1);
                putint(p, cs.team);
                putint(p, cs.total);
            }
        }
        putint(p, N_BASES);
        putint(p, bases.length());
        loopv(bases)
        {
            baseinfo &b = bases[i];
            putint(p, b.ammotype);
            putint(p, b.owner);
            putint(p, b.enemy);
            putint(p, b.converted);
            putint(p, b.ammo);
        }
    }

    void endcheck()
    {
        int lastteam = NULL;

        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.owner)
            {
                if(!lastteam) lastteam = b.owner;
                else if(lastteam!=b.owner)
                {
                    lastteam = NULL;
                    break;
                }
            }
            else
            {
                lastteam = NULL;
                break;
            }
        }

        if(!lastteam) return;
        findscore(lastteam).total = 10000;
        sendf(-1, 1, "ri4", N_BASESCORE, -1, lastteam, 10000);
        startintermission();
    }

    void entergame(clientinfo *ci)
    {
        if(notgotbases || ci->state.state!=CS_ALIVE || ci->gameclip) return;
        enterbases(ci->team, ci->state.o, ci->state.state==CS_ALIVE);
    }

    void spawned(clientinfo *ci)
    {
        if(notgotbases || ci->gameclip) return;
        enterbases(ci->team, ci->state.o, ci->state.lastspawn>3000);
    }

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
        if(notgotbases || ci->state.state!=CS_ALIVE || ci->gameclip) return;
        leavebases(ci->team, ci->state.o, ci->state.state==CS_ALIVE);
    }

    void died(clientinfo *ci, clientinfo *actor)
    {
        if(notgotbases || ci->gameclip) return;
        leavebases(ci->team, ci->state.o, ci->state.state==CS_ALIVE);
    }

    bool canspawn(clientinfo *ci, bool connecting)
    {
        return connecting || !ci->state.lastdeath || gamemillis+curtime-ci->state.lastdeath >= RESPAWNSECS*1000;
    }

    void moved(clientinfo *ci, const vec &oldpos, bool oldclip, const vec &newpos, bool newclip, bool isalive)
    {
        if(notgotbases) return;
        movebases(ci->team, oldpos, oldclip, newpos, newclip, isalive);
    }

    void changeteam(clientinfo *ci, int oldteam, int newteam)
    {
        if(notgotbases || ci->gameclip) return;
        leavebases(oldteam, ci->state.o, ci->state.state==CS_ALIVE);
        enterbases(newteam, ci->state.o, ci->state.state==CS_ALIVE);
    }

    void parsebases(ucharbuf &p, bool commit)
    {
        int numbases = getint(p);
        loopi(numbases)
        {
            int ammotype = getint(p);
            vec o;
            loopk(3) o[k] = max(getint(p)/DMF, 0.0f);
            if(p.overread()) break;
            if(commit && notgotbases) addbase(ammotype>=GUN_ELEC && ammotype<=GUN_GLOCK ? ammotype : min(ammotype, 0), o);
        }
        if(commit && notgotbases)
        {
            notgotbases = false;
            sendbases();
            loopv(clients) if(clients[i]->state.state==CS_ALIVE) entergame(clients[i]);
        }
    }

    bool extinfoteam(int team, ucharbuf &p)
    {
        int numbases = 0;
        loopvj(bases) if(bases[j].owner==team) numbases++;
        putint(p, numbases);
        loopvj(bases) if(bases[j].owner==team) putint(p, j);
        return true;
    }
};

#endif

#elif SERVMODE

case N_BASES:
    if(smode==&capturemode) capturemode.parsebases(p, (ci->state.state!=CS_SPECTATOR || ci->privilege || ci->local) && !strcmp(ci->clientmap, smapname));
    break;

case N_REPAMMO:
    if((ci->state.state!=CS_SPECTATOR || ci->local || ci->privilege) && cq && smode==&capturemode) capturemode.replenishammo(cq);
    break;

#else

case N_BASEINFO:
{
    int base = getint(p);
    int owner = getint(p), enemy = getint(p);
    int converted = getint(p), ammo = getint(p);
    if(m_capture) capturemode.updatebase(base, owner, enemy, converted, ammo);
    break;
}

case N_SCOREBASE:
{
    int ocn = getint(p), score = getint(p);
    gameent *o = ocn==player1->clientnum ? player1 : newclient(ocn);
    if(o && m_capture) capturemode.cnbasescore(o, score);
    break;
}

case N_BASEREGEN:
{
    int rcn = getint(p), health = getint(p), armour = getint(p), mana = getint(p), ammotype = getint(p), ammo = getint(p);
    gameent *regen = rcn==player1->clientnum ? player1 : getclient(rcn);
    if(regen && m_capture)
    {
        vec regenPos = (regen == hudplayer() ? vec(0, 0, 0) : regen->o);
        regen->health = health;
        regen->mana = mana;
        if(ammotype >= GUN_ELEC && ammotype <= GUN_GLOCK) regen->ammo[ammotype] = ammo;
        if(!regen->armour) regen->armourtype=A_WOOD;
        regen->armour = armour;
        if(regen->armour==10) playSound(S_ITEMBBOIS, regenPos, 300, 150);
        switch(regen->armourtype)
        {
            case A_WOOD: if(regen->armour>=750) { regen->armourtype=A_IRON; playSound(S_ITEMBFER, regenPos, 300, 50); } break;
            case A_IRON: if(regen->armour>=1250) { regen->armourtype=A_GOLD; playSound(S_ITEMBOR, regenPos, 300, 50); } break;
            case A_GOLD: if(regen->armour>=2000) { regen->armourtype=A_ASSIST; playSound(S_ITEMARMOUR, regenPos, 300, 50); }
            case A_ASSIST: regen->ammo[GUN_ASSISTXPL] = 1;
        }
        if(autowield > 0 && player1->gunselect != ammotype) gunselect(ammotype, player1);
    }
    break;
}

case N_BASES:
{
    int numbases = getint(p);
    loopi(numbases)
    {
        int ammotype = getint(p);
        int owner = getint(p), enemy = getint(p);
        int converted = getint(p), ammo = getint(p);
        capturemode.initbase(i, ammotype, owner, enemy, converted, ammo);
    }
    break;
}

case N_BASESCORE:
{
    int base = getint(p), team = getint(p), total = getint(p);
    if(m_capture) capturemode.setscore(base, team, total);
    break;
}

case N_REPAMMO:
{
    int rcn = getint(p), ammotype = getint(p);
    gameent *r = rcn==player1->clientnum ? player1 : getclient(rcn);
    if(r && m_capture) capturemode.receiveammo(r, ammotype);
    break;
}

#endif
