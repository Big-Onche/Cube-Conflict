#include "game.h"

using namespace game;

namespace bouncers
{
    VARP(bouncersfade, 2500, 15000, 120000);

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

    static constexpr int MAXBOUNCERVARIANTS = 8;
    static constexpr int MAXBOUNCERPATHLEN = 64;
    static char bouncerPaths[NUMBOUNCERS][MAXBOUNCERVARIANTS + 1][MAXBOUNCERPATHLEN];
    static int maxBouncerVariants = 0;

    void initPaths() // store the path of all bouncers and their variants
    {
        maxBouncerVariants = 0;

        loopi(NUMBOUNCERS)
        {
            int variants = bouncers[i].variants;
            if(variants > MAXBOUNCERVARIANTS) variants = MAXBOUNCERVARIANTS;
            if(variants > maxBouncerVariants) maxBouncerVariants = variants;

            for (int variant = 0; variant <= variants; ++variant)
            {
                if(!bouncers[i].name)
                {
                    bouncerPaths[i][variant][0] = '\0';
                    continue;
                }

                if(variant) snprintf(bouncerPaths[i][variant], MAXBOUNCERPATHLEN, "bouncers/%s/%d", bouncers[i].name, variant);
                else snprintf(bouncerPaths[i][variant], MAXBOUNCERPATHLEN, "bouncers/%s", bouncers[i].name);
            }
        }
    }

    inline const char *getPath(int type, int variant)
    {
        if(type < 0 || type >= NUMBOUNCERS || variant < 0 || variant > maxBouncerVariants) return "";
        return bouncerPaths[type][variant];
    }

    void preload()
    {
        loopi(NUMBOUNCERS)
        {
            if(i == BNC_LIGHT) continue;

            int variants = bouncers[i].variants;
            if(variants <= 0)
            {
                preloadmodel(getPath(i, 0));
                continue;
            }

            for(int variant = 1; variant <= variants; ++variant) preloadmodel(getPath(i, variant));
        }
    }

    vector<bouncer *> curBouncers;
    static vector<bouncer *> freeBouncers;

    static inline bouncer *allocBouncer() { return freeBouncers.empty() ? new bouncer : freeBouncers.pop(); }

    static inline void freeBouncer(bouncer *bnc) { freeBouncers.add(bnc); }

    void add(const vec &from, const vec &to, bool local, int id, gameent *owner, int type, int lifetime, int speed, vec2 yawPitch)
    {
        bouncer &bnc = *curBouncers.add(allocBouncer());
        const auto &cfg = bouncers[type];

        bnc.reset();
        bnc.entityId = entitiesIds::getNewId();
        bnc.type = ENT_BOUNCE;
        bnc.bounces = 0;
        bnc.roll = 0;
        bnc.variant = 0;
        bnc.particles = vec(-1, -1, -1);
        bnc.yaw = 0;
        bnc.pitch = 0;
        bnc.offset = vec(0, 0, 0);

        bnc.o = from;
        bnc.radius = bnc.xradius = bnc.yradius = cfg.size;
        bnc.eyeheight = bnc.radius;
        bnc.aboveeye = bnc.radius;
        bnc.lifetime = lifetime;
        bnc.local = local;
        bnc.owner = owner;
        bnc.bouncetype = type;
        bnc.id = local ? lastmillis : id;
        bnc.inwater = ((lookupmaterial(from) & MAT_WATER) == MAT_WATER);
        bnc.seed = rnd(2);

        if(!yawPitch.iszero())
        {
            bnc.yaw = yawPitch.x;
            bnc.pitch = yawPitch.y;
        }

        int variants = cfg.variants;
        if(variants > MAXBOUNCERVARIANTS) variants = MAXBOUNCERVARIANTS;
        if(variants > 0) bnc.variant = rnd(variants) + 1;

        bnc.collidetype = COLLIDE_ELLIPSE;

        vec dir = vec(to).sub(from).safenormalize();
        bnc.vel = vec(dir).mul(speed);

        avoidcollision(&bnc, dir, owner, 0.1f);

        if(type==BNC_GRENADE || type==BNC_MOLOTOV)
        {
            bool isGrenade = (type==BNC_GRENADE);
            bnc.offset = hudgunorigin(isGrenade ? GUN_M32 : GUN_MOLOTOV, from, to, owner);
            if(owner==hudplayer() && !isthirdperson()) bnc.offset.sub(owner->o).rescale(16).add(owner->o);
            bnc.offset.sub(from);
            if(isGrenade) playSound(S_GRENADE, bnc.o, 300, 100, SND_FIXEDPITCH|SND_NOCULL, bnc.entityId);
        }

        bnc.offsetmillis = OFFSETMILLIS;

        bnc.resetinterp();
    }

    void spawn(const vec &p, const vec &vel, gameent *d, int type, int speed, int lifetime, bool frommonster)
    {
        const auto &cfg = bouncers[type];
        const float maxd = float(cfg.cullDist);
        if(p.isneg() || (type!=BNC_GRENADE && camera1->o.fastsquaredist(p) > maxd*maxd)) return; // culling distant ones, except grenades, grenades are important

        vec dir;
        vec2 yawPitch;
        if(!speed) speed = 50 + rnd(20);

        switch(type)
        {
            case BNC_CASING:
            case BNC_BIGCASING:
            case BNC_CARTRIDGE:
                vecfromyawpitch(d->yaw, 0, 0, -50, dir);
                dir.add(vec(-10 + rnd(21), -10 + rnd(21), 100));
                yawPitch.x = camera1->yaw;
                yawPitch.y = camera1->pitch;
                break;
            case BNC_GLASS:
                dir = vec(-150 + rnd(301), -150 + rnd(301), rnd(50));
                break;
            default:
                dir = vec(-50 + rnd(101), -50 + rnd(101), -50 + rnd(101));
        }

        if(dir.iszero()) dir.z += 1;
        dir.normalize();

        vec to = vec(dir).mul(speed).add(vel).add(p); // Combine direction with player's momentum

        add(p, to, true, 0, d, type, lifetime, speed, yawPitch);
    }

    void bounceEffect(physent *d, const vec &surface)
    {
        bouncer *b = (bouncer *)d;

        if(d->type != ENT_BOUNCE || b->type == BNC_LIGHT) return;

        if(bouncers[b->bouncetype].bounceSoundRad && b->bounces < 5)
        {
            int soundRadius = bouncers[b->bouncetype].bounceSoundRad;
            playSound(bouncers[b->bouncetype].bounceSound, b->o, soundRadius, soundRadius * 0.5f, SND_LOWPRIORITY|bouncers[b->bouncetype].soundFlag);
        }

        if(b->bouncetype == BNC_GRENADE)
        {
            addstain(STAIN_PLASMA_GLOW, vec(b->o).sub(vec(surface).mul(b->radius)), surface, 4.f, 0x0000FF);
        }

        b->bounces++;
    }

    void update(int time)
    {
        loopv(curBouncers)
        {
            bouncer &bnc = *curBouncers[i];
            bool isGrenade = (bnc.bouncetype == BNC_GRENADE);
            vec old(bnc.o);

            bool stopped = false;
            if(isGrenade)
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

            if(bnc.bouncetype != BNC_LIGHT)
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
                if(isGrenade || bnc.bouncetype == BNC_MOLOTOV)
                {
                    int atk = (isGrenade ? ATK_M32 : ATK_MOLOTOV);
                    hits.setsize(0);
                    explode(bnc.local, bnc.owner, bnc.o, bnc.o, NULL, 1, atk);

                    if(isGrenade)
                    {
                        stopLinkedSound(bnc.entityId);
                        removeEntityPos(bnc.entityId);
                    }

                    if(bnc.local)
                    {
                        addmsg(N_EXPLODE, "rci3iv", bnc.owner, lastmillis-maptime, atk, bnc.id-maptime, hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
                    }

                }
                freeBouncer(curBouncers.remove(i--));
            }
            else bnc.offsetmillis = max(bnc.offsetmillis-time, 0);
        }
    }

    void render()
    {
        bool isPaused = game::ispaused();

        loopv(curBouncers)
        {
            bouncer &bnc = *curBouncers[i];
            vec pos(bnc.o);
            pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));

            bool inWater = ((lookupmaterial(pos) & MATF_VOLUME) == MAT_WATER);
            int bouncerType = bnc.bouncetype;
            int numBounces = bnc.bounces;
            float magnitude = bnc.vel.magnitude();
            bool stopped = (numBounces > 4);

            if(!isPaused)
            {
                if(!stopped)
                {
                    float rot = (magnitude / 4.f);

                    if(numBounces)
                    {
                        bool oddBounces = (numBounces % 2 != 0);
                        bnc.yaw += oddBounces ? - rot : rot;
                        bnc.pitch += oddBounces ? rot : - rot;
                    }

                    if(bouncerType==BNC_MOLOTOV || bouncerType == BNC_GRENADE)
                    {
                        int rot = (bnc.seed ? 2 : -2);
                        if(bouncerType == BNC_GRENADE) rot *= 3;
                        bnc.roll += rot;
                        bnc.yaw -= rot;
                        bnc.pitch += rot;
                    }
                    else bnc.roll += rot;
                }
                else bnc.pitch = lerp(bnc.pitch, 180, 0.5f);
            }

            if(bouncerType == BNC_MOLOTOV)
            {
                modelattach a[2];
                bnc.particles = vec(-1, -1, -1);
                a[0] = modelattach("tag_particles", &bnc.particles);
                rendermodel(getPath(bouncerType, bnc.variant), ANIM_MAPMODEL|ANIM_LOOP, pos, bnc.yaw, bnc.pitch, bnc.roll, MDL_NOBATCH, NULL, a[0].tag ? a : NULL);
            }

            rendermodel(getPath(bouncerType, bnc.variant), ANIM_MAPMODEL|ANIM_LOOP, pos, bnc.yaw, bnc.pitch, bnc.roll, MDL_CULL_VFC|MDL_CULL_EXTDIST|MDL_CULL_OCCLUDED);

            if(!isPaused && (!stopped || bouncerType == BNC_GRENADE))
            {
                switch(bouncerType)
                {
                    case BNC_CASING:
                        particle_splash(PART_SMOKE, 1, 150, pos, 0x505050, 0.8f, 50, -20, 1);
                        break;

                    case BNC_BIGCASING:
                        particle_splash(PART_SMOKE, 1, 150, pos, 0x404040, 1.75f, 50, -20, 2);
                        break;

                    case BNC_CARTRIDGE:
                        particle_splash(PART_SMOKE, 1, 150, pos, 0x252525, 1.5f, 50, -20, 3);
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
                        particle_fireball(pos, growth, PART_EXPLOSION, 20, bnc.owner->hasRoids() ? 0xFF0000 : 0x0055FF, growth, hasShrooms());
                        particle_splash(PART_SMOKE, 1, 150, pos, 0x404088, 2.5f, 50, -20, 0, hasShrooms());
                        adddynlight(pos, 40, vec(0.5f, 0.5f, 2.0f), 0, 0, L_NOSHADOW);
                        break;
                    }
                    case BNC_SCRAP:
                        particle_splash(inWater ? PART_BUBBLE : PART_SMOKE, inWater ? 1 : 3, 250, pos, 0x222222, 2.5f, 50, -50, 0, hasShrooms());
                        particle_splash(PART_FIRE_BALL, 2, 75, pos, 0x994400, 0.7f, 30, -30, 0, hasShrooms());
                        break;

                    case BNC_GLASS:
                        particle_splash(PART_SMOKE, 1, 1200, pos, 0x303030, 2.5f, 50, -50, 10, hasShrooms());
                        particle_splash(PART_FIRE_BALL, 1, 250, pos, bnc.owner->hasRoids() ? 0xFF0000 : 0x996600, 1.3f, 50, -50, 12, hasShrooms());
                        if(rnd(2)) particle_splash(PART_AR, 1, 500, pos, 0xFFFFFF, 12.f, 50, -25, 50);
                        break;

                    case BNC_MOLOTOV:
                        particle_splash(PART_FIRE_BALL, 2, 80, bnc.particles, 0xFFC864, 1, 30, 30, 0, hasShrooms());
                        particle_splash(PART_SMOKE, 3, 180, bnc.particles, 0x444444, 2, 40, 50, 0, hasShrooms());
                        particle_splash(PART_AR, 2, 250, bnc.particles, 0xFFFFFF, 2, 40, 50, 5);
                        //particle_splash(PART_SMOKE, 3, 180, pos, 0x6A6A6A, 2, 40, 50, 0, hasShrooms());
                        break;

                    case BNC_BURNINGDEBRIS:
                    {
                        int flamesColor = (rnd(2) ? 0x993A00 : 0x856611);
                        if(!rnd(2)) particle_splash(PART_SMOKE, 1, 1800 + rnd(400), pos, 0x282828, 2.f, 50, -100, 12, hasShrooms());
                        particle_splash(PART_FIRE_BALL, 1, 175, pos, flamesColor, 1.f, 20, 0, 4, hasShrooms());
                        particle_splash(PART_FIRE_BALL, 1, 175, bnc.o, flamesColor, 1.f, 20, 0, 4, hasShrooms());
                        break;
                    }
                    case BNC_LIGHT:
                        adddynlight(pos, 115, vec(0.25f, 0.12f, 0.0f), 0, 0, L_VOLUMETRIC|L_NOSHADOW|L_NOSPEC);
                        break;
                }
            }
        }
    }

    void remove(gameent *owner)
    {
        loopv(curBouncers)
        {
            if(curBouncers[i]->owner == owner)
            {
                removeEntityPos(curBouncers[i]->entityId);
                freeBouncer(curBouncers.remove(i--));
            }
        }
    }

    void clear()
    {
        loopv(curBouncers)
        {
            bouncer &bnc = *curBouncers[i];
            if(bnc.bouncetype == BNC_GRENADE)
            {
                stopLinkedSound(bnc.entityId);
                removeEntityPos(bnc.entityId);
            }
        }
        while(!curBouncers.empty()) freeBouncer(curBouncers.pop());
    }
}
