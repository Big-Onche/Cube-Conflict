// weapon.cpp: all shooting and effects code, projectile management
#include "game.h"
#include "engine.h"
#include "gfx.h"
#include "stats.h"
#include <string>

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

    void findSpecialWeapon(gameent *d, int baseWeapon, int maxWeapons, inventoryCallback callback)
    {
        loopi(maxWeapons)
        {
            int gunId = baseWeapon + i;
            if(d->ammo[gunId])
            {
                callback(gunId);
                return;
            }
        }
    }

    void gunselect(int gun, gameent *d, bool force, bool shortcut)
    {
        if((gun==GUN_POWERARMOR && !force) || !validgun(gun)) return;
        if(gun != d->gunselect)
        {
            if(d==player1 && gun != GUN_POWERARMOR && !shortcut) player1->lastweap = gun;
            addmsg(N_GUNSELECT, "rci", d, gun);
            playSound(attacks[gun-GUN_ELECTRIC].picksound, d==hudplayer() ? vec(0, 0, 0) : d->o, 200, 50, NULL, d->entityId);
            d->lastgunselect = lastmillis;
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
        if(!isconnected()) return;
        if(player1->character==C_NINJA) gunselect(GUN_NINJA, player1, false, true);
        else if (m_identique) return;
        else findSpecialWeapon(player1, GUN_M_BUSTER, NUMMELEEWEAPONS, [](int gunId) { gunselect(gunId, player1, false, true); });
        doaction(ACT_SHOOT);
        execute("sleep 500 [shoot ; getoldweap]");
    );

    ICOMMAND(getoldweap, "", (), { if(isconnected() || !m_identique) gunselect(player1->lastweap, player1); });

    void nextweapon(int dir, bool force = false)
    {
        if(player1->state!=CS_ALIVE) return;
        if(m_identique)
        {
            int gunId;
            switch(player1->character)
            {
                case C_KAMIKAZE: gunId = GUN_KAMIKAZE; break;
                case C_NINJA: gunId = GUN_NINJA; break;
                default: gunId = currentIdenticalWeapon;
            }

            if(player1->gunselect == currentIdenticalWeapon)
            {
                if(dir - 1) gunselect(gunId, player1);
                else findSpecialWeapon(player1, GUN_S_NUKE, NUMSUPERWEAPONS, [](int gunId) { gunselect(gunId, player1); });
            }
            else gunselect(currentIdenticalWeapon, player1);
            return;
        }
        dir = (dir < 0 ? NUMGUNS-1 : 1);
        int gun = player1->gunselect;
        loopi(NUMGUNS)
        {
            gun = (gun + dir)%NUMGUNS;
            if(gun==GUN_POWERARMOR) gun = (gun + dir)%NUMGUNS;
            if(force || player1->ammo[gun]) break;
        }
        if(gun != player1->gunselect) gunselect(gun, player1);
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
        if(player1->state!=CS_ALIVE || !validgun(gun) || gun==GUN_POWERARMOR) return;
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
            if(gun>=0 && gun<NUMGUNS && (force || player1->ammo[gun]) && gun!=GUN_POWERARMOR)
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
            if(d->ammo[i] && d->gunselect!=i && i!=GUN_POWERARMOR)
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
                if(validgun(gun) && gun != player1->gunselect && player1->ammo[gun] && gun!=GUN_POWERARMOR) { gunselect(gun, player1); return; }
            } else { weaponswitch(player1); return; }
        }
        playSound(S_NOAMMO);
    });

    bool spreadLimit(gameent *d) { return d->gunselect == GUN_FLAMETHROWER || d->gunselect == GUN_MOSSBERG || d->gunselect == GUN_HYDRA; }

    void offsetray(const vec &from, const vec &to, float spread, float range, vec &dest, gameent *d)
    {
        vec offset;
        do offset = vec(rndscale(1), rndscale(1), rndscale(1)).sub(0.5f);
        while(offset.squaredlen() > 0.5f*0.5f);

        if(d->boostmillis[B_SHROOMS]) spread /= d->character==C_JUNKIE ? 1.75f : 1.5f;
        if(d->character==C_WIZARD && d->abilitymillis[ABILITY_2]) spread /= 3.f;

        if(!spreadLimit(d))
        {
            if(d->crouching) spread /= d->character==C_CAMPER ? 2.5f : 1.3f;
            if(d->boostmillis[B_ROIDS] || d->boostmillis[B_RAGE]) spread *= 1.75f;
        }

        spread = (spread*100) / classes[d->character].accuracy;
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
        { "molotov",            1.0f, 1.0f,  0,  750, -1,                 0,    0              },
        { "pixel",              1.0f, 0.6f,  8,  600, S_B_PIXEL,        120,    0              },
        { "debris",             0.5f, 0.5f,  0, 1000, -1,                 0,    0              },
        { "rock",               0.8f, 0.8f,  4,  750, S_B_ROCK,         120,    0              },
        { "rock/big",           2.5f, 0.4f,  3, 1250, S_B_BIGROCK,      300,    0              },
        { "casing",             0.2f, 1.0f,  0,  250, S_B_CASING,       120,    SND_FIXEDPITCH },
        { "casing/big",         0.2f, 1.0f,  0,  250, S_B_BIGCASING,    120,    SND_FIXEDPITCH },
        { "casing/cartridge",   0.2f, 1.0f,  0,  250, S_B_CARTRIDGE,    120,    SND_FIXEDPITCH },
        { "scrap",              1.5f, 0.7f,  3,  750, S_B_SCRAP,        180,    0              },
        { "glass",              0.5f, 0.2f,  0,  500, -1,                 0,    0              },
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

        if(type==BNC_GRENADE || type==BNC_MOLOTOV)
        {
            bool isGrenade = (type==BNC_GRENADE);
            bnc.offset = hudgunorigin(isGrenade ? GUN_M32 : GUN_MOLOTOV, from, to, owner);
            if(owner==hudplayer() && !isthirdperson()) bnc.offset.sub(owner->o).rescale(16).add(owner->o);
            if(isGrenade) playSound(S_GRENADE, bnc.o, 300, 100, SND_FIXEDPITCH|SND_NOCULL, bnc.entityId);
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
                bool inWater = ((lookupmaterial(bnc.o) & MATF_VOLUME) == MAT_WATER);
                if(!bnc.inwater && inWater) // the bouncer enter in water
                {
                    bnc.inwater = true;
                    particle_splash(PART_WATER, (20 * bnc.radius), 150, bnc.o, 0x28282A, (5.f * bnc.radius), (200.f * bnc.radius), -300, (10.f * bnc.radius), hasShrooms());
                    if(bnc.radius > 0.5f) playSound(bnc.radius > 1.f ? S_SPLASH : S_IMPACTWATER, vec(bnc.o).addz(5), 250, 100, SND_LOWPRIORITY);
                }
                else if(bnc.inwater && !inWater) bnc.inwater = false; // the bouncer bounced outside the water
            }

            if(stopped || (bnc.bouncetype == BNC_MOLOTOV && bnc.bounces))
            {
                if(bnc.bouncetype == BNC_GRENADE || bnc.bouncetype == BNC_MOLOTOV)
                {
                    int atk = (bnc.bouncetype == BNC_GRENADE ? ATK_M32 : ATK_MOLOTOV);
                    hits.setsize(0);
                    explode(bnc.local, bnc.owner, bnc.o, bnc.o, NULL, 1, atk);
                    stopLinkedSound(bnc.entityId);
                    removeEntityPos(bnc.entityId);
                    if(bnc.local)
                        addmsg(N_EXPLODE, "rci3iv", bnc.owner, lastmillis-maptime, atk, bnc.id-maptime,
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
        if(from.isneg()) return;
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
            case ATK_FIREWORKS: p.lifetime=attacks[atk].ttl+rnd(400); break;
            case ATK_CROSSBOW: p.lifetime=temptrisfade+rnd(5000); break;
            default: p.lifetime = attacks[atk].ttl;
        }
        p.exploded = false;
        p.offsetmillis = OFFSETMILLIS;
        p.id = local ? lastmillis : id;
        p.inwater = (lookupmaterial(camera1->o) & MATF_VOLUME) == MAT_WATER;

        switch(p.atk)
        {
            case ATK_PLASMA: p.projsound = S_FLYBYPLASMA; break;
            case ATK_GRAP1: p.projsound = S_FLYBYGRAP1; break;
            case ATK_SPOCKGUN: p.projsound = S_FLYBYSPOCK; break;
            case ATK_SMAW: p.projsound = S_ROCKET; break;
            case ATK_S_ROCKETS: p.projsound = S_MINIROCKET; break;
            case ATK_S_NUKE: p.projsound = S_MISSILENUKE; break;
            case ATK_FIREWORKS: p.projsound = S_FLYBYFIREWORKS;
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
        if(p.isneg() || ((camera1->o.dist(p) > bouncers[type].cullDist) && type!=BNC_GRENADE)) return; // culling distant ones, except grenades, grenades are important

        vec dir;
        if(!speed) speed = 50 + rnd(20);

        switch(type)
        {
            case BNC_CASING:
            case BNC_BIGCASING:
            case BNC_CARTRIDGE:
                vecfromyawpitch(d->yaw, 0, 0, -50, dir);
                dir.add(vec(-10 + rnd(21), -10 + rnd(21), 100));
                break;
            case BNC_GLASS:
                dir = vec(-150 + rnd(301), -150 + rnd(301), rnd(50));
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

        int actorClass = actor->character, targetClass = d->character;
        bool isHudPlayer = (actor == hudplayer());
        bool teamDamage = (isteam(d->team, actor->team) && d!=actor);

        if(isHudPlayer) d->curdamagecolor = 0xFFAA00;

        damage = ((damage*classes[actorClass].damage)/(classes[targetClass].resistance)); // calc damage based on the class's stats

        switch(actorClass) // recalc damage based on the actor's passive/active skills
        {
            case C_AMERICAN:
                if(atk >= ATK_S_NUKE && atk <= ATK_S_CAMPER)
                {
                    damage *= 1.5f;
                    if(isHudPlayer) d->curdamagecolor = 0xFF0000;
                }
                break;

            case C_NINJA:
                if(atk == ATK_NINJA && actor==hudplayer()) d->curdamagecolor = 0xFF0000;
                break;

            case C_WIZARD:
                if(actor->abilitymillis[ABILITY_2])
                {
                    damage *= 1.25f;
                    if(isHudPlayer) d->curdamagecolor = 0xFF00FF;
                }
                break;

            case C_CAMPER:
                damage *= ((actor->o.dist(d->o)/1800.f)+1.f);
                break;

            case C_VIKING:
                if(actor->boostmillis[B_RAGE])
                {
                    damage *= 1.25f;
                    if(isHudPlayer) d->curdamagecolor = 0xFF7700;
                }
                break;

            case C_SHOSHONE:
                if(actor->abilitymillis[ABILITY_3])
                {
                    damage *= 1.3f;
                    if(isHudPlayer) d->curdamagecolor = 0xFF7700;
                }
                if(targetClass == C_AMERICAN)
                {
                    damage /= 1.25f;
                    if(isHudPlayer) d->curdamagecolor = 0xFFFF00;
                }
                break;

            case C_PHYSICIST:
                if(d==player1 && actor==player1 && player1->armour && player1->abilitymillis[ABILITY_1]) unlockAchievement(ACH_BRICOLEUR);
        }

        switch(targetClass) // recalc damage based on the victim's passive/active
        {
            case C_WIZARD:
                if(d->abilitymillis[ABILITY_3])
                {
                    damage /= 5.0f;
                    d->curdamagecolor = 0x8855AA;
                }
                break;

            case C_PRIEST:
                if(isHudPlayer && d->abilitymillis[ABILITY_2] && targetClass == C_PRIEST && d->mana) d->curdamagecolor = 0xAA00AA;
                break;

            case C_SHOSHONE:
                if(d->abilitymillis[ABILITY_1]) damage /= 1.3f;
                if(actorClass == C_AMERICAN)
                {
                    damage *= 1.25f;
                    if(isHudPlayer) d->curdamagecolor = 0xFF7700;
                }
        }

        if(actor->boostmillis[B_ROIDS]) // recalc damage if actor has roids
        {
            damage *= actorClass == C_JUNKIE ? 3 : 2;
            if(isHudPlayer) d->curdamagecolor = 0xFF0000;
        }
        if(d->boostmillis[B_JOINT]) // recalc victim if actor has joint
        {
            damage /= targetClass == C_JUNKIE ? 1.875f : 1.25f;
            if(isHudPlayer) d->curdamagecolor = 0xAAAA55;
        }

        if(teamDamage && isHudPlayer) // recalc if ally or not
        {
            if(actorClass == C_MEDIC) return;
            damage /= (actorClass == C_JUNKIE ? 1.5f : 3.f);
            d->curdamagecolor = 0x888888;
        }

        if(!d->armour || d->armourtype!=A_MAGNET)
        {
            if(blood) particle_splash(PART_BLOOD, damage > 300 ? 3 : damage/100, 1000, p, 0x60FFFF, 2.96f);
            gibeffect(damage, vec(0,0,0), d);
        }

        damage /= 10.f; // rescale damage value

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
            if(d->armourtype != A_POWERARMOR) spawnbouncer(d->o, d->vel, d, BNC_PIXEL, 100 + rnd(50));
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
                if(player1->boostmillis[B_JOINT]) damage/=(player1->character==C_JUNKIE ? 1.875f : 1.25f);
                switch(player1->character)
                {
                    case C_WIZARD: {if(player1->abilitymillis[ABILITY_3]) damage/=5.f; } break;
                    case C_VIKING: player1->boostmillis[B_RAGE]+=damage; break;
                    case C_PRIEST: if(player1->abilitymillis[ABILITY_2] && player1->mana) {player1->mana-=damage/10; damage=0; if(player1->mana<0)player1->mana=0;} break;
                    case C_SHOSHONE: if(player1->abilitymillis[ABILITY_1]) damage/=1.3f;
                }
                damage = (damage/classes[player1->character].resistance)*(m_dmsp ? 15.f : 100);
                damageeffect(damage, f, at, atk);
                damaged(damage, f, at, true, atk);
                f->hitphyspush(damage, vel, at, atk, f);
            }
            else if(at==player1)
            {
                if(player1->boostmillis[B_ROIDS]) damage *= (player1->character==C_JUNKIE ? 3 : 2);
                switch(player1->character)
                {
                    case C_AMERICAN: {if(atk==ATK_S_NUKE || atk==ATK_S_GAU8 || atk==ATK_S_ROCKETS || atk==ATK_S_CAMPER) damage *= 1.5f; break;}
                    case C_VIKING: if(player1->boostmillis[B_RAGE]) damage*=1.25f; break;
                    case C_WIZARD: {if(player1->abilitymillis[ABILITY_2]) damage *= 1.25f; break;}
                    case C_CAMPER: damage *= ((player1->o.dist(f->o)/1800.f)+1.f); break;
                    case C_VAMPIRE: {player1->health = min(player1->health + damage/2, player1->maxhealth); player1->vampiremillis+=damage*1.5f;} break;
                    case C_SHOSHONE: if(player1->abilitymillis[ABILITY_1]) damage*=1.3f;
                }
                damage = (damage*classes[player1->character].damage)/100;
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
            bool hasRegenAbility = f->character==C_PHYSICIST && f->abilitymillis[ABILITY_1];

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
                        else if(!rnd(atk==ATK_FLAMETHROWER ? 5 : 2)) playSound(impactSound);
                    }
                    else playSound(S_IMPACTBODY);
                }
            }
            else
            {
                if(f->armour)
                {
                    if(hasRegenAbility) { playSound(S_PHY_1, f->o, 200, 100, SND_LOWPRIORITY); playSound(regenSound, f->o, 200, 100, SND_LOWPRIORITY); }
                    else if(!rnd(atk==ATK_FLAMETHROWER ? 5 : 2)) playSound(impactSound, f->o, 250, 50, SND_LOWPRIORITY);
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
            float damage = o==at && atk==ATK_POWERARMOR ? 0 : attacks[atk].damage*(1-dist/EXP_DISTSCALE/attacks[atk].exprad);
            if(damage > 0) hit(max(int(damage), 1), o, at, dir, atk, dist);
        }
    }

    void startShake(const vec &v, int maxdist, int atk, float factorMod = 1.f)
    {
        float distance = camera1->o.dist(v);
        if(distance > maxdist) return;
        float factor = lerp(1.0f, 0.0f, distance / maxdist); // As normalized distance increases, the factor linearly decreases from 1 to 0
        factor *= factorMod;
        shakeScreen(factor);
    }

    void explode(bool local, gameent *owner, const vec &v, const vec &vel, dynent *safe, int damage, int atk)
    {
        vec safeLoc = vec(v).sub(vec(vel).mul(15)); // avoid spawn in wall
        bool inWater = ((lookupmaterial(v) & MATF_VOLUME) == MAT_WATER);
        bool isFar = camera1->o.dist(v) >= 300;

        switch(atk)
        {
            case ATK_PLASMA:
            case ATK_GRAP1:
            case ATK_SPOCKGUN:
                renderProjectileExplosion(owner, v, vel, safe, atk);
                playSound(atk==ATK_GRAP1 ? S_IMPACTGRAP1 : atk==ATK_PLASMA ? S_IMPACTPLASMA : S_IMPACTSPOCK, v, 250, 50, SND_LOWPRIORITY);
                break;

            case ATK_SMAW:
            case ATK_S_ROCKETS:
                loopi(atk==ATK_S_ROCKETS ? 3 : 4 + rnd(3))
                {
                    vec pos = safeLoc;
                    pos.add(vec(-4 + rnd(8), -4 + rnd(8), -4 + rnd(8)));
                    spawnbouncer(pos, vec(0, 0, 0), owner, BNC_ROCK, 200);
                    spawnbouncer(pos, vec(0, 0, 0), owner, BNC_BURNINGDEBRIS, 250, 400);
                }
                renderExplosion(owner, safeLoc, vel, atk);
                playSound(S_EXPL_MISSILE, safeLoc, 400, 150);
                if(inWater) playSound(S_EXPL_INWATER, vec(v).addz(15), 300, 100);
                if(isFar) playSound(S_EXPL_FAR, safeLoc, 1500, 400, SND_LOWPRIORITY);
                startShake(v, 1.25f * attacks[atk].exprad, atk);
                break;

            case ATK_KAMIKAZE:
            case ATK_POWERARMOR:
                loopi(10 + rnd(5))
                {
                    if(atk==ATK_POWERARMOR) spawnbouncer(v, owner->vel, owner, BNC_SCRAP, 50 + rnd(250));
                    else spawnbouncer(v, owner->vel, owner, rnd(2) ? BNC_PIXEL : BNC_ROCK, 100 + rnd(300));
                }
                renderExplosion(owner, owner->o, vel, atk);
                playSound(atk==ATK_KAMIKAZE ? S_EXPL_KAMIKAZE : S_EXPL_PARMOR, owner->o, 400, 150);
                if(inWater) playSound(S_EXPL_INWATER, vec(owner->o).addz(15), 300, 100);
                if(isFar) playSound(S_BIGEXPL_FAR, owner->o, 2000, 400);
                startShake(v, 1.5f * attacks[atk].exprad, atk);
                break;

            case ATK_S_NUKE:
                renderExplosion(owner, v, vel, atk);
                playSound(S_EXPL_NUKE, v, 5000, 3000, SND_NOOCCLUSION);
                startShake(v, 2 * attacks[atk].exprad, atk, 2);
                break;

            case ATK_FIREWORKS:
                renderExplosion(owner, safeLoc, vel, atk);
                playSound(S_EXPL_FIREWORKS, safeLoc, 400, 200);
                if(inWater) playSound(S_EXPL_INWATER, vec(v).addz(15), 300, 100);
                if(isFar) playSound(S_FIREWORKSEXPL_FAR, safeLoc, 2000, 750);
                startShake(v, attacks[atk].exprad, atk, 0.75f);
                break;

            case ATK_M32:
                loopi(5+rnd(3)) spawnbouncer(v, vec(0, 0, 0), owner, BNC_ROCK, 200);
                renderExplosion(owner, v, vel, atk);
                playSound(S_EXPL_GRENADE, v, 400, 150);
                if(inWater) playSound(S_EXPL_INWATER, vec(v).addz(15), 300, 100);
                if(isFar) playSound(S_EXPL_FAR, v, 2000, 400, SND_LOWPRIORITY);
                startShake(v, 1.5f * attacks[atk].exprad, atk);
                break;

            case ATK_MOLOTOV:
            {
                vec debrisLoc = v;
                debrisLoc.addz(3);
                loopi(6+rnd(2)) spawnbouncer(debrisLoc, vec(0, 0, 0), owner, BNC_GLASS, 300, 350+rnd(200));
                renderExplosion(owner, v, vel, atk);
                playSound(S_EXPL_MOLOTOV, v, 300, 150);
                if(inWater) playSound(S_EXPL_INWATER, vec(v).addz(15), 300, 100);
                if(isFar) playSound(S_MOLOTOVEXPL_FAR, v, 1500, 750, SND_LOWPRIORITY);
                startShake(v, 1.25f * attacks[atk].exprad, atk, 0.5f);
                break;
            }
            case ATK_MINIGUN:
            case ATK_AK47:
            case ATK_UZI:
            case ATK_FAMAS:
            case ATK_GLOCK:
            case ATK_CROSSBOW:
                renderBulletImpact(owner, v, vel, safe, atk);
                if(!inWater) playSound(atk==ATK_CROSSBOW ? S_IMPACTARROW : S_LITTLERICOCHET, v, 175, 75, SND_LOWPRIORITY);
                break;

            case ATK_SV98:
            case ATK_SKS:
            case ATK_S_GAU8:
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
            case ATK_PLASMA:
                addstain(STAIN_PLASMA_SCORCH, pos, dir, 7.5f);
                addstain(STAIN_BULLET_HOLE, pos, dir, 7.5f, 0x882200);
                return;
            case ATK_SMAW:
            case ATK_KAMIKAZE:
            case ATK_POWERARMOR:
            case ATK_S_ROCKETS:
            {
                float rad = attacks[p.atk].exprad*0.35f;
                addstain(STAIN_EXPL_SCORCH, pos, dir, rad);
                return;
            }
            case ATK_MOLOTOV:
            {
               loopi(3) addstain(STAIN_BURN, pos, dir, attacks[p.atk].exprad);
            }
            case ATK_MINIGUN:
            case ATK_SV98:
            case ATK_AK47:
            case ATK_S_GAU8:
            case ATK_SKS:
                addstain(STAIN_BULLET_HOLE, pos, dir, 1.5f+(rnd(2)));
                addstain(STAIN_BULLET_GLOW, pos, dir, 2.0f+(rnd(2)), 0x883300);
                return;
            case ATK_SPOCKGUN:
                addstain(STAIN_PLASMA_SCORCH, pos, dir, 5);
                addstain(STAIN_SPOCK, pos, dir, 5, hasRoids(p.owner) ? 0xFF0000 : 0x22FF22);
                return;
            case ATK_UZI:
            case ATK_CROSSBOW:
            case ATK_GLOCK:
            case ATK_FAMAS:
                addstain(STAIN_BULLET_HOLE, pos, dir, 0.5f);
                addstain(STAIN_BULLET_GLOW, pos, dir, 1.0f, 0x883300);
                return;
            case ATK_GRAP1:
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
            if((b.bouncetype == BNC_GRENADE || b.bouncetype == BNC_MOLOTOV) && b.owner == d && b.id == id && !b.local)
            {
                vec pos = vec(b.offset).mul(b.offsetmillis/float(OFFSETMILLIS)).add(b.o);
                explode(b.local, b.owner, pos, vec(0,0,0), NULL, 0, atk);
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

            if((p.lifetime -= time)<0 && (p.atk==ATK_FIREWORKS || p.atk==ATK_S_NUKE || p.atk==ATK_KAMIKAZE || p.atk==ATK_POWERARMOR))
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

                if(!p.inwater && (lookupmaterial(p.o) & MATF_VOLUME) == MAT_WATER)
                {
                    p.inwater = true;
                    vec effectPos = p.o;
                    effectPos.addz(2.5f);
                    particles::dirSplash(PART_WATER, 0x40403A, 150, 10, 75, effectPos, vec(0, 0, 1), 2.f, 300, 15, hasShrooms());
                    particles::dirSplash(PART_WATER, 0x50503A, 500, 5, 150, effectPos, vec(0, 0, 1), 3.f, 150, 15, hasShrooms());
                    playSound(S_IMPACTWATER, effectPos, 250, 50, SND_LOWPRIORITY|SND_NOOCCLUSION);
                }

                if(p.atk!=ATK_SMAW && p.atk!=ATK_FIREWORKS && p.atk!=ATK_S_ROCKETS && p.atk!=ATK_S_NUKE)
                {
                    renderProjectilesTrails(p.owner, pos, dv, p.from, p.offset, p.atk, p.exploded);
                }

                if(p.projsound && !game::ispaused()) // play and update the sound only if the projectile is passing by
                {
                    bool bigRadius = (p.atk==ATK_S_NUKE || p.atk==ATK_FIREWORKS);

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
                if(p.atk != ATK_CROSSBOW) projs.remove(i--);
                else if((p.lifetime -= time)<0 || removearrow) projs.remove(i--);
            }
            else p.o = v;
        }
    }

    bool noMuzzle(int atk, gameent *d)
    {
        return (d->character==C_SPY && d->abilitymillis[ABILITY_2]) || (atk==ATK_M_BUSTER || atk==ATK_M_FLAIL || atk==ATK_M_HAMMER || atk==ATK_M_MASTER || atk==ATK_NINJA);
    }

    float recoilReduce()
    {
        float factor = (hudplayer()->boostmillis[B_ROIDS] ? 0.75f : 1.f);

        switch(hudplayer()->character)
        {
            case C_SOLDIER:
                return factor *= 2;

            case C_VIKING:
                return (hudplayer()->boostmillis[B_RAGE] ? (factor * 0.75f) : factor);

            case C_WIZARD:
                return (hudplayer()->abilitymillis[ABILITY_2] ? (factor * 5) : factor);
        }

        return factor * (classes[hudplayer()->character].accuracy / 100.f);
    }

    float recoilSide(int time)
    {
        static float phaseShift = 0; // Random phase shift - changes less frequently to maintain a sinusoidal pattern
        static int lastUpdate = 0;
        if(totalmillis - lastUpdate > time * 2) // Update phase shift every second, for example
        {
            phaseShift = (rand() % 360) * (M_PI / 180.0); // Convert degrees to radians
            lastUpdate = totalmillis;
        }

        return (1.0f + (rand() % 20 - 10) / 100.0f) * sin((totalmillis * (2 * M_PI / static_cast<float>(time))) + phaseShift); // Slight random amplitude modulation (±10% variation)
    }

    void shoteffects(int atk, const vec &from, const vec &to, gameent *d, bool local, int id, int prevaction, bool isMonster)     // create visual effect from a shot
    {
        int gun = attacks[atk].gun;
        int gunSound = attacks[atk].sound;
        bool isHudPlayer = d==hudplayer();
        vec muzzleOrigin = noMuzzle(atk, d) ? hudgunorigin(d->gunselect, d->o, to, d) : d->muzzle;
        vec casingOrigin = (d->character==C_SPY && d->abilitymillis[ABILITY_2]) ? d->o : d->balles;
        bool wizardAbility = (d->character==C_WIZARD && d->abilitymillis[ABILITY_2]);
        if(wizardAbility) playSound(S_WIZ_2, isHudPlayer ? vec(0, 0, 0) : d->muzzle, 250, 150, NULL, d->entityId);

        switch(atk)
        {
            case ATK_PLASMA:
            case ATK_SPOCKGUN:
            {
                bool isPlasma = (atk == ATK_PLASMA);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                renderMuzzleEffects(from, to, d, atk);
                if(isHudPlayer)
                {
                    float deviationAmount = (isPlasma ? ((0.1f / recoilReduce()) * recoilSide(500)) : 0.f);
                    startCameraAnimation(CAM_ANIM_SHOOT, isPlasma ? attacks[atk].attackdelay * 2.f : attacks[atk].attackdelay, vec(0, 0, 0), vec(0, 0, 0), vec(deviationAmount / recoilReduce(), (isPlasma ? 0.4f : 0.3f) / recoilReduce(), 0), vec(0, 25, 0));
                }
                if(d->type == ENT_PLAYER && isPlasma) gunSound = S_PLASMARIFLE_SFX;
                break;
            }

            case ATK_ELECTRIC:
                renderMuzzleEffects(from, to, d, atk);
                renderInstantImpact(from, to, muzzleOrigin, atk, hasRoids(d));
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / 2.f, vec(0, 0, 0), vec(0, 0, 0), vec((0.1f * recoilSide(300)) / recoilReduce(), 0.1f / recoilReduce(), 0));
                else soundNearmiss(S_FLYBYELEC, from, to);
                playSound(S_IMPACTELEC, to, 250, 50, SND_LOWPRIORITY);
                break;

            case ATK_SMAW:
            case ATK_S_ROCKETS:
            case ATK_S_NUKE:
            case ATK_FIREWORKS:
            {
                bool isNuke = (atk==ATK_S_NUKE);
                bool isRockets = (atk==ATK_S_ROCKETS);
                renderMuzzleEffects(from, to, d, atk);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(isHudPlayer)
                {
                    float recoilAmount = (isNuke ? 3.5f : (atk==ATK_FIREWORKS || isRockets ? 0.8f : 0.4f)) / recoilReduce();
                    startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / (isNuke ? 3.5f : (isRockets ? 1.25f : 3.5f)), vec(0, 0, 0), vec(0, 0, 0), vec(0, recoilAmount, 0));
                    if(d==player1 && atk==ATK_S_NUKE)
                    {
                        unlockAchievement(ACH_ATOME);
                        updateStat(1, STAT_ATOM);
                    }
                }
                break;
            }

            case ATK_MINIGUN:
            case ATK_AK47:
            case ATK_S_GAU8:
            {
                bool isGau = (atk == ATK_S_GAU8);
                renderMuzzleEffects(from, to, d, atk);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(isHudPlayer)
                {
                    float recoilAccel = (hudplayer()->gunaccel ? (recoilReduce() * hudplayer()->gunaccel) : 1.f);
                    float recoilAmount = (isGau ? 0.3f : 0.4f) / recoilAccel;
                    float deviationAmount = ((isGau ? 0.5f : 0.3f) / recoilAccel) * recoilSide(isGau ? 500 : 1250);
                    startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay * (isGau ? 10 : 3), vec(0, 0, 0), vec(0, 0, 0), vec(deviationAmount, recoilAmount, 0), vec(0, atk==ATK_AK47 ? 35 : 45, 0));
                }
                else soundNearmiss(isGau ? S_BIGBULLETFLYBY : S_BULLETFLYBY, from, to);

                if(isGau)
                {
                    if(d->type==ENT_PLAYER) gunSound = S_GAU8;
                    if(d==player1 && player1->character==C_PRIEST && player1->boostmillis[B_SHROOMS] && player1->abilitymillis[ABILITY_3]) unlockAchievement(ACH_CADENCE);
                }
                spawnbouncer(casingOrigin, d->vel, d, isGau ? BNC_BIGCASING : BNC_CASING);
                break;
            }
            case ATK_MOSSBERG:
            case ATK_HYDRA:
            {
                bool isHydra = (atk==ATK_HYDRA);
                renderMuzzleEffects(from, to, d, atk);
                if(!local) createrays(gun, from, to, d);
                loopi(isHydra ? 3 : 2) spawnbouncer(casingOrigin, d->vel, d, BNC_CARTRIDGE);
                if(isHudPlayer)
                {
                    float recoilAmount = (isHydra ? 0.6f : 1.5f) / recoilReduce();
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

            case ATK_SV98:
            case ATK_SKS:
            case ATK_S_CAMPER:
            {
                bool isSv98 = (atk==ATK_SV98);
                renderMuzzleEffects(from, to, d, atk);
                if(isHudPlayer)
                {
                    float recoilAmount = (isSv98 ? 1.5f : 0.5f) / recoilReduce();
                    startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / (isSv98 ? 2.7f : 1.5f), vec(0, 0, 0), vec(0, 0, 0), vec(0, recoilAmount, 0));
                }
                if(atk==ATK_S_CAMPER)
                {
                    loopi(attacks[atk].rays)
                    {
                        float originOffset = 0.4f - (rnd(9) / 10.f);
                        particle_flare(muzzleOrigin.add(vec(originOffset, originOffset, originOffset)), rays[i], 100, PART_F_SHOTGUN, hasRoids(d) ? 0xFF2222 : 0xFFFF22, 0.4f, d, hasShrooms());
                        particles::trail(PART_SMOKE, 800, hudgunorigin(gun, from, to, d), rays[i], 0x999999, 0.6f, 20);
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

            case ATK_CROSSBOW:
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d->boostmillis[B_RAGE]) particle_splash(PART_SPARK,  8, 500, d->muzzle, 0xFF2222, 1.0f,  50, 200, 0, hasShrooms());
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / 1.5f, vec(0, 0, 0), vec(0, 0, 0), vec(0, 0.2f / recoilReduce(), 0));
                else soundNearmiss(S_FLYBYARROW, from, to);
                break;

            case ATK_UZI:
            case ATK_FAMAS:
            case ATK_GLOCK:
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                renderMuzzleEffects(from, to, d, atk);
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay * 3, vec(0, 0, 0), vec(0, 0, 0), vec(0, (atk==ATK_GLOCK ? 0.2f : 0.5f) / recoilReduce(), 0), vec(0, 30, 0));
                else soundNearmiss(S_BULLETFLYBY, from, to);
                spawnbouncer(casingOrigin, d->vel, d, BNC_CASING);
                break;

            case ATK_FLAMETHROWER:
            {
                if(!local) createrays(gun, from, to, d);
                renderMuzzleEffects(from, to, d, atk);
                loopi(attacks[atk].rays)
                {
                    vec dest = vec(rays[i]).sub(muzzleOrigin).normalize().mul(1450.0f + rnd(200));
                    if(rnd(2)) particle_flying_flare(muzzleOrigin, dest, 900, PART_AR, 0xEEEEEE, 15.f, 100, 50);
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
                if(!rnd(2)) newbouncer(muzzleOrigin, to, local, id, d, BNC_LIGHT, 650, 400);
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay * 1.5f, vec(0, 0, 0), vec(0, 0, 0), vec((0.15f * recoilSide(300)) / recoilReduce(), 0, 0));
                gunSound = (d->type==ENT_AI ? S_PYRO_A : S_FLAMETHROWER);
                break;
            }
            case ATK_GRAP1:
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                renderMuzzleEffects(from, to, d, atk);
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / 1.5f, vec(0, 0, 0), vec(0, 0, 0), vec((0.1f * recoilSide(1500)) / recoilReduce(), 0.15f / recoilReduce(), 0), vec(0, 12, 0));
                break;

            case ATK_M32:
            {
                particle_splash(PART_SMOKE, 10, 600, d->muzzle, wizardAbility ? 0x550044 : 0x444444, 4.0f, 20, 500, 0, hasShrooms());
                if(d->boostmillis[B_RAGE]) particle_splash(PART_SPARK,  8, 500, d->muzzle, 0xFF4444, 1.0f, 50, 200, 0, hasShrooms());
                float dist = from.dist(to); vec up = to; up.z += dist/8;
                newbouncer(isHudPlayer && !thirdperson ? d->muzzle : hudgunorigin(gun, d->o, to, d), up, local, id, d, BNC_GRENADE, attacks[atk].ttl, attacks[atk].projspeed);
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / 3, vec(0, 0, 0), vec(0, 0, 0), vec(0, 0.5f / recoilReduce(), 0));
                break;
            }
            case ATK_MOLOTOV:
            {
                if(d->boostmillis[B_RAGE]) particle_splash(PART_SPARK,  8, 500, d->muzzle, 0xFF4444, 1.0f, 50, 200, 0, hasShrooms());
                float dist = from.dist(to); vec up = to; up.z += dist/6;
                newbouncer(isHudPlayer && !thirdperson ? d->muzzle : hudgunorigin(gun, d->o, to, d), up, local, id, d, BNC_MOLOTOV, attacks[atk].ttl, attacks[atk].projspeed);
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / 3, vec(0, 0, 0), vec(0, 0, 0), vec(0, 0.5f / recoilReduce(), 0));
                break;
            }
            case ATK_KAMIKAZE:
            case ATK_POWERARMOR:
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                break;
        }

        if(!rnd(attacks[atk].specialsounddelay))
        {
            if(d->boostmillis[B_ROIDS])
            {
                playSound(S_ROIDS_SHOOT, isHudPlayer ? vec(0, 0, 0) : d->o, 500, 100, NULL, d->entityId);
                if(camera1->o.dist(hudgunorigin(gun, d->o, to, d)) >= 400 && d!=player1) playSound(S_ROIDS_SHOOT_FAR, d->o, 1000, 500, NULL, d->entityId);
            }
            else if(d->boostmillis[B_RAGE]) playSound(S_RAGETIR, isHudPlayer ? vec(0, 0, 0) : d->o, 500, 100);
        }

        if(d->abilitymillis[ABILITY_3] && d->character==C_PRIEST) adddynlight(muzzleOrigin, 6, vec(1.5f, 1.5f, 0.0f), 80, 40, L_NOSHADOW|L_VOLUMETRIC|DL_FLASH);
        if(d==player1) updateStat(1, STAT_MUNSHOOTED);

        bool incraseDist = atk==ATK_POWERARMOR || atk==ATK_KAMIKAZE || atk==ATK_S_GAU8 || atk==ATK_S_NUKE;
        int distance = camera1->o.dist(hudgunorigin(gun, d->o, to, d));
        int loopedSoundFlags = SND_LOOPED|SND_FIXEDPITCH|SND_NOCULL;
        float pitch = d->boostmillis[B_SHROOMS] ? (d->character==C_JUNKIE ? 1.4f : 1.2f) : (d->character==C_PRIEST && d->abilitymillis[ABILITY_3] ? (1.5f - (d->abilitymillis[ABILITY_3] / 8000.0f)) : 0);
        if(gamespeed!=100) pitch *= (game::gamespeed / 100.f);

        if(!d->attacksound)
        {
            switch(gunSound)
            {
                case S_FLAMETHROWER:
                case S_GAU8:
                    playSound(gunSound, isHudPlayer ? vec(0, 0, 0) : d->o, incraseDist ? 600 : 500, incraseDist ? 350 : 200, loopedSoundFlags, d->entityId, PL_ATTACK, pitch);
                    d->attacksound = 1;
                    if(!isHudPlayer && distance > 300)
                    {
                        playSound(attacks[atk].middistsnd, d->o, incraseDist ? 3200 : 600, incraseDist ? 1600 : 400, loopedSoundFlags, d->entityId, PL_ATTACK_FAR, pitch);
                    }
                    return;

                case S_PLASMARIFLE_SFX:
                    playSound(gunSound, isHudPlayer ? vec(0, 0, 0) : d->o, 250, 150, loopedSoundFlags, d->entityId, PL_ATTACK, pitch);
                    d->attacksound = 1;
                    break;
            }
        }
        else if(gunSound != S_PLASMARIFLE_SFX) return;

        playSound(attacks[atk].sound, isHudPlayer ? vec(0, 0, 0) : d->o, incraseDist ? 600 : 500, incraseDist ? 350 : 200, NULL, (isMonster ? -1 : d->entityId), NULL, pitch);

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

    void shoot(gameent *d, const vec &targ, bool isMonster)
    {
        int prevaction = d->lastaction, attacktime = lastmillis-prevaction;
        bool specialAbility = d->character==C_PRIEST && d->abilitymillis[ABILITY_3];

        if(d->aitype==AI_BOT && (d->gunselect==GUN_GLOCK || d->gunselect==GUN_SPOCKGUN || d->gunselect==GUN_HYDRA || d->gunselect==GUN_SKS || d->gunselect==GUN_S_CAMPER))
        {
            switch(rnd(d->gunselect==GUN_GLOCK || d->gunselect==GUN_SPOCKGUN || d->gunselect==GUN_HYDRA ? 5 : 15)) {case 0: d->gunwait+=(specialAbility ? 500 : 1200) / curfps; return; }
        }

        switch(d->gunselect)
        {
            case GUN_MINIGUN:
            case GUN_PLASMA:
            case GUN_S_ROCKETS:
                if(!d->attacking) d->gunselect==GUN_PLASMA ? d->gunaccel=4 : d->gunselect==GUN_S_ROCKETS ? d->gunaccel=3 : d->gunaccel=12;
                break;
            default: d->gunaccel=0;
        }

        if(attacktime < d->gunwait + d->gunaccel*(d->gunselect==GUN_PLASMA ? 50 : d->gunselect==GUN_S_ROCKETS ? 150 : 8) + (d==player1 || specialAbility ? 0 : attacks[d->gunselect].attackdelay)) return;
        d->gunwait = 0;

        if(d->character==C_KAMIKAZE)
        {
            if(d->abilitymillis[ABILITY_2]>0 && d->abilitymillis[ABILITY_2]<2000 && d->ammo[GUN_KAMIKAZE]>0 && !d->powerarmorexploded)
            {
                gunselect(GUN_KAMIKAZE, d);
                d->attacking = ACT_SHOOT;
                d->powerarmorexploded = true;
            }
        }

        if(d->armourtype==A_POWERARMOR && !d->armour && !d->powerarmorexploded && d->ammo[GUN_POWERARMOR])
        {
            d->wasAttacking = d->attacking;
            gunselect(GUN_POWERARMOR, d, true);
            d->attacking = ACT_SHOOT;
            d->powerarmorexploded = true;
        }

        if(!d->attacking) return;
        int gun = d->gunselect, act = d->attacking, atk = guns[gun].attacks[act];

        if(d->gunaccel) d->gunaccel -= 1;
        d->lastaction = lastmillis;
        d->lastattack = atk;

        if(d==player1)
        {
            lastshoot = totalmillis;
            if(atk==ATK_SMAW || atk==ATK_S_NUKE || atk==ATK_M_HAMMER || atk == ATK_MOSSBERG || atk == ATK_SV98) lastshoot+=750;
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

        int kickfactor = (m_tutorial && !canMove) || d->character==C_AMERICAN ? 0 : (d->crouched() ? -1.25f : -2.5f);
        vec kickback = (d->character==C_AMERICAN ? vec(0, 0, 0) : vec(dir).mul(attacks[atk].kickamount*kickfactor));
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

        shoteffects(atk, from, to, d, true, 0, prevaction, isMonster);

        if(d==player1 || d->ai || d->type==ENT_AI)
        {
            addmsg(N_SHOOT, "rci2i6iv", d, lastmillis-maptime, atk,
                   (int)(from.x*DMF), (int)(from.y*DMF), (int)(from.z*DMF),
                   (int)(to.x*DMF), (int)(to.y*DMF), (int)(to.z*DMF),
                   hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
        }
        float waitfactor = 1;
        if(d->character==C_PRIEST && d->abilitymillis[ABILITY_3]) waitfactor = 2.5f + ((4000.f - d->abilitymillis[ABILITY_3])/1000.f);
        if(d->boostmillis[B_SHROOMS]) waitfactor *= d->character==C_JUNKIE ? 1.75f : 1.5f;
        d->gunwait = attacks[atk].attackdelay/waitfactor;
        d->totalshots += (attacks[atk].damage*attacks[atk].rays) * (d->boostmillis[B_ROIDS] ? 1 : 2);
        if(d->powerarmorexploded) {d->attacking = d->wasAttacking; execute("getoldweap"); d->powerarmorexploded = false;}
        if((atk==ATK_GLOCK || atk==ATK_SPOCKGUN || atk==ATK_HYDRA || d->gunselect==GUN_SKS || d->gunselect==GUN_S_CAMPER) && !specialAbility) d->attacking = ACT_IDLE;
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
                case ATK_PLASMA: adddynlight(pos, 30, vec(1.00f, 0.75f, 0.0f)); break;
                case ATK_SPOCKGUN: adddynlight(pos, 30, vec(0.00f, 1.00f, 0.0f)); break;
                case ATK_GRAP1: adddynlight(pos, 50, vec(0.3f, 0.00f, 0.2f)); break;
                case ATK_FIREWORKS:
                case ATK_SMAW:
                case ATK_S_ROCKETS: adddynlight(pos, 50+lightradiusvar, vec(1.2f, 0.75f, 0.0f)); break;
                case ATK_S_NUKE: adddynlight(pos, 100, vec(1.2f, 0.75f, 0.0f)); break;
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

            bool inWater = ((lookupmaterial(pos) & MATF_VOLUME) == MAT_WATER);

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
                        particle_splash(PART_SMOKE, 1, 150, pos, 0x404040, 2.5f, 50, -20, 0, hasShrooms());
                        break;

                    case BNC_BIGROCK:
                        particle_splash(PART_SMOKE, 1, 500, pos, 0x151515, 8.f, 50, -20, 0, hasShrooms());
                        break;

                    case BNC_GRENADE:
                    {
                        float growth = (1000 - (bnc.lifetime - curtime))/150.f;
                        particle_fireball(pos, growth, PART_EXPLOSION, 20, hasRoids(bnc.owner) ? 0xFF0000 : 0x0055FF, growth, hasShrooms());
                        particle_splash(PART_SMOKE, 1, 150, pos, 0x404088, 2.5f, 50, -20, 0, hasShrooms());
                        break;
                    }
                    case BNC_SCRAP:
                        particle_splash(inWater ? PART_BUBBLE : PART_SMOKE, inWater ? 1 : 3, 250, pos, 0x222222, 2.5f, 50, -50, 0, hasShrooms());
                        particle_splash(PART_FIRE_BALL, 2, 75, pos, 0x994400, 0.7f, 30, -30, 0, hasShrooms());
                        break;

                    case BNC_GLASS:
                        particle_splash(PART_SMOKE, 1, 1200, pos, 0x303030, 2.5f, 50, -50, 10, hasShrooms());
                        particle_splash(PART_FIRE_BALL, 1, 250, pos, (hasRoids(bnc.owner) ? 0xFF0000 : 0x996600), 1.3f, 50, -50, 12, hasShrooms());
                        if(rnd(2)) particle_splash(PART_AR, 1, 500, pos, 0xFFFFFF, 12.f, 50, -25, 50);
                        break;

                    case BNC_MOLOTOV:
                        particle_splash(PART_SMOKE, 3, 180, pos, 0x6A6A6A, 2, 40, 50, 0, hasShrooms());
                        break;

                    case BNC_BURNINGDEBRIS:
                        int flamesColor = (rnd(2) ? 0x993A00 : 0x856611);
                        if(!rnd(2)) particle_splash(PART_SMOKE, 1, 1800 + rnd(400), pos, 0x282828, 2.f, 50, -100, 12, hasShrooms());
                        particle_splash(PART_FIRE_BALL, 1, 175, pos, flamesColor, 1.f, 20, 0, 4, hasShrooms());
                        particle_splash(PART_FIRE_BALL, 1, 175, bnc.o, flamesColor, 1.f, 20, 0, 4, hasShrooms());
                        break;
                }
            }
        }
    }

    std::map<int, std::string> projsPaths =
    {
        {ATK_SMAW, "projectiles/missile"},
        {ATK_FIREWORKS, "projectiles/feuartifice"},
        {ATK_CROSSBOW, "projectiles/fleche"},
        {ATK_S_ROCKETS, "projectiles/minimissile"},
        {ATK_S_NUKE, "projectiles/missilenuke"}
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

            if(p.atk==ATK_SMAW || p.atk==ATK_FIREWORKS || p.atk==ATK_CROSSBOW || p.atk==ATK_S_ROCKETS || p.atk==ATK_S_NUKE)
            {

                float dist = min(p.o.dist(p.to)/32.0f, 1.0f);
                vec pos = vec(p.o).add(vec(p.offset).mul(dist*p.offsetmillis/float(OFFSETMILLIS))),
                    v = dist < 1e-6f ? p.dir : vec(p.to).sub(pos).normalize();

                vectoyawpitch(v, yaw, pitch);
                v.mul(3);
                v.add(pos);

                rendermodel(projsPaths[p.atk].c_str(), ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, MDL_NOSHADOW|MDL_CULL_VFC|MDL_CULL_OCCLUDED);
                if(p.atk != ATK_CROSSBOW) renderProjectilesTrails(p.owner, pos, v, p.from, p.offset, p.atk, p.exploded);
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
