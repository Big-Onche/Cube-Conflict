// weapon.cpp: all shooting and effects code, projectile management
#include "game.h"
#include "engine.h"
#include "gfx.h"
#include "stats.h"
#include <string>

namespace game
{
    vec rays[MAXRAYS];
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
        if((gun==GUN_POWERARMOR && !force) || !validgun(gun) || gun==d->gunselect) return;

        if(d==player1 && gun != GUN_POWERARMOR && !shortcut) player1->pickups.lastWeapon = gun;
        addmsg(N_GUNSELECT, "rci", d, gun);
        playSound(attacks[gun-GUN_ELECTRIC].picksound, d==hudplayer() ? vec(0, 0, 0) : d->o, 200, 50, NULL, d->entityId);
        d->action.lastGun = lastmillis;
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
        if(player1->gameplay.classId==C_NINJA) gunselect(GUN_NINJA, player1, false, true);
        else if (m_identique) return;
        else findSpecialWeapon(player1, GUN_M_BUSTER, NUMMELEEWEAPONS, [](int gunId) { gunselect(gunId, player1, false, true); });
        doaction(ACT_SHOOT);
        execute("sleep 500 [shoot ; getoldweap]");
    );

    ICOMMAND(getoldweap, "", (), { if(isconnected() || !m_identique) gunselect(player1->pickups.lastWeapon, player1); });

    void nextweapon(int dir, bool force = false)
    {
        if(player1->state!=CS_ALIVE) return;

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

        if(d->boostmillis[B_SHROOMS]) spread /= d->gameplay.classId==C_JUNKIE ? 1.75f : 1.5f;
        if(d->gameplay.classId==C_WIZARD && d->abilitymillis[ABILITY_2]) spread /= 3.f;

        if(!spreadLimit(d))
        {
            if(d->crouching) spread /= d->gameplay.classId==C_CAMPER ? 2.5f : 1.3f;
            if(d->hasRoids() || d->boostmillis[B_RAGE]) spread *= 1.75f;
        }

        spread = (spread*100) / classes[d->gameplay.classId].accuracy;
        offset.mul((to.dist(from)/1024) * spread);

        offset.z /= 2;
        dest = vec(offset).add(to);
        if(dest != from)
        {
            vec dir = vec(dest).sub(from).normalize();
            raycubepos(from, dir, dest, range, RAY_CLIPMAT|RAY_ALPHAPOLY);
        }
    }

    static inline float getAimProgress(gameent *d)
    {
        if(!d) return 0.0f;
        if(d != player1 || d != hudplayer()) return d->aiming ? 1.0f : 0.0f;
        if(!validgun(d->gunselect)) return d->aiming ? 1.0f : 0.0f;

        float zoomRange = fabsf(float(fov - guns[d->gunselect].maxzoomfov));
        if(zoomRange <= 0.0f) return d->aiming ? 1.0f : 0.0f;

        float zoomOffset = fabsf(float(fov) - curfov);
        return clamp(zoomOffset / zoomRange, 0.0f, 1.0f);
    }

    void updateAiming()
    {
        if(!player1) return;

        if(player1 != hudplayer())
        {
            player1->aiming = false;
            return;
        }

        player1->aiming = getAimProgress(player1) > 0.0f;
    }

    float spread(int atk, gameent *d)
    {
        float aimProgress = getAimProgress(d);
        return lerp((float)attacks[atk].noaimspread, (float)attacks[atk].aimspread, aimProgress);
    }

    void createrays(int atk, const vec &from, const vec &to, gameent *d)             // create random spread of rays
    {
        loopi(attacks[atk].rays) offsetray(from, to, spread(atk, d), attacks[atk].range, rays[i], d);
    }

    VARP(blood, 0, 1, 1);
    void damageeffect(int damage, gameent *d, gameent *actor, int atk, bool isAfterburnHit)
    {
        vec p = d->o;
        p.z += 0.6f*(d->eyeheight + d->aboveeye) - d->eyeheight;

        int actorClass = actor->gameplay.classId, targetClass = d->gameplay.classId;
        bool isHudPlayer = (actor == hudplayer());
        bool teamDamage = (isteam(d->gameplay.team, actor->gameplay.team) && d!=actor);

        if(isHudPlayer) d->curdamagecolor = 0xFFAA00;

        const int calcflags = DCF_APPLY_ACTOR_SCALE |
                              DCF_APPLY_TARGET_RESIST |
                              DCF_APPLY_ACTOR_MODIFIERS |
                              DCF_APPLY_TARGET_MODIFIERS |
                              DCF_APPLY_ACTOR_BOOSTS |
                              DCF_APPLY_TARGET_BOOSTS;
        float hitdistance = actor->o.dist(d->o);
        totalDamage calc = getDamage(damage, atk, actorClass, actor, targetClass, d, hitdistance, calcflags, actor==d);
        damage = calc.damage;

        switch(actorClass)
        {
            case C_AMERICAN:
                if(isSuperWeapon(atk) && isHudPlayer) d->curdamagecolor = 0xFF0000;
                break;

            case C_NINJA:
                if(atk == ATK_NINJA && isHudPlayer) d->curdamagecolor = 0xFF0000;
                break;

            case C_WIZARD:
                if(actor->abilitymillis[ABILITY_2] && isHudPlayer) d->curdamagecolor = 0xFF00FF;
                break;

            case C_VIKING:
                if(actor->boostmillis[B_RAGE] && isHudPlayer) d->curdamagecolor = 0xFF7700;
                break;

            case C_SHOSHONE:
                if(actor->abilitymillis[ABILITY_3] && isHudPlayer) d->curdamagecolor = 0xFF7700;
                if(targetClass == C_AMERICAN && isHudPlayer) d->curdamagecolor = 0xFFFF00;
                break;

            case C_PHYSICIST:
                if(d==player1 && actor==player1 && player1->armour && player1->abilitymillis[ABILITY_1]) unlockAchievement(ACH_BRICOLEUR);
        }

        switch(targetClass)
        {
            case C_WIZARD:
                if(d->abilitymillis[ABILITY_3]) d->curdamagecolor = 0x8855AA;
                break;

            case C_PRIEST:
                if(isHudPlayer && d->abilitymillis[ABILITY_2] && d->mana) d->curdamagecolor = 0xAA00AA;
                break;

            case C_SHOSHONE:
                if(actorClass == C_AMERICAN && isHudPlayer) d->curdamagecolor = 0xFF7700;
                break;
        }

        if(actor->hasRoids() && isHudPlayer) d->curdamagecolor = 0xFF0000;
        if(d->boostmillis[B_JOINT] && isHudPlayer) d->curdamagecolor = 0xAAAA55;

        if(teamDamage && isHudPlayer) // recalc if ally or not
        {
            if(actorClass == C_MEDIC) return;
            damage /= (actorClass == C_JUNKIE ? 1.5f : 3.f);
            d->curdamagecolor = 0x888888;
        }

        if(d->armour <= 0 || d->armourtype!=A_MAGNET)
        {
            if(blood) particle_splash(PART_BLOOD, damage > 300 ? 3 : damage/100, 1000, p, 0x60FFFF, 2.96f);
            gibeffect(damage, vec(0,0,0), d);
        }

        damage *= 0.1f; // rescale damage value
        if(damage <= 0) return;
        recordHitlogDamage(actor, d, atk, damage, isAfterburnHit, hitdistance);

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
            if(d->armourtype != A_POWERARMOR) bouncers::spawn(d->o, d->vel, d, BNC_PIXEL, 100 + rnd(50));
            else if(d->armour) bouncers::spawn(d->o, d->vel, d, BNC_SCRAP, 100 + rnd(50));
        }
    }

    int avgdmg[4];

    void hit(int damage, dynent *d, gameent *at, const vec &vel, int atk, float info1, int info2)
    {
        if(at==player1 && d!=at)
        {
            extern int hitsound;
            if(hitsound && lasthit != lastmillis) playSound(S_HIT, vec(0, 0, 0), 0, 0, SND_UI);
            lasthit = lastmillis;
        }

        gameent *f = (gameent *)d;

        f->spawn.lastPain = lastmillis;
        if(at->type==ENT_PLAYER && !isteam(at->gameplay.team, f->gameplay.team)) at->stats.damage += damage;
        if(m_dmsp || m_classicsp || m_tutorial)
        {
            if(f==player1)
            {
                const int calcflags = DCF_APPLY_TARGET_RESIST |
                                      DCF_APPLY_TARGET_MODIFIERS |
                                      DCF_APPLY_TARGET_BOOSTS |
                                      DCF_MUTATE_TARGET_STATE;
                totalDamage calc = getDamage(damage, atk, at->gameplay.classId, at, f->gameplay.classId, f, at->o.dist(f->o), calcflags, at==f);
                damage = calc.damage/(m_dmsp ? 5 : 2);
                doLocalAfterburn(f, at, atk);
                damageeffect(damage, f, at, atk);
                damaged(damage, f, at, true, atk);
                f->applyHitPush(damage, vel, at, atk);
            }
            else if(at==player1)
            {
                const int calcflags = DCF_APPLY_ACTOR_MODIFIERS | DCF_APPLY_ACTOR_BOOSTS;
                totalDamage calc = getDamage(damage, atk, at->gameplay.classId, at, f->gameplay.classId, f, at->o.dist(f->o), calcflags, at==f);
                int rawdamage = calc.damage;
                if(player1->gameplay.classId==C_VAMPIRE)
                {
                    player1->health = min(player1->health + rawdamage/2, player1->maxhealth);
                    player1->vampiremillis += rawdamage*1.5f;
                }
                damage = (rawdamage*classes[player1->gameplay.classId].damage)/100;
                hitmonster(damage, (monster *)f, at, vel, atk);
                avgdmg[dmgsecs[0]] += damage/10.f;
            }
        }

        if(!m_mp(gamemode) || f==at) f->applyHitPush(damage, vel, at, atk);
        if(!m_mp(gamemode)) damaged(damage, f, at, false, atk);
        else
        {
            hitmsg &h = hits.add();
            h.target = f->net.clientNum;
            h.lifesequence = f->spawn.lifeSequence;
            h.info1 = int(info1*DMF);
            h.info2 = info2;
            h.dir = f==at ? ivec(0, 0, 0) : ivec(vec(vel).mul(DNF));

            int impactSound = S_IMPACTWOOD + f->armourtype;
            int regenSound = S_PHY_1_WOOD + f->armourtype;
            bool hasRegenAbility = f->gameplay.classId==C_PHYSICIST && f->abilitymillis[ABILITY_1];

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

    void radialeffect(dynent *o, const vec &v, const vec &vel, int qdam, gameent *at, int atk)
    {
        if(o->state!=CS_ALIVE) return;
        vec dir;
        float dist = projectiles::distance(o, dir, v, vel);
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
                    bouncers::spawn(pos, vec(0, 0, 0), owner, BNC_ROCK, 200);
                    bouncers::spawn(pos, vec(0, 0, 0), owner, BNC_BURNINGDEBRIS, 250, 400);
                }
                renderExplosion(owner, safeLoc, vel, atk);
                playSound(S_EXPL_MISSILE, safeLoc, 400, 150);
                if(inWater(v)) playSound(S_EXPL_INWATER, vec(v).addz(15), 300, 100);
                if(isFar) playSound(S_EXPL_FAR, safeLoc, 1500, 400, SND_LOWPRIORITY);
                startShake(v, 1.25f * attacks[atk].exprad, atk);
                break;

            case ATK_KAMIKAZE:
            case ATK_POWERARMOR:
                loopi(10 + rnd(5))
                {
                    if(atk==ATK_POWERARMOR) bouncers::spawn(v, owner->vel, owner, BNC_SCRAP, 50 + rnd(250));
                    else bouncers::spawn(v, owner->vel, owner, rnd(2) ? BNC_PIXEL : BNC_ROCK, 100 + rnd(300));
                }
                renderExplosion(owner, owner->o, vel, atk);
                playSound(atk==ATK_KAMIKAZE ? S_EXPL_KAMIKAZE : S_EXPL_PARMOR, owner->o, 400, 150);
                if(inWater(v)) playSound(S_EXPL_INWATER, vec(owner->o).addz(15), 300, 100);
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
                if(inWater(v)) playSound(S_EXPL_INWATER, vec(v).addz(15), 300, 100);
                if(isFar) playSound(S_FIREWORKSEXPL_FAR, safeLoc, 2000, 750);
                startShake(v, attacks[atk].exprad, atk, 0.75f);
                break;

            case ATK_M32:
                loopi(5+rnd(3)) bouncers::spawn(v, vec(0, 0, 0), owner, BNC_ROCK, 200);
                renderExplosion(owner, v, vel, atk);
                playSound(S_EXPL_GRENADE, v, 400, 150);
                if(inWater(v)) playSound(S_EXPL_INWATER, vec(v).addz(15), 300, 100);
                if(isFar) playSound(S_EXPL_FAR, v, 2000, 400, SND_LOWPRIORITY);
                startShake(v, 1.5f * attacks[atk].exprad, atk);
                break;

            case ATK_MOLOTOV:
            {
                vec debrisLoc = v;
                debrisLoc.addz(3);
                loopi(6+rnd(2)) bouncers::spawn(debrisLoc, vec(0, 0, 0), owner, BNC_GLASS, 300, 350+rnd(200));
                renderExplosion(owner, v, vel, atk);
                playSound(S_EXPL_MOLOTOV, v, 300, 150);
                if(inWater(v)) playSound(S_EXPL_INWATER, vec(v).addz(15), 300, 100);
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
                if(!inWater(v)) playSound(atk==ATK_CROSSBOW ? S_IMPACTARROW : S_LITTLERICOCHET, v, 175, 75, SND_LOWPRIORITY);
                break;

            case ATK_SV98:
            case ATK_SKS:
            case ATK_S_GAU8:
                renderBulletImpact(owner, v, vel, safe, atk);
                if(!inWater(v))
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

    void explodeeffects(int atk, gameent *d, bool local, int id)
    {
        if(local) return;

        loopv(projectiles::curProjectiles)
        {
            projectiles::projectile &p = projectiles::curProjectiles[i];
            if(p.atk == atk && p.ownerId == d->entityId && p.id == id && !p.local)
            {
                vec pos = vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)).add(p.o);
                explode(p.local, d, pos, p.dir, NULL, 0, atk);
                projectiles::stain(p, pos, atk);
                removeEntityPos(p.entityId);
                projectiles::curProjectiles.remove(i);
                break;
            }
        }

        loopv(bouncers::curBouncers)
        {
            bouncers::bouncer &b = *bouncers::curBouncers[i];
            if((b.bouncetype == BNC_GRENADE || b.bouncetype == BNC_MOLOTOV) && b.ownerId == d->entityId && b.id == id && !b.local)
            {
                vec pos = vec(b.offset).mul(b.offsetmillis/float(OFFSETMILLIS)).add(b.o);
                explode(b.local, d, pos, vec(0,0,0), NULL, 0, atk);
                bouncers::curBouncers.remove(i);
                break;
            }
        }
    }

    bool noMuzzle(int atk, gameent *d)
    {
        return (d->gameplay.classId==C_SPY && d->abilitymillis[ABILITY_2]) || (atk==ATK_M_BUSTER || atk==ATK_M_FLAIL || atk==ATK_M_HAMMER || atk==ATK_M_MASTER || atk==ATK_NINJA);
    }

    float recoilReduce()
    {
        float factor = (hudplayer()->hasRoids() ? 0.75f : 1.f);

        switch(hudplayer()->gameplay.classId)
        {
            case C_SOLDIER:
                return factor *= 2;

            case C_VIKING:
                return (hudplayer()->boostmillis[B_RAGE] ? (factor * 0.75f) : factor);

            case C_WIZARD:
                return (hudplayer()->abilitymillis[ABILITY_2] ? (factor * 5) : factor);
        }

        return factor * (classes[hudplayer()->gameplay.classId].accuracy / 100.f);
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

        return (1.0f + (rand() % 20 - 10) / 100.0f) * sin((totalmillis * (2 * M_PI / static_cast<float>(time))) + phaseShift); // Slight random amplitude modulation (+/-10% variation)
    }

    void shoteffects(int atk, const vec &from, const vec &to, gameent *d, bool local, int id, int prevaction, bool isMonster)     // create visual effect from a shot
    {
        int gun = attacks[atk].gun;
        int gunSound = attacks[atk].sound;
        bool isHudPlayer = d==hudplayer();
        vec muzzleOrigin = noMuzzle(atk, d) ? hudgunorigin(d->gunselect, d->o, to, d) : d->render.muzzlePos;
        vec casingOrigin = (d->gameplay.classId==C_SPY && d->abilitymillis[ABILITY_2]) ? d->o : d->render.casingPos;
        bool wizardAbility = (d->gameplay.classId==C_WIZARD && d->abilitymillis[ABILITY_2]);
        if(wizardAbility) playSound(S_WIZ_2, isHudPlayer ? vec(0, 0, 0) : muzzleOrigin, 250, 150, NULL, d->entityId);

        switch(atk)
        {
            case ATK_PLASMA:
            case ATK_SPOCKGUN:
            {
                bool isPlasma = (atk == ATK_PLASMA);
                projectiles::add(from, to, attacks[atk].projspeed, local, id, d, atk);
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
                renderInstantImpact(from, to, muzzleOrigin, atk, d->hasRoids());
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
                projectiles::add(from, to, attacks[atk].projspeed, local, id, d, atk);
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
                projectiles::add(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(isHudPlayer)
                {
                    float recoilAccel = (hudplayer()->action.gunAcceleration ? (recoilReduce() * hudplayer()->action.gunAcceleration) : 1.f);
                    float recoilAmount = (isGau ? 0.3f : 0.4f) / recoilAccel;
                    float deviationAmount = ((isGau ? 0.5f : 0.3f) / recoilAccel) * recoilSide(isGau ? 500 : 1250);
                    startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay * (isGau ? 10 : 3), vec(0, 0, 0), vec(0, 0, 0), vec(deviationAmount, recoilAmount, 0), vec(0, atk==ATK_AK47 ? 35 : 45, 0));
                }
                else soundNearmiss(isGau ? S_BIGBULLETFLYBY : S_BULLETFLYBY, from, to);

                if(isGau)
                {
                    if(d->type==ENT_PLAYER) gunSound = S_GAU8;
                    if(d==player1 && player1->gameplay.classId==C_PRIEST && player1->boostmillis[B_SHROOMS] && player1->abilitymillis[ABILITY_3]) unlockAchievement(ACH_CADENCE);
                }
                bouncers::spawn(casingOrigin, d->vel, d, isGau ? BNC_BIGCASING : BNC_CASING);
                break;
            }
            case ATK_MOSSBERG:
            case ATK_HYDRA:
            {
                bool isHydra = (atk==ATK_HYDRA);
                bool hasRoids = d->hasRoids();
                renderMuzzleEffects(from, to, d, atk);
                if(!local) createrays(gun, from, to, d);
                loopi(isHydra ? 3 : 2) bouncers::spawn(casingOrigin, d->vel, d, BNC_CARTRIDGE);
                if(isHudPlayer)
                {
                    float recoilAmount = (isHydra ? 0.6f : 1.5f) / recoilReduce();
                    startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / (isHydra ? 0.75f : 2.f), vec(0, 0, 0), vec(0, 0, 0), vec(0, recoilAmount, 0));
                }

                loopi(attacks[atk].rays)
                {
                    float originOffset = 0.4f - (rnd(9) / 10.f);
                    projectiles::add(muzzleOrigin.add(vec(originOffset, originOffset, originOffset)), rays[i], 3000, false, id, d, atk);
                    renderInstantImpact(from, rays[i], muzzleOrigin, atk, hasRoids);
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
                bool hasRoids = d->hasRoids();
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
                        particle_flare(muzzleOrigin.add(vec(originOffset, originOffset, originOffset)), rays[i], 100, PART_F_SHOTGUN, hasRoids ? 0xFF2222 : 0xFFFF22, 0.4f, d, hasShrooms());
                        particles::trail(PART_CSMOKE, 600, hudgunorigin(gun, from, to, d), rays[i], 0x99999988, 0.6f, 20);
                        renderInstantImpact(from, rays[i], muzzleOrigin, atk, hasRoids);
                        if(!isHudPlayer) soundNearmiss(S_BIGBULLETFLYBY, from, rays[i], 512);
                        playSound(S_BIGRICOCHET, rays[i], 250, 100);
                    }
                    loopi(3) bouncers::spawn(casingOrigin, d->vel, d, BNC_BIGCASING);
                }
                else
                {
                    if(!isHudPlayer) soundNearmiss(S_BIGBULLETFLYBY, from, to);
                    projectiles::add(from, to, attacks[atk].projspeed, local, id, d, atk);
                    bouncers::spawn(casingOrigin, d->vel, d, BNC_BIGCASING);
                }
                break;
            }

            case ATK_CROSSBOW:
                projectiles::add(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d->boostmillis[B_RAGE]) particle_splash(PART_SPARK,  8, 500, muzzleOrigin, 0xFF2222, 1.0f,  50, 200, 0, hasShrooms());
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / 1.5f, vec(0, 0, 0), vec(0, 0, 0), vec(0, 0.2f / recoilReduce(), 0));
                else soundNearmiss(S_FLYBYARROW, from, to);
                break;

            case ATK_UZI:
            case ATK_FAMAS:
            case ATK_GLOCK:
                projectiles::add(from, to, attacks[atk].projspeed, local, id, d, atk);
                renderMuzzleEffects(from, to, d, atk);
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay * 3, vec(0, 0, 0), vec(0, 0, 0), vec(0, (atk==ATK_GLOCK ? 0.2f : 0.5f) / recoilReduce(), 0), vec(0, 30, 0));
                else soundNearmiss(S_BULLETFLYBY, from, to);
                bouncers::spawn(casingOrigin, d->vel, d, BNC_CASING);
                break;

            case ATK_FLAMETHROWER:
            {
                bool hasRoids = d->hasRoids();
                if(!local) createrays(gun, from, to, d);
                renderMuzzleEffects(from, to, d, atk);
                loopi(attacks[atk].rays)
                {
                    vec dest = vec(rays[i]).sub(muzzleOrigin).normalize().mul(1450.0f + rnd(200));
                    if(rnd(2)) particle_flying_flare(muzzleOrigin, dest, 900, PART_HAZE_SMALL, 50, 10.f, 100, 65);
                    switch(rnd(4))
                    {
                        case 0: particle_flying_flare(muzzleOrigin, dest, 700, PART_FIRE_BALL, hasRoids ? 0x881111 : 0x604930, 1.25f, 100, 16, hasShrooms()); break;
                        case 1: particle_flying_flare(muzzleOrigin, dest, 700, PART_FIRE_BALL, hasRoids ? 0x770000 : 0x474747, 1.25f, 100, 16, hasShrooms()); break;
                        case 2: particle_flying_flare(muzzleOrigin, dest, 700, PART_FIRE_BALL, hasRoids ? 0x991111 : 0x383838, 1.25f, 100, 16, hasShrooms()); break;
                        default:
                            particle_flying_flare(muzzleOrigin, dest, 1100, PART_SMOKE, 0x000000, 3.5f, -20, 25, hasShrooms());
                            renderInstantImpact(from, rays[i], muzzleOrigin, atk, hasRoids);
                            if(rnd(2) && !isHudPlayer) soundNearmiss(S_FLYBYFLAME, from, rays[i]);
                    }
                }
                if(!rnd(2)) bouncers::add(muzzleOrigin, to, local, id, d, BNC_LIGHT, 650, 400);
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay * 1.5f, vec(0, 0, 0), vec(0, 0, 0), vec((0.15f * recoilSide(300)) / recoilReduce(), 0, 0));
                gunSound = (d->type==ENT_AI ? S_PYRO_A : S_FLAMETHROWER);
                break;
            }
            case ATK_GRAP1:
                projectiles::add(from, to, attacks[atk].projspeed, local, id, d, atk);
                renderMuzzleEffects(from, to, d, atk);
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / 1.5f, vec(0, 0, 0), vec(0, 0, 0), vec((0.1f * recoilSide(1500)) / recoilReduce(), 0.15f / recoilReduce(), 0), vec(0, 12, 0));
                break;

            case ATK_M32:
            {
                particle_splash(PART_SMOKE, 2, 600, muzzleOrigin, wizardAbility ? 0x550044 : 0x444444, 0.5f, 10, 200, 15, hasShrooms());
                if(d->boostmillis[B_RAGE]) particle_splash(PART_SPARK,  8, 500, muzzleOrigin, 0xFF4444, 1.0f, 50, 200, 0, hasShrooms());
                float dist = from.dist(to); vec up = to; up.z += dist/8;
                bouncers::add(isHudPlayer && !thirdperson ? muzzleOrigin : hudgunorigin(gun, d->o, to, d), up, local, id, d, BNC_GRENADE, attacks[atk].ttl, attacks[atk].projspeed);
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / 3, vec(0, 0, 0), vec(0, 0, 0), vec(0, 0.5f / recoilReduce(), 0));
                break;
            }
            case ATK_MOLOTOV:
            {
                if(d->boostmillis[B_RAGE]) particle_splash(PART_SPARK,  8, 500, muzzleOrigin, 0xFF4444, 1.0f, 50, 200, 0, hasShrooms());
                float dist = from.dist(to); vec up = to; up.z += dist/6;
                bouncers::add(isHudPlayer && !thirdperson ? muzzleOrigin : hudgunorigin(gun, d->o, to, d), up, local, id, d, BNC_MOLOTOV, attacks[atk].ttl, attacks[atk].projspeed);
                if(isHudPlayer) startCameraAnimation(CAM_ANIM_SHOOT, attacks[atk].attackdelay / 3, vec(0, 0, 0), vec(0, 0, 0), vec(0, 0.5f / recoilReduce(), 0));
                break;
            }
            case ATK_KAMIKAZE:
            case ATK_POWERARMOR:
                projectiles::add(from, to, attacks[atk].projspeed, local, id, d, atk);
                break;
        }

        if(!rnd(attacks[atk].specialsounddelay))
        {
            if(d->hasRoids())
            {
                playSound(S_ROIDS_SHOOT, isHudPlayer ? vec(0, 0, 0) : d->o, 500, 100, NULL, d->entityId);
                if(camera1->o.dist(hudgunorigin(gun, d->o, to, d)) >= 400 && d!=player1) playSound(S_ROIDS_SHOOT_FAR, d->o, 1000, 500, NULL, d->entityId);
            }
            else if(d->boostmillis[B_RAGE]) playSound(S_RAGETIR, isHudPlayer ? vec(0, 0, 0) : d->o, 500, 100);
        }

        if(d->abilitymillis[ABILITY_3] && d->gameplay.classId==C_PRIEST) adddynlight(muzzleOrigin, 6, vec(1.5f, 1.5f, 0.0f), 80, 40, L_NOSHADOW|L_VOLUMETRIC|DL_FLASH);
        if(d==player1) updateStat(1, STAT_MUNSHOOTED);

        bool incraseDist = atk==ATK_POWERARMOR || atk==ATK_KAMIKAZE || atk==ATK_S_GAU8 || atk==ATK_S_NUKE;
        int distance = camera1->o.dist(hudgunorigin(gun, d->o, to, d));
        int loopedSoundFlags = SND_LOOPED|SND_FIXEDPITCH|SND_NOCULL;
        float pitch = d->boostmillis[B_SHROOMS] ? (d->gameplay.classId==C_JUNKIE ? 1.4f : 1.2f) : (d->gameplay.classId==C_PRIEST && d->abilitymillis[ABILITY_3] ? (1.5f - (d->abilitymillis[ABILITY_3] / 8000.0f)) : 0);
        if(gamespeed!=100) pitch *= (game::gamespeed / 100.f);

        if(!d->sound.currentAttack)
        {
            switch(gunSound)
            {
                case S_FLAMETHROWER:
                case S_GAU8:
                    playSound(gunSound, isHudPlayer ? vec(0, 0, 0) : d->o, incraseDist ? 600 : 500, incraseDist ? 350 : 200, loopedSoundFlags, d->entityId, PL_ATTACK, pitch);
                    d->sound.currentAttack = 1;
                    if(!isHudPlayer && distance > 300)
                    {
                        playSound(attacks[atk].middistsnd, d->o, incraseDist ? 3200 : 600, incraseDist ? 1600 : 400, loopedSoundFlags, d->entityId, PL_ATTACK_FAR, pitch);
                    }
                    return;

                case S_PLASMARIFLE_SFX:
                    playSound(gunSound, isHudPlayer ? vec(0, 0, 0) : d->o, 250, 150, loopedSoundFlags, d->entityId, PL_ATTACK, pitch);
                    d->sound.currentAttack = 1;
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
        if(pl->render.muzzlePos.x < 0 || pl->action.lastAttackType < 0 || attacks[pl->action.lastAttackType].gun != pl->gunselect) return;
        float dist = o.dist(d);
        o = pl->render.muzzlePos;
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
        if(pl->render.muzzlePos.x < 0 || pl->action.lastAttackType < 0 || attacks[pl->action.lastAttackType].gun != pl->gunselect) return;
        o = pl->render.muzzlePos;
        hud = owner == followingplayer(player1) ? vec(pl->o).add(vec(0, 0, 2)) : pl->render.muzzlePos;
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

    inline bool isFastSemoAutoGun(int gun)
    {
        return (gun == GUN_GLOCK || gun == GUN_SPOCKGUN || gun == GUN_HYDRA);
    }

    inline bool isSemiAutoGun(int gun)
    {
        return (isFastSemoAutoGun(gun) || gun == GUN_SKS || gun == GUN_S_CAMPER);
    }

    bool updateWeaponsCadencies(gameent *d, int gun, int attacktime, bool specialAbility)
    {
        int weaponDelay = d->gunwait;

        if(d->aitype == AI_BOT && isSemiAutoGun(gun)) // gives a natural rate of fire with semi automatic weapons for bots
        {
            const int randomRange = isFastSemoAutoGun(gun) ? 3 : 6;
            if(!rnd(randomRange))
            {
                weaponDelay += (specialAbility ? 1250 : 2500) / curfps;
                return false;
            }
        }

        if(!d->action.attackAction || lastmillis - d->action.lastGun < 50)
        {
            switch(gun)
            {
                case GUN_MINIGUN:   d->action.gunAcceleration = 12;   break;
                case GUN_PLASMA:    d->action.gunAcceleration = 4;    break;
                case GUN_S_ROCKETS: d->action.gunAcceleration = 3;    break;
                default: d->action.gunAcceleration = 0;
            }
        }

        if(gun == GUN_PLASMA) weaponDelay += d->action.gunAcceleration * 50;
        else if (gun == GUN_S_ROCKETS) weaponDelay += d->action.gunAcceleration * 150;
        else weaponDelay += d->action.gunAcceleration * 8;

        if(d != player1 && !specialAbility) weaponDelay += attacks[d->gunselect].attackdelay;

        if(attacktime < weaponDelay) return false;

        d->gunwait = 0;
        return true;
    }

    void updateAttacks(gameent *d, const vec &targ, bool isMonster)
    {
        bool specialAbility = (d->gameplay.classId==C_PRIEST && d->abilitymillis[ABILITY_3]) || d->boostmillis[B_SHROOMS];
        int gun = d->gunselect,
            prevaction = d->action.lastAttack,
            attacktime = lastmillis - prevaction;

        if(!validgun(gun)) return;
        if(targ.x != targ.x || targ.y != targ.y || targ.z != targ.z) return;

        if(!updateWeaponsCadencies(d, gun, attacktime, specialAbility)) return;

        if(d->gameplay.classId == C_KAMIKAZE && kamikazeExploding(d))
        {
            gun = GUN_KAMIKAZE;
            d->action.attackAction = ACT_SHOOT;
        }

        if(!d->action.attackAction) return;
        int act = d->action.attackAction;
        if(act < 0 || act >= NUMACTS) return;
        int atk = guns[gun].attacks[act];
        if(!validatk(atk)) return;

        if(d->action.gunAcceleration) d->action.gunAcceleration -= 1;
        d->action.lastAttack = lastmillis;
        d->action.lastAttackType = atk;

        if(!d->ammo[gun])
        {
            if(d==player1) msgsound(S_NOAMMO, d);
            d->gunwait = 500;
            d->action.lastAttackType = -1;
            weaponswitch(d);
            return;
        }

        if(!m_muninfinie || server::noInfiniteAmmo(atk)) d->ammo[gun] -= attacks[atk].use;

        vec from = d->o, to = targ, dir = vec(to).sub(from).safenormalize();
        float dist = to.dist(from);

        int kickfactor = (m_tutorial && !canMove) || d->gameplay.classId==C_AMERICAN ? 0 : (d->crouched() ? -1.25f : -2.5f);
        vec kickback = (d->gameplay.classId==C_AMERICAN ? vec(0, 0, 0) : vec(dir).mul(attacks[atk].kickamount*kickfactor));
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
        if(d->gameplay.classId==C_PRIEST && d->abilitymillis[ABILITY_3]) waitfactor = 2.5f + ((4000.f - d->abilitymillis[ABILITY_3])/1000.f);
        if(d->boostmillis[B_SHROOMS]) waitfactor *= d->gameplay.classId==C_JUNKIE ? 1.75f : 1.5f;
        d->gunwait = attacks[atk].attackdelay/waitfactor;
        d->stats.shots += (attacks[atk].damage*attacks[atk].rays) * (d->hasRoids() ? 1 : 2);
        if(d==player1 && isSemiAutoGun(gun) && !specialAbility) d->action.attackAction = ACT_IDLE;
    }

    void removeweapons(gameent *d)
    {
        bouncers::remove(d);
        projectiles::remove(d);
    }

    void updateweapons(int curtime)
    {
        projectiles::update(curtime);
        if(player1->net.clientNum>=0 && player1->state==CS_ALIVE) updateAttacks(player1, worldpos); // only shoot when connected to server
        bouncers::update(curtime); // need to do this after the player shoots so bouncers don't end up inside player's BB next frame
        gameent *following = followingplayer();
        if(!following) following = player1;
    }
};

