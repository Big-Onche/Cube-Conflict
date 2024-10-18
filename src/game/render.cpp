#include "gfx.h"
#include "customs.h"
#include "stats.h"
#include "engine.h"
#include <string>

namespace game
{
    VARP(ragdoll, 0, 1, 1);
    VARP(ragdollmillis, 0, 10000, 300000);
    VARP(ragdollfade, 0, 500, 5000);
    VARP(forceplayermodels, 0, 0, 1);
    VARP(hidedead, 0, 0, 1);

    vector<gameent *> curGraves;

    void saveGrave(gameent *d)
    {
        if(!ragdollmillis || (!ragdollfade && lastmillis > d->lastpain + ragdollmillis)) return;
        gameent *r = new gameent(*d);
        r->lastupdate = ragdollfade && lastmillis > d->lastpain + max(ragdollmillis - ragdollfade, 0) ? lastmillis - max(ragdollmillis - ragdollfade, 0) : d->lastpain;
        r->edit = NULL;
        r->ai = NULL;
        curGraves.add(r);
        d->ragdoll = NULL;
    }

    void clearGraves()
    {
        curGraves.deletecontents();
    }

    void moveGraves()
    {
        loopv(curGraves)
        {
            gameent *d = curGraves[i];
            if(lastmillis > d->lastupdate + ragdollmillis)
            {
                delete curGraves.remove(i--);
                continue;
            }
            moveGrave(d);
        }
    }

    static const int playercolors[] =
    {
        0xA12020,
        0xA15B28,
        0xB39D52,
        0x3E752F,
        0x3F748C,
        0x214C85,
        0xB3668C,
        0x523678,
        0xB3ADA3
    };

    static const int playercolorsazul[] =
    {
        0x27508A,
        0x3F748C,
        0x3B3B80,
        0x5364B5
    };

    static const int playercolorsrojo[] =
    {
        0xAC2C2A,
        0x992417,
        0x802438,
        0xA3435B
    };

    extern void changedplayercolor();
    VARFP(playercolor, 0, 4, sizeof(playercolors)/sizeof(playercolors[0])-1, changedplayercolor());
    VARFP(playercolorazul, 0, 0, sizeof(playercolorsazul)/sizeof(playercolorsazul[0])-1, changedplayercolor());
    VARFP(playercolorrojo, 0, 0, sizeof(playercolorsrojo)/sizeof(playercolorsrojo[0])-1, changedplayercolor());

    static const playermodelinfo playermodels[10] =
    {
        { { "smileys/hap/red", "smileys/hap" }, "smileys/hap/cb" },
        { { "smileys/noel/red", "smileys/noel" }, "smileys/noel/cb" },
        { { "smileys/malade/red", "smileys/malade" }, "smileys/malade/cb" },
        { { "smileys/content/red", "smileys/content" }, "smileys/content/cb" },
        { { "smileys/colere/red", "smileys/colere" }, "smileys/colere/cb" },
        { { "smileys/sournois/red", "smileys/sournois" }, "smileys/sournois/cb" },
        { { "smileys/fou/red", "smileys/fou" }, "smileys/fou/cb"  },
        { { "smileys/clindoeil/red", "smileys/clindoeil" }, "smileys/clindoeil/cb" },
        { { "smileys/cool/red", "smileys/cool" }, "smileys/cool/cb" },
        { { "smileys/bug/red", "smileys/bug" }, "smileys/bug/cb" }
    };

    extern void changedplayermodel();
    VARFP(playermodel, 0, 0, sizeof(playermodels)/sizeof(playermodels[0])-1, changedplayermodel());

    int chooserandomtraits(int seed, int trait)
    {
        switch(trait)
        {
            case T_CLASSE: return (seed&0xFFFF)%(sizeof(classes)/sizeof(classes[0]));
            case T_PLAYERMODEL: return (seed&0xFFFF)%(sizeof(playermodels)/sizeof(playermodels[0]));
            case T_CAPE: return (seed&0xFFFF)%(sizeof(capes)/sizeof(capes[0]));
            case T_GRAVE: return (seed&0xFFFF)%(sizeof(graves)/sizeof(graves[0]));
            case T_TAUNT: return 0; //(seed&0xFFFF)%(sizeof(customsdance)/sizeof(customsdance[0]));
            default: return 0;
        }
    }

    const playermodelinfo *getplayermodelinfo(int n)
    {
        if(size_t(n) >= sizeof(playermodels)/sizeof(playermodels[0])) return NULL;
        return &playermodels[n];
    }

    const playermodelinfo &getplayermodelinfo(gameent *d)
    {
        const playermodelinfo *mdl = getplayermodelinfo(d==player1 || forceplayermodels ? playermodel : d->playermodel);
        if(!mdl) mdl = getplayermodelinfo(playermodel);
        return *mdl;
    }

    int getplayercolor(int team, int color)
    {
        #define GETPLAYERCOLOR(playercolors) \
            return playercolors[color%(sizeof(playercolors)/sizeof(playercolors[0]))];
        switch(team)
        {
            case 1: GETPLAYERCOLOR(playercolorsazul)
            case 2: GETPLAYERCOLOR(playercolorsrojo)
            default: GETPLAYERCOLOR(playercolors)
        }
    }

    ICOMMAND(getplayercolor, "ii", (int *color, int *team), intret(getplayercolor(*team, *color)));

    int getplayercolor(gameent *d, int team)
    {
        if(d==player1) switch(team)
        {
            case 1: return getplayercolor(1, playercolorazul);
            case 2: return getplayercolor(2, playercolorrojo);
            default: return getplayercolor(0, playercolor);
        }
        else return getplayercolor(team, (d->playercolor>>(5*team))&0x1F);
    }

    void changedplayermodel()
    {
        if(!smiley[playermodel]) { conoutf(CON_ERROR, "\f3%s", readstr("Console_Shop_SmileyNotOwned")); playSound(S_ERROR, vec(0, 0, 0), 0, 0, SND_FIXEDPITCH); playermodel = 0; return; }
        if(player1->clientnum < 0) player1->playermodel = playermodel;
        if(player1->ragdoll) cleanGrave(player1);
        loopv(curGraves)
        {
            gameent *d = curGraves[i];
            if(!d->ragdoll) continue;
            if(!forceplayermodels)
            {
                const playermodelinfo *mdl = getplayermodelinfo(d->playermodel);
                if(mdl) continue;
            }
            cleanGrave(d);
        }
        loopv(players)
        {
            gameent *d = players[i];
            if(d == player1 || !d->ragdoll) continue;
            if(!forceplayermodels)
            {
                const playermodelinfo *mdl = getplayermodelinfo(d->playermodel);
                if(mdl) continue;
            }
            cleanGrave(d);
        }
    }

    void changedplayercolor()
    {
        if(player1->clientnum < 0) player1->playercolor = playercolor | (playercolorazul<<5) | (playercolorrojo<<10);
    }

    void syncplayer()
    {
        if(player1->playermodel != playermodel)
        {
            player1->playermodel = playermodel;
            addmsg(N_SWITCHMODEL, "ri", player1->playermodel);
        }

        int col = playercolor | (playercolorazul<<5) | (playercolorrojo<<10);
        if(player1->playercolor != col)
        {
            player1->playercolor = col;
            addmsg(N_SWITCHCOLOR, "ri", player1->playercolor);
        }

        loopi(NUMSKINS) addmsg(N_SENDSKIN, "ri2", i, player1->skin[i]);
    }

    std::map<std::pair<int, bool>, std::string> weaponsPaths;

    void initWeaponsPaths()
    {
        for(int weapon = 0; weapon < NUMGUNS; ++weapon)
        {
            weaponsPaths[std::make_pair(weapon, true)] = std::string("weapons/hud/") + guns[weapon].name;
            weaponsPaths[std::make_pair(weapon, false)] = std::string("weapons/ext/") + guns[weapon].name;
        }
    }

    const char* getWeaponDir(int weapon, bool hud)
    {
        return weaponsPaths[std::make_pair(weapon, hud)].c_str();
    }

    char *getShieldDir(int type, int health, bool hud = false)
    {
        static char dir[32];
        int armourDir = 20;
        int steps = (armours[type].max / 5);

        loopi(4)
        {
            if(health < steps) break;
            health -= steps;
            armourDir += 20;
        }

        snprintf(dir, sizeof(dir), "shields/%s/%s/%d", hud || type==A_POWERARMOR ? "hud" : "ext", armours[type].name, armourDir);
        return dir;
    }

    std::map<std::pair<int, bool>, std::string> capesPaths;

    void initCapesPaths()
    {
        loopi(sizeof(capes)/sizeof(capes[0]))
        {
            capesPaths[std::make_pair(i, true)] = std::string("capes/") + capes[i].name + std::string("/enemy");
            capesPaths[std::make_pair(i, false)] = std::string("capes/") + capes[i].name;
        }
    }

    const char* getCapeDir(int cape, bool enemy = false)
    {
        return capesPaths[std::make_pair(cape, enemy)].c_str();
    }

    std::map<int, std::string> gravesPaths;

    void initGravesPaths()
    {
        loopi(sizeof(graves)/sizeof(graves[0])) gravesPaths[i] = std::string("graves/") + graves[i].name;
    }

    const char* getGraveDir(int grave)
    {
        return gravesPaths[grave].c_str();
    }

    void preloadplayermodel()
    {
        loopi(5) // Preloading all shields
        {   // armour health loop (i)
            loopj(5)
            {   // armour type loop (j)
                if(j!=A_POWERARMOR) preloadmodel(getShieldDir(j, (armours[j].max / 5) * i, false));
                preloadmodel(getShieldDir(j, (armours[j].max / 5) * i, true));
            }
        }

        loopi(sizeof(capes) / sizeof(capes[0])) // Preloading all capes
        {
            preloadmodel(getCapeDir(i));
            preloadmodel(getCapeDir(i, true));
        }

        loopi(sizeof(classes) / sizeof(classes[0])) preloadmodel(classes[i].hatDir); // Preloading all classe's hats
        loopi(sizeof(graves) / sizeof(graves[0])) preloadmodel(getGraveDir(i)); // Preloading all graves
        loopi(4) getdisguisement(i); //Preloading all spy's disguisement

        preloadmodel("smileys/armureassistee"); //Preloading powered armor playermodel
        preloadmodel("smileys/armureassistee/red");
        preloadmodel("objets/piecerobotique");
        preloadmodel("boosts/epo"); //Preloading boosts models
        preloadmodel("boosts/joint");
        preloadmodel("boosts/steros");
        preloadmodel("hudboost/joint");
        preloadmodel("mapmodel/smileys/mort");

        loopi(sizeof(playermodels)/sizeof(playermodels[0]))
        {
            const playermodelinfo *mdl = getplayermodelinfo(i);
            if(!mdl) break;
            if(i != playermodel && (!multiplayer(false) || forceplayermodels)) continue;
            if(m_teammode)
            {
                loopj(MAXTEAMS) preloadmodel(mdl->model[j]);
            }
            else preloadmodel(mdl->model[0]);
        }
    }

    int numanims() { return NUMANIMS; }

    void findanims(const char *pattern, vector<int> &anims)
    {
        loopi(sizeof(animnames)/sizeof(animnames[0])) if(matchanim(animnames[i], pattern)) anims.add(i);
    }

    VAR(animoverride, -1, 0, NUMANIMS-1);

    void renderGrave(gameent *d, float fade)
    {
        if(validGrave(d->skin[SKIN_GRAVE]))
        {
            rendermodel(getGraveDir(d->skin[SKIN_GRAVE]), ANIM_MAPMODEL|ANIM_LOOP, vec(d->o.x, d->o.y, d->o.z-16.0f), d->yaw, 0, 0, MDL_CULL_VFC|MDL_CULL_DIST|MDL_CULL_OCCLUDED, d, NULL, 0, 0, fade);
        }
    }

    static const int dirs[9] =
    {
        ANIM_LEFT,  ANIM_FORWARD,   ANIM_RIGHT,
        ANIM_LEFT,  0,              ANIM_RIGHT,
        ANIM_LEFT,  ANIM_BACKWARD,  ANIM_RIGHT
    };

    void renderplayer(gameent *d, const playermodelinfo &mdl, int color, int team, float fade, int flags = 0, bool mainpass = true)
    {
        /////////////////////////// Dead ///////////////////////////
        if(d->state==CS_DEAD && d->lastpain)
        {
            flags |= MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY;

            if(validGrave(d->skin[SKIN_GRAVE]))
            {
                d->tombepop = min(d->tombepop + (3.f / curfps), 1.0f);
                rendermodel(getGraveDir(d->skin[SKIN_GRAVE]), ANIM_MAPMODEL|ANIM_LOOP, d->feetpos(), d->yaw, 0, 0, flags, NULL, NULL, 0, 0, d->tombepop, vec4(vec::hexcolor(color), 5));
            }

            d->skeletonfade = max(d->skeletonfade - (3.f / curfps), 0.0f);
            if(d->skeletonfade) rendermodel("mapmodel/smileys/mort", ANIM_MAPMODEL, d->feetpos(), d->yaw+90, 0, 0, flags, NULL, NULL, 0, 0, d->skeletonfade);
            return;
        }

        /////////////////////////// Animations and gfx ///////////////////////////
        bool powerArmor = hasPowerArmor(d);
        int anim = ANIM_IDLE|ANIM_LOOP, lastaction = d->lastaction;
        int basetime = 0;
        if(animoverride) anim = (animoverride<0 ? ANIM_ALL : animoverride)|ANIM_LOOP;

        if(d->state==CS_EDITING || d->state==CS_SPECTATOR) anim = ANIM_EDIT|ANIM_LOOP;
        else if(d->state==CS_LAGGED)                       anim = ANIM_LAG|ANIM_LOOP;

        if(intermission && d->state!=CS_DEAD)
        {
            anim = ANIM_LOSE|ANIM_LOOP;
            if(validteam(team) ? bestteams.htfind(team)>=0 : bestplayers.find(d)>=0) anim = ANIM_WIN|ANIM_LOOP;
        }
        //else if(d->lasttaunt && lastmillis-d->lasttaunt<1000)
        //{
        //    lastaction = d->lasttaunt;
        //    anim = ANIM_TAUNT|ANIM_LOOP;
        //}
        else if(!intermission && forcecampos<0)
        {
            if(d->inwater && d->physstate<=PHYS_FALL)
            {
                anim |= (((d->move || d->strafe) || d->vel.z+d->falling.z>0 ? ANIM_SWIM : ANIM_SINK)|ANIM_LOOP)<<ANIM_SECONDARY;
                if(d->move && rndevent(95)) particle_splash(PART_WATER, powerArmor ? 3 : 2, 120, d->o, 0x222222, 8.0f+rnd(powerArmor ? 8 : 5), 150, 15);
            }
            else
            {
                int dir = dirs[(d->move+1)*3 + (d->strafe+1)];
                if(d->timeinair>50) anim |= ((ANIM_JUMP) | ANIM_END) << ANIM_SECONDARY;
                else if(dir) anim |= (dir | ANIM_LOOP) << ANIM_SECONDARY;

                if(d->move && d->physstate==PHYS_FLOOR && rndevent(95)) particle_splash(atmos && (lookupmaterial(d->feetpos())==MAT_WATER || map_atmo==4 || map_atmo==8) ? PART_WATER : PART_SMOKE, powerArmor ? 5 : 3, 120, d->feetpos(), map_atmo==4 && atmos ? 0x131313 : map_atmo==9 ? 0xFFFFFF : 0x333022, 6.0f+rnd(d->armourtype==A_POWERARMOR ? 10 : 5), 150, 15);
            }
            if(d->crouching && d->timeinair<50) anim |= (ANIM_CROUCH|ANIM_END)<<ANIM_SECONDARY;

            if((anim&ANIM_INDEX)==ANIM_IDLE && (anim>>ANIM_SECONDARY)&ANIM_INDEX) anim >>= ANIM_SECONDARY;
        }
        if(!((anim>>ANIM_SECONDARY)&ANIM_INDEX)) anim |= (ANIM_IDLE|ANIM_LOOP)<<ANIM_SECONDARY;

        /////////////////////////// Main player model ///////////////////////////
        const char *mdlname;
        if(d==player1 || m_tutorial) mdlname = (powerArmor || d->ammo[GUN_POWERARMOR] ? "smileys/armureassistee" : mdl.model[1]); // player1 is always yellow
        else
        {
            if(powerArmor || d->ammo[GUN_POWERARMOR]) mdlname = d->team==player1->team && validteam(team) ? "smileys/armureassistee" : "smileys/armureassistee/red";
            else mdlname =  d->abilitymillis[ABILITY_2] && d->aptitude==C_PHYSICIST ? "smileys/phy_2" : postfx::cbfilter && d->team==player1->team ? mdl.cbmodel : mdl.model[validteam(team) && d->team==player1->team ? 1 : 0];
        }

        modelattach a[10];
        int ai = 0;

        /////////////////////////// Gun ///////////////////////////
        if(validgun(d->gunselect))
        {
            int vanim = ANIM_VWEP_IDLE|ANIM_LOOP, vtime = 0;
            if(isAttacking(d))
            {
                vanim = ANIM_VWEP_SHOOT;
                vtime = lastaction;
            }

            a[ai++] = modelattach("tag_weapon", getWeaponDir(d->gunselect), vanim, vtime);

            if(mainpass && !(flags&MDL_ONLYSHADOW))
            {
                d->muzzle = d->balles = vec(-1, -1, -1);
                a[ai++] = modelattach("tag_muzzle", &d->muzzle);
                a[ai++] = modelattach("tag_balles", &d->balles);
            }
        }

        /////////////////////////// Shield ///////////////////////////
        if(d->armour && d->armourtype >= A_WOOD && d->armourtype <= A_MAGNET)
        {
            a[ai++] = modelattach("tag_shield", getShieldDir(d->armourtype, d->armour), 0, 0);
        }

        /////////////////////////// Boosts ///////////////////////////
        if(d->boostmillis[B_JOINT]) a[ai++] = modelattach("tag_boost1", "boosts/joint", 0, 0);
        if(d->boostmillis[B_ROIDS]) a[ai++] = modelattach("tag_boost1", "boosts/steros", 0, 0);
        if(d->boostmillis[B_EPO])   a[ai++] = modelattach("tag_boost2", "boosts/epo", 0, 0);

        /////////////////////////// Classe's hat ///////////////////////////
        if(validClass(d->aptitude)) a[ai++] = modelattach("tag_hat", classes[d->aptitude].hatDir, 0, 0);

        /////////////////////////// Player's cape ///////////////////////////
        if(validCape(d->skin[SKIN_CAPE]))
        {
            a[ai++] = modelattach("tag_cape", getCapeDir(d->skin[SKIN_CAPE], d->team == player1->team ? false : true), 0, 0);
        }

        if(d!=player1) flags |= MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY;
        if(d->type==ENT_PLAYER) flags |= MDL_FULLBRIGHT;
        else flags |= MDL_CULL_DIST;
        if(!mainpass) flags &= ~(MDL_FULLBRIGHT | MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY | MDL_CULL_DIST);
        float trans = d->state == CS_LAGGED ? 0.5f : 1.0f;
        if(d->abilitymillis[ABILITY_2] && d->aptitude==C_PHYSICIST) trans = 0.f;
        else if(d->abilitymillis[ABILITY_1] && d->aptitude==C_WIZARD) trans = 0.7f;

        if(d->aptitude==C_SPY && d->abilitymillis[ABILITY_1])
        {
            if(d!=hudplayer()) flags = NULL;
            vec doublepos = d->feetpos();
            const int positions[4][2] = { {25, 25}, {-25, -25}, {25, -25}, {-25, 25} };
            doublepos.add(vec(positions[d->seed][0], positions[d->seed][1], 0));
            rendermodel(mdlname, anim, doublepos, d->yaw, clamp(d->pitch, -25, 12), 0, MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY, d, a[0].tag ? a : NULL, basetime, 0, fade, vec4(vec::hexcolor(color), d==player1 ? 0.3f : trans));
        }

        if(d->aptitude==C_SPY && d->abilitymillis[ABILITY_2])
        {
            rendermodel(getdisguisement(d->seed), anim, d->feetpos(), d->yaw, clamp(d->pitch, -25, 12), 0, flags, d, NULL, basetime, 0, fade, vec4(vec::hexcolor(color), 1.0f));
            return;
        }

        rendermodel(mdlname, anim, d->feetpos(), d->yaw, clamp(d->pitch, -25, 12), 0, flags, d, a[0].tag ? a : NULL, basetime, 0, fade, vec4(vec::hexcolor(color), trans));

        /////////////////////////// first person body ///////////////////////////
        if(d==hudplayer() && forcecampos<0 && !thirdperson)
        {
            vec pos = d->feetpos();
            rendermodel(mdlname, anim, pos.addz(3.5f), d->yaw, 28, 0, MDL_NOSHADOW, d, NULL, basetime, 0, 1.f, vec4(vec::hexcolor(color), trans));
        }
    }

    void renderplayerui(gameent *d, const playermodelinfo &mdl, int cape, int color, int team)
    {
        int anim = ANIM_IDLE|ANIM_LOOP;

        modelattach a[4];
        int ai = 0;
        if(validgun(d->gunselect))
        {
            int vanim = ANIM_VWEP_IDLE|ANIM_LOOP, vtime = 0;
            a[ai++] = modelattach("tag_weapon", getWeaponDir(d->gunselect), vanim, vtime);
        }

        vec o = d->feetpos();

        const char *mdlname = mdl.model[validteam(team) ? team : 0];

        a[ai++] = modelattach("tag_hat", classes[player1->aptitude].hatDir, 0, 0);
        a[ai++] = modelattach("tag_cape", getCapeDir(cape, !team), 0, 0);

        rendermodel(mdlname, anim, o, d->yaw, d->pitch, 0, NULL, d, a[0].tag ? a : NULL, 0, 0, 1, vec4(vec::hexcolor(color), 5));
    }

    static inline void renderplayer(gameent *d, float fade = 1, int flags = 0)
    {
        int team = m_teammode && validteam(d->team) ? d->team : 0;
        renderplayer(d, getplayermodelinfo(d), getplayercolor(d, team), team, fade, flags);
    }

    bool drawManaStat(gameent *d)
    {
        return d->aptitude==C_WIZARD || d->aptitude==C_PHYSICIST || d->aptitude==C_PRIEST || d->aptitude==C_SHOSHONE || d->aptitude==C_SPY;
    }

    FVARP(huddamagesize, 0.01f, 0.11f, 1.f);
    VARP(showspecplayerinfo, 0, 0, 1);

    void renderWeaponParticles(gameent *d)
    {
        if(!rndevent(95)) return;
        switch(d->gunselect)
        {
            case GUN_MOLOTOV:
                particle_splash(PART_FIRE_BALL, 2, 80, d->balles, 0xFFC864, 1, 30, 30, 0, hasShrooms());
                particle_splash(PART_SMOKE, 3, 180, d->balles, 0x444444, 2, 40, 50, 0, hasShrooms());
                particle_splash(PART_AR, 2, 250, d->balles, 0xFFFFFF, 2, 40, 50, 5);
                break;
        }
    }

    void renderPlayerTextInfo(gameent *d, float dist)
    {
        copystring(d->info, colorname(d));

        if(d->curdamage) // damage dealt displayed on hud
        {
            vec pos = d->abovehead();
            float dist = d->o.dist(camera1->o);
            float up = 5 + dist/40.f + (((totalmillis - d->lastcurdamage) / 50.f) / (dist <= 160 ? 160.f - dist : 1)); // particle going up effect
            if(!ispaused()) pos.z += up - (15 * (1 - (clamp(dist, 0.f, 160.f) / 160.f)));
            float size = (zoom ? huddamagesize * (guns[player1->gunselect].maxzoomfov) / 100.f : huddamagesize) * 1.5f;
            particles::text(pos, tempformatstring("%d", d->curdamage), PART_TEXT, 1, d->curdamagecolor, size, 0, true);
        }

        if(player1->state==CS_SPECTATOR && showspecplayerinfo)
        {
            float metersize = 0.08f;
            vec textpos = d->abovehead();
            textpos.addz(dist/32.f);
            particles::text(textpos, tempformatstring("%s", d->name), PART_TEXT, 1, d->state==CS_ALIVE ? (d->team==1 ? 0xFFFF00 : 0xFF0000) : 0x595959, metersize, 0, true);
            if(d->state==CS_ALIVE) particles::meter(d->abovehead(), d->health/1000.0f, PART_METER, 1, rygbGradient(d->health/10), 0x000000, metersize, true);
        }
    }

    void spawnPlayerBouncers(gameent *d, vec pos)
    {
        if(d->health < 300)
        {
            if(rndevent(1)) spawnbouncer(d->o, d->vel, d, BNC_PIXEL, 75);
            if(d->health < 150 && rndevent(94)) particle_splash(PART_BLOOD, 1, 2500, pos, 0x60FFFF, 1.f+rnd(2), 50);
        }

        if(hasPowerArmor(d) && d->armour < 1500)
        {
            bool lowArmour = (d->armour < 750);
            if(rndevent(lowArmour ? 98 : 95)) regularflame(PART_SMOKE, pos, 15, 3, (lowArmour ? 0x222222 : 0x777777), 1, (lowArmour ? 2.5f : 2), 50, (lowArmour ? 1750 : 1250), -10, 3);
            if(d->armour < 1000 && rndevent(lowArmour ? 95 : 92)) particle_splash(PART_FIRE_BALL, (1 + lowArmour), 500, pos, rnd(2) ? 0x992200 : 0x886622, (d->armour<500 ? 5 : 3), 50, -20);
            if(lowArmour && rndevent(1)) spawnbouncer(d->o, d->vel, d, BNC_SCRAP, 50);
        }
    }

    void spawnPlayerParticles(gameent *d, vec pos, bool exceptHud)
    {
        if(d->afterburnmillis && rndevent(94)) particle_splash(PART_FIRE_BALL, 2, 350, pos, rnd(2) ? 0x992200 : 0x886622, 5, 70, -20, 5);

        if(d->boostmillis[B_SHROOMS] && rndevent(97))
        {
            regularflame(PART_SPARK, d->feetpos(), 12, 2, particles::getRandomColor(), 2, 0.4f, 10.f, 500, 0, -2);
            if(exceptHud) particle_splash(PART_SMOKE, 2, 150, d->o, particles::getRandomColor(), 12+rnd(5), 400, 200);
        }

        if(d->boostmillis[B_JOINT] && rndevent(93)) regularflame(PART_SMOKE, d->abovehead().add(vec(-12, 5, -19)), 2, 3, 0x888888, 1, 1.6f, 50.0f, 1000.0f, -10);

        switch(d->aptitude)
        {
            case C_WIZARD:
                if(d->abilitymillis[ABILITY_1]) particle_splash(PART_SMOKE, 2, 120, d->o, 0xFF33FF, 10+rnd(5), 400,400);
                if(d->abilitymillis[ABILITY_3] && exceptHud) particle_fireball(pos, 15.2f, PART_EXPLOSION, 5,  0x880088, 13.0f);
                break;
            case C_PHYSICIST:
                if(d->abilitymillis[ABILITY_2] && rndevent(97)) particle_splash(PART_SMOKE, 1, 300, d->o, 0x7777FF, 10+rnd(5), 400, 400);
                if(d->abilitymillis[ABILITY_3] && rndevent(98))
                {
                    particle_splash(PART_SMOKE, 1, 200, d->feetpos(), 0x665544, 7+rnd(4), 175, -200);
                    particle_splash(PART_FIRE_BALL, 4, 150, d->feetpos(), !rnd(2) ? 0xFFAA00 : 0xFF3300, 1+rnd(2), 150, -50);
                }
                break;
            case C_PRIEST:
                if(d->abilitymillis[ABILITY_2]) particle_fireball(pos , 16.0f, PART_SHOCKWAVE, 5, 0xFFFF00, 16.0f);
                break;
            case C_VIKING:
                if(d->boostmillis[B_RAGE] && rndevent(97) && exceptHud) particle_splash(PART_SMOKE, 2, 150, d->o, 0xFF3300, 12+rnd(5), 400, 200);
                break;
            case C_SHOSHONE:
                if(rndevent(98))
                {
                    if(d->abilitymillis[ABILITY_1]) regularflame(PART_SPARK, d->feetpos(), 12, 2, 0xAAAAAA, 2, 0.4f, 10.f, 500, 0, -2);
                    if(d->abilitymillis[ABILITY_2]) regularflame(PART_SPARK, d->feetpos(), 12, 2, 0xFF33FF, 2, 0.4f, 10.f, 500, 0, -2);
                    if(d->abilitymillis[ABILITY_3]) regularflame(PART_SPARK, d->feetpos(), 12, 2, 0xFF3333, 2, 0.4f, 10.f, 500, 0, -2);
                }
        }
    }

    void renderPlayerIcons(gameent *d, vec pos, float dist)
    {
        if(isteam(hudplayer()->team, d->team))
        {
            float metersize = 0.08f / (dist / 125);
            if(hudplayer()->aptitude==C_MEDIC)
            {
                if(dist <= 250) particles::meter(d->abovehead(), d->health/1000.0f, PART_METER, 1, rygbGradient(d->health/10), 0x000000, metersize, true);
                if(d->health < 500)
                {
                    int blinkSpeed = 1001 - (500 - d->health) / 2;
                    particles::hudIcon(PART_HEALTH, pos, (totalmillis % blinkSpeed < blinkSpeed / 2) ? 0x111111 : 0xFFFFFF, 0.075f);
                }
            }
            else if(hudplayer()->aptitude==C_JUNKIE)
            {
                if(drawManaStat(d))
                {
                    if(dist <= 250) particles::meter(d->abovehead(), d->mana/150.0f, PART_METER, 1, 0xFF00FF, 0x000000, metersize, true);
                    if(d->mana < 50)
                    {
                        int blinkSpeed = 1001 - (50 - d->mana) / 2;
                        particles::hudIcon(PART_MANA, pos, (totalmillis % blinkSpeed < blinkSpeed / 2) ? 0xFF00FF : 0xFFFFFF, 0.075f);
                    }
                }
            }
        }
        else if((hudplayer()->aptitude==C_SPY && hudplayer()->abilitymillis[ABILITY_3]) || totalmillis - getspyability < 2000)
        {
            particles::hudIcon(PART_VISEUR, pos, 0xBBBBBB);
        }
    }

    void rendergame()
    {
        ai::render();
        bool thirdPerson = isthirdperson();

        gameent *f = followingplayer(), *exclude = thirdPerson ? NULL : f;

        loopv(players)
        {
            gameent *d = players[i];

            if(d->state==CS_SPECTATOR || d->state==CS_SPAWNING || d->lifesequence < 0 || d == exclude || (d->state==CS_DEAD && hidedead)) continue; // skip invisible players

            bool isHudPlayer = (d==hudplayer());
            bool exceptHud = (!isHudPlayer || thirdPerson);
            float distance = d->o.dist(camera1->o);

            if(d!=player1) renderplayer(d);
            renderPlayerTextInfo(d, distance);

            if(d->state==CS_ALIVE && !ispaused())
            {
                vec playerCenter = d->o;
                playerCenter.subz(8);

                renderWeaponParticles(d);
                spawnPlayerBouncers(d, playerCenter);
                spawnPlayerParticles(d, playerCenter, exceptHud);
                if(!isHudPlayer) renderPlayerIcons(d, playerCenter, distance);
            }
        }

        loopv(curGraves)
        {
            gameent *d = curGraves[i];
            float fade = 1.0f;
            if(ragdollmillis && ragdollfade) fade -= clamp(float(lastmillis - (d->lastupdate + max(ragdollmillis - ragdollfade, 0)))/min(ragdollmillis, ragdollfade), 0.0f, 1.0f);
            renderGrave(d, fade);
        }

        rendermonsters();
        entities::renderentities();
        renderbouncers();
        renderprojectiles();

        if(exclude) renderplayer(exclude, 1, MDL_ONLYSHADOW);
        else if(!f && (player1->state==CS_ALIVE || (player1->state==CS_EDITING && thirdPerson) || (player1->state==CS_DEAD && !hidedead))) renderplayer(player1, 1, thirdPerson ? 0 : MDL_ONLYSHADOW);

        if(cmode) cmode->rendergame();
    }

    VARP(hudgun, 0, 1, 1);
    VARP(hudgunsway, 0, 1, 1);

    FVAR(swaystep, 1, 35.0f, 100);
    FVAR(swayside, 0, 3, 3);
    FVAR(swayup, -1, 2, 2);

    float swayfade = 0, swayspeed = 0, swaydist = 0;
    vec swaydir(0, 0, 0);

    void swayhudgun(int curtime)
    {
        gameent *d = hudplayer();
        if(d->state != CS_SPECTATOR)
        {
            if(d->physstate >= PHYS_SLOPE)
            {
                swayspeed = min(sqrtf(d->vel.x*d->vel.x + d->vel.y*d->vel.y), 200 - classes[d->aptitude].speed*0.12f);
                swaydist += swayspeed*curtime/1000.0f;
                swaydist = fmod(swaydist, 2*swaystep);
                swayfade = 1;
            }
            else if(swayfade > 0)
            {
                swaydist += swayspeed*swayfade*curtime/1000.0f;
                swaydist = fmod(swaydist, 2*swaystep);
                swayfade -= 0.5f*(curtime*d->maxspeed)/(swaystep*1000.0f);
            }

            float k = pow(0.7f, curtime/10.0f);
            swaydir.mul(k);
            vec vel(d->vel);
            vel.add(d->falling);
            swaydir.add(vec(vel).mul((1-k)/(15*max(vel.magnitude(), d->maxspeed))));
        }
    }

    struct hudent : dynent
    {
        hudent() { type = ENT_CAMERA; }
    } guninterp;

    vec2 hudgunDisp(0, 40);

    void drawhudmodel(gameent *d, int anim, int basetime)
    {
        if(!validgun(d->gunselect)) return;

        int delta = max((250 / max(curfps, 1)) * (game::gamespeed / 100.f), 1.f);
        int h = zoom ? 2 : -2;
        int v = zoom ? -1 : 1;

        if(!ispaused())
        {
            hudgunDisp.x = clamp(int(hudgunDisp.x) + h * delta, 1, int(guns[hudplayer()->gunselect].weapDisp.x));
            hudgunDisp.y = clamp(int(hudgunDisp.y) + v * delta, 1, int(guns[hudplayer()->gunselect].weapDisp.y));
        }

        vec sway;
        vecfromyawpitch(d->yaw, 0, 0, 1, sway);
        float steps = swaydist/swaystep*M_PI;
        sway.mul((swayside)*cosf(steps));
        vec gunAim;

        sway.z = -hudgunDisp.y - (zoom ? 2 : 0);
        if(!zoom) vecfromyawpitch(d->yaw, 0, -10, hudgunDisp.x, gunAim);

        sway.z += swayup*(fabs(sinf(steps)) - 1);
        sway.add(swaydir).add(d->o);
        if(!hudgunsway) sway = d->o;

        vecfromyawpitch(d->yaw, 0, 0, hudgunDisp.x, gunAim);

        int team = m_teammode && validteam(d->team) ? d->team : 0,
            color = getplayercolor(d, team);

        modelattach a[3];
        int ai = 0;
        d->muzzle = d->balles = vec(-1, -1, -1);
        a[ai++] = modelattach("tag_muzzle", &d->muzzle);
        a[ai++] = modelattach("tag_balles", &d->balles);
        if((d->gunselect==GUN_MINIGUN || d->gunselect==GUN_FLAMETHROWER || d->gunselect==GUN_PLASMA || d->gunselect==GUN_UZI || d->gunselect==GUN_S_GAU8) && anim!=ANIM_GUN_MELEE)
        {
            anim |= ANIM_LOOP;
            basetime = 0;
        }

        float trans = 1.0f;
        if(d->abilitymillis[ABILITY_2] && d->aptitude==C_PHYSICIST) trans = 0.08f;
        else if(d->abilitymillis[ABILITY_1] && d->aptitude==C_WIZARD) trans = 0.7f;

        if(d->boostmillis[B_JOINT])
        {
            vec sway3;
            vecfromyawpitch(d->yaw, 0, 0, 1, sway3);
            sway3.mul((swayside/1.5f)*cosf(steps));
            sway3.z += (swayup/1.5f)*(fabs(sinf(steps)) - 1);
            sway3.add(swaydir).add(d->o);

            modelattach a[2];
            d->weed = vec(-1, -1, -1);
            a[0] = modelattach("tag_joint", &d->weed);
            rendermodel("hudboost/joint", anim, sway3, d->yaw, d->pitch, 0, MDL_NOBATCH, NULL, a, basetime, 0, 1, vec4(vec::hexcolor(color), trans));
            if(d->weed.x >= 0) d->weed = calcavatarpos(d->weed, 12);
            if(rndevent(93)) {regularflame(PART_SMOKE, d->weed, 2, 3, 0x888888, 1, 1.3f, 50.0f, 1000.0f, -10); particle_splash(PART_FIRE_BALL,  4, 50, d->weed, 0xFF6600, 0.6f, 20, 150);}
        }

        rendermodel(getWeaponDir(d->gunselect, true), anim, gunAim.add(sway), d->yaw, d->pitch, 0, MDL_NOBATCH, NULL, a, basetime, 0, 1, vec4(vec::hexcolor(color), trans));

        if(d->muzzle.x >= 0) d->muzzle = calcavatarpos(d->muzzle, 12);
        if(d->balles.x >= 0) d->balles = calcavatarpos(d->balles, 12);

        if(d->armour && d->armourtype >= A_WOOD && d->armourtype < NUMSHIELDS)
        {
            bool powerArmor = d->armourtype==A_POWERARMOR;
            vec sway2;
            vecfromyawpitch(d->yaw, 0, 0, 1, sway2);
            float swaydiv = powerArmor ? 6.f : 3.f;
            sway2.mul((swayside/swaydiv)*cosf(steps));
            sway2.z += (swayup/swaydiv)*(fabs(sinf(steps)) - 1);
            if(powerArmor || !zoom) sway2.add(swaydir).add(d->o);
            rendermodel(getShieldDir(d->armourtype, d->armour, true), anim, sway2, d->yaw, d->pitch, 0, MDL_NOBATCH, NULL, a, basetime, 0, 1, vec4(vec::hexcolor(color), trans));
        }
    }

    void drawhudgun()
    {
        gameent *d = hudplayer();
        if(d->state==CS_SPECTATOR || d->state==CS_EDITING || !hudgun || editmode)
        {
            d->muzzle = player1->muzzle = vec(-1, -1, -1);
            d->balles = player1->balles = vec(-1, -1, -1);
            return;
        }

        int anim = ANIM_GUN_IDLE|ANIM_LOOP, basetime = 0;

        if(isAttacking(d)) { anim = ANIM_GUN_SHOOT; basetime = d->lastaction; }

        drawhudmodel(d, anim, basetime);
    }

    void renderavatar()
    {
        drawhudgun();
    }

    void renderplayerpreview(int model, int cape, int color, int team, int weap, int yaw, bool rot)
    {
        static gameent *previewent = NULL;
        if(!previewent)
        {
            previewent = new gameent;
            loopi(NUMGUNS) previewent->ammo[i] = 1;
        }
        float height = previewent->eyeheight + previewent->aboveeye,
              zrad = height/1.6f;
        vec2 xyrad = vec2(previewent->xradius, previewent->yradius).max(height/3);
        if(rot) previewent->yaw = fmod(lastmillis/20000.0f*360.0f, 360.0f);
        else previewent->yaw = yaw;
        previewent->o = calcmodelpreviewpos(vec(xyrad, zrad)).addz(previewent->eyeheight - zrad);
        previewent->gunselect = validgun(weap) ? weap : GUN_ELECTRIC;
        const playermodelinfo *mdlinfo = getplayermodelinfo(model);
        if(!mdlinfo) return;
        renderplayerui(previewent, *mdlinfo, cape, getplayercolor(team, color), team);
    }

    vec hudgunorigin(int gun, const vec &from, const vec &to, gameent *d)
    {
        if(d->muzzle.x >= 0) return d->muzzle;
        vec offset(from);
        if(d!=hudplayer() || isthirdperson())
        {
            vec front, right;
            vecfromyawpitch(d->yaw, d->pitch, 1, 0, front);
            offset.add(front.mul(d->radius));
            if(d->type!=ENT_AI)
            {
                offset.z += (d->aboveeye + d->eyeheight)*0.75f - d->eyeheight;
                vecfromyawpitch(d->yaw, 0, 0, -1, right);
                offset.add(right.mul(0.5f*d->radius));
                offset.add(front);
            }
            return offset;
        }
        offset.add(vec(to).sub(from).normalize().mul(2));
        if(hudgun)
        {
            offset.sub(vec(camup).mul(1.0f));
            offset.add(vec(camright).mul(0.8f));
        }
        else offset.sub(vec(camup).mul(0.8f));
        return offset;
    }

    void preloadweapons()
    {
        loopi(NUMGUNS)
        {
            preloadmodel(getWeaponDir(i));
            preloadmodel(getWeaponDir(i, true));
        }
    }

    void initAssetsPaths()
    {
        initWeaponsPaths();
        initCapesPaths();
        initGravesPaths();
        initBouncersPaths();
    }

    void preload()
    {
        if(hudgun) preloadweapons();
        if(m_tutorial || m_sp || m_dmsp) preloadmonsters();
        preloadbouncers();
        preloadProjectiles();
        preloadplayermodel();
        entities::preloadentities();
    }
}

