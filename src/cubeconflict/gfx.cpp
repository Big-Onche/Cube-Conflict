//gfx.cpp: fancy and laggy graphics effects

#include "ccheader.h"

VARP(epilepsyfriendly, 0, 0, 1);

namespace gfx
{
    bool champicolor = isconnected() ? game::hudplayer()->champimillis : false;

    void projgunexplosion(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk)
    {
        vec lightloc = vec(v).sub(vec(vel).mul(10));

        switch(atk)
        {
            case ATK_PULSE_SHOOT:
            {
                particle_splash(PART_FIRESPARK, 15, 150, v, owner->steromillis ? 0xFF0000 : 0xFF6600, 1.f, 150, 500, 2, champicolor);
                particle_fireball(v, 7, PART_PULSE_BURST, 300, owner->steromillis ? 0xFF0000 : 0xCC9900, 3, champicolor);
                adddynlight(safe ? v : lightloc, 2*attacks[atk].exprad, vec(1.5f, 0.75f, 0.0f), 150, 50, 0, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                return;
            }
            case ATK_GRAP1_SHOOT:
            {
                particle_splash(PART_GLOWSPARK, 20, 100, v, owner->steromillis ? 0xFF0000 : 0xAA4466, 0.5f, 400, 400, 0, champicolor);
                loopi(2)particle_fireball(v, 5, PART_EXPLOSION, 200, owner->steromillis ? 0xFF0000 : 0x550055, 1.f, champicolor);
                adddynlight(safe ? v : lightloc, 2*attacks[atk].exprad, vec(1.5f, 0.0f, 1.5f), 200, 100, L_NODYNSHADOW, attacks[atk].exprad/2, vec(0.5f, 0.0f, 0.5f));
                return;
            }
            case ATK_SPOCKGUN_SHOOT:
            {
                particle_splash(PART_GLOWSPARK, 15, 100, v, owner->steromillis ? 0xFF0000 : 0x00FF66, 0.5f, 250, 250, 0, champicolor);
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

                particle_splash(PART_SMOKE, atk==ATK_ROQUETTES_SHOOT ? 5 : 9, 2000, v, 0x333333, 40.0f, 150+rnd(50), 300+rnd(100), 0, champicolor);
                particle_splash(PART_SMOKE, atk==ATK_ROQUETTES_SHOOT ? 5 : 9, 1300, v, 0x333333, 25.0f, 150+rnd(50), 600+rnd(100), 0, champicolor);
                particle_splash(PART_SPARK, atk==ATK_ROQUETTES_SHOOT ? 7 : 10, 300, v, owner->steromillis ? 0xFF4444 : 0xFFBB55,  1.7f+rnd(2), 3500, 3500, 0, champicolor);
                loopi(lookupmaterial(v)&MAT_WATER ? 1 : 3) particle_splash(PART_FLAME1+rnd(2), atk==ATK_ROQUETTES_SHOOT ? 9 : 17, 80+rnd(40), v, owner->steromillis ? 0xFF4444 : i==0 ? 0x383838 : i==1 ? 0x474747: 0x604930, 9.f+rnd(6), atk==ATK_ROQUETTES_SHOOT ? 300+rnd(150) : 400+rnd(200), 800, 1, champicolor);

                particle_fireball(v, 350, PART_ONDECHOC, 300, owner->steromillis ? 0xFF0000 : 0xFFFFFF, 20.0f, champicolor);

                if((lookupmaterial(v)&MAT_WATER))
                {
                    particle_splash(PART_EAU, 40, 200, v, 0x18181A, 12.0f+rnd(14), 600, 300);
                    particle_splash(PART_BULLE, 15, 150, v, 0x18181A, 4.0f+rnd(8), 300, 150);
                }

                if(!epilepsyfriendly)
                {
                    adddynlight(safe ? v : lightloc, 5*attacks[atk].exprad, vec(7.0f, 4.0f, 0.0f), 80, 40, L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(1.f, 0.f, 0.f));
                    adddynlight(safe ? v : lightloc, 3*attacks[atk].exprad, vec(1.5f, 0.5f, 0.0f), 80, 40, L_VOLUMETRIC|L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(1.0f, 0.f, 0.f));
                }
                return;

            case ATK_ARTIFICE_SHOOT:

                particle_splash(PART_FLAME1+rnd(2), 5, 40, v, 0xFFC864, 5, 800, 1600, 0, champicolor);
                loopi(4) particle_splash(PART_GLOWSPARK, 16+rnd(10), 200+rnd(200), v, owner->steromillis ? 0xFF0000 : rndcolor[rnd(6)].color,  0.4f+(rnd(5)/10.f), 500+rnd(300), 5000+rnd(3000), 2, champicolor);

                if(lookupmaterial(v)&MAT_WATER)
                {
                    particle_splash(PART_EAU, 50, 200, v, 0x18181A, 12.0f+rnd(14), 600, 300);
                    particle_splash(PART_BULLE, 15, 150, v, 0x18181A, 4.0f+rnd(8), 300, 150);
                }

                if(!epilepsyfriendly)
                {
                    adddynlight(safe ? v : lightloc, 6*attacks[atk].exprad, vec(1+rnd(4), 1+rnd(4), 1+rnd(4)), 80, 40, 0, attacks[atk].exprad/2, vec(1+rnd(4), 1+rnd(4), 1+rnd(4)));
                    adddynlight(safe ? v : lightloc, 5*attacks[atk].exprad, vec(rnd(15)/10.0f, rnd(15)/10.0f, rnd(15)/10.0f), 80, 40, L_VOLUMETRIC|L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(1+rnd(4), 1+rnd(4), 1+rnd(4)));
                }
                return;

            case ATK_M32_SHOOT:
                loopi(3) particle_splash(PART_SPARK, 8, 150+rnd(150), v, owner->steromillis ? 0xFF0000 : 0xFFFFFF,  2.0f+rnd(2), 1500+rnd(2250), 1500+rnd(2250), 0, champicolor);
                loopi(2) particle_splash(PART_SMOKE, 7, 1300+rnd(800), v, 0x555555, 40.0f, 150+rnd(150), 300+rnd(700), 0, champicolor);

                loopi(3) particle_fireball(v, 40+rnd(50), PART_PLASMA, 300, owner->steromillis ? 0xFF0000 : 0xFFFFFF, 1.0f, champicolor);
                particle_fireball(v, 400, PART_ONDECHOC, 300, owner->steromillis ? 0xFF0000 : 0xFFFFFF, 20.0f, champicolor);

                if(lookupmaterial(v)&MAT_WATER)
                {
                    particle_splash(PART_EAU, 50, 200, v, 0x18181A, 12.0f+rnd(14), 600, 300);
                    particle_splash(PART_BULLE, 15, 150, v, 0x18181A, 4.0f+rnd(8), 300, 150);
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
                    particle_splash(PART_SMOKE, 4, atk==ATK_KAMIKAZE_SHOOT ? 5000 : 3000, pos, 0x333333, 60.f, 200+rnd(75), 100, 0, champicolor);
                    particle_splash(PART_SMOKE, 3, atk==ATK_KAMIKAZE_SHOOT ? 3000 : 2000, pos, 0x151515, 40.f, 200+rnd(75), 250, 0, champicolor);
                    particle_splash(PART_SPARK, 6, 300, pos, owner->steromillis ? 0xFF4444 : 0xFFBB55,  1.7f+rnd(2), 3500, 3500, 0, champicolor);
                    loopi(lookupmaterial(v)&MAT_WATER ? 1 : 3) particle_splash(PART_FLAME1+rnd(2), 6, 130+rnd(50), pos, owner->steromillis ? 0xFF4444 : i==0 ? 0x383838: i==1 ? 0x474747 : 0x6A4A3A, 9.f+rnd(6), 1200+rnd(700), 1200, 1, champicolor);

                    if((lookupmaterial(pos)&MAT_WATER))
                    {
                        particle_splash(PART_EAU, 20, 200, pos, 0x18181A, 12.0f+rnd(14), 600, 300);
                        particle_splash(PART_BULLE, 10, 150, pos, 0x18181A, 4.0f+rnd(8), 300, 150);
                    }
                }

                particle_fireball(v, 400, PART_ONDECHOC, 300, owner->steromillis ? 0xFF0000 : 0xFFFFFF, 20.0f, champicolor);

                if(!epilepsyfriendly)
                {
                    adddynlight(safe ? v : lightloc, 6*attacks[atk].exprad, vec(7.0f, 4.0f, 0.0f), 80, 40, L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(1.f, 0.f, 0.f));
                    adddynlight(safe ? v : lightloc, 4*attacks[atk].exprad, vec(1.5f, 0.5f, 0.0f), 80, 40, L_VOLUMETRIC|L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(1.f, 0.f, 0.f));
                }
                return;

            case ATK_NUKE_SHOOT:

                loopi(50)
                {
                    vec pos = vec(v).add(vec(rnd(200)-rnd(400), rnd(200)-rnd(400), rnd(200)-rnd(400)));
                    particle_splash(PART_SMOKE, 1, 2000,  pos, 0x212121, 150.0f, 700,   70, 0, champicolor);
                    particle_splash(PART_SMOKE, 1, 15000, pos, 0x222222, 200.0f,  35,  300, 0, champicolor);
                    particle_splash(PART_SMOKE, 2, 5000,  pos, 0x333333, 250.0f, 1000, 500, 0, champicolor);
                    particle_splash(i>25 ? PART_FLAME1 : PART_FLAME2, 3, 750+(i*10), pos, owner->steromillis ? 0xFF0000 : i>16 ? 0xFFFF00 : i>32 ? 0x224400 : 0xFF2222, 28+rnd(15), 700, 300, 0, champicolor);
                }

                if(!epilepsyfriendly)
                {
                    loopi(3) particle_fireball(v, 350, PART_ONDECHOC, 300, 0xFFCCAA, 800.0f, champicolor);
                    adddynlight(safe ? v : lightloc, 9*attacks[atk].exprad, vec(8.0f, 4.0f, 0.0f), 300, 40, L_NOSHADOW, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                    adddynlight(safe ? v : lightloc, 6*attacks[atk].exprad, vec(8.0f, 4.0f, 0.0f), 300, 40, L_NOSHADOW|L_VOLUMETRIC, attacks[atk].exprad/2, vec(0.0f, 0.0f, 1.5f));
                }
                return;
        }
    }

    void projgunhit(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk)
    {
        switch(atk)
        {
            case ATK_MINIGUN_SHOOT:
            case ATK_AK47_SHOOT:
            case ATK_UZI_SHOOT:
            case ATK_FAMAS_SHOOT:
            case ATK_GLOCK_SHOOT:
            case ATK_ARBALETE_SHOOT:
                if(atk!=ATK_ARBALETE_SHOOT)particle_splash(PART_GLOWSPARK, atk==ATK_MINIGUN_SHOOT || atk==ATK_AK47_SHOOT ? 12 : 9, 50, v, owner->steromillis ? 0xFF0000 : 0xFF8800, atk==ATK_MINIGUN_SHOOT || atk==ATK_AK47_SHOOT ? 0.3 : 0.2f, 150, 200, 0, champicolor);
                particle_splash(PART_SMOKE, 3, 600+rnd(300), v, 0x565656, ATK_MINIGUN_SHOOT==1 || ATK_AK47_SHOOT==1 ? 0.35f : 0.3f, 25, 300, 2, champicolor);
                particle_splash(PART_SMOKE, 6, 350+rnd(300), v, 0x552900, ATK_MINIGUN_SHOOT==1 || ATK_AK47_SHOOT==1 ? 0.35f : 0.3f, 15, 300, 2, champicolor);
                return;

            case ATK_SV98_SHOOT:
            case ATK_SKS_SHOOT:
            case ATK_GAU8_SHOOT:
            case ATK_CAMPOUZE_SHOOT:
                particle_splash(PART_SPARK, 12, 80, v, owner->steromillis ? 0xFF0000 : 0xFF5533, 0.6f,  400, 250, 0, champicolor);
                particle_splash(PART_SMOKE,  4, 800+rnd(300), v, 0x414141, 0.4f, 25, 300, 2, champicolor);
                particle_splash(PART_SMOKE,  4, 500+rnd(300), v, 0x442200, 0.4f, 15, 300, 2, champicolor);
                return;
        }
    }

    void instantrayhit(const vec &from, const vec &to, const vec &muzzle, int atk)
    {
        vec dir = vec(from).sub(to).safenormalize();

        switch(atk)
        {
            case ATK_RAIL_SHOOT:
                particle_splash(PART_SPARK, 50, 150, to, 0xFF4400, 0.45f, 300, 30, 0, champicolor);
                addstain(STAIN_RAIL_HOLE, to, dir, 2.0f);
                addstain(STAIN_RAIL_GLOW, to, dir, 1.5f, 0xFF2200);
                addstain(STAIN_RAIL_GLOW, to, dir, 2.5f, 0xFF8800);
                adddynlight(vec(to).madd(dir, 4), 10, vec(1.00f, 0.5f, 0.0f), 225, 75);
                return;

            case ATK_MOSSBERG_SHOOT:
            case ATK_HYDRA_SHOOT:
                switch(rnd(3))
                {
                    case 0: addstain(STAIN_BALLE_1, to, dir, 0.4f); break;
                    case 1: addstain(STAIN_BALLE_2, to, dir, 0.4f); break;
                    case 2: addstain(STAIN_BALLE_3, to, dir, 0.4f); break;
                }
                addstain(STAIN_BALLE_GLOW, to, dir, 0.8f, 0x991100);
                return;

            case ATK_LANCEFLAMMES_SHOOT:
                switch(rnd(2)){case 0: addstain(STAIN_BRULAGE, to, dir, 20.0f);}
                return;
        }
    }

    string bouclier;
    const char *getshielddir(gameent *d, bool hud) //récupère l'id d'un bouclier
    {
        switch(d->armourtype)
        {
            case A_BLUE:    {int shieldvalue = d->armour<=150 ? 20 : d->armour<=300  ? 40 : d->armour<=450  ? 60 : d->armour<=600  ? 80 : 100; formatstring(bouclier, "%s%s%d", hud ? "hudshield/" : "worldshield/", "bois/", shieldvalue);} break;
            case A_GREEN:   {int shieldvalue = d->armour<=250 ? 20 : d->armour<=500  ? 40 : d->armour<=750  ? 60 : d->armour<=1000 ? 80 : 100; formatstring(bouclier, "%s%s%d", hud ? "hudshield/" : "worldshield/", "fer/", shieldvalue);} break;
            case A_MAGNET:  {int shieldvalue = d->armour<=300 ? 20 : d->armour<=600  ? 40 : d->armour<=900  ? 60 : d->armour<=1200 ? 80 : 100; formatstring(bouclier, "%s%s%d", hud ? "hudshield/" : "worldshield/", "magnetique/", shieldvalue);} break;
            case A_YELLOW:  {int shieldvalue = d->armour<=400 ? 20 : d->armour<=800  ? 40 : d->armour<=1200 ? 60 : d->armour<=1600 ? 80 : 100; formatstring(bouclier, "%s%s%d", hud ? "hudshield/" : "worldshield/", "or/", shieldvalue);} break;
            case A_ASSIST:  {int shieldvalue = d->armour<=600 ? 20 : d->armour<=1200 ? 40 : d->armour<=1800 ? 60 : d->armour<=2400 ? 80 : 100; formatstring(bouclier, "%s%d", "hudshield/armureassistee/", shieldvalue);} break;
        }
        return bouclier;
    }
}
