//gfx.cpp: fancy and laggy graphics effects

#include "gfx.h"

VARP(epilepsyfriendly, 0, 0, 1);

void setcbfilter(int i)
{
    clearpostfx();
    addpostfx("mainfilter", 1, 1, 1, 1, vec4(1, 1, 1, 1));

    switch(i)
    {
        case 1: addpostfx("protanopia", 1, 1, 1, 1, vec4(1, 1, 1, 1)); break;
        case 2: addpostfx("deuteranopia", 1, 1, 1, 1, vec4(1, 1, 1, 1)); break;
        case 3: addpostfx("tritanopia", 1, 1, 1, 1, vec4(1, 1, 1, 1)); break;
        case 4: addpostfx("achromatopsia", 1, 1, 1, 1, vec4(1, 1, 1, 1));
    }
}

VARFP(cbcompensation, 0, 0, 4, if(!islaunching) setcbfilter(cbcompensation));

namespace gfx
{
    VAR(forcecampos, -1, -1, 1000);
    VAR(zoom, -1, 0, 1);

    int weapposside, weapposup, nbfps = 60;
    int zoomfov = 50;

    bool champicolor() { return isconnected() && game::hudplayer()->boostmillis[B_SHROOMS]; } //checks if player 1 or observed player is on shrooms.
    bool hasroids(gameent *owner) { return owner->boostmillis[B_ROIDS]; }

    void light_trail(const vec &s, const vec &e, int radius, int fade, int peak, const vec &color) //cast lights along a ray
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

    void projgunexplosion(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk) //particles and light effects on impact for slow projectiles
    {
        vec lightloc = vec(v).sub(vec(vel).mul(10));

        switch(atk)
        {
            case ATK_PULSE_SHOOT:
            {
                particle_splash(PART_FIRESPARK, 15, 150, v, hasroids(owner) ? 0xFF0000 : 0xFF6600, 1.f, 150, 500, 2, champicolor());
                particle_fireball(v, 9.f, PART_PLASMABURST, 300, hasroids(owner) ? 0xFF0000 : 0xCC9900, 3, champicolor());
                adddynlight(safe ? v : lightloc, 2*attacks[atk].exprad, vec(1.5f, 0.75f, 0.0f), 150, 50, 0, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                return;
            }
            case ATK_GRAP1_SHOOT:
            {
                particle_splash(PART_SPARK, 20, 100, v, hasroids(owner) ? 0xFF0000 : 0xAA4466, 0.5f, 400, 400, 0, champicolor());
                loopi(2)particle_fireball(v, 7.f, PART_EXPLOSION, 200, hasroids(owner) ? 0xFF0000 : 0x550055, 1.f, champicolor());
                adddynlight(safe ? v : lightloc, 2*attacks[atk].exprad, vec(1.5f, 0.0f, 1.5f), 200, 100, L_NODYNSHADOW, attacks[atk].exprad/2, vec(0.5f, 0.0f, 0.5f));
                return;
            }
            case ATK_SPOCKGUN_SHOOT:
            {
                particle_splash(PART_SPARK, 15, 100, v, hasroids(owner) ? 0xFF0000 : 0x00FF66, 0.5f, 250, 250, 0, champicolor());
                adddynlight(safe ? v : lightloc, 1.25f*attacks[atk].exprad, vec(0.0f, 1.5f, 0.0f), 200, 50, 0, attacks[atk].exprad/2, vec(1.f, 1.f, 1.f));
                return;
            }
        }
    }

    void projexplosion(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk) //big laggy flashy explosions
    {
        vec lightloc = vec(v).sub(vec(vel).mul(15));

        switch(atk)
        {
            case ATK_SMAW_SHOOT:
            case ATK_ROQUETTES_SHOOT:

                particle_splash(PART_SMOKE, atk==ATK_ROQUETTES_SHOOT ? 5 : 9, 2000, v, 0x333333, 40.0f, 150+rnd(50), 300+rnd(100), 0, champicolor());
                particle_splash(PART_SMOKE, atk==ATK_ROQUETTES_SHOOT ? 5 : 9, 1300, v, 0x333333, 25.0f, 150+rnd(50), 600+rnd(100), 0, champicolor());
                particle_splash(PART_SPARK, atk==ATK_ROQUETTES_SHOOT ? 7 : 10, 300, v, hasroids(owner) ? 0xFF4444 : 0xFFBB55,  1.7f+rnd(2), 3500, 3500, 0, champicolor());
                loopi(lookupmaterial(v)==MAT_WATER ? 1 : 3) particle_splash(PART_FIRE_BALL, atk==ATK_ROQUETTES_SHOOT ? 9 : 17, 80+rnd(40), v, hasroids(owner) ? 0xFF4444 : i==0 ? 0x383838 : i==1 ? 0x474747: 0x604930, 9.f+rnd(6), atk==ATK_ROQUETTES_SHOOT ? 300+rnd(150) : 400+rnd(200), 800, 20.f, champicolor());

                particle_fireball(v, 350, PART_SHOCKWAVE, 300, hasroids(owner) ? 0xFF0000 : 0xFFFFFF, 20.0f, champicolor());

                if(lookupmaterial(v)==MAT_WATER)
                {
                    particle_splash(PART_WATER, 40, 200, v, 0x18181A, 12.0f+rnd(14), 600, 300);
                    particle_splash(PART_BUBBLE, 15, 150, v, 0x18181A, 4.0f+rnd(8), 300, 150);
                }

                if(!epilepsyfriendly)
                {
                    adddynlight(safe ? v : lightloc, 5*attacks[atk].exprad, vec(7.0f, 4.0f, 0.0f), 80, 40, L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(1.f, 0.f, 0.f));
                    adddynlight(safe ? v : lightloc, 3*attacks[atk].exprad, vec(1.5f, 0.5f, 0.0f), 80, 40, L_VOLUMETRIC|L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(1.0f, 0.f, 0.f));
                }
                return;

            case ATK_ARTIFICE_SHOOT:

                particle_splash(PART_FIRE_BALL, 5, 40, v, 0xFFC864, 5, 800, 1600, 0, champicolor());
                loopi(4) particle_splash(PART_SPARK, 16+rnd(10), 200+rnd(200), v, hasroids(owner) ? 0xFF0000 : rndcolor[rnd(6)].color, 0.2f+(rnd(5)/10.f), 500+rnd(300), 5000+rnd(3000), 2.f, champicolor());

                if(lookupmaterial(v)==MAT_WATER)
                {
                    particle_splash(PART_WATER, 50, 200, v, 0x18181A, 12.0f+rnd(14), 600, 300);
                    particle_splash(PART_BUBBLE, 15, 150, v, 0x18181A, 4.0f+rnd(8), 300, 150);
                }

                if(!epilepsyfriendly)
                {
                    adddynlight(safe ? v : lightloc, 6*attacks[atk].exprad, vec(1+rnd(4), 1+rnd(4), 1+rnd(4)), 80, 40, 0, attacks[atk].exprad/2, vec(1+rnd(4), 1+rnd(4), 1+rnd(4)));
                    adddynlight(safe ? v : lightloc, 5*attacks[atk].exprad, vec(rnd(15)/10.0f, rnd(15)/10.0f, rnd(15)/10.0f), 80, 40, L_VOLUMETRIC|L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(1+rnd(4), 1+rnd(4), 1+rnd(4)));
                }
                return;

            case ATK_M32_SHOOT:

                loopi(3) particle_splash(PART_SPARK, 8, 150+rnd(150), v, hasroids(owner) ? 0xFF0000 : 0xFFFFFF,  2.0f+rnd(2), 1500+rnd(2250), 1500+rnd(2250), 0, champicolor());
                loopi(2) particle_splash(PART_SMOKE, 7, 1300+rnd(800), v, 0x555555, 40.0f, 150+rnd(150), 300+rnd(700), 0, champicolor());

                loopi(3) particle_fireball(v, 40+rnd(50), PART_PLASMAGRENADE, 300, hasroids(owner) ? 0xFF0000 : 0xFFFFFF, 1.0f, champicolor());
                particle_fireball(v, 400, PART_SHOCKWAVE, 300, hasroids(owner) ? 0xFF0000 : 0xFFFFFF, 20.0f, champicolor());

                if(lookupmaterial(v)==MAT_WATER)
                {
                    particle_splash(PART_WATER, 50, 200, v, 0x18181A, 12.0f+rnd(14), 600, 300);
                    particle_splash(PART_BUBBLE, 15, 150, v, 0x18181A, 4.0f+rnd(8), 300, 150);
                }

                if(!epilepsyfriendly)
                {
                    adddynlight(v, 6*attacks[atk].exprad, vec(0.0f, 3.0f, 9.0f), 80, 40, L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                    adddynlight(v, 4*attacks[atk].exprad, vec(0.0f, 0.5f, 1.5f), 80, 40, L_VOLUMETRIC|L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(0.0f, 0.0f, 1.5f));
                }
                return;

            case ATK_KAMIKAZE_SHOOT:
            case ATK_ASSISTXPL_SHOOT:

                loopi(9)
                {
                    vec pos = vec(v).add(vec(rnd(35)-rnd(70), rnd(35)-rnd(70), rnd(35)-rnd(70)));
                    particle_splash(PART_SMOKE, 4, atk==ATK_KAMIKAZE_SHOOT ? 5000 : 3000, pos, 0x333333, 60.f, 200+rnd(75), 100, 0, champicolor());
                    particle_splash(PART_SMOKE, 3, atk==ATK_KAMIKAZE_SHOOT ? 3000 : 2000, pos, 0x151515, 40.f, 200+rnd(75), 250, 0, champicolor());
                    particle_splash(PART_SPARK, 6, 300, pos, hasroids(owner) ? 0xFF4444 : 0xFFBB55,  1.7f+rnd(2), 3500, 3500, 0, champicolor());
                    loopi(lookupmaterial(v)==MAT_WATER ? 1 : 3) particle_splash(PART_FIRE_BALL, 6, 130+rnd(50), pos, hasroids(owner) ? 0xFF4444 : i==0 ? 0x383838: i==1 ? 0x474747 : 0x6A4A3A, 9.f+rnd(6), 1200+rnd(700), 1200, 20.f, champicolor());

                    if(lookupmaterial(pos)==MAT_WATER)
                    {
                        particle_splash(PART_WATER, 20, 200, pos, 0x18181A, 12.0f+rnd(14), 600, 300);
                        particle_splash(PART_BUBBLE, 10, 150, pos, 0x18181A, 4.0f+rnd(8), 300, 150);
                    }
                }

                particle_fireball(v, 400, PART_SHOCKWAVE, 300, hasroids(owner) ? 0xFF0000 : 0xFFFFFF, 20.0f, champicolor());

                if(!epilepsyfriendly)
                {
                    adddynlight(safe ? v : lightloc, 6*attacks[atk].exprad, vec(7.0f, 4.0f, 0.0f), 80, 40, L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(1.f, 0.f, 0.f));
                    adddynlight(safe ? v : lightloc, 4*attacks[atk].exprad, vec(1.5f, 0.5f, 0.0f), 80, 40, L_VOLUMETRIC|L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(1.f, 0.f, 0.f));
                }
                return;

            case ATK_NUKE_SHOOT:
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

                    particle_splash(PART_SMOKE, 1, 2000,  pos, 0x212121, 150.0f,  700,  70, 1, champicolor());
                    particle_splash(PART_SMOKE, 1, 15000, pos, 0x222222, 200.0f,  150, 300, 1, champicolor());
                    particle_splash(PART_SMOKE, 2, 5000,  pos, 0x333333, 250.0f, 1000, 500, 1, champicolor());
                    particle_splash(PART_FIRE_BALL, 7, 1000+rnd(300), pos, hasroids(owner) ? 0xFF0000 : i<16 ? 0xFFFF00 : i<32 ? 0x224400 : 0xFFAA22, 20+rnd(15), 400, 200, 3.f, champicolor());
                    if(i>30) particle_fireball(pos, 50+rnd(150), PART_EXPLOSION, 750, hasroids(owner) ? 0xFF0000 : i<37 ? 0xFFFF00 : i<43 ? 0x224400 : 0xFFAA22, 10+rnd(15), champicolor());
                }

                if(!epilepsyfriendly)
                {
                    loopi(3) particle_fireball(v, 350, PART_SHOCKWAVE, 600, 0xFFCCAA, 800.0f, champicolor());
                    adddynlight(safe ? v : lightloc, 9*attacks[atk].exprad, vec(8.0f, 4.0f, 0.0f), 300, 40, L_NOSHADOW, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                    adddynlight(safe ? v : lightloc, 6*attacks[atk].exprad, vec(8.0f, 4.0f, 0.0f), 300, 40, L_NOSHADOW|L_VOLUMETRIC, attacks[atk].exprad/2, vec(0.0f, 0.0f, 1.5f));
                }
                return;
        }
    }

    void projgunhit(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk) //particles and light effects on impact for fast projectiles
    {
        switch(atk)
        {
            case ATK_MINIGUN_SHOOT:
            case ATK_AK47_SHOOT:
            case ATK_UZI_SHOOT:
            case ATK_FAMAS_SHOOT:
            case ATK_GLOCK_SHOOT:
            case ATK_ARBALETE_SHOOT:
                if(atk!=ATK_ARBALETE_SHOOT)particle_splash(PART_SPARK, atk==ATK_MINIGUN_SHOOT || atk==ATK_AK47_SHOOT ? 12 : 9, 50, v, hasroids(owner) ? 0xFF0000 : 0xFF8800, atk==ATK_MINIGUN_SHOOT || atk==ATK_AK47_SHOOT ? 0.3 : 0.2f, 150, 200, 0, champicolor());
                particle_splash(PART_SMOKE, 3, 600+rnd(300), v, 0x565656, ATK_MINIGUN_SHOOT==1 || ATK_AK47_SHOOT==1 ? 0.35f : 0.3f, 25, 300, 2, champicolor());
                particle_splash(PART_SMOKE, 6, 350+rnd(300), v, 0x552900, ATK_MINIGUN_SHOOT==1 || ATK_AK47_SHOOT==1 ? 0.35f : 0.3f, 15, 300, 2, champicolor());
                return;

            case ATK_SV98_SHOOT:
            case ATK_SKS_SHOOT:
            case ATK_GAU8_SHOOT:
                particle_splash(PART_SPARK, 12, 80, v, hasroids(owner) ? 0xFF0000 : 0xFF5533, 0.6f,  400, 250, 0, champicolor());
                particle_splash(PART_SMOKE,  4, 800+rnd(300), v, 0x414141, 0.4f, 25, 300, 2, champicolor());
                particle_splash(PART_SMOKE,  4, 500+rnd(300), v, 0x442200, 0.4f, 15, 300, 2, champicolor());
                return;
        }
    }

    void instantrayhit(const vec &from, const vec &to, const vec &muzzle, int atk) //particles and light effects on impact for instant projectiles
    {
        vec dir = vec(from).sub(to).safenormalize();
        //vec stainloc = vec(to).msub(dir, 2);

        switch(atk)
        {
            case ATK_RAIL_SHOOT:
                particle_splash(PART_SPARK, 50, 150, to, 0xFF4400, 0.45f, 300, 30, 0, champicolor());
                addstain(STAIN_ELEC_HOLE, to, dir, 3.5f);
                addstain(STAIN_ELEC_GLOW, to, dir, 1.5f, 0xFF2200);
                addstain(STAIN_ELEC_GLOW, to, dir, 2.5f, 0xFF8800);
                adddynlight(vec(to).madd(dir, 4), 10, vec(1.00f, 0.5f, 0.0f), 225, 75);
                return;

            case ATK_MOSSBERG_SHOOT:
            case ATK_HYDRA_SHOOT:
                //if(lookupmaterial(stainloc)==MAT_GLASS) addstain(STAIN_BALLE_GLOW, to, dir, 5.f, 0xFF2200);
                addstain(STAIN_BULLET_HOLE, to, dir, 0.4f);
                addstain(STAIN_BULLET_GLOW, to, dir, 0.8f, 0x991100);
                return;

            case ATK_LANCEFLAMMES_SHOOT:
                if(randomevent(2)) addstain(STAIN_BURN, to, dir, 20.0f);
                return;
        }
    }

    float adaptpartsize(gameent *d) { return (d==game::hudplayer() && !thirdperson ? zoom ? 3.f : 1.5f : 1.f)*(d->boostmillis[B_RAGE] ? 1.33f : 1.f); }

    void shootgfx(const vec &from, const vec &to, gameent *d, int atk)
    {
        vec muzzlepos = d->aptitude==APT_ESPION && d->abilitymillis[game::ABILITY_2] ? game::hudgunorigin(d->gunselect, d->o, to, d) : d->muzzle;
        int lightflags = DL_FLASH|DL_SHRINK|L_NOSHADOW;

        switch(atk)
        {
            case ATK_PULSE_SHOOT:
                particle_flare(muzzlepos, muzzlepos, 100, PART_MF_PLASMA, d->boostmillis[B_RAGE] || hasroids(d) ? 0xFF4444 : d->abilitymillis[game::ABILITY_2] && d->aptitude==APT_MAGICIEN ? 0xFF44FF : 0xFF7911, 2.f/adaptpartsize(d), d, champicolor());
                adddynlight(muzzlepos, 100, vec(1.25f, 0.2f, 0.0f), 40, 2, lightflags, 100, vec(1.25f, 0.2f, 0.0f), d);
                break;
            case ATK_SPOCKGUN_SHOOT:
                particle_flare(muzzlepos, muzzlepos, 150, PART_MF_PLASMA, d->boostmillis[B_RAGE] || hasroids(d) ? 0xFF2222 : d->abilitymillis[game::ABILITY_2] && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0x22FF22, 1.5f/adaptpartsize(d), d, champicolor());
                adddynlight(muzzlepos, 75, vec(0.0f, 1.25f, 0.0f), 50, 2, lightflags, 75, vec(0.0f, 1.25f, 0.0f), d);
                break;
            case ATK_RAIL_SHOOT:
                loopi(2) particle_flare(muzzlepos, to, 50+rnd(50), PART_LIGHTNING, 0x8888FF, 1.5f+rnd(2), NULL, gfx::champicolor());
                light_trail(muzzlepos, to, 60, 50+rnd(50), 10, vec(0.2f, 0.6f, 2.f));
                particle_flare(muzzlepos, muzzlepos, 140, PART_MF_ELEC, d->boostmillis[B_RAGE] || hasroids(d) ? 0xFF2222 : d->abilitymillis[game::ABILITY_2] && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0x50CFFF, 4.f/adaptpartsize(d), d, champicolor());
                adddynlight(muzzlepos, 100, vec(0.25f, 0.75f, 2.0f), 40, 2, lightflags, 0, vec(0.25f, 0.75f, 2.0f), d);
                break;
            case ATK_SMAW_SHOOT:
            case ATK_ROQUETTES_SHOOT:
            case ATK_NUKE_SHOOT:
            case ATK_ARTIFICE_SHOOT:
                if(atk==ATK_ARBALETE_SHOOT)
                {
                    vec dest = to;
                    dest.sub(muzzlepos);
                    dest.normalize().mul(800.0f);
                    loopi(5+rnd(5))
                    {
                        dest.add(vec(-150+rnd(300), -150+rnd(300), -150+rnd(300)));
                        particle_flying_flare(muzzlepos, dest, 50, PART_SPARK, 0xFFFF00, rnd(3)/10.f + 0.1f, 100, 0, champicolor());
                    }
                }
                else
                {
                    particle_flare(muzzlepos, muzzlepos, 250, PART_MF_ROCKET, d->boostmillis[B_RAGE] || hasroids(d) ? 0xFF2222 : d->abilitymillis[game::ABILITY_2] && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0xFF7700, 3.5f/adaptpartsize(d), d, champicolor());
                    adddynlight(muzzlepos, 100, vec(1.25f, 0.75f, 0.3f), 75, 2, lightflags, 0, vec(1.25f, 0.75f, 0.3f), d);
                }
                break;
            case ATK_MINIGUN_SHOOT:
            case ATK_AK47_SHOOT:
            case ATK_GAU8_SHOOT:
                vec dest = to;
                dest.sub(muzzlepos);
                dest.normalize().mul(800.0f);
                loopi(atk==ATK_GAU8_SHOOT ? 1 : 2+rnd(3))
                {
                    dest.add(vec(-150+rnd(300), -150+rnd(300), -150+rnd(300)));
                    particle_flying_flare(muzzlepos, dest, 100, PART_SPARK, 0xFF7700, rnd(3)/10.f + 0.1f, 100, 0, champicolor());
                    particle_flying_flare(muzzlepos, dest, 300, PART_SMOKE, atk==ATK_GAU8_SHOOT ? 0x282828 : 0x444444, 2.f, 500, 4, champicolor());
                }
                particle_flare(muzzlepos, muzzlepos, 100, PART_MF_BIG, d->boostmillis[B_RAGE] || hasroids(d) ? 0xFF2222 : d->abilitymillis[game::ABILITY_2] && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0xCCAAAA, 3.5f/adaptpartsize(d), d, champicolor());
                adddynlight(muzzlepos, atk==ATK_GAU8_SHOOT ? 125 : 75, vec(1.25f, 0.75f, 0.3f), 35, 2, lightflags, 0, vec(1.25f, 0.75f, 0.3f), d);
                break;
        }
    }

    void resetpostfx()
    {
        clearpostfx();
        addpostfx("mainfilter", 1, 1, 1, 1, vec4(1, 1, 1, 1));
        switch(cbcompensation)
        {
            case 1: addpostfx("protanopia", 1, 1, 1, 1, vec4(1, 1, 1, 1)); break;
            case 2: addpostfx("deuteranopia", 1, 1, 1, 1, vec4(1, 1, 1, 1)); break;
            case 3: addpostfx("tritanopia", 1, 1, 1, 1, vec4(1, 1, 1, 1)); break;
            case 4: addpostfx("achromatopsia", 1, 1, 1, 1, vec4(1, 1, 1, 1));
        }
        fullbrightmodels = 0;
        setShroomsEfx(false);
    }

    char *getshielddir(int armourtype, int armourval, bool hud, bool preload) //récupère l'id d'un bouclier
    {
        static char dir[64];
        int armourvaldir = 20;

        if(!preload)
        {
            loopi(4)
            {
                if(armourval < armours[armourtype].armoursteps) break;
                armourval -= armours[armourtype].armoursteps;
                armourvaldir += 20;
            }
        }

        if(armourtype==A_ASSIST && hud) sprintf(dir, "%s%d", "hudshield/power/", preload ? armourval : armourvaldir);
        else if(armourtype!=A_ASSIST) sprintf(dir, "%s%s%d", hud ? "hudshield/" : "shields/", armours[armourtype].armournames, preload ? armourval : armourvaldir);
        return dir;
    }
}
