
#include "gfx.h"

using namespace game;

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

    void add(const vec &from, const vec &to, float speed, bool local, int id, gameent *owner, int atk)
    {
        if(from.isneg()) return;
        projectile &p = curProjectiles.add();
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
            case ATK_FIREWORKS: p.lifetime= attacks[atk].ttl + rnd(400); break;
            case ATK_CROSSBOW: p.lifetime = bouncers::bouncersfade + rnd(5000); break;
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

    void splash(projectile &p, const vec &v, dynent *safe)
    {
        explode(p.local, p.owner, v, p.dir, safe, attacks[p.atk].damage, p.atk);
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
        hit(attacks[p.atk].damage, o, p.owner, dir, p.atk, 0);
        return true;
    }

    void update(int time)
    {
        bool removearrow = false;

        if(curProjectiles.empty()) return;
        loopv(curProjectiles)
        {
            projectile &p = curProjectiles[i];
            p.offsetmillis = max(p.offsetmillis-time, 0);
            vec dv;
            float dist = p.to.dist(p.o, dv);
            dv.mul(time/max(dist*1000/p.speed, float(time)));
            vec v = vec(p.o).add(dv);
            bool exploded = false;
            hits.setsize(0);

            if((p.lifetime -= time)<0 && (p.atk==ATK_FIREWORKS || p.atk==ATK_S_NUKE || p.atk==ATK_KAMIKAZE || p.atk==ATK_POWERARMOR))
            {
                splash(p, v, NULL);
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
                    if(damage(o, p, v)) {exploded = true; removearrow = true; break; }
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
                    if(!p.exploded) splash(p, v, NULL);
                    exploded = true;
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
                if(p.atk != ATK_CROSSBOW) curProjectiles.remove(i--);
                else if((p.lifetime -= time)<0 || removearrow) curProjectiles.remove(i--);
            }
            else p.o = v;
        }
    }

    void checkSound(projectile &p, int atk, vec dv)
    {
        if(p.projsound && !game::ispaused()) // play and update the sound only if the projectile is passing by
        {
            bool bigRadius = (atk==ATK_S_NUKE || atk==ATK_FIREWORKS);

            if(camera1->o.dist(p.o) < (bigRadius ? 800 : 400))
            {
                updateEntPos(p.entityId, p.o, dv.mul(5));
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
        if(!p.inwater && (lookupmaterial(p.o) & MATF_VOLUME) == MAT_WATER)
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
            int atk = p.atk;

            vec dv;
            float dist = p.to.dist(p.o, dv);
            dv.mul(time/max(dist*1000/p.speed, float(time)));
            vec v = vec(p.o).add(dv);
            vec pos = vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)).add(v);

            checkSound(p, atk, dv);
            checkWater(p);

            if(atk==ATK_SMAW || atk==ATK_FIREWORKS || atk==ATK_CROSSBOW || atk==ATK_S_ROCKETS || atk==ATK_S_NUKE)
            {
                float yaw, pitch;
                vectoyawpitch((dist < 1e-6f ? p.dir : vec(p.to).sub(pos).normalize()), yaw, pitch);
                rendermodel(projsPaths[atk].c_str(), ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, MDL_NOSHADOW|MDL_CULL_VFC|MDL_CULL_OCCLUDED);
                renderProjectilesTrails(p.owner, pos, dv, p.from, p.offset, atk, p.exploded);
            }
            else renderProjectilesTrails(p.owner, pos, dv, p.from, p.offset, atk, p.exploded);
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
            if(curProjectiles[i].owner==owner)
            {
                if(curProjectiles[i].soundplaying) stopLinkedSound(curProjectiles[i].entityId);
                removeEntityPos(curProjectiles[i].entityId);
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
        }
        curProjectiles.shrink(0);
    }
}
