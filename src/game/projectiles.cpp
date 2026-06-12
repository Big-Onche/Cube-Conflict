
#include "gfx.h"
#include <string>

using namespace game;

extern int GRAVITY;

namespace projectiles
{
    std::map<int, std::string> projsPaths =
    {
        {ATK_SMAW, "projectiles/missile"},
        {ATK_FIREWORKS, "projectiles/feuartifice"},
        {ATK_CROSSBOW, "projectiles/fleche"},
        {ATK_S_ROCKETS, "projectiles/minimissile"},
        {ATK_S_NUKE, "projectiles/missilenuke"}
    };

    void preload()
    {
        for(const auto& pair : projsPaths)
        {
            preloadmodel(pair.second.c_str());
        }
    }

    vector<projectile> curProjectiles;

    static inline gameent *ownerOf(const projectile &p)
    {
        return findEntityById(p.ownerId);
    }

    static inline bool usesBallisticPath(int atk)
    {
        return validatk(atk) && attacks[atk].projspeed > 0 && attacks[atk].projgravity > 0.0f;
    }

    static inline float projectileTravelTime(float dist, float speed)
    {
        return max(dist*1000.0f/max(speed, 1e-6f), 1.0f);
    }

    static inline float projectileTravelTime(const vec &from, const vec &to, float speed)
    {
        return projectileTravelTime(from.dist(to), speed);
    }

    static inline float ballisticTravelTime(const vec &from, const vec &to, float speed, int atk)
    {
        if(!validatk(atk)) return projectileTravelTime(from, to, speed);
        const float maxdist = attacks[atk].range > 0 ? float(attacks[atk].range) : from.dist(to);
        return projectileTravelTime(maxdist, speed);
    }

    static inline float ballisticDrop(float gravity, float gravityScale, float millis)
    {
        float secs = millis/1000.0f;
        return gravityScale*gravity*secs*secs;
    }

    static inline vec ballisticPosition(const projectile &p, float millis)
    {
        float clamped = min(millis, p.traveltime);
        float secs = clamped/1000.0f;
        vec pos = vec(p.from).add(vec(p.vel).mul(secs));
        pos.z -= ballisticDrop(p.gravity, attacks[p.atk].projgravity, clamped);
        return pos;
    }

    static inline bool ballisticStep(const projectile &p, int time, vec &v, vec &dv)
    {
        float nextelapsed = min(p.elapsed + float(time), p.traveltime);
        v = ballisticPosition(p, nextelapsed);
        dv = vec(v).sub(p.o);
        return nextelapsed >= p.traveltime;
    }

    static inline bool worldCollision(const vec &from, const vec &to, vec &hit)
    {
        vec ray = vec(to).sub(from);
        float dist = ray.magnitude();
        if(dist <= 1e-6f)
        {
            hit = to;
            return false;
        }

        ray.div(dist);
        return raycubepos(from, ray, hit, dist, RAY_CLIPMAT|RAY_ALPHAPOLY) < dist;
    }

    static inline bool isStuckArrow(const projectile &p)
    {
        return p.atk == ATK_CROSSBOW && p.exploded;
    }

    static inline void stickArrow(projectile &p, const vec &pos)
    {
        p.o = pos;
        p.from = pos;
        p.to = pos;
        p.vel = vec(0, 0, 0);
        p.gravity = 0;
        p.elapsed = 0;
        p.offset = vec(0, 0, 0);
        p.offsetmillis = 0;
    }

    void add(const vec &from, const vec &to, float speed, bool local, int id, gameent *owner, int atk)
    {
        if(from.isneg()) return;
        projectile &p = curProjectiles.add();
        vec aimdir = vec(to).sub(from).safenormalize();
        p.o = from;
        p.from = from;
        p.to = to;
        p.speed = speed;
        p.traveltime = projectileTravelTime(from, to, speed);
        p.elapsed = 0;
        p.gravity = 0;
        p.atk = atk;
        p.ballistic = usesBallisticPath(atk);
        p.dir = aimdir;
        p.vel = vec(aimdir).mul(speed);
        if(p.ballistic)
        {
            p.gravity = GRAVITY;
            // Keep the aim direction from the crosshair hit, but let ballistic shots
            // fly until a real collision, explicit lifetime, or weapon range limit.
            p.traveltime = ballisticTravelTime(from, to, speed, atk);
            p.to = ballisticPosition(p, p.traveltime);
        }
        p.offset = hudgunorigin(attacks[atk].gun, from, to, owner);
        p.offset.sub(from);
        p.local = local;
        p.ownerId = owner ? owner->entityId : INVALID_ENTITY_ID;
        switch(p.atk)
        {
            case ATK_FIREWORKS: p.lifetime= attacks[atk].ttl + rnd(400); break;
            case ATK_CROSSBOW: p.lifetime = bouncers::bouncersfade + rnd(5000); break;
            default: p.lifetime = attacks[atk].ttl;
        }
        p.exploded = false;
        p.offsetmillis = OFFSETMILLIS;
        p.id = local ? lastmillis : id;
        p.inwater = inWater(p.o);

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

    void stain(const projectile &p, const vec &pos, int atk)
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
            {
                addstain(STAIN_PLASMA_SCORCH, pos, dir, 5);
                gameent *owner = ownerOf(p);
                addstain(STAIN_SPOCK, pos, dir, 5, owner && owner->hasRoids() ? 0xFF0000 : 0x22FF22);
                return;
            }
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

    void splash(projectile &p, const vec &v, dynent *safe)
    {
        gameent *owner = ownerOf(p);
        if(!owner) return;
        explode(p.local, owner, v, p.dir, safe, attacks[p.atk].damage, p.atk);
        stain(p, v, p.atk);
    }

    float distance(dynent *o, vec &dir, const vec &v, const vec &vel)
    {
        vec middle = o->o;
        middle.z += (o->aboveeye-o->eyeheight)/2;
        dir = vec(middle).sub(v).add(vec(vel).mul(5)).safenormalize();

        float low = min(o->o.z - o->eyeheight + o->radius, middle.z),
              high = max(o->o.z + o->aboveeye - o->radius, middle.z);
        vec closest(o->o.x, o->o.y, clamp(v.z, low, high));
        return max(closest.dist(v) - o->radius, 0.0f);
    }

    bool damage(dynent *o, projectile &p, const vec &v)
    {
        if(o->state!=CS_ALIVE) return false;
        if(!intersect(o, p.o, v, attacks[p.atk].margin)) return false;
        splash(p, v, o);
        vec dir;
        distance(o, dir, v, p.dir);
        gameent *owner = ownerOf(p);
        if(!owner) return false;
        hit(attacks[p.atk].damage, o, owner, dir, p.atk, 0);
        return true;
    }

    void update(int time)
    {
        if(curProjectiles.empty()) return;
        loopv(curProjectiles)
        {
            projectile &p = curProjectiles[i];
            gameent *owner = ownerOf(p);
            if(p.local && !owner)
            {
                if(p.soundplaying) stopLinkedSound(p.entityId);
                removeEntityPos(p.entityId);
                curProjectiles.remove(i--);
                continue;
            }
            p.offsetmillis = max(p.offsetmillis-time, 0);
            if(isStuckArrow(p))
            {
                if((p.lifetime -= time)<0) curProjectiles.remove(i--);
                continue;
            }

            bool removearrow = false;
            vec dv;
            vec v;
            float dist = 0;
            bool reachedEnd = false;
            if(p.ballistic)
            {
                reachedEnd = ballisticStep(p, time, v, dv);
            }
            else
            {
                dist = p.to.dist(p.o, dv);
                dv.mul(time/max(dist*1000/p.speed, float(time)));
                v = vec(p.o).add(dv);
            }
            bool exploded = false;
            hits.setsize(0);

            if((p.lifetime -= time)<0 && (p.atk==ATK_FIREWORKS || p.atk==ATK_S_NUKE || p.atk==ATK_KAMIKAZE || p.atk==ATK_POWERARMOR))
            {
                splash(p, v, NULL);
                exploded = true;
            }

            if(!exploded && !dv.iszero()) p.dir = vec(dv).normalize();

            bool hitWorld = false;
            if(!exploded && p.ballistic)
            {
                vec hitpos;
                if(worldCollision(p.o, v, hitpos))
                {
                    v = hitpos;
                    dv = vec(v).sub(p.o);
                    if(!dv.iszero()) p.dir = vec(dv).normalize();
                    hitWorld = true;
                }
            }

            if(p.local)
            {
                vec halfdv = vec(dv).mul(0.5f), bo = vec(p.o).add(halfdv);
                float br = max(fabs(halfdv.x), fabs(halfdv.y)) + 1 + attacks[p.atk].margin;
                loopj(numdynents())
                {
                    dynent *o = iterdynents(j);
                    if((owner && owner==o) || o->o.reject(bo, o->radius + br)) continue;
                    if(damage(o, p, v)) {exploded = true; removearrow = true; break; }
                }
            }

            if(!exploded)
            {
                if(p.ballistic)
                {
                    if(hitWorld || reachedEnd)
                    {
                        if(!p.exploded) splash(p, v, NULL);
                        exploded = true;
                    }
                }
                else if(dist<4)
                {
                    if(p.o!=p.to) // if original target was moving, reevaluate endpoint
                    {
                        if(raycubepos(p.o, p.dir, p.to, 0, RAY_CLIPMAT|RAY_ALPHAPOLY)>=4) continue;
                    }
                    if(!p.exploded) splash(p, v, NULL);
                    exploded = true;
                }
            }

            if(exploded)
            {
                if(p.local && owner && !p.exploded) addmsg(N_EXPLODE, "rci3iv", owner, lastmillis-maptime, p.atk, p.id-maptime, hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
                p.exploded = true;
                if(p.soundplaying)
                {
                    stopLinkedSound(p.entityId);
                    removeEntityPos(p.entityId);
                    p.soundplaying = false;
                }
                if(p.atk != ATK_CROSSBOW) curProjectiles.remove(i--);
                else if(removearrow || (p.lifetime -= time)<0) curProjectiles.remove(i--);
                else stickArrow(p, v);
            }
            else
            {
                p.o = v;
                if(p.ballistic) p.elapsed = min(p.elapsed + float(time), p.traveltime);
            }
        }
    }

    void checkSound(projectile &p, int atk, vec dv)
    {
        if(p.projsound && !game::ispaused()) // play and update the sound only if the projectile is passing by
        {
            bool bigRadius = (atk==ATK_S_NUKE || atk==ATK_FIREWORKS);

            if(camera1->o.dist(p.o) < (bigRadius ? 800 : 400))
            {
                updateEntPos(p.entityId, p.o, dv.mul(4));
                if(!p.soundplaying)
                {
                    playSound(p.projsound, p.o, bigRadius ? 800 : 400, 1, SND_LOOPED, p.entityId);
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

    void checkWater(projectile &p)
    {
        if(!p.inwater && inWater(p.o))
        {
            p.inwater = true;
            vec effectPos = p.o;
            effectPos.addz(2.5f);
            particles::dirSplash(PART_WATER, 0x40403A, 150, 10, 75, effectPos, vec(0, 0, 1), 2.f, 300, 15, hasShrooms());
            particles::dirSplash(PART_WATER, 0x50503A, 500, 5, 150, effectPos, vec(0, 0, 1), 3.f, 150, 15, hasShrooms());
            playSound(S_IMPACTWATER, effectPos, 250, 50, SND_LOWPRIORITY|SND_NOOCCLUSION);
        }
    }

    void render(int time)
    {
        if(curProjectiles.empty()) return;
        loopv(curProjectiles)
        {
            projectile &p = curProjectiles[i];
            gameent *owner = ownerOf(p);
            int atk = p.atk;

            vec dv;
            vec v;
            float dist = 0;
            if(isStuckArrow(p))
            {
                v = p.o;
                dv = vec(0, 0, 0);
            }
            else if(p.ballistic)
            {
                ballisticStep(p, time, v, dv);
            }
            else
            {
                dist = p.to.dist(p.o, dv);
                dv.mul(time/max(dist*1000/p.speed, float(time)));
                v = vec(p.o).add(dv);
            }
            vec pos = vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)).add(v);

            checkSound(p, atk, dv);
            checkWater(p);

            if(atk==ATK_SMAW || atk==ATK_FIREWORKS || atk==ATK_CROSSBOW || atk==ATK_S_ROCKETS || atk==ATK_S_NUKE)
            {
                float yaw, pitch;
                vec traj = (!dv.iszero() ? vec(dv).normalize() : p.dir);
                if(!p.ballistic && dist >= 1e-6f) traj = vec(p.to).sub(pos).normalize();
                vectoyawpitch(traj, yaw, pitch);
                rendermodel(projsPaths[atk].c_str(), ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, MDL_NOSHADOW|MDL_CULL_VFC|MDL_CULL_OCCLUDED);
                renderProjectilesTrails(owner, pos, dv, p.from, p.offset, atk, p.exploded);
            }
            else renderProjectilesTrails(owner, pos, dv, p.from, p.offset, atk, p.exploded);
        }
    }

    void avoid(ai::avoidset &obstacles, float radius)
    {
        loopv(curProjectiles)
        {
            projectile &p = curProjectiles[i];
            obstacles.avoidnear(NULL, p.o.z + attacks[p.atk].exprad + 1, p.o, radius + attacks[p.atk].exprad);
        }
    }

    void remove(gameent *owner)
    {   // can't use loopv here due to strange GCC optimizer bug
        int len = curProjectiles.length();
        loopi(len)
        {
            projectile &p = curProjectiles[i];
            if(curProjectiles[i].ownerId == (owner ? owner->entityId : INVALID_ENTITY_ID))
            {
                if(p.soundplaying) stopLinkedSound(p.entityId);
                removeEntityPos(p.entityId);
                curProjectiles.remove(i--);
                len--;
            }
        }
    }

    void clear()
    {
        loopv(curProjectiles)
        {
            projectile &p = curProjectiles[i];
            if(p.soundplaying) stopLinkedSound(p.entityId);
            removeEntityPos(p.entityId);
        }
        curProjectiles.shrink(0);
    }
}
