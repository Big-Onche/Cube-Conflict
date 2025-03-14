//gfx.cpp: fancy and laggy graphics effects

#include "gfx.h"
#include "game.h"

VARP(epilepsyfriendly, 0, 0, 1);

extern vec hitsurface;

namespace game
{
    VAR(forcecampos, -1, -1, 1000);
    VAR(zoom, -1, 0, 1);

    void lightTrail(const vec &s, const vec &e, int radius, int fade, int peak, const vec &color) //cast lights along a ray
    {
        vec v;
        float d = e.dist(s, v);
        int trails = (s.dist(e))/30;
        int steps = clamp(int(d*2), 1, trails);
        v.div(steps);
        vec p = s;
        loopi(steps)
        {
            p.add(v);
            adddynlight(p, radius, color, fade, peak, L_NOSHADOW, 0, color);
        }
    }

    void renderProjectileExplosion(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk) // particles and light effects on impact for slow projectiles
    {
        vec lightOrigin = vec(v).sub(vec(vel).mul(10));
        bool inWater = ((lookupmaterial(lightOrigin) & MATF_VOLUME) == MAT_WATER);
        bool hasRoids = owner->hasRoids();

        switch(atk)
        {
            case ATK_PLASMA:
            {
                if(!inWater) particle_splash(PART_FIRESPARK, 15, 150, v, hasRoids ? 0xFF0000 : 0xFF6600, 1.f, 150, 500, 2, hasShrooms());
                particle_fireball(v, 9.f, PART_PLASMABURST, 300, hasRoids ? 0xFF0000 : 0xCC9900, 3, hasShrooms());
                adddynlight(safe ? v : lightOrigin, 2*attacks[atk].exprad, vec(1.5f, 0.75f, 0.0f), 150, 50, 0, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                return;
            }
            case ATK_GRAP1:
            {
                if(!inWater) particle_splash(PART_SPARK, 20, 100, v, hasRoids ? 0xFF0000 : 0xAA4466, 0.5f, 400, 400, 0, hasShrooms());
                loopi(2)particle_fireball(v, 7.f, PART_EXPLOSION, 200, hasRoids ? 0xFF0000 : 0x550055, 1.f, hasShrooms());
                adddynlight(safe ? v : lightOrigin, 2*attacks[atk].exprad, vec(1.5f, 0.0f, 1.5f), 200, 100, L_NODYNSHADOW, attacks[atk].exprad/2, vec(0.5f, 0.0f, 0.5f));
                return;
            }
            case ATK_SPOCKGUN:
            {
                if(!inWater) particle_splash(PART_SPARK, 15, 100, v, hasRoids ? 0xFF0000 : 0x00FF66, 0.5f, 250, 250, 0, hasShrooms());
                adddynlight(safe ? v : lightOrigin, 1.25f*attacks[atk].exprad, hasRoids ? vec(1.5f, 0.f, 0.f) : vec(0.f, 1.5f, 0.f), 200, 50, 0, attacks[atk].exprad/2, vec(1.f, 1.f, 1.f));
                return;
            }
        }
    }

    void renderProjectilesTrails(gameent *owner, vec &pos, vec dv, vec &from, vec &offset, int atk, bool exploded) //particles and light effects on impact for slow projectiles
    {
        bool emitPart = canemitparticles();
        bool inWater = ((lookupmaterial(pos) & MATF_VOLUME) == MAT_WATER);
        bool firstPerson = (owner==game::hudplayer());
        bool hasRoids = owner->hasRoids();

        float len = min(100.f, vec(offset).add(from).dist(pos));
            vec dir = vec(dv).normalize(),
            tail = vec(dir).mul(-len).add(pos),
            head = vec(dir).mul(2.4f).add(pos);

        switch(atk)
        {
            case ATK_PLASMA:
                particle_splash(PART_PLASMA_FRONT, 1, 1, pos, hasRoids ? 0xFF4444 : 0xFF6600, 2.4f, 150, 20, 0, hasShrooms());
                particle_flare(tail, head, 1, PART_F_PLASMA, hasRoids ? 0xFF4444 : 0xFF6600, 2.f, owner, hasShrooms());
                if(inWater && emitPart) particle_splash(PART_BUBBLE, 1, 150, pos, 0x18181A, 2.0f+rnd(2), 20, -30);
                adddynlight(pos, 30, vec(1.00f, 0.75f, 0.0f));
                break;

            case ATK_GRAP1:
                particle_splash(PART_PLASMA_FRONT, 1, 1, pos, hasRoids ? 0xFF2222 : 0xFF33BB, 3.0f, 150, 20, 0, hasShrooms());
                particle_flare(tail, head, 1, PART_F_PLASMA, hasRoids ? 0xFF2222 : 0xEE22AA, 3.0f, owner, hasShrooms());
                if(emitPart) particle_splash(inWater ? PART_BUBBLE : PART_SMOKE, 1, inWater ? 150 : 300, pos, inWater ? 0x18181A : 0xAAAAAA, 4.0f, 25, 250, 0, hasShrooms());
                adddynlight(pos, 50, vec(0.3f, 0.00f, 0.2f));
                break;

            case ATK_SPOCKGUN:
                particle_splash(PART_SPOCK_FRONT, 1, 1, pos, hasRoids ? 0xFF4444 : 0x00FF00, 4.f, 150, 20, 0, hasShrooms());
                particle_flare(tail, head, 1, PART_F_PLASMA, hasRoids ? 0xFF4444 : 0x22FF22, 2.5f, owner, hasShrooms());
                adddynlight(pos, 30, vec(0.00f, 1.00f, 0.0f));
                break;

            case ATK_SV98:
            case ATK_SKS:
            case ATK_S_GAU8:
                if(inWater && emitPart) particle_splash(PART_BUBBLE, 1, 150, pos, 0x18181A, 2.0f+rnd(2), 20, -30);
                particle_flare(tail, head, 1, PART_F_BULLET, hasRoids ? 0xFF4444 : 0xFFBB88, atk==ATK_S_GAU8 ? 0.75f : 0.65f, owner, hasShrooms());
                particle_splash(PART_PLASMA_FRONT, 1, 1, pos, hasRoids ? 0xFF4444 : 0xFFBB88, firstPerson ? 0.65f : atk==ATK_S_GAU8 ? 0.45f : 0.3f, 150, 20, 0, hasShrooms());
                if(emitPart) particle_flare(tail, head, atk==ATK_SV98 ? 3000 : 2000, PART_F_SMOKE, 0x333333, atk==ATK_SV98 ? 1.4f : 1.f, owner, hasShrooms(), 3);
                break;

            case ATK_MINIGUN:
            case ATK_AK47:
            case ATK_UZI:
            case ATK_GLOCK:
            case ATK_FAMAS:
            {
                if(inWater && emitPart) particle_splash(PART_BUBBLE, 1, 150, pos, 0x18181A, 1.0f+rnd(2), 20, -30);
                bool bigBullet = (atk == ATK_MINIGUN || atk == ATK_AK47);
                particle_flare(tail, head, 1, PART_F_BULLET, hasRoids ? 0xFF4444 : 0xFFBB88, bigBullet ? 0.55f : 0.45f, owner, hasShrooms());
                particle_splash(PART_PLASMA_FRONT, 1, 1, pos, hasRoids ? 0xFF4444 : 0xFFBB88, firstPerson ? 0.4f : bigBullet ? 0.3f : 0.24f, 150, 20, 0, hasShrooms());
                if(emitPart) particle_flare(tail, head, bigBullet ? 2000 : 1250, PART_F_SMOKE, 0x252525, bigBullet ? 1.f : 0.75f, owner, hasShrooms(), 3);
                break;
            }

            case ATK_MOSSBERG:
            case ATK_HYDRA:
                if(inWater && emitPart) particle_splash(PART_BUBBLE, 1, 150, pos, 0x18181A, 1.0f+rnd(2), 20, -30);
                particle_flare(tail, head, 1, PART_F_BULLET, hasRoids ? 0xFF4444 : 0xFF7700, atk==ATK_MOSSBERG ? 0.45f : 0.35f, owner, hasShrooms());
                particle_splash(PART_PLASMA_FRONT, 1, 1, pos, hasRoids ? 0xFF4444 : 0xFF7700, firstPerson ? 0.4f : atk==ATK_MOSSBERG ? 0.3f : 0.24f, 150, 20, 0, hasShrooms());
                if(emitPart) particle_flare(tail, head, atk==ATK_MOSSBERG ? 2000 : 1000, PART_F_SMOKE, 0x101010, atk==ATK_MOSSBERG ? 0.75f : 0.55f, owner, hasShrooms(), 3);
                break;

            case ATK_CROSSBOW:
                if(!exploded)
                {
                    if(emitPart)
                    {
                        if(inWater) particle_splash(PART_BUBBLE, 1, 150, pos, 0x888888, 1.0f+rnd(2), 20, -30);
                        particle_flare(tail, head, 350, PART_F_SMOKE, 0x555555, 0.60f, owner, hasShrooms(), 10);
                    }

                    if(hasRoids)
                    {
                        particle_flare(tail, head, 1, PART_F_BULLET, 0xFF4444, 0.30f, owner, hasShrooms());
                        particle_splash(PART_PLASMA_FRONT, 1, 1, pos, 0xFF4444, firstPerson ? 0.5f : 0.25f, 150, 20, 0, hasShrooms());
                    }
                }
                break;

            case ATK_SMAW:
            case ATK_S_ROCKETS:
                if(emitPart) particle_splash(inWater ? PART_BUBBLE : PART_SMOKE, 1, 2000, pos, 0x666666, inWater ? 3.f : 6.f, 25, 250, 0, hasShrooms());
                particle_flare(pos, pos, 2, PART_MF_LITTLE, hasRoids ? 0xFF0000 : 0xFFC864, 3.0f + rndscale(2), NULL, hasShrooms());
                particles::lensFlare(pos, 0x331200, 300+rnd(300), 50);
                adddynlight(pos, (ispaused() ? 65 : 50 + rnd(31)), vec(1.2f, 0.75f, 0.0f));
                break;

            case ATK_FIREWORKS:
                if(emitPart)
                {
                    if(inWater) particle_splash(PART_BUBBLE, 3, 200, pos, 0x18181A, 2.5f, 25, 100, 0, hasShrooms());
                    particle_splash(PART_SPARK, 8, 100, pos, hasRoids ? 0xFF0000 : 0xFFC864, rnd(4)/10.f+0.1f, 50, 500, 0, hasShrooms());
                }
                particle_flare(pos, pos, 1, PART_MF_LITTLE, hasRoids ? 0xFF0000 : 0xFFC864, 0.5f+rndscale(2), NULL, hasShrooms());
                break;

            case ATK_S_NUKE:
                if(emitPart) particle_splash(inWater ? PART_BUBBLE : PART_SMOKE, 3, inWater ? 2000 : 5000, pos, inWater ? 0x18181A : 0x222222, 4.0f+rnd(5), 25, 200, 0, hasShrooms());
                particle_flare(pos, pos, 2, PART_MF_LITTLE, hasRoids ? 0xFF0000 : 0xFFC864, 10.f+rndscale(8), NULL, hasShrooms());
                particle_splash(PART_FIRE_BALL, 1, 100, pos, hasRoids ? 0xFF0000 : 0xFF6600, 1.0f+rndscale(4), 50, 500, 0, hasShrooms());
                particles::lensFlare(pos, 0x552500, 600+rnd(400), 75);
                adddynlight(pos, 100, vec(1.2f, 0.75f, 0.0f));
                break;
            }
    }

    int flamesColor[3] = {0x383838, 0x474747, 0x604930};

    static const struct colorsConfig{ uint32_t color; } colors[] =
    {
        {0xFF8888},
        {0x88FF88},
        {0x8888FF},
        {0xFFFF44},
        {0x44FFFF},
        {0xFF44FF}
    };

    uint32_t fireworkColor() { return colors[rnd(ARRAY_SIZE(colors))].color; }


    void renderExplosion(gameent *owner, const vec &v, const vec &vel, int atk) //big laggy flashy explosions
    {
        bool inWater = ((lookupmaterial(v) & MATF_VOLUME) == MAT_WATER);
        int initRadius = attacks[atk].exprad/2;
        int flags = L_NODYNSHADOW|DL_FLASH;
        bool hasRoids = owner->hasRoids();

        switch(atk)
        {
            case ATK_SMAW:
            case ATK_S_ROCKETS:
            {
                bool miniRockets = (atk == ATK_S_ROCKETS);

                particle_fireball(v, 350, PART_SHOCKWAVE, 300, hasRoids ? 0xFF0000 : 0xFFFFFF, 20.0f, hasShrooms());
                particle_splash(PART_AR, 5, 350, v, 0xCCCCCC, 50, 300, 800, 40);

                loopi(inWater ? 3 : 8)
                {
                    vec pos = v;
                    pos.add(vec(-6+rnd(13), -6+rnd(13), -3+rnd(7)));

                    if(!inWater) particle_splash(PART_SPARK, miniRockets ? 1 : 2, 300, pos, hasRoids ? 0xFF4444 : 0xFFBB55,  1.2f+rnd(2), 2500, 2500, -1, hasShrooms());
                    particle_splash(PART_FIRE_BALL, miniRockets ? 2 : 6, 150+rnd(75), pos, hasRoids ? 0xFF4444 : flamesColor[rnd(3)], 9.f+rnd(6), miniRockets ? 300+rnd(150) : 400+rnd(200), 800, 20.f, hasShrooms());

                    if(inWater)
                    {
                        particle_splash(PART_WATER, 16, 200, pos, 0x18181A, 12.0f+rnd(14), 600, 300);
                        particle_splash(PART_BUBBLE, 5, 150, pos, 0x18181A, 4.0f+rnd(8), 300, 150);
                    }

                }

                if(!epilepsyfriendly)
                {
                    adddynlight(v, 5*attacks[atk].exprad, vec(7.0f, 4.0f, 0.0f), 80, 40, flags, initRadius, vec(1.f, 0.f, 0.f));
                    adddynlight(v, 3*attacks[atk].exprad, vec(1.5f, 0.5f, 0.0f), 80, 40, L_VOLUMETRIC|flags, initRadius, vec(1.0f, 0.f, 0.f));
                }
                return;
            }
            case ATK_FIREWORKS:

                particle_splash(PART_FIRE_BALL, 5, 40, v, 0xFFC864, 5, 800, 1600, 0, hasShrooms());
                loopi(4) particle_splash(PART_SPARK_P, 16+rnd(10), 200+rnd(200), v, hasRoids ? 0xFF0000 : fireworkColor(), (0.2f + (rnd(5)*0.1f)), 500+rnd(300), 10, 1, hasShrooms());

                if(inWater)
                {
                    particle_splash(PART_WATER, 50, 200, v, 0x18181A, 12.0f+rnd(14), 600, 300);
                    particle_splash(PART_BUBBLE, 15, 150, v, 0x18181A, 4.0f+rnd(8), 300, 150);
                }

                if(!epilepsyfriendly)
                {
                    adddynlight(v, 5*attacks[atk].exprad, vec(1.0f, 1.0f, 0.6f), 80, 40, flags, initRadius, vec(1.0f, 1.0f, 0.6f));
                    adddynlight(v, 2*attacks[atk].exprad, vec(0.5f, 0.5f, 0.3f), 80, 40, L_VOLUMETRIC|flags, initRadius, vec(0.5f, 0.5f, 0.3f));
                }
                return;

            case ATK_M32:
                particle_splash(PART_AR, 6, 300, v, 0xFFFFFF, 85, 200, 200, 50);
                loopi(3) particle_splash(PART_SPARK, 8, 150+rnd(150), v, hasRoids ? 0xFF0000 : 0xFFFFFF,  1.2f, 1500+rnd(2250), 1500+rnd(2250), 0, hasShrooms());
                loopi(2) particle_splash(PART_SMOKE, 7, 1300+rnd(800), v, 0x555555, 40.0f, 150+rnd(150), 300+rnd(700), 0, hasShrooms());
                loopi(3) particle_fireball(v, 40+rnd(50), PART_PLASMAGRENADE, 300, hasRoids ? 0xFF0000 : 0xFFFFFF, 1.0f, hasShrooms());
                particle_fireball(v, 400, PART_SHOCKWAVE, 300, hasRoids ? 0xFF0000 : 0xFFFFFF, 20.0f, hasShrooms());

                if(inWater)
                {
                    particle_splash(PART_WATER, 50, 200, v, 0x18181A, 12.0f+rnd(14), 600, 300);
                    particle_splash(PART_BUBBLE, 15, 150, v, 0x18181A, 4.0f+rnd(8), 300, 150);
                }

                if(!epilepsyfriendly)
                {
                    adddynlight(v, 6*attacks[atk].exprad, vec(0.0f, 3.0f, 9.0f), 80, 40, flags, initRadius, vec(0.5f, 1.5f, 2.0f));
                    adddynlight(v, 4*attacks[atk].exprad, vec(0.0f, 0.5f, 1.5f), 80, 40, L_VOLUMETRIC|flags, initRadius, vec(0.0f, 0.0f, 1.5f));
                }
                return;

            case ATK_MOLOTOV:
            {
                particle_splash(PART_SMOKE, 8, 1800, v, 0x555555, 30.0f, 300+rnd(100), 1200+rnd(400), 5, hasShrooms());
                particle_splash(PART_SPARK, 5, 250, v, hasRoids ? 0xFF4444 : 0xFFBB55,  1.0f+rnd(2), 3500, 3500, 0, hasShrooms());
                loopi(inWater ? 1 : 4)
                {
                    vec pos = v;
                    if(!inWater) pos.add(vec(-30+rnd(61), -30+rnd(61), -10+rnd(21)));
                    particle_splash(PART_FIRE_BALL, 10, 150+rnd(50), pos, hasRoids ? 0xFF4444 : i==0 ? 0x484848 : i==1 ? 0x575757: 0x646464, 9.f+rnd(6), 800+rnd(400), 2000, 25, hasShrooms());
                }

                if(inWater)
                {
                    particle_splash(PART_WATER, 50, 200, v, 0x18181A, 12.0f+rnd(14), 600, 300);
                    particle_splash(PART_BUBBLE, 15, 150, v, 0x18181A, 4.0f+rnd(8), 300, 150);
                }

                if(!epilepsyfriendly)
                {
                    adddynlight(v, 3*attacks[atk].exprad, vec(6.0f, 3.0f, 0.0f), 150, 40, flags, initRadius, vec(2.0f, 1.5f, 0.5f));
                    adddynlight(v, 2*attacks[atk].exprad, vec(1.5f, 0.5f, 0.0f), 150, 40, L_VOLUMETRIC|flags, initRadius, vec(1.5f, 0.0f, 0.0f));
                }
                return;
            }
            case ATK_KAMIKAZE:
            case ATK_POWERARMOR:
            {
                bool kamikaze = (atk==ATK_KAMIKAZE);

                particle_splash(PART_AR, 6, 450, v, 0xCCCCCC, 60, 300, 800, 50);

                loopi(9)
                {
                    vec pos = vec(v).add(vec(rnd(35)-rnd(70), rnd(35)-rnd(70), rnd(35)-rnd(70)));
                    particle_splash(PART_SMOKE, 4, kamikaze ? 5000 : 3000, pos, 0x333333, 60.f, 200+rnd(75), 100, 0, hasShrooms());
                    particle_splash(PART_SMOKE, 3, kamikaze ? 3000 : 2000, pos, 0x151515, 40.f, 200+rnd(75), 250, 0, hasShrooms());
                    particle_splash(PART_SPARK, 6, 300, pos, hasRoids ? 0xFF4444 : 0xFFBB55,  1.7f+rnd(2), 3500, 3500, 0, hasShrooms());
                    loopi(inWater ? 1 : 3) particle_splash(PART_FIRE_BALL, 6, 130+rnd(50), pos, hasRoids ? 0xFF4444 : i==0 ? 0x383838: i==1 ? 0x474747 : 0x6A4A3A, 9.f+rnd(6), 1200+rnd(700), 1200, 20.f, hasShrooms());

                    if(inWater)
                    {
                        particle_splash(PART_WATER, 20, 200, pos, 0x18181A, 12.0f+rnd(14), 600, 300);
                        particle_splash(PART_BUBBLE, 10, 150, pos, 0x18181A, 4.0f+rnd(8), 300, 150);
                    }
                }

                particle_fireball(v, 400, PART_SHOCKWAVE, 300, hasRoids ? 0xFF0000 : 0xFFFFFF, 20.0f, hasShrooms());

                if(!epilepsyfriendly)
                {
                    adddynlight(v, 6*attacks[atk].exprad, vec(7.0f, 4.0f, 0.0f), 80, 40, flags, initRadius, vec(1.f, 0.f, 0.f));
                    adddynlight(v, 4*attacks[atk].exprad, vec(1.5f, 0.5f, 0.0f), 80, 40, L_VOLUMETRIC|flags, initRadius, vec(1.f, 0.f, 0.f));
                }
                return;
            }
            case ATK_S_NUKE:
                loopi(60)
                {
                    vec pos = v;

                    float radius = 7.f * i;
                    float theta = static_cast<float>(rnd(1000)) / 1000.0f * M_PI;
                    float phi = static_cast<float>(rnd(1000)) / 1000.0f * 2.0f * M_PI;
                    float r = static_cast<float>(rnd(1000)) / 1000.0f * radius;

                    if(i<30) theta = theta / 3.0f;
                    else theta = M_PI / 2 + theta / 3.0f;

                    pos.x += r * sin(theta) * cos(phi);
                    pos.y += r * sin(theta) * sin(phi);
                    pos.z += r * cos(theta);

                    if(!rnd(2)) particle_splash(PART_AR, 1, 3000, v, 0xCCCCCC, 150, 150, 300, 1);

                    particle_splash(PART_SMOKE, 1, 2000,  pos, 0x212121, 150.0f,  700,  70, 1, hasShrooms());
                    particle_splash(PART_SMOKE, 1, 15000, pos, 0x222222, 200.0f,  150, 300, 1, hasShrooms());
                    particle_splash(PART_SMOKE, 2, 5000,  pos, 0x333333, 250.0f, 1000, 500, 1, hasShrooms());
                    particle_splash(PART_FIRE_BALL, 7, 1000+rnd(300), pos, hasRoids ? 0xFF0000 : i<16 ? 0xFFFF00 : i<32 ? 0x224400 : 0xFFAA22, 20+rnd(15), 400, 200, 3.f, hasShrooms());
                    if(i>30) particle_fireball(pos, 50+rnd(150), PART_EXPLOSION, 750, hasRoids ? 0xFF0000 : i<37 ? 0xFFFF00 : i<43 ? 0x224400 : 0xFFAA22, 10+rnd(15), hasShrooms());
                }

                if(!epilepsyfriendly)
                {
                    loopi(3) particle_fireball(v, 350, PART_SHOCKWAVE, 600, 0xFFCCAA, 800.0f, hasShrooms());
                    adddynlight(v, 9*attacks[atk].exprad, vec(8.0f, 4.0f, 0.0f), 300, 40, L_NOSHADOW, initRadius, vec(0.5f, 1.5f, 2.0f));
                    adddynlight(v, 6*attacks[atk].exprad, vec(8.0f, 4.0f, 0.0f), 300, 40, L_NOSHADOW|L_VOLUMETRIC, initRadius, vec(0.0f, 0.0f, 1.5f));
                }
                return;
        }
    }

    vec getReflection(const vec pos, vec dir)
    {
        vec hitpos;
        raycubepos(pos, dir, hitpos, 200, RAY_CLIPMAT|RAY_POLY);
        vec surfaceNormal = hitsurface;
        return vec(dir.sub(surfaceNormal.mul(2 * dir.dot(surfaceNormal))));
    }

    void renderBulletImpact(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk) //particles and light effects on impact for fast projectiles
    {
        int distance = camera1->o.dist(v);
        if(distance < 15) return;

        vec dir = vec(vel).normalize();
        vec bounce;
        bool isClose = false;
        bool inWater = ((lookupmaterial(v) & MATF_VOLUME) == MAT_WATER);

        if(distance < 200) // if close, calculate a bouncing trajectory
        {
            vec r;
            isClose = true;
            bounce = getReflection(v, dir);
        }

        switch(atk)
        {
            case ATK_MINIGUN:
            case ATK_AK47:
            case ATK_UZI:
            case ATK_FAMAS:
            case ATK_GLOCK:
            case ATK_CROSSBOW:
            {
                bool bigGun = (atk == ATK_MINIGUN || atk == ATK_AK47);
                bool isCrossbow = (atk == ATK_CROSSBOW);

                if(isClose)
                {
                    if(!inWater && !isCrossbow) particles::dirSplash(PART_SPARK, owner->hasRoids() ? 0xFF0000 : 0xFF8800, 500, 6, 85, v, bounce, bigGun ? 0.3f : 0.2f, 150, 0, hasShrooms());
                    particles::dirSplash(PART_SMOKE_S, 0x565656, 150, 3, 600 + rnd(300), v, bounce, 3.f, 40, 2, hasShrooms());
                    particles::dirSplash(PART_SMOKE_S, 0x552900, 150, 3, 600 + rnd(300), v, bounce, 3.f, 40, 2, hasShrooms());
                }
                else
                {
                    if(!inWater && !isCrossbow) particle_splash(PART_SPARK, 5, 75, v, owner->hasRoids() ? 0xFF0000 : 0xFF8800, bigGun ? 0.3f : 0.2f, 150, 200, 0, hasShrooms());
                    particle_splash(PART_SMOKE, 3, 600+rnd(300), v, 0x565656, bigGun ? 0.35f : 0.3f, 25, 300, 2, hasShrooms());
                    particle_splash(PART_SMOKE, 6, 350+rnd(300), v, 0x552900, bigGun ? 0.35f : 0.3f, 15, 300, 2, hasShrooms());
                }
                return;
            }
            case ATK_SV98:
            case ATK_SKS:
            case ATK_S_GAU8:
                if(isClose)
                {
                    if(!inWater) particles::dirSplash(PART_SPARK, owner->hasRoids() ? 0xFF0000 : 0xFF5500, 800, 7, 100, v, bounce, 0.4f, 600, -0.5f, hasShrooms());
                    particles::dirSplash(PART_SMOKE_S, 0x414141, 150, 4, 800 + rnd(300), v, bounce, 6.f, 30, 2, hasShrooms());
                    particles::dirSplash(PART_SMOKE_S, 0x442200, 150, 4, 800 + rnd(300), v, bounce, 6.f, 30, 2, hasShrooms());
                }
                else
                {
                    if(!inWater) particle_splash(PART_SPARK, 5, 85, v, owner->hasRoids() ? 0xFF0000 : 0xFF5500, 0.5f, 150, 200, 0, hasShrooms());
                    particle_splash(PART_SMOKE, 3, 600+rnd(300), v, 0x414141, 0.4f, 25, 300, 2, hasShrooms());
                    particle_splash(PART_SMOKE, 3, 350+rnd(300), v, 0x442200, 0.4f, 15, 300, 2, hasShrooms());
                }
                return;
        }
    }

    void renderInstantImpact(const vec &from, const vec &to, const vec &muzzle, int atk, bool hasRoids) //particles and light effects on impact for instant projectiles
    {
        int distance = camera1->o.dist(to);
        if(distance < 15) return;

        vec dir = vec(from).sub(to).safenormalize();
        vec bounce;

        bool isClose = false;
        bool inWater = ((lookupmaterial(to) & MATF_VOLUME) == MAT_WATER);

        if(distance < 256) // if close, calculate a bouncing trajectory
        {
            isClose = true;
            bounce = getReflection(from, vec(to).sub(from).normalize());
        }

        switch(atk)
        {
            case ATK_ELECTRIC:
                if(!inWater)
                {
                    if(isClose) particles::dirSplash(PART_SPARK_L, 0x4488FF, 800, 20, 125, to, bounce, 0.5f, 600, -1, hasShrooms());
                    else
                    {
                        particle_splash(PART_SPARK, 40, 125, to, 0x4488FF, 0.04f, 400, 50, 0, hasShrooms());
                        adddynlight(vec(to).madd(dir, 4), 25, vec(0.25f, 0.50f, 1.25f), 200, 100, DL_EXPAND);
                    }
                }
                addstain(STAIN_ELEC_HOLE, to, dir, 3.5f);
                addstain(STAIN_ELEC_GLOW, to, dir, 1.0f, 0xFF2200);
                addstain(STAIN_ELEC_GLOW, to, dir, 1.5f, 0xFF8800);
                return;

            case ATK_MOSSBERG:
            case ATK_HYDRA:
                if(isClose)
                {
                    if(!inWater) particles::dirSplash(PART_SPARK, hasRoids ? 0xFF2222 : 0xFF4400, 500, 6, 85, to, bounce, 0.2f, 150, 0, hasShrooms());
                    particles::dirSplash(PART_SMOKE_S, 0x696969, 150, 2, 500 + rnd(300), to, bounce, 4.f, 40, 1, hasShrooms());
                    particles::dirSplash(PART_SMOKE_S, 0x452910, 150, 2, 500 + rnd(300), to, bounce, 4.f, 40, 1, hasShrooms());
                }
                else
                {
                    if(!inWater) particle_splash(PART_SPARK, 5, 75, to, hasRoids ? 0xFF2222 : 0xFF4400, 0.3f, 150, 100, 0, hasShrooms());
                    particle_splash(PART_SMOKE, 3, 500+rnd(300), to, 0x696969, 0.5f, 35, 200, 2, hasShrooms());
                    particle_splash(PART_SMOKE, 3, 500+rnd(300), to, 0x452910, 0.4f, 35, 200, 2, hasShrooms());
                }
                addstain(STAIN_BULLET_HOLE, to, dir, 0.4f);
                addstain(STAIN_BULLET_GLOW, to, dir, 0.8f, 0x991100);
                return;

            case ATK_FLAMETHROWER:
                if(!rnd(2)) addstain(STAIN_BURN, to, dir, 20.0f);
                return;

            case ATK_S_CAMPER:
                if(isClose)
                {
                    if(!inWater) particles::dirSplash(PART_SPARK, hasRoids ? 0xFF0000 : 0xFFAA00, 1000, 7, 100, to, bounce, 0.4f, 600, -0.5f, hasShrooms());
                    particles::dirSplash(PART_SMOKE_S, 0x797979, 150, 4, 900 + rnd(400), to, bounce, 6.f, 30, 2, hasShrooms());
                    particles::dirSplash(PART_SMOKE_S, 0x553915, 150, 4, 900 + rnd(400), to, bounce, 6.f, 30, 2, hasShrooms());
                }
                else
                {
                    if(!inWater) particle_splash(PART_SPARK, 9, 70, to, hasRoids ? 0xFF2222 : 0xFF9900, 0.6f, 150, 100, 0, hasShrooms());
                    particle_splash(PART_SMOKE, 4, 700+rnd(500), to, 0x797979, 0.2f, 35, 300, 2, hasShrooms());
                    particle_splash(PART_SMOKE, 4, 400+rnd(400), to, 0x553915, 0.15f, 35, 300, 2, hasShrooms());
                }
                addstain(STAIN_BULLET_HOLE, to, dir, 0.8f);
                addstain(STAIN_BULLET_GLOW, to, dir, 0.6f, 0xBB5500);
                return;
        }
    }

    float adaptMuzzleFlash(gameent *d) { return (d==game::hudplayer() && !thirdperson ? zoom ? 2.7f : 1.5f : 1.f)*(d->boostmillis[B_RAGE] ? 1.33f : 1.f); }

    void renderMuzzleEffects(const vec &from, const vec &to, gameent *d, int atk)
    {
        vec pos = d->character==C_SPY && d->abilitymillis[ABILITY_2] ? game::hudgunorigin(d->gunselect, d->o, to, d) : d->muzzle;
        int lightFlags = DL_FLASH|DL_SHRINK|L_NOSHADOW;
        bool wizardAbility = d->character==C_WIZARD && d->abilitymillis[ABILITY_2];
        bool increasedDamages = d->boostmillis[B_RAGE] || d->hasRoids();
        vec dir = vec(to).sub(from);

        switch(atk)
        {
            case ATK_ELECTRIC:
            {
                float mfSize = 4.f/adaptMuzzleFlash(d);
                loopi(2) particle_flare(pos, to, 50+rnd(50), PART_LIGHTNING, d->hasRoids() ? 0xFF0000 : 0x8888FF, 1.5f+rnd(2), NULL, hasShrooms());
                lightTrail(pos, to, 60, 50+rnd(50), 10, d->hasRoids() ? vec(2.5f, 0.f, 0.f) :  vec(0.2f, 0.6f, 2.f));
                particle_flare(pos, pos, 140, PART_MF_ELEC, increasedDamages ? 0xFF2222 : wizardAbility ? 0xFF22FF : 0x50CFFF, mfSize, d, hasShrooms());
                particle_flare(pos, pos, 200, PART_F_AR, 0xFFFFFF, mfSize, d, false, 10);
                adddynlight(pos, 100, vec(0.25f, 0.75f, 2.0f), 40, 2, lightFlags, 0, vec(0.25f, 0.75f, 2.0f), d);
                break;
            }
            case ATK_PLASMA:
            {
                float mfSize = 1.5f/adaptMuzzleFlash(d);
                particle_flare(pos, pos, 125, PART_MF_PLASMA, increasedDamages ? 0xFF4444 : wizardAbility ? 0xFF44FF : 0xFF7911, mfSize, d, hasShrooms(), 3);
                if(!d->gunaccel) particle_flare(pos, pos, 200, PART_F_AR, 0xFFFFFF, mfSize, d, false, 10);
                adddynlight(pos, 100, vec(1.25f, 0.2f, 0.0f), 40, 2, lightFlags, 100, vec(1.25f, 0.2f, 0.0f), d);
                break;
            }
            case ATK_SPOCKGUN:
            {
                float mfSize = 1.5f/adaptMuzzleFlash(d);
                particle_flare(pos, pos, 150, PART_MF_PLASMA, increasedDamages ? 0xFF2222 : wizardAbility ? 0xFF22FF : 0x22FF22, mfSize, d, hasShrooms());
                particle_flare(pos, pos, 200, PART_F_AR, 0xFFFFFF, mfSize, d, false, 10);
                adddynlight(pos, 75, vec(0.0f, 1.25f, 0.0f), 50, 2, lightFlags, 75, vec(0.0f, 1.25f, 0.0f), d);
                break;
            }

            case ATK_SMAW:
            case ATK_S_ROCKETS:
            case ATK_S_NUKE:
            case ATK_FIREWORKS:
            {
                float mfSize = 1.5f/adaptMuzzleFlash(d);
                particles::dirSplash(PART_SPARK, 0xFF5500, 300, 3 + rnd(3), 100, pos, dir, 0.4f, 400, -1, hasShrooms());
                particles::dirSplash(PART_SMOKE, atk == ATK_S_NUKE ? 0x202020 : 0x333333, 75, 2 + rnd(2), 1250, pos, dir, 2.5f, 30, 3, hasShrooms());
                adddynlight(pos, 100, vec(1.25f, 0.75f, 0.3f), 75, 2, lightFlags, 0, vec(1.25f, 0.75f, 0.3f), d);
                if(atk==ATK_FIREWORKS) break;
                particle_flare(pos, pos, 250, PART_MF_ROCKET, increasedDamages ? 0xFF2222 : wizardAbility ? 0xFF22FF : 0xFF7700, mfSize, d, hasShrooms(), 12);
                particle_flare(pos, pos, 600, PART_F_AR, 0xFFFFFF, mfSize, d, false, 10);
                break;
            }
            case ATK_MINIGUN:
            case ATK_AK47:
            case ATK_S_GAU8:
            {
                bool isGau = (atk == ATK_S_GAU8);
                float mfSize = 3.0f/adaptMuzzleFlash(d);
                particles::dirSplash(PART_SPARK, 0xFF5500, 250, 1 + rnd(3), 75, pos, dir, 0.4f, 400, -1, hasShrooms());
                particles::dirSplash(PART_SMOKE, isGau ? 0x282828 : 0x444444, 100, 1 + rnd(2), 750, pos, dir, 2.5f, 30, 3, hasShrooms());
                particle_flare(pos, pos, 100, PART_MF_BIG, increasedDamages ? 0xFF2222 : wizardAbility ? 0xFF22FF : 0xCCAAAA, mfSize - (d->gunaccel / 5.f), d, hasShrooms());
                if(!d->gunaccel) particle_flare(pos, pos, 200, PART_F_AR, 0xFFFFFF, mfSize, d, false, 10);
                adddynlight(pos, isGau ? 125 : 75, vec(1.25f, 0.75f, 0.3f), 35, 2, lightFlags, 0, vec(1.25f, 0.75f, 0.3f), d);
                break;
            }
            case ATK_UZI:
            case ATK_FAMAS:
            case ATK_GLOCK:
            {
                float mfSize = 2.0f/adaptMuzzleFlash(d);
                particles::dirSplash(PART_SPARK, 0xFF8800, 300, 1 + rnd(3), 500, pos, dir, 0.3f, 400, -1, hasShrooms());
                particles::dirSplash(PART_SMOKE, 0x444444, 300, 5, 500, pos, dir, 4.0f, 30, 2, hasShrooms());
                particle_flare(pos, pos, 125, PART_MF_LITTLE, increasedDamages ? 0xFF2222 : wizardAbility ? 0xFF22FF : 0xFFFFFF, mfSize / 2.f, d, hasShrooms());
                particle_flare(pos, pos, 75, PART_MF_BIG, increasedDamages ? 0xFF2222 : wizardAbility ? 0xFF22FF : 0xFFAA55, mfSize, d, hasShrooms());
                particle_flare(pos, pos, 200, PART_F_AR, 0xFFFFFF, mfSize, d, false, 10);
                adddynlight(pos, 60, vec(1.25f, 0.75f, 0.3f), 30, 2, lightFlags, 0, vec(1.25f, 0.75f, 0.3f), d);
                break;
            }
            case ATK_MOSSBERG:
            case ATK_HYDRA:
            {
                float mfSize = 1.0f/adaptMuzzleFlash(d);
                particles::dirSplash(PART_SPARK, 0xFF2200, 300, 7, 150, pos, dir, 0.4f, 400, -1, hasShrooms());
                particles::dirSplash(PART_SMOKE, 0x443333, 300, 4, 750, pos, dir, 3.5f, 30, 4, hasShrooms());
                particle_flare(pos, pos, 140, PART_MF_SHOTGUN, increasedDamages ? 0xFF2222 : wizardAbility ? 0xFF22FF : 0xCCAAAA, mfSize, d, hasShrooms(), 15);
                particle_flare(pos, pos, 300, PART_F_AR, 0xFFFFFF, (mfSize*2), d, false, 10);
                adddynlight(pos, 75, vec(1.25f, 0.25f, 0.f), 40, 2, lightFlags, 0, vec(1.25f, 0.25f, 0.f), d);
                break;
            }
            case ATK_SV98:
            case ATK_SKS:
            case ATK_S_CAMPER:
            {
                float mfSize = 5.0f/adaptMuzzleFlash(d);
                particles::dirSplash(PART_SPARK, 0xFFAA00, 400, 7, 50, pos, dir, 0.3f, 400, -1, hasShrooms());
                particles::dirSplash(PART_SMOKE, 0x222222, 300, 6, 600, pos, dir, 3.5f, 30, 3, hasShrooms());
                particle_flare(pos, pos, 100, PART_MF_LITTLE, increasedDamages ? 0xFF2222 : wizardAbility ? 0xFF22FF : 0xFFFFFF, 1.25f, d, hasShrooms());
                particle_flare(pos, pos, 100, PART_MF_SNIPER, increasedDamages ? 0xFF2222 : wizardAbility ? 0xFF22FF : 0xFFFFFF, (atk==ATK_S_CAMPER ? mfSize * 1.2 : mfSize), d, hasShrooms());
                particle_flare(pos, pos, 400, PART_F_AR, 0xFFFFFF, (mfSize*2), d, false, 10);
                adddynlight(pos, 50, vec(1.25f, 0.75f, 0.3f), 37, 2, lightFlags, 0, vec(1.25f, 0.75f, 0.3f), d);
                break;
            }
            case ATK_FLAMETHROWER:
            {
                float mfSize = 1.8f/adaptMuzzleFlash(d);
                particle_flare(pos, pos, 175, PART_MF_ROCKET, increasedDamages ? 0x880000 : wizardAbility ? 0x440044 : 0x663311, mfSize, d, hasShrooms(), 7);
                particle_flare(pos, pos, 1500, PART_F_AR, 0xFFFFFF, mfSize * 2, d);
                adddynlight(pos, 50, vec(0.6f, 0.3f, 0.1f), 100, 100, lightFlags, 10, vec(0.4f, 0, 0), d);
                break;
            }
            case ATK_GRAP1:
            {
                particle_flare(pos, pos, 150, PART_MF_PLASMA, increasedDamages ? 0xFF4444 : wizardAbility ? 0xFF00FF : 0xFF55FF, 1.75f, d, hasShrooms());
                particle_flare(pos, pos, 600, PART_F_AR, 0xFFFFFF, 1.5f, d, false, 10);
                adddynlight(pos, 70, vec(1.0f, 0.0f, 1.0f), 80, 100, lightFlags, 0, vec(0, 0, 0), d);
                break;
            }
        }
    }
}
