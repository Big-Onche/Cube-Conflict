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
            playsound(attacks[gun-GUN_RAIL].picksound, d==hudplayer() ? NULL : &d->o, 0, 0, 0 , 50, -1, 150);
        }
        d->gunselect = gun;
    }

    ICOMMAND(launchgrenade, "", (), // shortcut for grenade attack, then select old gun
    {
        if(!isconnected() || m_identique) return;
        if(!player1->ammo[GUN_M32]) playsound(S_NOAMMO, NULL);
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
         gunselect(cncurweapon, player1);
    }

    void nextweapon(int dir, bool force = false)
    {
        if(player1->state!=CS_ALIVE) return;
        if(m_identique)
        {
            switch(player1->aptitude)
            {
                case APT_KAMIKAZE:
                    if(player1->gunselect==cncurweapon) {dir-1 ? gunselect(GUN_KAMIKAZE, player1) : getsuperweap();}
                    else gunselect(cncurweapon, player1);
                    return;
                case APT_NINJA:
                    if(player1->gunselect==cncurweapon){dir-1 ? gunselect(GUN_CACNINJA, player1) : getsuperweap();}
                    else gunselect(cncurweapon, player1);
                    return;
                default:
                    if(player1->gunselect==cncurweapon) getsuperweap();
                    else gunselect(cncurweapon, player1);
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
        if(gun != player1->gunselect) {gunselect(gun, player1); gfx::weapposup=40; }
        else playsound(S_NOAMMO);
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
        else playsound(S_NOAMMO);
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
        playsound(S_NOAMMO);
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
        playsound(S_NOAMMO);
    });

    void offsetray(const vec &from, const vec &to, int spread, int nozoomspread, float range, vec &dest, gameent *d)
    {
        vec offset;
        do offset = vec(rndscale(1), rndscale(1), rndscale(1)).sub(0.5f);
        while(offset.squaredlen() > 0.5f*0.5f);

        if(d->boostmillis[B_SHROOMS])
        {
            spread/= 1.5f;
            nozoomspread*= 1.5f;
        }
        if(d->aptitude==APT_MAGICIEN && d->abilitymillis[ABILITY_2])
        {
            spread/=3;
            nozoomspread/=3;
        }
        if(d->crouching)
        {
            spread/=d->aptitude==APT_CAMPEUR ? 2.5f : 1.333f;
            nozoomspread/=d->aptitude==APT_CAMPEUR ? 2.5f : 1.333f;
        }
        if(d->boostmillis[B_ROIDS] || d->boostmillis[B_RAGE])
        {
            spread*=1.75f;
            nozoomspread*=1.75f;
        }

        spread = (spread*100)/aptitudes[d->aptitude].apt_precision;
        nozoomspread = (nozoomspread*100)/aptitudes[d->aptitude].apt_precision;

        if(d==player1)offset.mul((to.dist(from)/1024)*(gfx::zoom ? spread : nozoomspread));
        else offset.mul((to.dist(from)/1024)*spread);

        offset.z /= 2;
        dest = vec(offset).add(to);
        if(dest != from)
        {
            vec dir = vec(dest).sub(from).normalize();
            raycubepos(from, dir, dest, range, RAY_CLIPMAT|RAY_ALPHAPOLY);
        }
    }

    void createrays(int atk, const vec &from, const vec &to, gameent *d)             // create random spread of rays
    {
        loopi(attacks[atk].rays) offsetray(from, to, attacks[atk].spread, attacks[atk].nozoomspread, attacks[atk].range, rays[i], d);
    }

    struct bouncer : physent
    {
        int lifetime, bounces;
        float lastyaw, lastpitch, roll;
        bool local;
        gameent *owner;
        int bouncetype, variant;
        vec offset;
        int offsetmillis;
        int id;

        bouncer() : bounces(0), roll(0), variant(0)
        {
            type = ENT_BOUNCE;
        }
    };

    vector<bouncer *> bouncers;

    void newbouncer(const vec &from, const vec &to, bool local, int id, gameent *owner, int type, int lifetime, int speed)
    {
        bouncer &bnc = *bouncers.add(new bouncer);
        bnc.o = from;
        bnc.radius = bnc.xradius = bnc.yradius = type==BNC_DEBRIS ? 0.5f : 1.5f;
        bnc.eyeheight = bnc.radius;
        bnc.aboveeye = bnc.radius;
        bnc.lifetime = lifetime;
        bnc.local = local;
        bnc.owner = owner;
        bnc.bouncetype = type;
        bnc.id = local ? lastmillis : id;

        switch(type)
        {
            case BNC_ROBOT: bnc.variant = rnd(3); break;
            case BNC_DEBRIS: bnc.variant = rnd(4); break;
            case BNC_GIBS: bnc.variant = rnd(6); break;
        }

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

        if(b->bounces<=3)
        {
            switch(b->bouncetype)
            {
                case BNC_DOUILLES: case BNC_DOUILLESUZI: playsound(S_DOUILLE, &b->o, 0, 0, 0 , 50, -1, 150); break;
                case BNC_BIGDOUILLES: playsound(S_BIGDOUILLE, &b->o, 0, 0, 0 , 50, -1, 150); break;
                case BNC_CARTOUCHES: playsound(S_CARTOUCHE, &b->o, 0, 0, 0 , 50, -1, 150); break;
                case BNC_GRENADE: playsound(S_RGRENADE, &b->o, 0, 0, 0 , 100, -1, 350); break;
                case BNC_LIGHT: b->lifetime=0;
            }
        }

        b->bounces++;
        if(b->bouncetype == BNC_GIBS && b->bounces < 2 && randomevent(0.05*gfx::nbfps)) addstain(STAIN_BLOOD, vec(b->o).sub(vec(surface).mul(b->radius)), surface, 2.96f/b->bounces, bvec(0x60, 0xFF, 0xFF), rnd(4));
        if(b->bouncetype == BNC_GRENADE) addstain(STAIN_PLASMA_GLOW, vec(b->o).sub(vec(surface).mul(b->radius)), surface, 4.f, 0x0000FF);
    }

    void updatebouncers(int time)
    {
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            vec old(bnc.o);

            if(bnc.vel.magnitude() > 25.0f && bnc.bounces<=5)
            {
                vec pos(bnc.o);
                pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));

                switch(bnc.bouncetype)
                {
                    case BNC_DOUILLES: case BNC_DOUILLESUZI: case BNC_BIGDOUILLES: case BNC_CARTOUCHES: regular_particle_splash(PART_SMOKE, bnc.bouncetype==BNC_DOUILLESUZI ? randomevent(2) : 1, 150, pos, bnc.bouncetype==BNC_CARTOUCHES ? 0x252525 : 0x404040, bnc.bouncetype==BNC_DOUILLES ? 1.0f : bnc.bouncetype==BNC_DOUILLESUZI ? 0.5f : 1.75f, 50, -20); break;
                    case BNC_GRENADE: regular_particle_splash(PART_SMOKE, 1, 150, pos, 0x404040, 2.5f, 50, -20); break;
                    case BNC_DEBRIS:
                    case BNC_ROBOT:
                        regular_particle_splash(lookupmaterial(pos)==MAT_WATER ? PART_BUBBLE : PART_SMOKE, lookupmaterial(pos)==MAT_WATER ? 1 : 3, 250, pos, 0x222222, 2.5f, 50, -50);
                        regular_particle_splash(PART_FIRE_BALL, 2, 75, pos, 0x994400, 0.7f, 30, -30);
                        break;
                    case BNC_GIBS: if(randomevent(0.6f*gfx::nbfps)) regular_particle_splash(PART_BLOOD, 1, 9999, pos, 0x60FFFF, 1.0f, 50);
                }
            }

            bool stopped = false;
            if(bnc.bouncetype==BNC_GRENADE) stopped = bounce(&bnc, 0.6f, 0.5f, 0.8f) || (bnc.lifetime -= time)<0;
            else
            {
                for(int rtime = time; rtime > 0;)
                {
                    int qtime = min(30, rtime);
                    rtime -= qtime;
                    if(bnc.bounces<=5) bounce(&bnc, qtime/1000.0f, 0.6f, 0.5f, 1);
                    if((bnc.lifetime -= qtime)<0) { stopped = true; break; }
                }
            }

            if(stopped)
            {
                if(bnc.bouncetype==BNC_GRENADE)
                {
                    hits.setsize(0);
                    explode(bnc.local, bnc.owner, bnc.o, bnc.o, NULL, 1, ATK_M32_SHOOT);
                    if(bnc.local)
                        addmsg(N_EXPLODE, "rci3iv", bnc.owner, lastmillis-maptime, ATK_M32_SHOOT, bnc.id-maptime,
                                hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
                }
                delete bouncers.remove(i--);
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
        loopv(bouncers) if(bouncers[i]->owner==owner) { delete bouncers[i]; bouncers.remove(i--); }
    }

    void clearbouncers() { bouncers.deletecontents(); }

    struct projectile
    {
        vec dir, o, from, to, offset;
        float speed;
        gameent *owner;
        int atk;
        bool local;
        int offsetmillis;
        int id;
        int lifetime;
        bool exploded;
        int projchan, projsound;
        bool inwater;

        projectile() : projchan(-1), projsound(-1)
        {
        }
        ~projectile()
        {
            if(projchan>=0) stopsound(projsound, projchan);

        }
    };
    vector<projectile> projs;

    void clearprojectiles() { projs.shrink(0); }

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
        p.inwater = owner->inwater ? true : false;
    }

    void removeprojectiles(gameent *owner)
    {
        // can't use loopv here due to strange GCC optimizer bug
        int len = projs.length();
        loopi(len) if(projs[i].owner==owner) { projs.remove(i--); len--; }
    }

    VARP(blood, 0, 1, 1);

    void spawnbouncer(const vec &p, const vec &vel, gameent *d, int type, int lifetime, bool frommonster)
    {
        if(camera1->o.dist(p)>temptrisfade/10 && (type==BNC_DOUILLES || type==BNC_CARTOUCHES || type==BNC_DOUILLESUZI)) return;

        vec to(0, 0, 0);

        switch(type)
        {
            case BNC_GIBS:
                to.add(vec(rnd(100)-50, rnd(100)-50, rnd(100)-50));
                break;
            case BNC_ROBOT:
            case BNC_DEBRIS:
                to.add(vec(rnd(200)-100, rnd(200)-100, rnd(200)-100));
                break;
            case BNC_DOUILLESUZI:
                to.add(vec(0, 0, -1)); break;
            default:
                if(frommonster && type==BNC_GRENADE) to.add(vec(rnd(80)-40, rnd(80)-40, rnd(80)-40));
                else to.add(vec(0, 0, 1));
        }
        if(to.iszero()) to.z += 1;
        to.normalize();
        to.add(p);
        newbouncer(p, to, true, 0, d, type, lifetime, rnd(100)+20);
    }

    void damageeffect(int damage, gameent *d, gameent *actor, int atk)
    {
        vec p = d->o;
        p.z += 0.6f*(d->eyeheight + d->aboveeye) - d->eyeheight;

        damage = ((damage*aptitudes[actor->aptitude].apt_degats)/(aptitudes[d->aptitude].apt_resistance))/(d->boostmillis[B_JOINT] ? (d->aptitude==APT_JUNKIE ? 1.875f : 1.25f) : 1.f); //Dégats de base
        actor->boostmillis[B_ROIDS] ? damage*=actor->aptitude==APT_JUNKIE ? 3 : 2 : 1; //Stéros ou non
        if(d->abilitymillis[ABILITY_3] && d->aptitude==APT_MAGICIEN) damage/=damage/5.0f;
        if(d->aptitude==APT_SHOSHONE && d->abilitymillis[ABILITY_1]) damage/=1.3f;
        damage = damage/10.f;

        if(d->armourtype!=A_MAGNET)
        {
            if(blood) particle_splash(PART_BLOOD, damage > 300 ? 3 : damage/100, 1000, p, 0x60FFFF, 2.96f);
            gibeffect(!isteam(d->team, actor->team) ? damage : actor->aptitude==APT_MEDECIN ? 0 : actor->aptitude==APT_JUNKIE ? damage/=1.5f : damage/=3.f, vec(0,0,0), d);
        }

        if(isteam(d->team, actor->team) && actor!=d)
        {
            if(actor->aptitude==APT_MEDECIN) return;
            damage/=(actor->aptitude==APT_JUNKIE ? 1.5f : 3.f); //Divisé si allié sauf sois-même
            particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*1.0f), PART_TEXT, 1500, 0x666666, actor==player1 ? 5.0f : 2.2f, -8);
            return;
        }

        float draweddmg = damage;
        bool normaldamage = true;

        switch(actor->aptitude)
        {
            case APT_AMERICAIN:
                if(atk>=ATK_NUKE_SHOOT && atk<=ATK_CAMPOUZE_SHOOT)
                {
                    draweddmg*=1.5f;
                    if(d!=player1) particle_textcopy(d->abovehead(), tempformatstring("%.1f", draweddmg), PART_TEXT, 1500, 0xFF0000, actor==player1 ? 5.5f : 4.0f, -8);
                    normaldamage = false;
                }
                break;

            case APT_NINJA:
                if(atk==ATK_CACNINJA_SHOOT && d!=player1){particle_textcopy(d->abovehead(), tempformatstring("%.1f", draweddmg), PART_TEXT, 1500, 0xFF0000, actor==player1 ? 7.0f : 5.0f, -8); normaldamage = false; }
                break;

            case APT_MAGICIEN:
                if(actor->abilitymillis[ABILITY_2])
                {
                    draweddmg*=1.25f;
                    if(d!=player1) particle_textcopy(d->abovehead(), tempformatstring("%.1f", draweddmg), PART_TEXT, 1500, 0xFF00FF, actor==player1 ? 5.5f : 4.0f, -8);
                    normaldamage = false;
                }
                break;

            case APT_CAMPEUR:
                draweddmg *= ((actor->o.dist(d->o)/1800.f)+1.f);
                if(d!=player1) particle_textcopy(d->abovehead(), tempformatstring("%.1f", draweddmg), PART_TEXT, 1500, actor->boostmillis[B_ROIDS] ? 0xFF0000: 0xFF4B19, actor==player1 ? 7.0f : 3.0f, -8);
                normaldamage = false;
                break;

            case APT_VIKING:
                if(actor->boostmillis[B_RAGE])
                {
                    draweddmg*=1.25f;
                    if(d!=player1) particle_textcopy(d->abovehead(), tempformatstring("%.1f", draweddmg), PART_TEXT, 1500, 0xFF5500, actor==player1 ? 10.0f : 7.0f, -8);
                    normaldamage = false;
                }
                break;

            case APT_PRETRE:
                if(d->abilitymillis[ABILITY_2] && d->aptitude==APT_PRETRE && d->mana)
                {
                    adddynlight(d->o, 25, vec(1.0f, 0.0f, 1.0f), 300, 50, L_NOSHADOW|L_VOLUMETRIC);
                    playsound(S_PRI_2_2, d!=player1 ? &d->o : NULL, NULL, 0, 0, 0, -1, 150, 300);
                    if(d!=player1) particle_textcopy(d->abovehead(), tempformatstring("%.1f", draweddmg), PART_TEXT, 1500, 0xAA00AA, actor==player1 ? 7.0f : 3.0f, -8);
                    normaldamage = false;
                }
                break;

            case APT_VAMPIRE:
                if(d!=player1) particle_textcopy(actor->abovehead(), tempformatstring("%.1f", draweddmg*0.5f), PART_TEXT, 1500, 0xBBDDBB, 3.5f, -8);
                break;

            case APT_SHOSHONE:
                if(actor->abilitymillis[ABILITY_3])
                {
                    draweddmg*=1.3f;
                    if(d!=player1) particle_textcopy(d->abovehead(), tempformatstring("%.1f", draweddmg), PART_TEXT, 1500, 0xFF3333, actor==player1 ? 10.0f : 7.0f, -8);
                    normaldamage = false;
                }
                if(d->aptitude==APT_AMERICAIN) draweddmg/=1.3f;
                break;

            case APT_PHYSICIEN:
                if(d==player1 && actor==player1 && player1->armour && player1->abilitymillis[ABILITY_1]) unlockachievement(ACH_BRICOLEUR);
                break;
        }
        if(normaldamage && d!=player1) particle_textcopy(d->abovehead(), tempformatstring("%.1f", draweddmg), PART_TEXT, 1500, actor->boostmillis[B_ROIDS] ? 0xFF0000 : 0xFF4400, actor==player1 ? 7.0f : 3.0f, -8);

        if(actor==player1) addstat(draweddmg, STAT_TOTALDAMAGEDEALT);
        else if(d==player1) addstat(draweddmg, STAT_TOTALDAMAGERECIE);
    }

    void gibeffect(int damage, const vec &vel, gameent *d)
    {
        if(damage <= 0) return;
        loopi(damage/300) spawnbouncer(d->o, vec(0,0,0), d, BNC_GIBS);
    }

    int avgdmg[4];

    void hit(int damage, dynent *d, gameent *at, const vec &vel, int atk, float info1, int info2 = 1)
    {
        if(at==player1 && d!=at)
        {
            extern int hitsound;
            if(hitsound && lasthit != lastmillis) playsound(S_HIT);
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
                    case APT_VIKING: player1->boostmillis[B_RAGE]+=damage*5; break;
                    case APT_PRETRE: if(player1->abilitymillis[ABILITY_2] && player1->mana) {player1->mana-=damage/10; damage=0; if(player1->mana<0)player1->mana=0;} break;
                    case APT_SHOSHONE: if(player1->abilitymillis[ABILITY_1]) damage/=1.3f;
                }
                damage = (damage/aptitudes[player1->aptitude].apt_resistance)*(m_dmsp ? 15.f : 100);
                damageeffect(damage, f, at, atk);
                damaged(damage, f, at, true, atk);
                f->hitphyspush(damage, vel, at, atk, f);
            }
            else if(at==player1)
            {
                if(player1->boostmillis[B_ROIDS]) damage*=(player1->aptitude==APT_JUNKIE ? 3.f : 1.5f);
                switch(player1->aptitude)
                {
                    case APT_VIKING: if(player1->boostmillis[B_RAGE]) damage*=1.25f; break;
                    case APT_MAGICIEN: {if(player1->abilitymillis[game::ABILITY_2]) damage *= 1.25f; break;}
                    case APT_CAMPEUR: damage *= ((player1->o.dist(f->o)/1800.f)+1.f); break;
                    case APT_VAMPIRE: {player1->health = min(player1->health + damage/2, player1->maxhealth); player1->vampimillis+=damage*1.5f;} break;
                    case APT_SHOSHONE: if(player1->abilitymillis[ABILITY_1]) damage*=1.3f;
                }
                hitmonster((damage*aptitudes[player1->aptitude].apt_degats)/100, (monster *)f, at, vel, atk);
                avgdmg[dmgsecs[0]] += damage/10;
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

            if(at==player1)
            {
                damageeffect(damage, f, at, atk);
                if(f==player1)
                {
                    damageblend(damage);
                    damagecompass(damage, at ? at->o : f->o);

                    if(player1->aptitude==APT_PHYSICIEN && player1->abilitymillis[ABILITY_1] && player1->armour) {playsound(S_PHY_1); playsound(S_PHY_1_WOOD+f->armourtype);}
                    else if(player1->armour && atk!=ATK_LANCEFLAMMES_SHOOT && randomevent(2)) playsound(S_IMPACTWOOD+f->armourtype);
                    else playsound(S_IMPACTBODY);
                }
            }
            else
            {
                if(randomevent(2))
                {
                    if(f->aptitude==APT_PHYSICIEN && f->abilitymillis[ABILITY_1] && f->armour) {playsound(S_PHY_1, &f->o, 0, 0, 0 , 100, -1, 200); playsound(S_PHY_1_WOOD+f->armourtype, &f->o, 0, 0, 0 , 100, -1, 200);}
                    else if(f->armour && atk!=ATK_LANCEFLAMMES_SHOOT) playsound(S_IMPACTWOOD+f->armourtype, &f->o, 0, 0, 0 , 100, -1, 200);
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

    void startshake(const vec &v, int maxdist, int atk)
    {
        if(camera1->o.dist(v) > maxdist) return;

        int shakereduce = atk==ATK_NUKE_SHOOT ? 0 : camera1->o.dist(v) < 75 ? 0 : camera1->o.dist(v) < 125 ? 1 : 2;

        defformatstring(cmd, "%s %d", "screenshake", shakereduce);
        int sleep = 0; loopi(16){addsleep(&sleep, cmd); sleep +=25;}
    }

    void explode(bool local, gameent *owner, const vec &v, const vec &vel, dynent *safe, int damage, int atk)
    {
        vec debrisvel = owner->o==v ? vec(0, 0, 0) : vec(owner->o).sub(v).normalize(), debrisorigin(v);
        debrisorigin = vec(v).sub(vec(vel).mul(10)).add(vec(5-rnd(10), 5-rnd(10), 5-rnd(10)));

        switch(atk)
        {
            case ATK_PULSE_SHOOT:
            case ATK_GRAP1_SHOOT:
            case ATK_SPOCKGUN_SHOOT:
                playsound(atk==ATK_GRAP1_SHOOT ? S_IMPACTGRAP1 : atk==ATK_PULSE_SHOOT ? S_IMPACTPLASMA : S_IMPACTSPOCK, &v, 0, 0, 0 , 75, -1, 225);
                gfx::projgunexplosion(owner, v, vel, safe, atk);
                break;

            case ATK_SMAW_SHOOT:
            case ATK_ROQUETTES_SHOOT:
                playsound(S_EXPL_MISSILE, &v, 0, 0, 0 , 100, -1, 350);
                if(lookupmaterial(v)==MAT_WATER) playsound(S_EXPL_INWATER, &v, 0, 0, 0 , 100, -1, 350);
                if(camera1->o.dist(v) >= 300) playsound(S_EXPL_FAR, &v, NULL, 0, 0, 400, -1, 1500);
                loopi(5+rnd(3)) spawnbouncer(debrisorigin, debrisvel, owner, BNC_DEBRIS);
                gfx::projexplosion(owner, v, vel, safe, atk);
                startshake(v, 150, atk);
                break;

            case ATK_KAMIKAZE_SHOOT:
            case ATK_ASSISTXPL_SHOOT:
                if(camera1->o.dist(v) >= 300) playsound(S_BIGEXPL_FAR, &v, NULL, 0, 0, 400, -1, 2000);
                if((lookupmaterial(v)==MAT_WATER)) playsound(S_EXPL_INWATER, &v, 0, 0, 0 , 100, -1, 350);
                loopi(5+rnd(3)) spawnbouncer(debrisorigin, debrisvel, owner, BNC_DEBRIS);
                if(atk==ATK_ASSISTXPL_SHOOT) loopi(10+rnd(5)) spawnbouncer(debrisorigin, debrisvel, owner, BNC_ROBOT);
                gfx::projexplosion(owner, v, vel, safe, atk);
                startshake(v, 225, atk);
                break;

            case ATK_NUKE_SHOOT:
                playsound(S_EXPL_NUKE);
                gfx::projexplosion(owner, v, vel, safe, atk);
                startshake(v, 5000, atk);
                break;

            case ATK_ARTIFICE_SHOOT:
                playsound(S_EXPL_FIREWORKS, &v, 0, 0, 0 , 100, -1, 300);
                if(lookupmaterial(v)==MAT_WATER) playsound(S_EXPL_INWATER, &v, 0, 0, 0 , 100, -1, 350);
                if(camera1->o.dist(v) >= 300) playsound(S_FIREWORKSEXPL_FAR, &v, NULL, 0, 0, 400, -1, 1500);
                gfx::projexplosion(owner, v, vel, safe, atk);
                startshake(v, 100, atk);
                break;

            case ATK_M32_SHOOT:
                playsound(S_EXPL_GRENADE, &v, 0, 0, 0 , 100, -1, 350);
                if(lookupmaterial(v)==MAT_WATER) playsound(S_EXPL_INWATER, &v, 0, 0, 0 , 100, -1, 350);
                if(camera1->o.dist(v) >= 300) playsound(S_EXPL_FAR, &v, NULL, 0, 0, 400, -1, 1500);
                loopi(5+rnd(3)) spawnbouncer(debrisorigin, debrisvel, owner, BNC_DEBRIS);
                gfx::projexplosion(owner, v, vel, safe, atk);
                startshake(v, 150, atk);
                break;

            case ATK_MINIGUN_SHOOT:
            case ATK_AK47_SHOOT:
            case ATK_UZI_SHOOT:
            case ATK_FAMAS_SHOOT:
            case ATK_GLOCK_SHOOT:
            case ATK_ARBALETE_SHOOT:
                if(!(lookupmaterial(v)==MAT_WATER)) playsound(atk==ATK_ARBALETE_SHOOT ? S_IMPACTARROW : S_LITTLERICOCHET, &v, 0, 0, 0 , 100, -1, 250);
                gfx::projgunhit(owner, v, vel, safe, atk);
                break;

            case ATK_SV98_SHOOT:
            case ATK_SKS_SHOOT:
            case ATK_GAU8_SHOOT:
            case ATK_CAMPOUZE_SHOOT:
            {
                if(!(lookupmaterial(v)==MAT_WATER))
                {
                    playsound(S_IMPACTLOURDLOIN, &v, 0, 0, 0 , 500, -1, 1000);
                    playsound(S_BIGRICOCHET, &v, 0, 0, 0 , 100, -1, 300);
                }
                gfx::projgunhit(owner, v, vel, safe, atk);
            }
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
            case ATK_CAMPOUZE_SHOOT:
            case ATK_SKS_SHOOT:
                addstain(STAIN_BULLET_HOLE, pos, dir, 1.5f+(rnd(2)));
                addstain(STAIN_BULLET_GLOW, pos, dir, 2.0f+(rnd(2)), 0x883300);
                return;
            case ATK_SPOCKGUN_SHOOT:
                addstain(STAIN_PLASMA_SCORCH, pos, dir, 5);
                addstain(STAIN_SPOCK, pos, dir, 5, 0x22FF22);
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
                projs.remove(i);
                break;
            }
        }
        loopv(bouncers)
        {
            bouncer &b = *bouncers[i];
            if(b.bouncetype == BNC_GRENADE && b.owner == d && b.id == id && !b.local)
            {
                vec pos = vec(b.offset).mul(b.offsetmillis/float(OFFSETMILLIS)).add(b.o);
                explode(b.local, b.owner, pos, vec(0,0,0), NULL, 0, atk);
                switch(atk)
                {
                    case ATK_M32_SHOOT:
                    default: break;
                }
                bouncers.remove(i);
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
                else
                {
                    vec pos = vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)).add(v);

                    float len = min(100.f, vec(p.offset).add(p.from).dist(pos));
                        vec dir = vec(dv).normalize(),
                        tail = vec(dir).mul(-len).add(pos),
                        head = vec(dir).mul(2.4f).add(pos);

                    bool canplaysound = false;
                    if(!p.inwater && lookupmaterial(pos)==MAT_WATER)
                    {
                        p.inwater = true;
                        particle_splash(PART_WATER, 15, 100, v, 0x28282A, 0.75f, 50, -300, 1, gfx::champicolor());
                        playsound(S_IMPACTWATER, &v, NULL, 0, 0 , 150, -1, 250);
                    }

                    switch(p.atk)
                    {
                        case ATK_PULSE_SHOOT:
                            particle_splash(PART_PLASMA_FRONT, 1, 1, pos, p.owner->boostmillis[B_ROIDS] ? 0xFF4444 : 0xFF6600, 2.4f, 150, 20, 0, gfx::champicolor());
                            particle_flare(tail, head, 1, PART_F_PLASMA, p.owner->boostmillis[B_ROIDS] ? 0xFF4444 : 0xFF6600, 2.0f, p.owner, gfx::champicolor());
                            if(lookupmaterial(pos)==MAT_WATER) particle_splash(PART_BUBBLE, 1, 150, v, 0x18181A, 2.0f+rnd(2), 20, -30);
                            p.projsound = S_FLYBYPLASMA;
                            canplaysound = true;
                            break;
                        case ATK_GRAP1_SHOOT:
                            particle_splash(PART_PLASMA_FRONT, 1, 1, pos, p.owner->boostmillis[B_ROIDS] ? 0xFF4444 : 0xFF33BB, 3.0f, 150, 20, 0, gfx::champicolor());
                            particle_flare(tail, head, 1, PART_F_PLASMA, p.owner->boostmillis[B_ROIDS] ? 0xFF4444 : 0xEE22AA, 3.0f, p.owner, gfx::champicolor());
                            particle_splash(lookupmaterial(pos)==MAT_WATER ? PART_BUBBLE : PART_SMOKE, 1, lookupmaterial(pos)==MAT_WATER ? 150 : 300, pos, lookupmaterial(pos)&MAT_WATER ? 0x18181A : 0xAAAAAA, 4.0f, 25, 250, 0, gfx::champicolor());
                            p.projsound = S_FLYBYGRAP1;
                            canplaysound = true;
                            break;
                        case ATK_SPOCKGUN_SHOOT:
                            particle_splash(PART_SPOCK_FRONT, 1, 1, pos, p.owner->boostmillis[B_ROIDS] ? 0xFF4444 : 0x00FF00, 4.f, 150, 20, 0, gfx::champicolor());
                            particle_flare(tail, head, 1, PART_F_PLASMA, p.owner->boostmillis[B_ROIDS] ? 0xFF4444 : 0x22FF22, 2.5f, p.owner, gfx::champicolor());
                            p.projsound = S_FLYBYSPOCK;
                            canplaysound = true;
                            break;
                        case ATK_SV98_SHOOT:
                        case ATK_SKS_SHOOT:
                        case ATK_CAMPOUZE_SHOOT:
                        case ATK_GAU8_SHOOT:
                            if(lookupmaterial(pos)==MAT_WATER) particle_splash(PART_BUBBLE, 1, 150, v, 0x18181A, 2.0f+rnd(2), 20, -30);
                            particle_flare(tail, head, 1, PART_F_BULLET, p.owner->boostmillis[B_ROIDS] ? 0xFF4444 : 0xFFBB88, ATK_GAU8_SHOOT==1 ? 0.75f : 0.65f, p.owner, gfx::champicolor());
                            particle_splash(PART_PLASMA_FRONT, 1, 1, pos, p.owner->boostmillis[B_ROIDS] ? 0xFF4444 : 0xFFBB88,  p.owner==player1 ? 0.65f : ATK_GAU8_SHOOT==1 ? 0.45f : 0.3f, 150, 20, 0, gfx::champicolor());
                            particle_flare(tail, head, ATK_SV98_SHOOT==1 || ATK_CAMPOUZE_SHOOT==1 ? 1750 : 750, PART_F_SMOKE, 0x333333, ATK_SV98_SHOOT==1 || ATK_CAMPOUZE_SHOOT==1 ? 1.4f : 1.f, p.owner, gfx::champicolor());
                            break;
                        case ATK_MINIGUN_SHOOT:
                        case ATK_AK47_SHOOT:
                        case ATK_UZI_SHOOT:
                        case ATK_GLOCK_SHOOT:
                        case ATK_FAMAS_SHOOT:
                            if(lookupmaterial(pos)==MAT_WATER) particle_splash(PART_BUBBLE, 1, 150, v, 0x18181A, 1.0f+rnd(2), 20, -30);
                            particle_flare(tail, head, 1, PART_F_BULLET, p.owner->boostmillis[B_ROIDS] ? 0xFF4444 : 0xFFBB88, ATK_MINIGUN_SHOOT==1 || ATK_AK47_SHOOT==1 ? 0.55f : 0.45f, p.owner, gfx::champicolor());
                            particle_splash(PART_PLASMA_FRONT, 1, 1, pos, p.owner->boostmillis[B_ROIDS] ? 0xFF4444 : 0xFFBB88, p.owner==player1 ? 0.4f : ATK_MINIGUN_SHOOT==1 || ATK_AK47_SHOOT==1 ? 0.3f : 0.24f, 150, 20, 0, gfx::champicolor());
                            particle_flare(tail, head, ATK_MINIGUN_SHOOT==1 || ATK_AK47_SHOOT==1 ? 1250 : 750, PART_F_SMOKE, 0x252525, ATK_MINIGUN_SHOOT==1 || ATK_AK47_SHOOT==1 ? 1.f : 0.75f, p.owner, gfx::champicolor());
                            break;
                        case ATK_ARBALETE_SHOOT:
                            if(!p.exploded)
                            {
                                if(lookupmaterial(pos)==MAT_WATER) particle_splash(PART_BUBBLE, 1, 150, v, 0x888888, 1.0f+rnd(2), 20, -30);
                                particle_flare(tail, head, 250, PART_F_SMOKE, 0x444444, 0.60f, p.owner, gfx::champicolor());

                                if(p.owner->boostmillis[B_ROIDS])
                                {
                                    particle_flare(tail, head, 1, PART_F_BULLET, 0xFF4444, 0.30f, p.owner, gfx::champicolor());
                                    particle_splash(PART_PLASMA_FRONT, 1, 1, pos, 0xFF4444, p.owner==player1 ? 0.5f : 0.25f, 150, 20, 0, gfx::champicolor());
                                }
                            }
                            break;
                        case ATK_SMAW_SHOOT:
                            p.projsound = S_ROCKET;
                            canplaysound = true;
                            break;
                        case ATK_ROQUETTES_SHOOT:
                            p.projsound = S_MINIROCKET;
                            canplaysound = true;
                            break;
                        case ATK_NUKE_SHOOT:
                            p.projsound = S_MISSILENUKE;
                            canplaysound = true;
                            break;
                        case ATK_ARTIFICE_SHOOT:
                            p.projsound = S_FLYBYFIREWORKS;
                            canplaysound = true;
                            break;
                    }
                    if(canplaysound) p.projchan = playsound(p.projsound, &pos, NULL, 0, -1, 128, p.projchan, p.atk==ATK_NUKE_SHOOT || p.atk==ATK_ROQUETTES_SHOOT || p.atk==ATK_ARTIFICE_SHOOT ? 500 : 375);
                }
            }
            if(exploded)
            {
                if(p.local && !p.exploded) addmsg(N_EXPLODE, "rci3iv", p.owner, lastmillis-maptime, p.atk, p.id-maptime, hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
                p.exploded = true;

                if((p.lifetime -= time)<0 || removearrow) projs.remove(i--);
                else if(p.atk!=ATK_ARBALETE_SHOOT) projs.remove(i--);
            }
            else p.o = v;
        }
    }

    void sound_nearmiss(int sound, const vec &s, const vec &e)
    {
        vec v;
        float d = e.dist(s, v);
        int steps = clamp(int(d*2), 1, 2048);
        v.div(steps);
        vec p = s;
        bool soundused = false;
        loopi(steps)
        {
            p.add(v);
            if(camera1->o.dist(p) <= 32 && !soundused)
            {
                playsound(sound, &p, 0, 0, 0 , 100, -1, 250);
                soundused = true;
            }
        }
    }

    bool looped;

    void shoteffects(int atk, const vec &from, const vec &to, gameent *d, bool local, int id, int prevaction)     // create visual effect from a shot
    {
        if(d==player1) addstat(1, STAT_MUNSHOOTED);

        int gun = attacks[atk].gun;
        int sound = attacks[atk].sound;
        //int soundwater = attacks[atk]].soundwater;
        if(player1->abilitymillis[ABILITY_2] && d==player1 && player1->aptitude==APT_MAGICIEN) playsound(S_WIZ_2);
        if(d->abilitymillis[ABILITY_3] && d->aptitude==APT_PRETRE)adddynlight(d->muzzle, 6, vec(1.5f, 1.5f, 0.0f), 80, 40, L_NOSHADOW|L_VOLUMETRIC|DL_FLASH);

        int movefactor = game::player1->aptitude==APT_SOLDAT ? 2 : 1;

        switch(atk)
        {
            case ATK_PULSE_SHOOT: case ATK_SPOCKGUN_SHOOT:
                if(d->type==ENT_PLAYER && atk==ATK_PULSE_SHOOT) sound = S_PLASMARIFLE_SFX;
                gfx::shootgfx(from, to, d, atk);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d==player1) mousemove((-5+rnd(11))/movefactor, (-5+rnd(11))/movefactor, player1->abilitymillis[ABILITY_2] && player1->aptitude==APT_MAGICIEN ? true : false);
                break;

            case ATK_RAIL_SHOOT:
                playsound(S_IMPACTELEC, &to, 0, 0, 0 , 100, -1, 250);
                if(d!=hudplayer()) sound_nearmiss(S_FLYBYELEC, from, to);
                gfx::shootgfx(from, to, d, atk);
                gfx::instantrayhit(from, to, d->aptitude==APT_ESPION && d->abilitymillis[ABILITY_2] ? hudgunorigin(d->gunselect, d->o, to, d) : d->muzzle, atk);
                break;

            case ATK_SMAW_SHOOT:
            case ATK_ROQUETTES_SHOOT:
            case ATK_NUKE_SHOOT:
            case ATK_ARTIFICE_SHOOT:
                if(d==player1 && atk==ATK_NUKE_SHOOT)
                {
                    unlockachievement(ACH_ATOME);
                    addstat(1, STAT_ATOM);
                }
                gfx::shootgfx(from, to, d, atk);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d==player1 && atk==ATK_ROQUETTES_SHOOT) mousemove((-7+rnd(15))/movefactor, (-7+rnd(15))/movefactor, player1->abilitymillis[ABILITY_2] && player1->aptitude==APT_MAGICIEN ? true : false);
                break;
            case ATK_MINIGUN_SHOOT:
            case ATK_AK47_SHOOT:
            case ATK_GAU8_SHOOT:
                if(atk==ATK_GAU8_SHOOT)
                {
                    if(d->type==ENT_PLAYER) sound = S_GAU8;
                     if(d==player1 && player1->aptitude==APT_PRETRE && player1->boostmillis[B_SHROOMS] && player1->abilitymillis[ABILITY_3]) unlockachievement(ACH_CADENCE);
                }
                spawnbouncer(d->aptitude==APT_ESPION && d->abilitymillis[ABILITY_2] ? d->o : d->balles, vec(0, 0, 0), d, atk==ATK_GAU8_SHOOT ? BNC_BIGDOUILLES : BNC_DOUILLES);
                gfx::shootgfx(from, to, d, atk);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d!=hudplayer()) sound_nearmiss(atk==ATK_GAU8_SHOOT ? S_BIGBULLETFLYBY : S_BULLETFLYBY, from, to);
                if(d==player1) mousemove(atk==ATK_MINIGUN_SHOOT ? (-7+rnd(15))/movefactor : (-3+rnd(7))/movefactor, atk==ATK_MINIGUN_SHOOT ? (-7+rnd(15))/movefactor : (-3+rnd(7))/movefactor, player1->abilitymillis[ABILITY_2] && player1->aptitude==APT_MAGICIEN ? true : false);
                break;
            case ATK_MOSSBERG_SHOOT:
            case ATK_HYDRA_SHOOT:
                {
                    if(!local) createrays(gun, d->aptitude==APT_ESPION && d->abilitymillis[ABILITY_2] ? d->o : from, to, d);
                    loopi(atk==ATK_HYDRA_SHOOT ? 3 : 2) spawnbouncer(d->aptitude==APT_ESPION && d->abilitymillis[ABILITY_2] ? hudgunorigin(gun, d->o, to, d) : d->balles, vec(0, 0, 0), d, BNC_CARTOUCHES);
                    particle_flare(d->muzzle, d->muzzle, 140, PART_MF_SHOTGUN, d->boostmillis[B_ROIDS] ? 0xFF2222 : d->abilitymillis[ABILITY_2] && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0xCCAAAA, d==hudplayer() ? gfx::zoom ? 1.25f : 3.50f : 4.5f, d, gfx::champicolor());
                    if(d->boostmillis[B_RAGE]) particle_flare(d->muzzle, d->muzzle, 140, PART_MF_SHOTGUN, 0xFF2222, d==hudplayer() ? gfx::zoom ? 2.00f : 5.5f : 6.5f, d, gfx::champicolor());
                    particle_splash(PART_SMOKE,  4, 500, d->muzzle, 0x443333, 3.5f, 20, 500, 0, gfx::champicolor());
                    particle_splash(PART_SPARK, d==hudplayer() ? 4 : 7, 40, d->muzzle, 0xFF2200, 0.5f, 300, 500, 0, gfx::champicolor());
                    adddynlight(hudgunorigin(gun, d->o, to, d), 75, vec(1.25f, 0.25f, 0.f), 40, 2, DL_FLASH, 0, vec(1.25f, 0.25f, 0.f), d);
                    loopi(attacks[atk].rays)
                    {
                        if(!(lookupmaterial(rays[i])==MAT_WATER)) playsound(S_LITTLERICOCHET, &rays[i], 0, 0, 0 , 100, -1, 250);
                        particle_splash(PART_SPARK, 9, 60, rays[i], d->boostmillis[B_ROIDS] ? 0xFF2222 : 0xAA1100, 0.4, 150, 100, 0, gfx::champicolor());
                        particle_splash(PART_SMOKE, 3, 500+rnd(300), rays[i], 0x797979, 0.2f, 35, 300, 2, gfx::champicolor());
                        particle_splash(PART_SMOKE, 3, 275+rnd(275), rays[i], 0x553915, 0.15f, 35, 300, 2, gfx::champicolor());
                        vec origin = d->aptitude==APT_ESPION && d->abilitymillis[ABILITY_2] ? hudgunorigin(gun, d->o, to, d) : d->muzzle;
                        origin.add(vec(0.2f-(rnd(5)/10.f), 0.2f-(rnd(5)/10.f), 0.2f-(rnd(5)/10.f)));
                        particle_flare(origin, rays[i], 30, PART_F_SHOTGUN, d->boostmillis[B_ROIDS] ? 0xFF2222 : atk==ATK_HYDRA_SHOOT ? 0xFF8800 : 0xFFFF22, atk==ATK_HYDRA_SHOOT ? 0.15f : 0.2f, d, gfx::champicolor());
                        gfx::instantrayhit(from, rays[i], d->aptitude==APT_ESPION && d->abilitymillis[ABILITY_2] ? hudgunorigin(d->gunselect, d->o, to, d) : d->muzzle, atk);
                        if(d!=hudplayer()) sound_nearmiss(S_BULLETFLYBY, from, rays[i]);
                    }
                }
                break;
            case ATK_SV98_SHOOT:
            case ATK_SKS_SHOOT:
            case ATK_CAMPOUZE_SHOOT:
                if(atk==ATK_CAMPOUZE_SHOOT) {loopi(3)spawnbouncer(d->aptitude==APT_ESPION && d->abilitymillis[ABILITY_2] ? d->o : d->balles, vec(0, 0, 0), d, BNC_BIGDOUILLES);}
                else spawnbouncer(d->aptitude==APT_ESPION && d->abilitymillis[ABILITY_2] ? d->o : d->balles, vec(0, 0, 0), d, BNC_BIGDOUILLES);
                particle_splash(PART_SMOKE, d==hudplayer() ? 4 : 6, d==hudplayer() ? 350 : 600, d->muzzle, 0x222222, d==hudplayer() ? 3.5f : 6.5f, 40, 500, 0, gfx::champicolor());
                particle_splash(PART_SPARK, d==hudplayer() ? 4 : 7, 40, d->muzzle, 0xFFFFFF, 0.5f, 300, 500, 0, gfx::champicolor());
                particle_flare(d->muzzle, d->muzzle, 100, PART_MF_LITTLE, d->boostmillis[B_ROIDS] ? 0xFF2222 : d->abilitymillis[ABILITY_2] && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0xFFFFFF, 1.25f, d, gfx::champicolor());
                particle_flare(d->muzzle, d->muzzle, 100, PART_MF_SNIPER, d->boostmillis[B_ROIDS] ? 0xFF2222 : d->abilitymillis[ABILITY_2] && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0xFFFFFF, atk==ATK_CAMPOUZE_SHOOT ? 5.0f : 3.5f, d, gfx::champicolor());
                if(d->boostmillis[B_RAGE]) particle_flare(d->muzzle, d->muzzle, 75, PART_MF_SNIPER, 0xFF2222, 6.0f, d, gfx::champicolor());
                adddynlight(hudgunorigin(gun, d->o, to, d), 50, vec(1.25f, 0.75f, 0.3f), 37, 2, DL_FLASH, 0, vec(1.25f, 0.75f, 0.3f), d);
                if(atk==ATK_CAMPOUZE_SHOOT)
                {
                    loopi(attacks[atk].rays)
                    {
                        newprojectile(from, rays[i], attacks[atk].projspeed, local, id, d, atk);
                        if(d!=hudplayer()) sound_nearmiss(S_BULLETFLYBY, from, rays[i]);
                    }
                }
                else newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d!=hudplayer()) sound_nearmiss(S_BIGBULLETFLYBY, from, to);
                break;
            case ATK_ARBALETE_SHOOT:
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d->boostmillis[B_RAGE]) particle_splash(PART_SPARK,  8, 500, d->muzzle, 0xFF2222, 1.0f,  50,   200, 0, gfx::champicolor());
                if(d!=hudplayer()) sound_nearmiss(S_FLYBYARROW, from, to);
                break;
            case ATK_UZI_SHOOT:
            case ATK_FAMAS_SHOOT:
            case ATK_GLOCK_SHOOT:
                spawnbouncer(d->aptitude==APT_ESPION && d->abilitymillis[ABILITY_2] ? d->o : d->balles, vec(0, 0, 0), d, atk==ATK_UZI_SHOOT ? BNC_DOUILLESUZI : BNC_DOUILLES);
                particle_flare(d->muzzle, d->muzzle, 125, PART_MF_LITTLE, d->boostmillis[B_ROIDS] ? 0xFF2222 : d->abilitymillis[ABILITY_2] && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0xFFFFFF, d==hudplayer() ? gfx::zoom ? 0.5f : 0.75f : 1.75f, d, gfx::champicolor());
                particle_flare(d->muzzle, d->muzzle, 75, PART_MF_BIG, d->boostmillis[B_ROIDS] ? 0xFF2222 : d->abilitymillis[ABILITY_2] && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0xFFAA55, d==hudplayer() ? gfx::zoom ? 0.75f : 2.f : 3.f, d, gfx::champicolor());
                if(d->boostmillis[B_RAGE]) particle_flare(d->muzzle, d->muzzle, 80, PART_MF_BIG, 0xFF2222, d==hudplayer() ? gfx::zoom ? 1.5f : 4.f : 5.f, d, gfx::champicolor());
                particle_splash(PART_SMOKE, d==hudplayer() ? 3 : 5, d==hudplayer() ? 350 : 500, d->muzzle, 0x444444, d==hudplayer() ? 3.5f : 4.5f, 20, 500, 0, gfx::champicolor());
                particle_splash(PART_SPARK, d==hudplayer() ? 3 : 5, 35, d->muzzle, 0xFF4400, 0.35f, 300, 500, 0, gfx::champicolor());
                adddynlight(hudgunorigin(gun, d->o, to, d), 60, vec(1.25f, 0.75f, 0.3f), 30, 2, DL_FLASH, 0, vec(1.25f, 0.75f, 0.3f), d);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d!=hudplayer()) sound_nearmiss(S_BULLETFLYBY, from, to);
                if(d==player1) {if(atk!=ATK_GLOCK_SHOOT) mousemove((-3+rnd(7))/movefactor, (-3+rnd(7))/movefactor, player1->abilitymillis[ABILITY_2] && player1->aptitude==APT_MAGICIEN ? true : false);}
                break;
            case ATK_LANCEFLAMMES_SHOOT:
            {
                if(!local) createrays(gun, from, to, d);
                d->type==ENT_AI ? sound = S_PYRO_A : sound = S_FLAMETHROWER;
                loopi(attacks[atk].rays)
                {
                    vec origin = d->aptitude==APT_ESPION && d->abilitymillis[ABILITY_2] ? hudgunorigin(gun, d->o, to, d) : d->muzzle;

                    vec iflames(rays[i]);
                    iflames.sub(origin);
                    iflames.normalize().mul(1450.0f + rnd(200));

                    switch(rnd(4))
                    {
                        case 0: particle_flying_flare(origin, iflames, 700, PART_FIRE_BALL, d->boostmillis[B_ROIDS] ? 0x881111 :  0x604930, (12.f+rnd(16))/8.f, 100, 10+rnd(5), gfx::champicolor()); break;
                        case 1: particle_flying_flare(origin, iflames, 700, PART_FIRE_BALL, d->boostmillis[B_ROIDS] ? 0x770000 :  0x474747, (12.f+rnd(16))/8.f, 100, 10+rnd(5), gfx::champicolor()); break;
                        case 2: particle_flying_flare(origin, iflames, 700, PART_FIRE_BALL, d->boostmillis[B_ROIDS] ? 0x991111 :  0x383838, (12.f+rnd(16))/8.f, 100, 10+rnd(5), gfx::champicolor()); break;
                        default:
                            particle_flying_flare(origin, iflames, 1100, PART_SMOKE, 0x111111, (15.f+rnd(18))/3.f, -20, 15+rnd(10), gfx::champicolor());
                            adddynlight(hudgunorigin(gun, d->o, iflames, d), 50, vec(0.40f, 0.2f, 0.1f), 100, 100, L_NODYNSHADOW, 10, vec(0.50f, 0, 0), d);
                            gfx::instantrayhit(from, rays[i], d->aptitude==APT_ESPION && d->abilitymillis[ABILITY_2] ? hudgunorigin(d->gunselect, d->o, to, d) : d->muzzle, atk);
                            switch(rnd(2)){case 0: if(d!=hudplayer()) sound_nearmiss(S_FLYBYFLAME, from, rays[i]);}
                    }
                }
                if(d->boostmillis[B_RAGE]) particle_splash(PART_SPARK,  3, 500, d->muzzle, 0xFF2222, 1.0f, 50, 200, 0, gfx::champicolor());
                particle_flare(d->muzzle, d->muzzle, 150, PART_MF_ROCKET, d->boostmillis[B_ROIDS] ? 0x880000 : d->abilitymillis[ABILITY_2] && d->aptitude==APT_MAGICIEN ? 0x440044 : 0x663311,  d==hudplayer() ? gfx::zoom ? 2.00f : 3.5f : 4.5f, d, gfx::champicolor());

                float dist = from.dist(to);
                vec up = to;
                up.z += dist/8;
                if(randomevent(2))newbouncer(d==player1 && !thirdperson ? d->muzzle : hudgunorigin(gun, d->o, to, d), up, local, id, d, BNC_LIGHT, 650, 400);
                break;
            }
            case ATK_GRAP1_SHOOT:
            {
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                particle_flare(d->muzzle, d->muzzle, 150, PART_MF_PLASMA, d->boostmillis[B_ROIDS] ? 0xFF4444 : d->abilitymillis[ABILITY_2] && d->aptitude==APT_MAGICIEN ? 0xFF00FF : 0xFF55FF, 1.75f, d, gfx::champicolor());
                if(d->boostmillis[B_RAGE]) particle_splash(PART_SPARK,  3, 500, d->muzzle, 0xFF4444, 1.0f, 50, 200, 0, gfx::champicolor());
                adddynlight(hudgunorigin(gun, d->o, to, d), 70, vec(1.0f, 0.0f, 1.0f), 80, 100, DL_FLASH, 0, vec(0, 0, 0), d);
                break;
            }
            case ATK_M32_SHOOT:
            {
                float dist = from.dist(to);
                vec up = to;
                up.z += dist/8;
                particle_splash(PART_SMOKE,  10, 600, d->muzzle, d->abilitymillis[ABILITY_2] && d->aptitude==APT_MAGICIEN ? 0x550044 : 0x444444, 4.0f, 20, 500, 0, gfx::champicolor());
                if(d->boostmillis[B_RAGE]) particle_splash(PART_SPARK,  8, 500, d->muzzle, 0xFF4444, 1.0f, 50, 200, 0, gfx::champicolor());
                newbouncer(d==player1 && !thirdperson ? d->muzzle : hudgunorigin(gun, d->o, to, d), up, local, id, d, BNC_GRENADE, attacks[atk].ttl, attacks[atk].projspeed);
                break;
            }
            case ATK_KAMIKAZE_SHOOT:
            case ATK_ASSISTXPL_SHOOT:
            {
                float dist = from.dist(to);
                vec up = to;
                up.z += dist/8;
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                break;
            }
            default:
                break;
        }

        if(d->boostmillis[B_ROIDS] && randomevent(attacks[atk].specialsounddelay))
        {
            playsound(S_ROIDS_SHOOT, d==hudplayer() ? NULL : &d->o, NULL, 0, 0 , 300, -1, 500);
            if(camera1->o.dist(hudgunorigin(gun, d->o, to, d)) >= 400 && d!=player1) playsound(S_ROIDS_SHOOT_FAR, &d->o, NULL, 0, 0 , 600, -1, 800);
        }
        else if(d->boostmillis[B_RAGE] && randomevent(attacks[atk].specialsounddelay)) playsound(S_RAGETIR, d==hudplayer() ? NULL : &d->o, NULL, 0, 0 , 300, -1, 500);

        looped = false;
        if(d->attacksound >= 0 && d->attacksound != sound) d->stopattacksound(d);
        switch(sound)
        {
            case S_FLAMETHROWER:
            case S_GAU8:
            case S_PLASMARIFLE_SFX:
                if(d->attacksound >= 0) looped = true;
                d->attacksound = sound;
                d->attackchan = playsound(sound, d==hudplayer() ? NULL : &d->o, NULL, 0, -1, atk==ATK_GAU8_SHOOT ? 75 : 50, d->attackchan, atk==ATK_GAU8_SHOOT ? 600 : 350);
                if(sound==S_GAU8) return;
            default:
                {
                    if(d==hudplayer() && sound!=S_FLAMETHROWER) playsound(attacks[atk].sound, NULL);
                    else if(sound!=S_FLAMETHROWER) playsound(d->type==ENT_AI && atk==ATK_LANCEFLAMMES_SHOOT ? sound : attacks[atk].sound, &d->o, NULL, 0, 0, atk==ATK_ASSISTXPL_SHOOT || atk==ATK_KAMIKAZE_SHOOT ? 75 : 50, -1, atk==ATK_ASSISTXPL_SHOOT || atk==ATK_KAMIKAZE_SHOOT ? 600 : 400);
                }
        }

        if(camera1->o.dist(hudgunorigin(gun, d->o, to, d)) >= 300) playsound(attacks[atk].middistsnd, &d->muzzle, NULL, 0, 0 , 300, -1, 550);
        if(camera1->o.dist(hudgunorigin(gun, d->o, to, d)) >= 500) playsound(attacks[atk].fardistsnd, &d->muzzle, NULL, 0, 0 , 500, -1, 800);
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

        if(d->aitype==AI_BOT && (d->gunselect==GUN_GLOCK || d->gunselect==GUN_SPOCKGUN || d->gunselect==GUN_HYDRA || d->gunselect==GUN_SKS || d->gunselect==GUN_S_CAMPOUZE))
        {
            switch(rnd(d->gunselect==GUN_GLOCK || d->gunselect==GUN_SPOCKGUN || d->gunselect==GUN_HYDRA ? 5 : 15)) {case 0: d->gunwait+=(d->aptitude==APT_PRETRE && d->abilitymillis[ABILITY_3] ? 500 : 1200)/gfx::nbfps;}
        }

        switch(d->gunselect)
        {
            case GUN_MINIGUN:
            case GUN_PULSE:
            case GUN_S_ROQUETTES:
                if(!d->attacking) d->gunselect==GUN_PULSE ? d->gunaccel=4 : d->gunselect==GUN_S_ROQUETTES ? d->gunaccel=3 : d->gunaccel=12;
                break;
            default: d->gunaccel=0;
        }

        if(attacktime < d->gunwait + d->gunaccel*(d->gunselect==GUN_PULSE ? 50 : d->gunselect==GUN_S_ROQUETTES ? 150 : 8) + (d==player1 || (d->aptitude==APT_PRETRE && d->abilitymillis[ABILITY_3]) ? 0 : attacks[d->gunselect].attackdelay)) return;
        d->gunwait = 0;

        if(d->aptitude==APT_KAMIKAZE)
        {
            if(d->abilitymillis[ABILITY_2]>0 && d->abilitymillis[ABILITY_2]<2000 && d->ammo[GUN_KAMIKAZE]>0 && !d->playerexploded)
            {
                gunselect(GUN_KAMIKAZE, d);
                d->attacking = ACT_SHOOT;
                d->lastattack = -1;
                d->playerexploded = true;
            }
        }

        if(d->armourtype==A_ASSIST && !d->armour && !d->playerexploded && d->ammo[GUN_ASSISTXPL])
        {
            gunselect(GUN_ASSISTXPL, d, true);
            d->attacking = ACT_SHOOT;
            d->lastattack = -1;
            d->playerexploded = true;
        }

        if(!d->attacking) return;
        int gun = d->gunselect, act = d->attacking, atk = guns[gun].attacks[act];

        if(d->gunaccel>0)d->gunaccel-=1;
        d->lastaction = lastmillis;
        d->lastattack = atk;

        if(d==player1)
        {
            lastshoot = totalmillis;
            if(atk==ATK_SMAW_SHOOT || atk==ATK_NUKE_SHOOT || atk==ATK_CACMARTEAU_SHOOT || atk == ATK_MOSSBERG_SHOOT || atk == ATK_SV98_SHOOT) lastshoot+=750;
        }

        if (!d->ammo[gun])
        {
            if(d==player1) msgsound(S_NOAMMO, d);
            d->gunwait = 600;
            d->lastattack = -1;
            weaponswitch(d);
            return;
        }

        if(!m_muninfinie || atk==ATK_GAU8_SHOOT || atk==ATK_NUKE_SHOOT || atk==ATK_CAMPOUZE_SHOOT || atk==ATK_ROQUETTES_SHOOT || atk==ATK_KAMIKAZE_SHOOT || atk==ATK_ASSISTXPL_SHOOT) d->ammo[gun] -= attacks[atk].use;

        vec from = d->o, to = targ, dir = vec(to).sub(from).safenormalize();
        float dist = to.dist(from);

        int kickfactor = (m_tutorial && !canmove) || d->aptitude==APT_AMERICAIN ? 0 : (d->crouched() ? -1.25f : -2.5f);
        vec kickback = (d->aptitude==APT_AMERICAIN ? vec(0, 0, 0) : vec(dir).mul(attacks[atk].kickamount*kickfactor));
        d->vel.add(kickback);

        float shorten = attacks[atk].range && dist > attacks[atk].range ? attacks[atk].range : 0,
              barrier = raycube(d->o, dir, dist, RAY_CLIPMAT|RAY_ALPHAPOLY);
        if(barrier > 0 && barrier < dist && (!shorten || barrier < shorten))
            shorten = barrier;
        if(shorten) to = vec(dir).mul(shorten).add(from);

        if(attacks[atk].rays > 1) createrays(atk, from, to, d);
        else if(attacks[atk].spread) offsetray(from, to, attacks[atk].spread, attacks[atk].nozoomspread, attacks[atk].range, to, d);

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
        if(d->boostmillis[B_SHROOMS]>0) waitfactor*=1.5f;
        d->gunwait = attacks[atk].attackdelay/waitfactor;
        //if(d->ai) d->gunwait += int(d->gunwait*(((101-d->skill)+rnd(111-d->skill))/100.f));
        d->boostmillis[B_ROIDS] ? d->totalshots += (attacks[atk].damage*attacks[atk].rays)*2: d->totalshots += attacks[atk].damage*attacks[atk].rays;

        if(d->playerexploded){d->attacking = ACT_IDLE; d->playerexploded = false; execute("getoldweap"); }
        if(atk==ATK_GLOCK_SHOOT || atk==ATK_SPOCKGUN_SHOOT || atk==ATK_HYDRA_SHOOT || d->gunselect==GUN_SKS || d->gunselect==GUN_S_CAMPOUZE) d->attacking = ACT_IDLE;
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
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            if(bnc.bouncetype==BNC_GRENADE || bnc.bouncetype==BNC_LIGHT)
            {
                vec pos(bnc.o);
                pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
                if(bnc.bouncetype==BNC_GRENADE) adddynlight(pos, 40, vec(0.5f, 0.5f, 2.0f));
                else adddynlight(pos, 80, vec(0.3f, 0.15f, 0.0f), 0, 0, L_VOLUMETRIC|L_NODYNSHADOW|L_NOSHADOW|L_NOSPEC);
            }
        }
    }

    static const char * const gibnames[6] = { "bouncers/pix_noir", "bouncers/pix_jaune", "bouncers/pix_rouge", "bouncers/pix_noir_alt", "bouncers/pix_jaune_alt", "bouncers/pix_rouge_alt" };
    static const char * const douillesnames[1] = { "bouncers/douille" };
    static const char * const bigdouillesnames[1] = { "bouncers/douille_big" };
    static const char * const cartouchessnames[1] = { "bouncers/cartouche" };
    static const char * const debrisnames[4] = { "bouncers/pierre_1", "bouncers/pierre_2", "bouncers/pierre_3", "bouncers/pierre_4" };
    static const char * const robotnames[3] = { "bouncers/robot_1", "bouncers/robot_2", "bouncers/robot_3" };

    void preloadbouncers()
    {
        loopi(sizeof(gibnames)/sizeof(gibnames[0])) preloadmodel(gibnames[i]);
        loopi(sizeof(douillesnames)/sizeof(douillesnames[0])) preloadmodel(douillesnames[i]);
        loopi(sizeof(bigdouillesnames)/sizeof(bigdouillesnames[0])) preloadmodel(bigdouillesnames[i]);
        loopi(sizeof(cartouchessnames)/sizeof(cartouchessnames[0])) preloadmodel(cartouchessnames[i]);
        loopi(sizeof(debrisnames)/sizeof(debrisnames[0])) preloadmodel(debrisnames[i]);
        loopi(sizeof(robotnames)/sizeof(robotnames[0])) preloadmodel(robotnames[i]);
    }

    void renderbouncers()
    {
        float yaw, pitch;
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            vec pos(bnc.o);
            pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
            vec vel(bnc.vel);
            pitch = -bnc.roll;
            if(vel.magnitude() <= 10.0f) {yaw = bnc.lastyaw; pitch = bnc.lastpitch;}
            else
            {
                vectoyawpitch(vel, yaw, pitch);
                if(!ispaused()) yaw += bnc.bounces < 5 ? 75+rnd(31) : 90;
                bnc.lastyaw = yaw;
                bnc.lastpitch = pitch;
            }
			int cull = MDL_CULL_VFC|MDL_CULL_EXTDIST|MDL_CULL_OCCLUDED;

            if(bnc.bouncetype==BNC_GRENADE) rendermodel("projectiles/grenade", ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, cull);
            else
            {
                const char *mdl = NULL;
                float fade = 1;
                if(bnc.lifetime < 500) fade = bnc.lifetime/500.0f;
                switch(bnc.bouncetype)
                {
                    case BNC_GIBS: mdl = gibnames[bnc.variant]; break;
                    case BNC_DEBRIS: mdl = debrisnames[bnc.variant]; break;
                    case BNC_ROBOT: mdl = robotnames[bnc.variant]; break;
                    case BNC_DOUILLES: case BNC_DOUILLESUZI: mdl = "bouncers/douille"; break;
                    case BNC_BIGDOUILLES: mdl = "bouncers/douille_big"; break;
                    case BNC_CARTOUCHES: mdl = "bouncers/cartouche"; break;
                    default: return;
                }
                rendermodel(mdl, ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, 0, cull, NULL, NULL, 0, 0, fade);
            }
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
                rendermodel(p.atk==ATK_SMAW_SHOOT ? "projectiles/missile" : p.atk==ATK_ARTIFICE_SHOOT ? "projectiles/feuartifice" : p.atk==ATK_ROQUETTES_SHOOT ? "projectiles/minimissile" : p.atk==ATK_NUKE_SHOOT ? "projectiles/missilenuke" :"projectiles/fleche", ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, MDL_CULL_VFC|MDL_CULL_OCCLUDED);
                if(ispaused()) return;
                switch(p.atk)
                {
                    case ATK_NUKE_SHOOT:
                        particle_flare(pos, pos, 1, PART_MF_LITTLE, p.owner->boostmillis[B_ROIDS] ? 0xFF0000 : 0xFFC864, 10.f+rndscale(8), NULL, gfx::champicolor());
                        particle_splash(lookupmaterial(pos)==MAT_WATER ? PART_BUBBLE : PART_SMOKE, 3, lookupmaterial(pos)==MAT_WATER ? 2000 : 5000, pos, lookupmaterial(pos)&MAT_WATER ? 0x18181A : 0x222222, 4.0f+rnd(5), 25, 200, 0, gfx::champicolor());
                        particle_splash(PART_FIRE_BALL, 1, 100, pos, p.owner->boostmillis[B_ROIDS] ? 0xFF0000 : 0xFF6600, 1.0f+rndscale(4), 50, 500, 0, gfx::champicolor());
                        regularflare(pos, 0x552500, 600+rnd(400), 75);
                        break;
                    case ATK_SMAW_SHOOT:
                    case ATK_ROQUETTES_SHOOT:
                        particle_splash(lookupmaterial(pos)==MAT_WATER ? PART_BUBBLE : PART_SMOKE, 1, 2000, pos, 0x666666, lookupmaterial(pos)==MAT_WATER ? 3.f : 6.f, 25, 250, 0, gfx::champicolor());
                        particle_flare(pos, pos, 1, PART_MF_LITTLE, p.owner->boostmillis[B_ROIDS] ? 0xFF0000 : 0xFFC864, 3.0f+rndscale(2), NULL, gfx::champicolor());
                        regularflare(pos, 0x331200, 300+rnd(300), 50);
                        break;
                    case ATK_ARTIFICE_SHOOT:
                        if(lookupmaterial(pos)==MAT_WATER)  particle_splash(PART_BUBBLE, 3, 200, pos, 0x18181A, 2.5f, 25, 100, 0, gfx::champicolor());
                        particle_splash(PART_SPARK, 8, 100, pos, p.owner->boostmillis[B_ROIDS] ? 0xFF0000 : 0xFFC864, rnd(4)/10.f+0.1f, 50, 500, 0, gfx::champicolor());
                        particle_flare(pos, pos, 1, PART_MF_LITTLE, p.owner->boostmillis[B_ROIDS] ? 0xFF0000 : 0xFFC864, 0.5f+rndscale(2), NULL, gfx::champicolor());
                        break;
                }
            }
        }
    }

    void checkattacksound(gameent *d, bool local)
    {
        int atk = -1;
        switch(d->attacksound)
        {
            case S_FLAMETHROWER:
                atk = ATK_LANCEFLAMMES_SHOOT;
                break;
            case S_GAU8:
                atk = ATK_GAU8_SHOOT;
                break;
            case S_PLASMARIFLE_SFX:
                atk = ATK_PULSE_SHOOT;
                break;
            default:
                return;
        }
        if(atk >= 0 && atk < NUMATKS &&
           d->clientnum >= 0 && d->state == CS_ALIVE &&
           d->lastattack == atk && lastmillis - d->lastaction < attacks[atk].attackdelay + 50)
        {
            d->attackchan = playsound(d->attacksound, local ? NULL : &d->o, NULL, 0, -1, -1, d->attackchan, 300);
            if(d->attackchan < 0) d->attacksound = -1;
        }
        else d->stopattacksound(d);
    }

    void checkdansesound(gameent *d, bool local)
    {
        if(!packtaunt) return;
        if(d->clientnum >= 0 && d->state == CS_ALIVE && lastmillis - d->lasttaunt < 5000)
        {
            d->dansechan = playsound(S_CGCORTEX+(d->customdanse), local ? NULL : &d->o, NULL, 0, -1, -1, d->dansechan, 400);
            if(d->dansechan < 0) d->dansesound = -1;
        }
        else d->stopdansesound(d);
    }

    void checkabiounds(gameent *d, bool local)
    {
        if(d->clientnum >= 0 && d->state == CS_ALIVE)
        {
            loopi(3) if(d->abisnd[i] >= 0) {d->abichan[i] = playsound(d->abisnd[i], local ? NULL : &d->o, NULL, 0, -1, -1, d->abichan[i], 300); if(d->abichan[i] < 0) d->abisnd[i] = -1;}
        }
        else d->stopabisound(d);
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
        loopv(players)
        {
            gameent *d = players[i];
            checkattacksound(d, d==following);
            checkdansesound(d, d==following);
            checkabiounds(d, d==following);
        }
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
