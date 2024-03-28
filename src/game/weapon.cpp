// weapon.cpp: all shooting and effects code, projectile management
#include "game.h"
#include "engine.h"
#include "gfx.h"
#include "stats.h"

int lastshoot;

namespace game
{
    VARP(temptrisfade, 2500, 15000, 120000);
    static const int OFFSETMILLIS = 500;
    vec rays[MAXRAYS];

    struct hitmsg
    {
        int target, lifesequence, info1, info2;
        ivec dir;
    };
    vector<hitmsg> hits;

    ICOMMAND(getweapon, "", (), intret(player1->gunselect));

    void gunselect(int gun, gameent *d, bool force, bool shortcut)
    {
        if((gun==GUN_ASSISTXPL && !force) || !validgun(gun)) return;
        if(gun!=d->gunselect)
        {
            if(d==player1 && gun!=GUN_ASSISTXPL && !shortcut) player1->lastweap = gun;
            addmsg(N_GUNSELECT, "rci", d, gun);
            playSound(attacks[gun-GUN_ELEC].picksound, d==hudplayer() ? vec(0, 0, 0) : d->o, 200, 50, NULL, d->entityId);
        }
        d->gunselect = gun;
    }

    ICOMMAND(launchgrenade, "", (), // shortcut for grenade attack, then select old gun
    {
        if(!isconnected() || m_identique) return;
        if(!player1->ammo[GUN_M32]) playSound(S_NOAMMO);
        else
        {
            gunselect(GUN_M32, player1, false, true);
            doaction(ACT_SHOOT);
            execute("sleep 500 [shoot ; getoldweap]");
        }
    });

    ICOMMAND(meleeattack, "", (), // shortcut for melee attack, then select old gun
        if(!isconnected() || m_identique) return;
        if(player1->aptitude==APT_NINJA) gunselect(GUN_CACNINJA, player1, false, true);
        else loopi(4) { if(player1->ammo[GUN_CAC349+i]) { gunselect(GUN_CAC349+i, player1, false, true); break; } }
        doaction(ACT_SHOOT);
        execute("sleep 500 [shoot ; getoldweap]");
    );

    ICOMMAND(getoldweap, "", (), { if(isconnected() || !m_identique) gunselect(player1->lastweap, player1); });

    void getsuperweap() // used to select superweapon with identical weapons mutator
    {
         loopi(4) { if(player1->ammo[GUN_S_NUKE+i]) { gunselect(GUN_S_NUKE+i, player1); return; } }
         gunselect(currentIdenticalWeapon, player1);
    }

    void nextweapon(int dir, bool force = false)
    {
        if(player1->state!=CS_ALIVE) return;
        if(m_identique)
        {
            switch(player1->aptitude)
            {
                case APT_KAMIKAZE:
                    if(player1->gunselect==currentIdenticalWeapon) {dir-1 ? gunselect(GUN_KAMIKAZE, player1) : getsuperweap();}
                    else gunselect(currentIdenticalWeapon, player1);
                    return;
                case APT_NINJA:
                    if(player1->gunselect==currentIdenticalWeapon){dir-1 ? gunselect(GUN_CACNINJA, player1) : getsuperweap();}
                    else gunselect(currentIdenticalWeapon, player1);
                    return;
                default:
                    if(player1->gunselect==currentIdenticalWeapon) getsuperweap();
                    else gunselect(currentIdenticalWeapon, player1);
                    return;
            }
        }
        dir = (dir < 0 ? NUMGUNS-1 : 1);
        int gun = player1->gunselect;
        loopi(NUMGUNS)
        {
            gun = (gun + dir)%NUMGUNS;
            if(gun==GUN_ASSISTXPL)gun = (gun + dir)%NUMGUNS;
            if(force || player1->ammo[gun]) break;
        }
        if(gun != player1->gunselect) {gunselect(gun, player1); weapposup=40; }
        else playSound(S_NOAMMO);
    }
    ICOMMAND(nextweapon, "ii", (int *dir, int *force), nextweapon(*dir, *force!=0));

    int getweapon(const char *name)
    {
        if(isdigit(name[0])) return parseint(name);
        else
        {
            int len = strlen(name);
            loopi(sizeof(guns)/sizeof(guns[0])) if(!strncasecmp(guns[i].name, name, len)) return i;
        }
        return -1;
    }

    void setweapon(const char *name, bool force = false)
    {
        int gun = getweapon(name);
        if(player1->state!=CS_ALIVE || !validgun(gun) || gun==GUN_ASSISTXPL) return;
        if(force || player1->ammo[gun]) gunselect(gun, player1);
        else playSound(S_NOAMMO);
    }
    ICOMMAND(setweapon, "si", (char *name, int *force), setweapon(name, *force!=0));

    void cycleweapon(int numguns, int *guns, bool force = false)
    {
        if(numguns<=0 || player1->state!=CS_ALIVE) return;
        int offset = 0;
        loopi(numguns) if(guns[i] == player1->gunselect) { offset = i+1; break; }
        loopi(numguns)
        {
            int gun = guns[(i+offset)%numguns];
            if(gun>=0 && gun<NUMGUNS && (force || player1->ammo[gun]) && gun!=GUN_ASSISTXPL)
            {
                gunselect(gun, player1);
                return;
            }
        }
        playSound(S_NOAMMO);
    }
    ICOMMAND(cycleweapon, "V", (tagval *args, int numargs),
    {
         int numguns = min(numargs, 3);
         int guns[3];
         loopi(numguns) guns[i] = getweapon(args[i].getstr());
         cycleweapon(numguns, guns);
    });

    void weaponswitch(gameent *d)
    {
        if(d->state != CS_ALIVE) return;

        loopi(NUMGUNS)
        {
            if(d->ammo[i] && d->gunselect!=i && i!=GUN_ASSISTXPL)
            {
                gunselect(i, d);
                break;
            }
        }
    }

    ICOMMAND(weapon, "V", (tagval *args, int numargs),
    {
        if(player1->state!=CS_ALIVE) return;
        loopi(3)
        {
            const char *name = i < numargs ? args[i].getstr() : "";
            if(name[0])
            {
                int gun = getweapon(name);
                if(validgun(gun) && gun != player1->gunselect && player1->ammo[gun] && gun!=GUN_ASSISTXPL) { gunselect(gun, player1); return; }
            } else { weaponswitch(player1); return; }
        }
        playSound(S_NOAMMO);
    });

    bool spreadLimit(gameent *d) { return d->gunselect == GUN_LANCEFLAMMES || d->gunselect == GUN_MOSSBERG || d->gunselect == GUN_HYDRA; }

    void offsetray(const vec &from, const vec &to, float spread, float range, vec &dest, gameent *d)
    {
        vec offset;
        do offset = vec(rndscale(1), rndscale(1), rndscale(1)).sub(0.5f);
        while(offset.squaredlen() > 0.5f*0.5f);

        if(d->boostmillis[B_SHROOMS]) spread /= d->aptitude==APT_JUNKIE ? 1.75f : 1.5f;
        if(d->aptitude==APT_MAGICIEN && d->abilitymillis[ABILITY_2]) spread /= 3.f;

        if(!spreadLimit(d))
        {
            if(d->crouching) spread /= d->aptitude==APT_CAMPEUR ? 2.5f : 1.3f;
            if(d->boostmillis[B_ROIDS] || d->boostmillis[B_RAGE]) spread *= 1.75f;
        }

        spread = (spread*100) / classes[d->aptitude].accuracy;
        offset.mul((to.dist(from)/1024) * spread);

        offset.z /= 2;
        dest = vec(offset).add(to);
        if(dest != from)
        {
            vec dir = vec(dest).sub(from).normalize();
            raycubepos(from, dir, dest, range, RAY_CLIPMAT|RAY_ALPHAPOLY);
        }
    }

    float spread(int atk, gameent *d) { return d->aiming ? attacks[atk].aimspread : attacks[atk].noaimspread; }

    void createrays(int atk, const vec &from, const vec &to, gameent *d)             // create random spread of rays
    {
        loopi(attacks[atk].rays) offsetray(from, to, spread(atk, d), attacks[atk].range, rays[i], d);
    }

    struct bouncer : physent
    {
        size_t entityId;
        int lifetime, bounces;
        float lastyaw, lastpitch, roll;
        bool local;
        gameent *owner;
        int bouncetype, variant;
        vec offset;
        int offsetmillis;
        int id;
        bool inwater;

        bouncer() : entityId(entitiesIds::getNewId()), bounces(0), roll(0), variant(0)
        {
            type = ENT_BOUNCE;
        }
    };

    vector<bouncer *> curBouncers;

    static const struct bouncerConfig { const char *name; float size, bounceIntensity; int variants, cullDist, bounceSound, bounceSoundRad, soundFlag; } bouncers[NUMBOUNCERS] =
    {
        { "grenade",            1.0f, 1.0f,  0,  750, S_B_GRENADE,      220,    0              },
        { "pixel",              1.0f, 0.6f,  8,  600, S_B_PIXEL,        120,    0              },
        { "rock",               0.8f, 0.8f,  4,  750, S_B_ROCK,         120,    0              },
        { "rock/big",           2.5f, 0.4f,  3, 1250, S_B_BIGROCK,      300,    0              },
        { "casing",             0.2f, 1.0f,  0,  250, S_B_CASING,       120,    SND_FIXEDPITCH },
        { "casing/big",         0.2f, 1.0f,  0,  250, S_B_BIGCASING,    120,    SND_FIXEDPITCH },
        { "casing/cartridge",   0.2f, 1.0f,  0,  250, S_B_CARTRIDGE,    120,    SND_FIXEDPITCH },
        { "scrap",              1.5f, 0.7f,  3,  750, S_B_SCRAP,        180,    0              },
        { NULL,                 0.1f, 1.0f,  0,  500, -1,                 0,    0              }
    };

    std::map<std::pair<int, int>, std::string> bouncersPaths;

    void initBouncersPaths() // store the path of all bouncers and their variants
    {
        for (int type = 0; type < NUMBOUNCERS; ++type)
        {
            for (int variant = 0; variant <= bouncers[type].variants; ++variant)
            {
                char dir[32];
                if(variant) snprintf(dir, sizeof(dir), "bouncers/%s/%d", bouncers[type].name, variant);
                else snprintf(dir, sizeof(dir), "bouncers/%s", bouncers[type].name);
                bouncersPaths[std::make_pair(type, variant)] = dir;
            }
        }
    }

    const char *getBouncerDir(int type, int variant)
    {
        return bouncersPaths[std::make_pair(type, variant)].c_str();
    }

    void newbouncer(const vec &from, const vec &to, bool local, int id, gameent *owner, int type, int lifetime, int speed)
    {
        bouncer &bnc = *curBouncers.add(new bouncer);
        bnc.o = from;
        bnc.radius = bnc.xradius = bnc.yradius = bouncers[type].size;
        bnc.eyeheight = bnc.radius;
        bnc.aboveeye = bnc.radius;
        bnc.lifetime = lifetime;
        bnc.local = local;
        bnc.owner = owner;
        bnc.bouncetype = type;
        bnc.id = local ? lastmillis : id;
        bnc.inwater = ((lookupmaterial(bnc.o) & MAT_WATER) == MAT_WATER);
        if(bouncers[type].variants) bnc.variant = rnd(bouncers[type].variants) + 1;
        else bnc.variant = 0;
        bnc.collidetype = COLLIDE_ELLIPSE;

        vec dir(to);
        dir.sub(from).safenormalize();
        bnc.vel = dir;
        bnc.vel.mul(speed);

        avoidcollision(&bnc, dir, owner, 0.1f);

        if(type==BNC_GRENADE)
        {
            bnc.offset = hudgunorigin(GUN_M32, from, to, owner);
            if(owner==hudplayer() && !isthirdperson()) bnc.offset.sub(owner->o).rescale(16).add(owner->o);
            playSound(S_GRENADE, bnc.o, 300, 100, SND_FIXEDPITCH|SND_NOCULL, bnc.entityId);
        }

        bnc.offset = from;
        bnc.offset.sub(bnc.o);
        bnc.offsetmillis = OFFSETMILLIS;

        bnc.resetinterp();
    }

    void bounced(physent *d, const vec &surface)
    {
        if(d->type != ENT_BOUNCE) return;
        bouncer *b = (bouncer *)d;

        if(bouncers[b->bouncetype].bounceSoundRad && b->bounces < 5 && b->type != BNC_LIGHT) playSound(bouncers[b->bouncetype].bounceSound, b->o, bouncers[b->bouncetype].bounceSoundRad, bouncers[b->bouncetype].bounceSoundRad / 2, SND_LOWPRIORITY|bouncers[b->bouncetype].soundFlag);
        if(b->bouncetype == BNC_GRENADE) addstain(STAIN_PLASMA_GLOW, vec(b->o).sub(vec(surface).mul(b->radius)), surface, 4.f, 0x0000FF);

        b->bounces++;
    }

    void updatebouncers(int time)
    {
        loopv(curBouncers)
        {
            bouncer &bnc = *curBouncers[i];
            vec old(bnc.o);

            bool stopped = false;

            if(bnc.bouncetype == BNC_GRENADE)
            {
                stopped = (bounce(&bnc, 0.6f, 0.5f, 0.8f) || (bnc.lifetime -= time) < 0);
                updateEntPos(bnc.entityId, bnc.o);
            }
            else
            {
                for(int rtime = time; rtime > 0;)
                {
                    int qtime = min(30, rtime);
                    rtime -= qtime;
                    stopped = (bnc.bounces && bnc.bouncetype == BNC_LIGHT);
                    if(bnc.bounces <= 5) bounce(&bnc, qtime / 1000.f, 0.6f, 0.5f, 1);
                    if((bnc.lifetime -= qtime) < 0) { stopped = true; break; }
                }
            }

            if(bnc.type != BNC_LIGHT)
            {
                bool inWater = ((lookupmaterial(bnc.o) & MAT_WATER) == MAT_WATER);
                if(!bnc.inwater && inWater) // the bouncer enter in water
                {
                    bnc.inwater = true;
                    particle_splash(PART_WATER, (20 * bnc.radius), 150, bnc.o, 0x28282A, (5.f * bnc.radius), (200.f * bnc.radius), -300, (10.f * bnc.radius), hasShrooms());
                    if(bnc.radius > 0.5f) playSound(bnc.radius > 1.f ? S_SPLASH : S_IMPACTWATER, vec(bnc.o).addz(5), 250, 100, SND_LOWPRIORITY);
                }
                else if(bnc.inwater && !inWater) bnc.inwater = false; // the bouncer bounced outside the water
            }

            if(stopped)
            {
                if(bnc.bouncetype == BNC_GRENADE)
                {
                    hits.setsize(0);
                    explode(bnc.local, bnc.owner, bnc.o, bnc.o, NULL, 1, ATK_M32_SHOOT);
                    removeEntityPos(bnc.entityId);
                    stopLinkedSound(bnc.entityId);
                    if(bnc.local)
                        addmsg(N_EXPLODE, "rci3iv", bnc.owner, lastmillis-maptime, ATK_M32_SHOOT, bnc.id-maptime,
                                hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
                }
                delete curBouncers.remove(i--);
            }
            else
            {
                bnc.roll += old.sub(bnc.o).magnitude()/(4*RAD);
                bnc.offsetmillis = max(bnc.offsetmillis-time, 0);
            }
        }
    }

    void removebouncers(gameent *owner)
    {
        loopv(curBouncers) if(curBouncers[i]->owner==owner) { removeEntityPos(curBouncers[i]->entityId); delete curBouncers[i]; curBouncers.remove(i--); }
    }

    void clearbouncers() { curBouncers.deletecontents(); }

    struct projectile
    {
        size_t entityId;
        vec dir, o, from, to, offset;
        float speed;
        gameent *owner;
        int atk;
        bool local;
        int offsetmillis;
        int id;
        int lifetime;
        bool exploded;
        bool inwater;
        int projsound;
        bool soundplaying;

        projectile()
            : entityId(entitiesIds::getNewId()) // initialize the new entityId field here
        {}
    };
    vector<projectile> projs;

    void clearprojectiles()
    {
        loopv(projs)
        {
            projectile &p = projs[i];
            if(p.soundplaying) stopLinkedSound(p.entityId);
        }
        projs.shrink(0);
    }

    void newprojectile(const vec &from, const vec &to, float speed, bool local, int id, gameent *owner, int atk)
    {
        projectile &p = projs.add();
        p.dir = vec(to).sub(from).safenormalize();
        p.o = from;
        p.from = from;
        p.to = to;
        p.offset = hudgunorigin(attacks[atk].gun, from, to, owner);
        p.offset.sub(from);
        p.speed = speed;
        p.local = local;
        p.owner = owner;
        p.atk = atk;
        switch(p.atk)
        {
            case ATK_ARTIFICE_SHOOT: p.lifetime=attacks[atk].ttl+rnd(400); break;
            case ATK_ARBALETE_SHOOT: p.lifetime=temptrisfade+rnd(5000); break;
            default: p.lifetime = attacks[atk].ttl;
        }
        p.exploded = false;
        p.offsetmillis = OFFSETMILLIS;
        p.id = local ? lastmillis : id;
        p.inwater = ((lookupmaterial(p.o) & MAT_WATER) == MAT_WATER);

        switch(p.atk)
        {
            case ATK_PULSE_SHOOT: p.projsound = S_FLYBYPLASMA; break;
            case ATK_GRAP1_SHOOT: p.projsound = S_FLYBYGRAP1; break;
            case ATK_SPOCKGUN_SHOOT: p.projsound = S_FLYBYSPOCK; break;
            case ATK_SMAW_SHOOT: p.projsound = S_ROCKET; break;
            case ATK_ROQUETTES_SHOOT: p.projsound = S_MINIROCKET; break;
            case ATK_NUKE_SHOOT: p.projsound = S_MISSILENUKE; break;
            case ATK_ARTIFICE_SHOOT: p.projsound = S_FLYBYFIREWORKS;
            default: p.projsound = 0;
        }

        p.soundplaying = false;
    }

    void removeprojectiles(gameent *owner)
    {   // can't use loopv here due to strange GCC optimizer bug
        int len = projs.length();
        loopi(len)
        {
            if(projs[i].owner==owner)
            {
                if(projs[i].soundplaying) stopLinkedSound(projs[i].entityId);
                removeEntityPos(projs[i].entityId);
                projs.remove(i--);
                len--;
            }
        }
    }

    VARP(blood, 0, 1, 1);

    void spawnbouncer(const vec &p, const vec &vel, gameent *d, int type, int speed, int lifetime, bool frommonster)
    {
        if((camera1->o.dist(p) > bouncers[type].cullDist) && type!=BNC_GRENADE) return; // culling distant ones, except grenades, grenades are important

        vec dir;
        if(!speed) speed = 50 + rnd(20);

        switch(type)
        {
            case BNC_CASING:
            case BNC_BIGCASING:
            case BNC_CARTRIDGE:
                dir = vec(-10 + rnd(21), -10 + rnd(21), 100);
                break;
            default:
                dir = vec(-50 + rnd(101), -50 + rnd(101), -50 + rnd(101));
        }

        if(dir.iszero()) dir.z += 1;
        dir.normalize();

        vec to = vec(dir).mul(speed).add(vel); // Combine direction with player's momentum
        to.add(p);

        newbouncer(p, to, true, 0, d, type, lifetime, speed);
    }

    void damageeffect(int damage, gameent *d, gameent *actor, int atk)
    {
        vec p = d->o;
        p.z += 0.6f*(d->eyeheight + d->aboveeye) - d->eyeheight;
        int actorClasse = actor->aptitude;
        int victimClasse = d->aptitude;
        bool isHudPlayer = (actor == hudplayer());

        if(d->armourtype!=A_MAGNET)
        {
            if(blood) particle_splash(PART_BLOOD, damage > 300 ? 3 : damage/100, 1000, p, 0x60FFFF, 2.96f);
            gibeffect(!isteam(d->team, actor->team) ? damage : actorClasse == APT_MEDECIN ? 0 : actorClasse == APT_JUNKIE ? damage/=1.5f : damage/=3.f, vec(0,0,0), d);
        }

        bool teamDamage = false;
        if(isHudPlayer) d->curdamagecolor = 0xFFAA00;

        damage = ((damage*classes[actorClasse].damage)/(classes[victimClasse].resistance)); // calc damage based on the class's stats

        switch(actorClasse) // recalc damage based on the actor's passive/active skills
        {
            case APT_AMERICAIN:
                if(atk >= ATK_NUKE_SHOOT && atk <= ATK_CAMPOUZE_SHOOT)
                {
                    damage *= 1.5f;
                    if(isHudPlayer) d->curdamagecolor = 0xFF0000;
                }
                break;

            case APT_NINJA:
                if(atk == ATK_CACNINJA_SHOOT && actor==hudplayer()) d->curdamagecolor = 0xFF0000;
                break;

            case APT_MAGICIEN:
                if(actor->abilitymillis[ABILITY_2])
                {
                    damage *= 1.25f;
                    if(isHudPlayer) d->curdamagecolor = 0xFF00FF;
                }
                break;

            case APT_CAMPEUR:
                damage *= ((actor->o.dist(d->o)/1800.f)+1.f);
                break;

            case APT_VIKING:
                if(actor->boostmillis[B_RAGE])
                {
                    damage *= 1.25f;
                    if(isHudPlayer) d->curdamagecolor = 0xFF7700;
                }
                break;

            case APT_SHOSHONE:
                if(actor->abilitymillis[ABILITY_3])
                {
                    damage *= 1.3f;
                    if(isHudPlayer) d->curdamagecolor = 0xFF7700;
                }
                if(victimClasse == APT_AMERICAIN)
                {
                    damage /= 1.25f;
                    if(isHudPlayer) d->curdamagecolor = 0xFFFF00;
                }
                break;

            case APT_PHYSICIEN:
                if(d==player1 && actor==player1 && player1->armour && player1->abilitymillis[ABILITY_1]) unlockAchievement(ACH_BRICOLEUR);
        }

        switch(victimClasse) // recalc damage based on the victim's passive/active
        {
            case APT_MAGICIEN:
                if(d->abilitymillis[ABILITY_3])
                {
                    damage /= 5.0f;
                    d->curdamagecolor = 0x8855AA;
                }
                break;

            case APT_PRETRE:
                if(isHudPlayer && d->abilitymillis[ABILITY_2] && victimClasse == APT_PRETRE && d->mana) d->curdamagecolor = 0xAA00AA;
                break;

            case APT_SHOSHONE:
                if(d->abilitymillis[ABILITY_1]) damage /= 1.3f;
                if(actorClasse == APT_AMERICAIN)
                {
                    damage *= 1.25f;
                    if(isHudPlayer) d->curdamagecolor = 0xFF7700;
                }
        }

        if(actor->boostmillis[B_ROIDS]) // recalc damage if actor has roids
        {
            damage *= actorClasse == APT_JUNKIE ? 3 : 2;
            if(isHudPlayer) d->curdamagecolor = 0xFF0000;
        }
        if(d->boostmillis[B_JOINT]) // recalc victim if actor has joint
        {
            damage /= victimClasse == APT_JUNKIE ? 1.875f : 1.25f;
            if(isHudPlayer) d->curdamagecolor = 0xAAAA55;
        }

        if(isteam(d->team, actor->team) && actor != d && isHudPlayer) // recalc if ally or not
        {
            if(actorClasse == APT_MEDECIN) return;
            damage /= (actorClasse == APT_JUNKIE ? 1.5f : 3.f);
            d->curdamagecolor = 0x888888;
            teamDamage = true;
        }

        damage = damage/10.f; // rescale damage value

        if(isHudPlayer)
        {
            d->curdamage += damage;
            d->lastcurdamage = totalmillis;
            if(!teamDamage && actor == player1) updateStat(damage, STAT_TOTALDAMAGEDEALT);
        }
        else if(d == player1) updateStat(damage, STAT_TOTALDAMAGERECIE);
    }

    void gibeffect(int damage, const vec &vel, gameent *d)
    {
        if(damage <= 0) return;
        loopi(damage/300)
        {
            if(d->armourtype != A_ASSIST) spawnbouncer(d->o, d->vel, d, BNC_PIXEL, 100 + rnd(50));
            else if(d->armour) spawnbouncer(d->o, d->vel, d, BNC_SCRAP, 100 + rnd(50));
        }
    }

    int avgdmg[4];

    void hit(int damage, dynent *d, gameent *at, const vec &vel, int atk, float info1, int info2 = 1)
    {
        if(at==player1 && d!=at)
        {
            extern int hitsound;
            if(hitsound && lasthit != lastmillis) playSound(S_HIT, vec(0, 0, 0), 0, 0, SND_UI);
            lasthit = lastmillis;
        }

        gameent *f = (gameent *)d;

        f->lastpain = lastmillis;
        if(at->type==ENT_PLAYER && !isteam(at->team, f->team)) at->totaldamage += damage;

        if(m_dmsp || m_classicsp || m_tutorial)
        {
            if(f==player1)
            {
                if(player1->boostmillis[B_JOINT]) damage/=(player1->aptitude==APT_JUNKIE ? 1.875f : 1.25f);
                switch(player1->aptitude)
                {
                    case APT_MAGICIEN: {if(player1->abilitymillis[game::ABILITY_3]) damage/=5.f; } break;
                    case APT_VIKING: player1->boostmillis[B_RAGE]+=damage; break;
                    case APT_PRETRE: if(player1->abilitymillis[ABILITY_2] && player1->mana) {player1->mana-=damage/10; damage=0; if(player1->mana<0)player1->mana=0;} break;
                    case APT_SHOSHONE: if(player1->abilitymillis[ABILITY_1]) damage/=1.3f;
                }
                damage = (damage/classes[player1->aptitude].resistance)*(m_dmsp ? 15.f : 100);
                damageeffect(damage, f, at, atk);
                damaged(damage, f, at, true, atk);
                f->hitphyspush(damage, vel, at, atk, f);
            }
            else if(at==player1)
            {
                if(player1->boostmillis[B_ROIDS]) damage *= (player1->aptitude==APT_JUNKIE ? 3 : 2);
                switch(player1->aptitude)
                {
                    case APT_AMERICAIN: {if(atk==ATK_NUKE_SHOOT || atk==ATK_GAU8_SHOOT || atk==ATK_ROQUETTES_SHOOT || atk==ATK_CAMPOUZE_SHOOT) damage *= 1.5f; break;}
                    case APT_VIKING: if(player1->boostmillis[B_RAGE]) damage*=1.25f; break;
                    case APT_MAGICIEN: {if(player1->abilitymillis[game::ABILITY_2]) damage *= 1.25f; break;}
                    case APT_CAMPEUR: damage *= ((player1->o.dist(f->o)/1800.f)+1.f); break;
                    case APT_VAMPIRE: {player1->health = min(player1->health + damage/2, player1->maxhealth); player1->vampimillis+=damage*1.5f;} break;
                    case APT_SHOSHONE: if(player1->abilitymillis[ABILITY_1]) damage*=1.3f;
                }
                damage = (damage*classes[player1->aptitude].damage)/100;
                hitmonster(damage, (monster *)f, at, vel, atk);
                avgdmg[dmgsecs[0]] += damage/10.f;
            }
        }

        if(!m_mp(gamemode) || f==at) f->hitphyspush(damage, vel, at, atk, f);
        if(!m_mp(gamemode)) damaged(damage, f, at, false, atk);
        else
        {
            hitmsg &h = hits.add();
            h.target = f->clientnum;
            h.lifesequence = f->lifesequence;
            h.info1 = int(info1*DMF);
            h.info2 = info2;
            h.dir = f==at ? ivec(0, 0, 0) : ivec(vec(vel).mul(DNF));

            int impactSound = S_IMPACTWOOD + f->armourtype;
            int regenSound = S_PHY_1_WOOD + f->armourtype;
            bool hasRegenAbility = f->aptitude==APT_PHYSICIEN && f->abilitymillis[ABILITY_1];

            if(at==player1)
            {
                damageeffect(damage, f, at, atk);

                if(f==player1)
                {
                    damageblend(damage);
                    damagecompass(damage, at ? at->o : f->o);

                    if(player1->armour)
                    {
                        if(hasRegenAbility) { playSound(S_PHY_1); playSound(regenSound); }
                        else if(!rnd(atk==ATK_LANCEFLAMMES_SHOOT ? 5 : 2)) playSound(impactSound);
                    }
                    else playSound(S_IMPACTBODY);
                }
            }
            else
            {
                if(f->armour)
                {
                    if(hasRegenAbility) { playSound(S_PHY_1, f->o, 200, 100, SND_LOWPRIORITY); playSound(regenSound, f->o, 200, 100, SND_LOWPRIORITY); }
                    else if(!rnd(atk==ATK_LANCEFLAMMES_SHOOT ? 5 : 2)) playSound(impactSound, f->o, 250, 50, SND_LOWPRIORITY);
                }
            }
        }
    }

    void hitpush(int damage, dynent *d, gameent *at, vec &from, vec &to, int atk, int rays)
    {
        hit(damage, d, at, vec(to).sub(from).safenormalize(), atk, from.dist(to), rays);
    }

    float projdist(dynent *o, vec &dir, const vec &v, const vec &vel)
    {
        vec middle = o->o;
        middle.z += (o->aboveeye-o->eyeheight)/2;
        dir = vec(middle).sub(v).add(vec(vel).mul(5)).safenormalize();

        float low = min(o->o.z - o->eyeheight + o->radius, middle.z),
              high = max(o->o.z + o->aboveeye - o->radius, middle.z);
        vec closest(o->o.x, o->o.y, clamp(v.z, low, high));
        return max(closest.dist(v) - o->radius, 0.0f);
    }

    void radialeffect(dynent *o, const vec &v, const vec &vel, int qdam, gameent *at, int atk)
    {
        if(o->state!=CS_ALIVE) return;
        vec dir;
        float dist = projdist(o, dir, v, vel);
        if(dist<attacks[atk].exprad)
        {
            float damage = o==at && atk==ATK_ASSISTXPL_SHOOT ? 0 : attacks[atk].damage*(1-dist/EXP_DISTSCALE/attacks[atk].exprad);
            if(damage > 0) hit(max(int(damage), 1), o, at, dir, atk, dist);
        }
    }

    void startShake(const vec &v, int maxdist, int atk)
    {
        float distance = camera1->o.dist(v);
        if(distance > maxdist) return;
        float factor = lerp(1.0f, 0.0f, distance / maxdist); // As normalized distance increases, the factor linearly decreases from 1 to 0
        if(atk == ATK_NUKE_SHOOT) factor *= 2;
        shakeScreen(factor);
    }

    void explode(bool local, gameent *owner, const vec &v, const vec &vel, dynent *safe, int damage, int atk)
    {
        vec safeLoc = vec(v).sub(vec(vel).mul(15)); // avoid false positive for occlusion effect
        bool inWater = ((lookupmaterial(v) & MAT_WATER) == MAT_WATER);
        bool isFar = camera1->o.dist(v) >= 300;

        switch(atk)
        {
            case ATK_PULSE_SHOOT:
            case ATK_GRAP1_SHOOT:
            case ATK_SPOCKGUN_SHOOT:
                renderProjectileExplosion(owner, v, vel, safe, atk);
                playSound(atk==ATK_GRAP1_SHOOT ? S_IMPACTGRAP1 : atk==ATK_PULSE_SHOOT ? S_IMPACTPLASMA : S_IMPACTSPOCK, v, 250, 50, SND_LOWPRIORITY);
                break;

            case ATK_SMAW_SHOOT:
            case ATK_ROQUETTES_SHOOT:
                loopi(5+rnd(3)) spawnbouncer(safeLoc, vec(0, 0, 0), owner, BNC_ROCK, 200);
                renderExplosion(owner, safeLoc, vel, atk);
                playSound(S_EXPL_MISSILE, safeLoc, 400, 150);
                if(inWater) playSound(S_EXPL_INWATER, vec(v).addz(15), 300, 100);
                if(isFar) playSound(S_EXPL_FAR, safeLoc, 1500, 400, SND_LOWPRIORITY);
                startShake(v, 1.25f * attacks[atk].exprad, atk);
                break;

            case ATK_KAMIKAZE_SHOOT:
            case ATK_ASSISTXPL_SHOOT:
                loopi(10 + rnd(5))
                {
                    if(atk==ATK_ASSISTXPL_SHOOT) spawnbouncer(v, owner->vel, owner, BNC_SCRAP, 50 + rnd(250));
                    else spawnbouncer(v, owner->vel, owner, rnd(2) ? BNC_PIXEL : BNC_ROCK, 100 + rnd(300));
                }
                renderExplosion(owner, owner->o, vel, atk);
                playSound(atk==ATK_KAMIKAZE_SHOOT ? S_EXPL_KAMIKAZE : S_EXPL_PARMOR, owner->o, 400, 150);
                if(inWater) playSound(S_EXPL_INWATER, vec(owner->o).addz(15), 300, 100);
                if(isFar) playSound(S_BIGEXPL_FAR, owner->o, 2000, 400);
                startShake(v, 1.5f * attacks[atk].exprad, atk);
                break;

            case ATK_NUKE_SHOOT:
                renderExplosion(owner, v, vel, atk);
                playSound(S_EXPL_NUKE, v, 5000, 3000, SND_NOOCCLUSION);
                startShake(v, 2 * attacks[atk].exprad, atk);
                break;

            case ATK_ARTIFICE_SHOOT:
                renderExplosion(owner, safeLoc, vel, atk);
                playSound(S_EXPL_FIREWORKS, safeLoc, 400, 200);
                if(inWater) playSound(S_EXPL_INWATER, vec(v).addz(15), 300, 100);
                if(isFar) playSound(S_FIREWORKSEXPL_FAR, safeLoc, 2000, 750);
                startShake(v, attacks[atk].exprad, atk);
                break;

            case ATK_M32_SHOOT:
                renderExplosion(owner, v, vel, atk);
                playSound(S_EXPL_GRENADE, v, 400, 150);
                if(inWater) playSound(S_EXPL_INWATER, vec(v).addz(15), 300, 100);
                if(isFar) playSound(S_EXPL_FAR, v, 2000, 400, SND_LOWPRIORITY);
                loopi(5+rnd(3)) spawnbouncer(v, vec(0, 0, 0), owner, BNC_ROCK, 200);
                startShake(v, 1.5f * attacks[atk].exprad, atk);
                break;

            case ATK_MINIGUN_SHOOT:
            case ATK_AK47_SHOOT:
            case ATK_UZI_SHOOT:
            case ATK_FAMAS_SHOOT:
            case ATK_GLOCK_SHOOT:
            case ATK_ARBALETE_SHOOT:
                renderBulletImpact(owner, v, vel, safe, atk);
                if(!inWater) playSound(atk==ATK_ARBALETE_SHOOT ? S_IMPACTARROW : S_LITTLERICOCHET, v, 175, 75, SND_LOWPRIORITY);
                break;

            case ATK_SV98_SHOOT:
            case ATK_SKS_SHOOT:
            case ATK_GAU8_SHOOT:
                renderBulletImpact(owner, v, vel, safe, atk);
                if(!inWater)
                {
                    playSound(S_IMPACTLOURDLOIN, v, 750, 400, SND_LOWPRIORITY);
                    playSound(S_BIGRICOCHET, v, 250, 75, SND_LOWPRIORITY);
                }
                break;
        }

        if(!local) return;
        int numdyn = numdynents();
        loopi(numdyn)
        {
            dynent *o = iterdynents(i);
            if(o->o.reject(v, o->radius + attacks[atk].exprad) || o==safe) continue;
            radialeffect(o, v, vel, damage, owner, atk);
        }
    }

    void projstain(const projectile &p, const vec &pos, int atk)
    {
        vec dir = vec(p.dir).neg();

        switch(atk)
        {
            case ATK_PULSE_SHOOT:
                addstain(STAIN_PLASMA_SCORCH, pos, dir, 7.5f);
                addstain(STAIN_BULLET_HOLE, pos, dir, 7.5f, 0x882200);
                return;
            case ATK_SMAW_SHOOT:
            case ATK_KAMIKAZE_SHOOT:
            case ATK_ASSISTXPL_SHOOT:
            case ATK_ROQUETTES_SHOOT:
            {
                float rad = attacks[p.atk].exprad*0.35f;
                addstain(STAIN_EXPL_SCORCH, pos, dir, rad);
                return;
            }
            case ATK_MINIGUN_SHOOT:
            case ATK_SV98_SHOOT:
            case ATK_AK47_SHOOT:
            case ATK_GAU8_SHOOT:
            case ATK_SKS_SHOOT:
                addstain(STAIN_BULLET_HOLE, pos, dir, 1.5f+(rnd(2)));
                addstain(STAIN_BULLET_GLOW, pos, dir, 2.0f+(rnd(2)), 0x883300);
                return;
            case ATK_SPOCKGUN_SHOOT:
                addstain(STAIN_PLASMA_SCORCH, pos, dir, 5);
                addstain(STAIN_SPOCK, pos, dir, 5, hasRoids(p.owner) ? 0xFF0000 : 0x22FF22);
                return;
            case ATK_UZI_SHOOT:
            case ATK_ARBALETE_SHOOT:
            case ATK_GLOCK_SHOOT:
            case ATK_FAMAS_SHOOT:
                addstain(STAIN_BULLET_HOLE, pos, dir, 0.5f);
                addstain(STAIN_BULLET_GLOW, pos, dir, 1.0f, 0x883300);
                return;
            case ATK_GRAP1_SHOOT:
                addstain(STAIN_ELEC_GLOW, pos, dir, 5.f, 0x992299);
                addstain(STAIN_PLASMA_SCORCH, pos, dir, 5);
                return;
        }
    }

    void projsplash(projectile &p, const vec &v, dynent *safe)
    {
        explode(p.local, p.owner, v, p.dir, safe, attacks[p.atk].damage, p.atk);
        projstain(p, v, p.atk);
    }

    void explodeeffects(int atk, gameent *d, bool local, int id)
    {
        if(local) return;

        loopv(projs)
        {
            projectile &p = projs[i];
            if(p.atk == atk && p.owner == d && p.id == id && !p.local)
            {
                vec pos = vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)).add(p.o);
                explode(p.local, p.owner, pos, p.dir, NULL, 0, atk);
                projstain(p, pos, atk);
                removeEntityPos(p.entityId);
                projs.remove(i);
                break;
            }
        }
        loopv(curBouncers)
        {
            bouncer &b = *curBouncers[i];
            if(b.bouncetype == BNC_GRENADE && b.owner == d && b.id == id && !b.local)
            {
                vec pos = vec(b.offset).mul(b.offsetmillis/float(OFFSETMILLIS)).add(b.o);
                explode(b.local, b.owner, pos, vec(0,0,0), NULL, 0, atk);
                removeEntityPos(b.entityId);
                curBouncers.remove(i);
                break;
            }
        }
    }

    bool projdamage(dynent *o, projectile &p, const vec &v)
    {
        if(o->state!=CS_ALIVE) return false;
        if(!intersect(o, p.o, v, attacks[p.atk].margin)) return false;
        projsplash(p, v, o);
        vec dir;
        projdist(o, dir, v, p.dir);
        hit(attacks[p.atk].damage, o, p.owner, dir, p.atk, 0);
        return true;
    }

    void updateprojectiles(int time)
    {
        bool removearrow = false;

        if(projs.empty()) return;
        loopv(projs)
        {
            projectile &p = projs[i];
            p.offsetmillis = max(p.offsetmillis-time, 0);
            vec dv;
            float dist = p.to.dist(p.o, dv);
            dv.mul(time/max(dist*1000/p.speed, float(time)));
            vec v = vec(p.o).add(dv);
            bool exploded = false;
            hits.setsize(0);

            if((p.lifetime -= time)<0 && (p.atk==ATK_ARTIFICE_SHOOT || p.atk==ATK_NUKE_SHOOT || p.atk==ATK_KAMIKAZE_SHOOT || p.atk==ATK_ASSISTXPL_SHOOT))
            {
                projsplash(p, v, NULL);
                exploded = true;
            }

            if(p.local)
            {
                vec halfdv = vec(dv).mul(0.5f), bo = vec(p.o).add(halfdv);
                float br = max(fabs(halfdv.x), fabs(halfdv.y)) + 1 + attacks[p.atk].margin;
                loopj(numdynents())
                {
                    dynent *o = iterdynents(j);
                    if(p.owner==o || o->o.reject(bo, o->radius + br)) continue;
                    if(projdamage(o, p, v)) {exploded = true; removearrow = true; break; }
                }
            }

            if(!exploded)
            {
                if(dist<4)
                {
                    if(p.o!=p.to) // if original target was moving, reevaluate endpoint
                    {
                        if(raycubepos(p.o, p.dir, p.to, 0, RAY_CLIPMAT|RAY_ALPHAPOLY)>=4) continue;
                    }
                    if(!p.exploded) projsplash(p, v, NULL);
                    exploded = true;
                }

                vec pos = vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)).add(v);

                if(!p.inwater && ((lookupmaterial(pos) & MAT_WATER) == MAT_WATER))
                {
                    p.inwater = true;
                    vec effectPos = pos;
                    effectPos.addz(2.5f);
                    particle_splash(PART_WATER, 15, 100, effectPos, 0x28282A, 1.5f, 50, -300, 6, hasShrooms());
                    playSound(S_IMPACTWATER, effectPos, 250, 50, SND_LOWPRIORITY|SND_NOOCCLUSION);
                }

                if(p.atk!=ATK_SMAW_SHOOT && p.atk!=ATK_ARTIFICE_SHOOT && p.atk!=ATK_ROQUETTES_SHOOT && p.atk!=ATK_NUKE_SHOOT)
                {
                    renderProjectilesTrails(p.owner, pos, dv, p.from, p.offset, p.atk, p.exploded);
                }

                if(p.projsound) // play and update the sound only if the projectile is passing by
                {
                    bool bigRadius = (p.atk==ATK_NUKE_SHOOT || p.atk==ATK_ARTIFICE_SHOOT);

                    if(camera1->o.dist(pos) < (bigRadius ? 800 : 400))
                    {
                        updateEntPos(p.entityId, pos);
                        if(!p.soundplaying)
                        {
                            playSound(p.projsound, pos, bigRadius ? 800 : 400, 1, SND_LOOPED, p.entityId);
                            p.soundplaying = true;
                        }
                    }
                    else if(p.soundplaying)
                    {
                        p.soundplaying = false;
                        stopLinkedSound(p.entityId);
                        removeEntityPos(p.entityId);
                    }
                }
            }
            if(exploded)
            {
                if(p.local && !p.exploded) addmsg(N_EXPLODE, "rci3iv", p.owner, lastmillis-maptime, p.atk, p.id-maptime, hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
                p.exploded = true;
                if(p.soundplaying)
                {
                    stopLinkedSound(p.entityId);
                    removeEntityPos(p.entityId);
                    p.soundplaying = false;
                }
                if(p.atk != ATK_ARBALETE_SHOOT) projs.remove(i--);
                else if((p.lifetime -= time)<0 || removearrow) projs.remove(i--);
            }
            else p.o = v;
        }
    }

    bool noMuzzle(int atk, gameent *d)
    {
        return (d->aptitude==APT_ESPION && d->abilitymillis[ABILITY_2]) || (atk==ATK_CAC349_SHOOT || atk==ATK_CACFLEAU_SHOOT || atk==ATK_CACMARTEAU_SHOOT || atk==ATK_CACMASTER_SHOOT || atk==ATK_CACNINJA_SHOOT);
    }

    float recoilReduce(bool wizardAbility) { return wizardAbility ? 5 : (hudplayer()->aptitude==APT_SOLDAT ? 2 : (1 * (classes[hudplayer()->aptitude].accuracy / 100.f))); }

    bool switchRecoilSide(int time) { return (totalmillis % (time * 2) < time); }

    void shoteffects(int atk, const vec &from, const vec &to, gameent *d, bool local, int id, int prevaction)     // create visual effect from a shot
    {
        int gun = attacks[atk].gun;
        int gunSound = attacks[atk].sound;
        bool isHudPlayer = d==hudplayer();
        vec muzzleOrigin = noMuzzle(atk, d) ? hudgunorigin(d->gunselect, d->o, to, d) : d->muzzle;
        vec casingOrigin = (d->aptitude==APT_ESPION && d->abilitymillis[ABILITY_2]) ? d->o : d->balles;
        int lightFlags = DL_FLASH|L_NODYNSHADOW;

        bool wizardAbility = (d->aptitude==APT_MAGICIEN && d->abilitymillis[ABILITY_2]);
        if(wizardAbility) playSound(S_WIZ_2, isHudPlayer ? vec(0, 0, 0) : d->muzzle, 250, 150, NULL, d->entityId);

        switch(atk)
        {
            case ATK_PULSE_SHOOT:
            case ATK_SPOCKGUN_SHOOT:
            {
                bool isPlasma = (atk == ATK_PULSE_SHOOT);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                renderMuzzleEffects(from, to, d, atk);
                if(isHudPlayer)
                {
                    float deviationAmount = (isPlasma ? (0.05f * (rnd(2) ? 1 : -1)) / recoilReduce(wizardAbility) : 0.f);
                    startCameraAnimation(CAM_ANIM_SHOOT, isPlasma ? attacks[atk].attackdelay * 2.f : attacks[atk].attackdelay, vec(0, 0, 0), vec(0, 0, 0), vec(deviationAmount / recoilReduce(wizardAbility), 0.4f / recoilReduce(wizardAbility), 0), vec(0, 25, 0));
                }
                if(d->type == ENT_PLAYER && isPlasma) gunSound = S_PLASMARIFLE_SFX;
                break;
            }

            case ATK_RAIL_SHOOT:
                renderMuzzleEffects(from, to, d, atk);
                renderInstantImpact(from, to, muzzleOrigin, atk, hasRoids(d));
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / 2.f, vec(0, 0, 0), vec(0, 0, 0), vec((0.05f * (rnd(2) ? 1 : -1)) / recoilReduce(wizardAbility), 0.1f / recoilReduce(wizardAbility), 0));
                else soundNearmiss(S_FLYBYELEC, from, to);
                playSound(S_IMPACTELEC, to, 250, 50, SND_LOWPRIORITY);
                break;

            case ATK_SMAW_SHOOT:
            case ATK_ROQUETTES_SHOOT:
            case ATK_NUKE_SHOOT:
            case ATK_ARTIFICE_SHOOT:
            {
                bool isNuke = (atk==ATK_NUKE_SHOOT);
                bool isRockets = (atk==ATK_ROQUETTES_SHOOT);
                renderMuzzleEffects(from, to, d, atk);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(isHudPlayer)
                {
                    float recoilAmount = (isNuke ? 3.5f : (atk==ATK_ARTIFICE_SHOOT || isRockets ? 0.9f : 0.8f)) / recoilReduce(wizardAbility);
                    startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / (isNuke ? 3.5f : (isRockets ? 1.25f : 3.5f)), vec(0, 0, 0), vec(0, 0, 0), vec(0, recoilAmount, 0));
                    if(d==player1 && atk==ATK_NUKE_SHOOT)
                    {
                        unlockAchievement(ACH_ATOME);
                        updateStat(1, STAT_ATOM);
                    }
                }
                break;
            }

            case ATK_MINIGUN_SHOOT:
            case ATK_AK47_SHOOT:
            case ATK_GAU8_SHOOT:
            {
                bool isGau = (atk == ATK_GAU8_SHOOT);
                renderMuzzleEffects(from, to, d, atk);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(isHudPlayer)
                {
                    float recoilAccel = (hudplayer()->gunaccel ? (recoilReduce(wizardAbility) * hudplayer()->gunaccel) : 1.f);
                    float recoilAmount = (isGau ? 0.3f : 0.4f) / recoilAccel;
                    float deviationAmount = ((isGau ? 0.3f : 0.15f) / recoilAccel) * (switchRecoilSide(isGau ? 400 : 750) ? 1 : -1);
                    startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay * (isGau ? 10 : 3), vec(0, 0, 0), vec(0, 0, 0), vec(deviationAmount, recoilAmount, 0), vec(0, atk==ATK_AK47_SHOOT ? 35 : 45, 0));
                }
                else soundNearmiss(isGau ? S_BIGBULLETFLYBY : S_BULLETFLYBY, from, to);

                if(isGau)
                {
                    if(d->type==ENT_PLAYER) gunSound = S_GAU8;
                    if(d==player1 && player1->aptitude==APT_PRETRE && player1->boostmillis[B_SHROOMS] && player1->abilitymillis[ABILITY_3]) unlockAchievement(ACH_CADENCE);
                }
                spawnbouncer(casingOrigin, d->vel, d, isGau ? BNC_BIGCASING : BNC_CASING);
                break;
            }
            case ATK_MOSSBERG_SHOOT:
            case ATK_HYDRA_SHOOT:
            {
                bool isHydra = (atk==ATK_HYDRA_SHOOT);
                if(!local) createrays(gun, from, to, d);
                particle_flare(d->muzzle, d->muzzle, 140, PART_MF_SHOTGUN, hasRoids(d) ? 0xFF2222 : wizardAbility ? 0xFF22FF : 0xCCAAAA, isHudPlayer ? zoom ? 1.25f : 3.50f : 4.5f, d, hasShrooms());
                particle_splash(PART_SMOKE, 4, 500, d->muzzle, 0x443333, 3.5f, 20, 500, 0, hasShrooms());
                particle_splash(PART_SPARK, isHudPlayer ? 4 : 7, 40, d->muzzle, 0xFF2200, 0.5f, 300, 500, 0, hasShrooms());
                if(d->boostmillis[B_RAGE]) particle_flare(d->muzzle, d->muzzle, 140, PART_MF_SHOTGUN, 0xFF2222, isHudPlayer ? zoom ? 2.00f : 5.5f : 6.5f, d, hasShrooms());
                adddynlight(muzzleOrigin, 75, vec(1.25f, 0.25f, 0.f), 40, 2, lightFlags, 0, vec(1.25f, 0.25f, 0.f), d);
                loopi(isHydra ? 3 : 2) spawnbouncer(casingOrigin, d->vel, d, BNC_CARTRIDGE);
                if(isHudPlayer)
                {
                    float recoilAmount = (isHydra ? 0.8f : 1.8f) / recoilReduce(wizardAbility);
                    startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / (isHydra ? 0.75f : 2.f), vec(0, 0, 0), vec(0, 0, 0), vec(0, recoilAmount, 0));
                }

                loopi(attacks[atk].rays)
                {
                    float originOffset = 0.4f - (rnd(9) / 10.f);
                    newprojectile(muzzleOrigin.add(vec(originOffset, originOffset, originOffset)), rays[i], 3000, false, id, d, atk);
                    renderInstantImpact(from, rays[i], muzzleOrigin, atk, hasRoids(d));
                    if(i < 6)
                    {
                        playSound(S_LITTLERICOCHET, rays[i], 250, 100, SND_LOWPRIORITY);
                        if(!isHudPlayer) soundNearmiss(S_BULLETFLYBY, from, rays[i], 512);
                    }
                }
                break;
            }

            case ATK_SV98_SHOOT:
            case ATK_SKS_SHOOT:
            case ATK_CAMPOUZE_SHOOT:
            {
                bool isSv98 = (atk==ATK_SV98_SHOOT);
                particle_splash(PART_SMOKE, isHudPlayer ? 4 : 6, isHudPlayer ? 350 : 600, d->muzzle, 0x222222, isHudPlayer ? 3.5f : 6.5f, 40, 500, 0, hasShrooms());
                particle_splash(PART_SPARK, isHudPlayer ? 4 : 7, 40, d->muzzle, 0xFFFFFF, 0.5f, 300, 500, 0, hasShrooms());
                particle_flare(d->muzzle, d->muzzle, 100, PART_MF_LITTLE, hasRoids(d) ? 0xFF2222 : wizardAbility ? 0xFF22FF : 0xFFFFFF, 1.25f, d, hasShrooms());
                particle_flare(d->muzzle, d->muzzle, 100, PART_MF_SNIPER, hasRoids(d) ? 0xFF2222 : wizardAbility ? 0xFF22FF : 0xFFFFFF, atk==ATK_CAMPOUZE_SHOOT ? 5.0f : 3.5f, d, hasShrooms());
                if(d->boostmillis[B_RAGE]) particle_flare(d->muzzle, d->muzzle, 75, PART_MF_SNIPER, 0xFF2222, 6.0f, d, hasShrooms());
                adddynlight(hudgunorigin(gun, d->o, to, d), 50, vec(1.25f, 0.75f, 0.3f), 37, 2, lightFlags, 0, vec(1.25f, 0.75f, 0.3f), d);
                if(isHudPlayer)
                {
                    float recoilAmount = (isSv98 ? 1.7f : 0.5f) / recoilReduce(wizardAbility);
                    startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / (isSv98 ? 2.7f : 1.5f), vec(0, 0, 0), vec(0, 0, 0), vec(0, recoilAmount, 0));
                }
                if(atk==ATK_CAMPOUZE_SHOOT)
                {
                    loopi(attacks[atk].rays)
                    {
                        float originOffset = 0.4f - (rnd(9) / 10.f);
                        particle_flare(muzzleOrigin.add(vec(originOffset, originOffset, originOffset)), rays[i], 100, PART_F_SHOTGUN, hasRoids(d) ? 0xFF2222 : 0xFFFF22, 0.4f, d, hasShrooms());
                        particle_trail(PART_SMOKE, 800, hudgunorigin(gun, from, to, d), rays[i], 0x999999, 0.6f, 20);
                        renderInstantImpact(from, rays[i], muzzleOrigin, atk, hasRoids(d));
                        if(!isHudPlayer) soundNearmiss(S_BIGBULLETFLYBY, from, rays[i], 512);
                        playSound(S_BIGRICOCHET, rays[i], 250, 100);
                    }
                    loopi(3) spawnbouncer(casingOrigin, d->vel, d, BNC_BIGCASING);
                }
                else
                {
                    if(!isHudPlayer) soundNearmiss(S_BIGBULLETFLYBY, from, to);
                    newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                    spawnbouncer(casingOrigin, d->vel, d, BNC_BIGCASING);
                }
                break;
            }

            case ATK_ARBALETE_SHOOT:
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d->boostmillis[B_RAGE]) particle_splash(PART_SPARK,  8, 500, d->muzzle, 0xFF2222, 1.0f,  50, 200, 0, hasShrooms());
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / 1.5f, vec(0, 0, 0), vec(0, 0, 0), vec(0, 0.2f / recoilReduce(wizardAbility), 0));
                else soundNearmiss(S_FLYBYARROW, from, to);
                break;

            case ATK_UZI_SHOOT:
            case ATK_FAMAS_SHOOT:
            case ATK_GLOCK_SHOOT:
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                particle_flare(d->muzzle, d->muzzle, 125, PART_MF_LITTLE, hasRoids(d) ? 0xFF2222 : wizardAbility ? 0xFF22FF : 0xFFFFFF, isHudPlayer ? zoom ? 0.5f : 0.75f : 1.75f, d, hasShrooms());
                particle_flare(d->muzzle, d->muzzle, 75, PART_MF_BIG, hasRoids(d) ? 0xFF2222 : wizardAbility ? 0xFF22FF : 0xFFAA55, isHudPlayer ? zoom ? 0.75f : 2.f : 3.f, d, hasShrooms());
                particle_splash(PART_SMOKE, isHudPlayer ? 3 : 5, isHudPlayer ? 350 : 500, d->muzzle, 0x444444, isHudPlayer ? 3.5f : 4.5f, 20, 500, 0, hasShrooms());
                particle_splash(PART_SPARK, isHudPlayer ? 3 : 5, 35, d->muzzle, 0xFF4400, 0.35f, 300, 500, 0, hasShrooms());
                if(d->boostmillis[B_RAGE]) particle_flare(d->muzzle, d->muzzle, 80, PART_MF_BIG, 0xFF2222, isHudPlayer ? zoom ? 1.5f : 4.f : 5.f, d, hasShrooms());
                adddynlight(hudgunorigin(gun, d->o, to, d), 60, vec(1.25f, 0.75f, 0.3f), 30, 2, lightFlags, 0, vec(1.25f, 0.75f, 0.3f), d);
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay * 3, vec(0, 0, 0), vec(0, 0, 0), vec(0, (atk==ATK_GLOCK_SHOOT ? 0.2f : 0.5f) / recoilReduce(wizardAbility), 0), vec(0, 30, 0));
                else soundNearmiss(S_BULLETFLYBY, from, to);
                spawnbouncer(casingOrigin, d->vel, d, BNC_CASING);
                break;

            case ATK_LANCEFLAMMES_SHOOT:
            {
                if(!local) createrays(gun, from, to, d);
                particle_flare(d->muzzle, d->muzzle, 150, PART_MF_ROCKET, hasRoids(d) ? 0x880000 : wizardAbility ? 0x440044 : 0x663311, isHudPlayer ? zoom ? 2.00f : 3.5f : 4.5f, d, hasShrooms());
                if(d->boostmillis[B_RAGE]) particle_splash(PART_SPARK,  3, 500, d->muzzle, 0xFF2222, 1.0f, 50, 200, 0, hasShrooms());
                loopi(attacks[atk].rays)
                {
                    vec dest = vec(rays[i]).sub(muzzleOrigin).normalize().mul(1450.0f + rnd(200));
                    switch(rnd(4))
                    {
                        case 0: particle_flying_flare(muzzleOrigin, dest, 700, PART_FIRE_BALL, hasRoids(d) ? 0x881111 : 0x604930, (12.f+rnd(16))/8.f, 100, 10+rnd(5), hasShrooms()); break;
                        case 1: particle_flying_flare(muzzleOrigin, dest, 700, PART_FIRE_BALL, hasRoids(d) ? 0x770000 : 0x474747, (12.f+rnd(16))/8.f, 100, 10+rnd(5), hasShrooms()); break;
                        case 2: particle_flying_flare(muzzleOrigin, dest, 700, PART_FIRE_BALL, hasRoids(d) ? 0x991111 : 0x383838, (12.f+rnd(16))/8.f, 100, 10+rnd(5), hasShrooms()); break;
                        default:
                            particle_flying_flare(muzzleOrigin, dest, 1100, PART_SMOKE, 0x111111, (15.f+rnd(18))/3.f, -20, 15+rnd(10), hasShrooms());
                            renderInstantImpact(from, rays[i], muzzleOrigin, atk, hasRoids(d));
                            if(rnd(2) && !isHudPlayer) soundNearmiss(S_FLYBYFLAME, from, rays[i]);
                    }
                }
                adddynlight(muzzleOrigin, 50, vec(0.6f, 0.3f, 0.1f), 100, 100, lightFlags, 10, vec(0.4f, 0, 0), d);
                if(!rnd(2)) newbouncer(muzzleOrigin, to, local, id, d, BNC_LIGHT, 650, 400);
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay * 1.5f, vec(0, 0, 0), vec(0, 0, 0), vec((0.15f * (switchRecoilSide(300) ? 1 : -1)) / recoilReduce(wizardAbility), 0, 0));
                gunSound = (d->type==ENT_AI ? S_PYRO_A : S_FLAMETHROWER);
                break;
            }
            case ATK_GRAP1_SHOOT:
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                particle_flare(d->muzzle, d->muzzle, 150, PART_MF_PLASMA, hasRoids(d) ? 0xFF4444 : wizardAbility ? 0xFF00FF : 0xFF55FF, 1.75f, d, hasShrooms());
                if(d->boostmillis[B_RAGE]) particle_splash(PART_SPARK, 3, 500, d->muzzle, 0xFF4444, 1.0f, 50, 200, 0, hasShrooms());
                adddynlight(hudgunorigin(gun, d->o, to, d), 70, vec(1.0f, 0.0f, 1.0f), 80, 100, lightFlags, 0, vec(0, 0, 0), d);
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / 1.5f, vec(0, 0, 0), vec(0, 0, 0), vec((0.1f * (rnd(2) ? 1 : -1)) / recoilReduce(wizardAbility), 0.15f / recoilReduce(wizardAbility), 0), vec(0, 12, 0));
                break;

            case ATK_M32_SHOOT:
            {
                particle_splash(PART_SMOKE, 10, 600, d->muzzle, wizardAbility ? 0x550044 : 0x444444, 4.0f, 20, 500, 0, hasShrooms());
                if(d->boostmillis[B_RAGE]) particle_splash(PART_SPARK,  8, 500, d->muzzle, 0xFF4444, 1.0f, 50, 200, 0, hasShrooms());
                float dist = from.dist(to); vec up = to; up.z += dist/8;
                newbouncer(isHudPlayer && !thirdperson ? d->muzzle : hudgunorigin(gun, d->o, to, d), up, local, id, d, BNC_GRENADE, attacks[atk].ttl, attacks[atk].projspeed);
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / 3, vec(0, 0, 0), vec(0, 0, 0), vec(0, 0.6 / recoilReduce(wizardAbility), 0));
                break;
            }
            case ATK_KAMIKAZE_SHOOT:
            case ATK_ASSISTXPL_SHOOT:
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                break;
        }

        if(!rnd(attacks[atk].specialsounddelay))
        {
            if(d->boostmillis[B_ROIDS])
            {
                playSound(S_ROIDS_SHOOT, isHudPlayer ? vec(0, 0, 0) : d->o, 500, 100, NULL, d->entityId);
                if(camera1->o.dist(hudgunorigin(gun, d->o, to, d)) >= 400 && d!=player1) playSound(S_ROIDS_SHOOT_FAR, d->o, 800, 450, NULL, d->entityId);
            }
            else if(d->boostmillis[B_RAGE]) playSound(S_RAGETIR, isHudPlayer ? vec(0, 0, 0) : d->o, 500, 100);
        }

        if(d->abilitymillis[ABILITY_3] && d->aptitude==APT_PRETRE) adddynlight(muzzleOrigin, 6, vec(1.5f, 1.5f, 0.0f), 80, 40, L_NOSHADOW|L_VOLUMETRIC|DL_FLASH);
        if(d==player1) updateStat(1, STAT_MUNSHOOTED);

        bool incraseDist = atk==ATK_ASSISTXPL_SHOOT || atk==ATK_KAMIKAZE_SHOOT || atk==ATK_GAU8_SHOOT || atk==ATK_NUKE_SHOOT;
        int distance = camera1->o.dist(hudgunorigin(gun, d->o, to, d));
        int loopedSoundFlags = SND_LOOPED|SND_FIXEDPITCH|SND_NOCULL;
        float pitch = d->boostmillis[B_SHROOMS] ? 1.2f : (d->aptitude==APT_PRETRE && d->abilitymillis[ABILITY_3] ? (1.5f - (d->abilitymillis[ABILITY_3] / 8000.0f)) : 0);

        if(!d->attacksound)
        {
            switch(gunSound)
            {
                case S_FLAMETHROWER:
                case S_GAU8:
                    playSound(gunSound, isHudPlayer ? vec(0, 0, 0) : d->o, incraseDist ? 600 : 500, incraseDist ? 350 : 200, loopedSoundFlags, d->entityId, PL_ATTACK, pitch);
                    d->attacksound = 1;
                    if(distance > 300 && !isHudPlayer)
                    {
                        playSound(attacks[atk].middistsnd, d->o, incraseDist ? 3200 : 600, incraseDist ? 1600 : 400, loopedSoundFlags, d->entityId, PL_ATTACK_FAR, pitch);
                        d->attacksound = 2;
                    }
                    return;

                case S_PLASMARIFLE_SFX:
                    playSound(gunSound, isHudPlayer ? vec(0, 0, 0) : d->o, 250, 150, loopedSoundFlags, d->entityId, PL_ATTACK, pitch);
                    d->attacksound = 1;
                    break;
            }
        }
        else if(gunSound != S_PLASMARIFLE_SFX) return;

        playSound(attacks[atk].sound, isHudPlayer ? vec(0, 0, 0) : d->o, incraseDist ? 600 : 500, incraseDist ? 350 : 200, NULL, d->entityId, NULL, pitch);

        if(distance > 350)
        {
            playSound(attacks[atk].middistsnd, d->o, 700, 300, SND_LOWPRIORITY, d->entityId);
            if(distance > 600) playSound(attacks[atk].fardistsnd, d->o, 1000, 600, SND_LOWPRIORITY, d->entityId, NULL, pitch);
        }
    }

    void particletrack(physent *owner, vec &o, vec &d)
    {
        if(owner->type!=ENT_PLAYER && owner->type!=ENT_AI) return;
        gameent *pl = (gameent *)owner;
        if(pl->muzzle.x < 0 || pl->lastattack < 0 || attacks[pl->lastattack].gun != pl->gunselect) return;
        float dist = o.dist(d);
        o = pl->muzzle;
        if(dist <= 0) d = o;
        else
        {
            vecfromyawpitch(owner->yaw, owner->pitch, 1, 0, d);
            float newdist = raycube(owner->o, d, dist, RAY_CLIPMAT|RAY_ALPHAPOLY);
            d.mul(min(newdist, dist)).add(owner->o);
        }
    }

    void dynlighttrack(physent *owner, vec &o, vec &hud)
    {
        if(owner->type!=ENT_PLAYER && owner->type!=ENT_AI) return;
        gameent *pl = (gameent *)owner;
        if(pl->muzzle.x < 0 || pl->lastattack < 0 || attacks[pl->lastattack].gun != pl->gunselect) return;
        o = pl->muzzle;
        hud = owner == followingplayer(player1) ? vec(pl->o).add(vec(0, 0, 2)) : pl->muzzle;
    }

    float intersectdist = 1e16f;

    bool intersect(dynent *d, const vec &from, const vec &to, float margin, float &dist)   // if lineseg hits entity bounding box
    {
        vec bottom(d->o), top(d->o);
        bottom.z -= d->eyeheight + margin;
        top.z += d->aboveeye + margin;
        return linecylinderintersect(from, to, bottom, top, d->radius + margin, dist);
    }

    dynent *intersectclosest(const vec &from, const vec &to, gameent *at, float margin, float &bestdist)
    {
        dynent *best = NULL;
        bestdist = 1e16f;
        loopi(numdynents())
        {
            dynent *o = iterdynents(i);
            if(o==at || o->state!=CS_ALIVE) continue;
            float dist;
            if(!intersect(o, from, to, margin, dist)) continue;
            if(dist<bestdist)
            {
                best = o;
                bestdist = dist;
            }
        }
        return best;
    }

    void shorten(const vec &from, vec &target, float dist)
    {
        target.sub(from).mul(min(1.0f, dist)).add(from);
    }

    void raydamage(vec &from, vec &to, gameent *d, int atk)
    {
        dynent *o;
        float dist;
        int margin = attacks[atk].margin;

        if(attacks[atk].rays > 1)
        {
            dynent *hits[MAXRAYS];
            int maxrays = attacks[atk].rays;
            loopi(maxrays)
            {
                if((hits[i] = intersectclosest(from, rays[i], d, margin, dist))) shorten(from, rays[i], dist);
            }
            loopi(maxrays) if(hits[i])
            {
                o = hits[i];
                hits[i] = NULL;
                int numhits = 1;
                for(int j = i+1; j < maxrays; j++) if(hits[j] == o)
                {
                    hits[j] = NULL;
                    numhits++;
                }
                hitpush(numhits*attacks[atk].damage, o, d, from, to, atk, numhits);
            }
        }
        else if((o = intersectclosest(from, to, d, margin, dist)))
        {
            shorten(from, to, dist);
            hitpush(attacks[atk].damage, o, d, from, to, atk, 1);
        }
    }

    void shoot(gameent *d, const vec &targ)
    {
        int prevaction = d->lastaction, attacktime = lastmillis-prevaction;
        bool specialAbility = d->aptitude==APT_PRETRE && d->abilitymillis[ABILITY_3];

        if(d->aitype==AI_BOT && (d->gunselect==GUN_GLOCK || d->gunselect==GUN_SPOCKGUN || d->gunselect==GUN_HYDRA || d->gunselect==GUN_SKS || d->gunselect==GUN_S_CAMPOUZE))
        {
            switch(rnd(d->gunselect==GUN_GLOCK || d->gunselect==GUN_SPOCKGUN || d->gunselect==GUN_HYDRA ? 5 : 15)) {case 0: d->gunwait+=(specialAbility ? 500 : 1200) / curfps; return; }
        }

        switch(d->gunselect)
        {
            case GUN_MINIGUN:
            case GUN_PLASMA:
            case GUN_S_ROQUETTES:
                if(!d->attacking) d->gunselect==GUN_PLASMA ? d->gunaccel=4 : d->gunselect==GUN_S_ROQUETTES ? d->gunaccel=3 : d->gunaccel=12;
                break;
            default: d->gunaccel=0;
        }

        if(attacktime < d->gunwait + d->gunaccel*(d->gunselect==GUN_PLASMA ? 50 : d->gunselect==GUN_S_ROQUETTES ? 150 : 8) + (d==player1 || specialAbility ? 0 : attacks[d->gunselect].attackdelay)) return;
        d->gunwait = 0;

        if(d->aptitude==APT_KAMIKAZE)
        {
            if(d->abilitymillis[ABILITY_2]>0 && d->abilitymillis[ABILITY_2]<2000 && d->ammo[GUN_KAMIKAZE]>0 && !d->playerexploded)
            {
                gunselect(GUN_KAMIKAZE, d);
                d->attacking = ACT_SHOOT;
                d->playerexploded = true;
            }
        }

        if(d->armourtype==A_ASSIST && !d->armour && !d->playerexploded && d->ammo[GUN_ASSISTXPL])
        {
            d->wasAttacking = d->attacking;
            gunselect(GUN_ASSISTXPL, d, true);
            d->attacking = ACT_SHOOT;
            d->playerexploded = true;
        }

        if(!d->attacking) return;
        int gun = d->gunselect, act = d->attacking, atk = guns[gun].attacks[act];

        if(d->gunaccel) d->gunaccel -= 1;
        d->lastaction = lastmillis;
        d->lastattack = atk;

        if(d==player1)
        {
            lastshoot = totalmillis;
            if(atk==ATK_SMAW_SHOOT || atk==ATK_NUKE_SHOOT || atk==ATK_CACMARTEAU_SHOOT || atk == ATK_MOSSBERG_SHOOT || atk == ATK_SV98_SHOOT) lastshoot+=750;
        }

        if(!d->ammo[gun])
        {
            if(d==player1) msgsound(S_NOAMMO, d);
            d->gunwait = 600;
            d->lastattack = -1;
            weaponswitch(d);
            return;
        }

        if(!m_muninfinie || server::noInfiniteAmmo(atk)) d->ammo[gun] -= attacks[atk].use;

        vec from = d->o, to = targ, dir = vec(to).sub(from).safenormalize();
        float dist = to.dist(from);

        int kickfactor = (m_tutorial && !canMove) || d->aptitude==APT_AMERICAIN ? 0 : (d->crouched() ? -1.25f : -2.5f);
        vec kickback = (d->aptitude==APT_AMERICAIN ? vec(0, 0, 0) : vec(dir).mul(attacks[atk].kickamount*kickfactor));
        d->vel.add(kickback);

        float shorten = attacks[atk].range && dist > attacks[atk].range ? attacks[atk].range : 0,
              barrier = raycube(d->o, dir, dist, RAY_CLIPMAT|RAY_ALPHAPOLY);
        if(barrier > 0 && barrier < dist && (!shorten || barrier < shorten))
            shorten = barrier;
        if(shorten) to = vec(dir).mul(shorten).add(from);

        if(attacks[atk].rays > 1) createrays(atk, from, to, d);
        else if(attacks[atk].aimspread) offsetray(from, to, spread(atk, d), attacks[atk].range, to, d);

        hits.setsize(0);

        if(!attacks[atk].projspeed) raydamage(from, to, d, atk);

        shoteffects(atk, from, to, d, true, 0, prevaction);

        if(d==player1 || d->ai || d->type==ENT_AI)
        {
            addmsg(N_SHOOT, "rci2i6iv", d, lastmillis-maptime, atk,
                   (int)(from.x*DMF), (int)(from.y*DMF), (int)(from.z*DMF),
                   (int)(to.x*DMF), (int)(to.y*DMF), (int)(to.z*DMF),
                   hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
        }
        float waitfactor = 1;
        if(d->aptitude==APT_PRETRE && d->abilitymillis[ABILITY_3]) waitfactor = 2.5f + ((4000.f - d->abilitymillis[ABILITY_3])/1000.f);
        if(d->boostmillis[B_SHROOMS]) waitfactor *= d->aptitude==APT_JUNKIE ? 1.75f : 1.5f;
        d->gunwait = attacks[atk].attackdelay/waitfactor;
        d->boostmillis[B_ROIDS] ? d->totalshots += (attacks[atk].damage*attacks[atk].rays)*2: d->totalshots += attacks[atk].damage*attacks[atk].rays;

        if(d->playerexploded) {d->attacking = d->wasAttacking; execute("getoldweap"); d->playerexploded = false;}
        if((atk==ATK_GLOCK_SHOOT || atk==ATK_SPOCKGUN_SHOOT || atk==ATK_HYDRA_SHOOT || d->gunselect==GUN_SKS || d->gunselect==GUN_S_CAMPOUZE) && !specialAbility) d->attacking = ACT_IDLE;
    }

    void adddynlights()
    {
        int lightradiusvar = 0;
        loopv(projs)
        {
            projectile &p = projs[i];
            vec pos(p.o);
            pos.add(vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)));
            if(!ispaused()) lightradiusvar = rnd(30);

            switch(p.atk)
            {
                case ATK_PULSE_SHOOT: adddynlight(pos, 30, vec(1.00f, 0.75f, 0.0f)); break;
                case ATK_SPOCKGUN_SHOOT: adddynlight(pos, 30, vec(0.00f, 1.00f, 0.0f)); break;
                case ATK_GRAP1_SHOOT: adddynlight(pos, 50, vec(0.3f, 0.00f, 0.2f)); break;
                case ATK_ARTIFICE_SHOOT:
                case ATK_SMAW_SHOOT:
                case ATK_ROQUETTES_SHOOT: adddynlight(pos, 50+lightradiusvar, vec(1.2f, 0.75f, 0.0f)); break;
                case ATK_NUKE_SHOOT: adddynlight(pos, 100, vec(1.2f, 0.75f, 0.0f)); break;
            }
        }
        loopv(curBouncers)
        {
            bouncer &bnc = *curBouncers[i];
            if(bnc.bouncetype==BNC_GRENADE || bnc.bouncetype==BNC_LIGHT)
            {
                vec pos(bnc.o);
                pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
                if(bnc.bouncetype==BNC_GRENADE) adddynlight(pos, 40, vec(0.5f, 0.5f, 2.0f), 0, 0, L_NOSHADOW);
                else adddynlight(pos, 115, vec(0.25f, 0.12f, 0.0f), 0, 0, L_VOLUMETRIC|L_NOSHADOW|L_NOSPEC);
            }
        }
    }

    void preloadbouncers()
    {
        loopi(NUMBOUNCERS)
        {
            if(i == BNC_LIGHT) break;
            bool hasVariants = bouncers[i].variants;
            if(!hasVariants) preloadmodel(getBouncerDir(i, 0));
            else { loopj(bouncers[i].variants) preloadmodel(getBouncerDir(i, j + 1)); }
        }
    }

    void renderbouncers()
    {
        bool isPaused = ispaused();
        float yaw, pitch;
        loopv(curBouncers)
        {
            bouncer &bnc = *curBouncers[i];
            vec pos(bnc.o);
            pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
            vec vel(bnc.vel);
            pitch = -bnc.roll;
            yaw = -bnc.yaw;

            bool inWater = ((lookupmaterial(pos) & MAT_WATER) == MAT_WATER);

            if(vel.magnitude() <= 3.f) {yaw = bnc.lastyaw; pitch = bnc.lastpitch;}
            else if (!isPaused)
            {
                vectoyawpitch(vel, yaw, pitch);
                yaw += bnc.bounces < 5 ? 75 + rnd(31) : 90;
                bnc.lastpitch = pitch;
            }

            rendermodel(getBouncerDir(bnc.bouncetype, bnc.variant), ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, 0, MDL_CULL_VFC|MDL_CULL_EXTDIST|MDL_CULL_OCCLUDED);

            if(!isPaused && ((bnc.vel.magnitude() > 25.f && bnc.bounces < 5) || bnc.bouncetype == BNC_GRENADE))
            {
                vec pos(bnc.o);
                pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));

                switch(bnc.bouncetype)
                {
                    case BNC_CASING:
                    case BNC_BIGCASING:
                    case BNC_CARTRIDGE:
                        particle_splash(PART_SMOKE, 1, 150, pos, bnc.bouncetype==BNC_CARTRIDGE ? 0x252525 : 0x404040, bnc.bouncetype==BNC_CASING ? 1.0f : 1.75f, 50, -20);
                        break;

                    case BNC_ROCK:
                        particle_splash(PART_SMOKE, 1, 150, pos, 0x404040, 2.5f, 50, -20);
                        break;

                    case BNC_BIGROCK:
                        particle_splash(PART_SMOKE, 1, 500, pos, 0x151515, 8.f, 50, -20);
                        break;

                    case BNC_GRENADE:
                    {
                        float growth = (1000 - (bnc.lifetime - curtime))/150.f;
                        particle_fireball(pos, growth, PART_EXPLOSION, 20, hasRoids(bnc.owner) ? 0xFF0000 : 0x0055FF, growth, hasShrooms());
                        particle_splash(PART_SMOKE, 1, 150, pos, 0x404088, 2.5f, 50, -20);
                        break;
                    }
                    case BNC_SCRAP:
                        particle_splash(inWater ? PART_BUBBLE : PART_SMOKE, inWater ? 1 : 3, 250, pos, 0x222222, 2.5f, 50, -50);
                        particle_splash(PART_FIRE_BALL, 2, 75, pos, 0x994400, 0.7f, 30, -30);
                        break;
                }
            }
        }
    }

    std::map<int, std::string> projsPaths =
    {
        {ATK_SMAW_SHOOT, "projectiles/missile"},
        {ATK_ARTIFICE_SHOOT, "projectiles/feuartifice"},
        {ATK_ARBALETE_SHOOT, "projectiles/fleche"},
        {ATK_ROQUETTES_SHOOT, "projectiles/minimissile"},
        {ATK_NUKE_SHOOT, "projectiles/missilenuke"}
    };

    void preloadProjectiles()
    {
        for(const auto& pair : projsPaths)
        {
            preloadmodel(pair.second.c_str());
        }
    }

    void renderprojectiles()
    {
        float yaw, pitch;
        loopv(projs)
        {
            projectile &p = projs[i];

            if(p.atk==ATK_SMAW_SHOOT || p.atk==ATK_ARTIFICE_SHOOT || p.atk==ATK_ARBALETE_SHOOT || p.atk==ATK_ROQUETTES_SHOOT || p.atk==ATK_NUKE_SHOOT)
            {

                float dist = min(p.o.dist(p.to)/32.0f, 1.0f);
                vec pos = vec(p.o).add(vec(p.offset).mul(dist*p.offsetmillis/float(OFFSETMILLIS))),
                    v = dist < 1e-6f ? p.dir : vec(p.to).sub(pos).normalize();

                vectoyawpitch(v, yaw, pitch);
                v.mul(3);
                v.add(pos);

                rendermodel(projsPaths[p.atk].c_str(), ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, MDL_NOSHADOW|MDL_CULL_VFC|MDL_CULL_OCCLUDED);
                if(p.atk != ATK_ARBALETE_SHOOT) renderProjectilesTrails(p.owner, pos, v, p.from, p.offset, p.atk, p.exploded);
            }
        }
    }

    void removeweapons(gameent *d)
    {
        removebouncers(d);
        removeprojectiles(d);
    }

    void updateweapons(int curtime)
    {
        updateprojectiles(curtime);
        if(player1->clientnum>=0 && player1->state==CS_ALIVE) shoot(player1, worldpos); // only shoot when connected to server
        updatebouncers(curtime); // need to do this after the player shoots so bouncers don't end up inside player's BB next frame
        gameent *following = followingplayer();
        if(!following) following = player1;
    }

    void avoidweapons(ai::avoidset &obstacles, float radius)
    {
        loopv(projs)
        {
            projectile &p = projs[i];
            obstacles.avoidnear(NULL, p.o.z + attacks[p.atk].exprad + 1, p.o, radius + attacks[p.atk].exprad);
        }
    }
};
