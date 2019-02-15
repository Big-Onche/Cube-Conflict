// weapon.cpp: all shooting and effects code, projectile management
#include "game.h"
#include "engine.h"
#include "../cubeconflict/cubedef.h"

namespace game
{
    static const int OFFSETMILLIS = 500;
    vec rays[MAXRAYS];

    struct hitmsg
    {
        int target, lifesequence, info1, info2;
        ivec dir;
    };
    vector<hitmsg> hits;

#if 0
    #define MINDEBRIS 3
    VARP(maxdebris, MINDEBRIS, 10, 100);
    VARP(maxgibs, 0, 4, 100);
#endif

    ICOMMAND(getweapon, "", (), intret(player1->gunselect));

    void gunselect(int gun, gameent *d)
    {
        if(gun!=d->gunselect)
        {
            addmsg(N_GUNSELECT, "rci", d, gun);
            playsound(S_WEAPLOAD, &d->o, 0, 0, 0 , 100, -1, 300);
        }
        d->gunselect = gun;
    }

    void nextweapon(int dir, bool force = false)
    {
        if(player1->state!=CS_ALIVE) return;
        dir = (dir < 0 ? NUMGUNS-1 : 1);
        int gun = player1->gunselect;
        loopi(NUMGUNS)
        {
            gun = (gun + dir)%NUMGUNS;
            if(force || player1->ammo[gun]) break;
        }
        if(gun != player1->gunselect) {gunselect(gun, player1); weapposup=40; }
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
        if(player1->state!=CS_ALIVE || !validgun(gun)) return;
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
            if(gun>=0 && gun<NUMGUNS && (force || player1->ammo[gun]))
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
        if(d->state!=CS_ALIVE) return;
        int s = d->gunselect;
        if(s!=GUN_PULSE             && d->ammo[GUN_PULSE])          s = GUN_PULSE;
        else if(s!=GUN_RAIL         && d->ammo[GUN_RAIL])           s = GUN_RAIL;
        else if(s!=GUN_SMAW         && d->ammo[GUN_SMAW])           s = GUN_SMAW;
        else if(s!=GUN_MINIGUN      && d->ammo[GUN_MINIGUN])        s = GUN_MINIGUN;
        else if(s!=GUN_SPOCKGUN     && d->ammo[GUN_SPOCKGUN])       s = GUN_SPOCKGUN;
        else if(s!=GUN_LANCEFLAMMES && d->ammo[GUN_LANCEFLAMMES])   s = GUN_LANCEFLAMMES;
        else if(s!=GUN_UZI          && d->ammo[GUN_UZI])            s = GUN_UZI;
        else if(s!=GUN_FAMAS        && d->ammo[GUN_FAMAS])          s = GUN_FAMAS;
        else if(s!=GUN_MOSSBERG     && d->ammo[GUN_MOSSBERG])       s = GUN_MOSSBERG;
        else if(s!=GUN_HYDRA        && d->ammo[GUN_HYDRA])          s = GUN_HYDRA;
        else if(s!=GUN_SV98         && d->ammo[GUN_SV98])           s = GUN_SV98;
        else if(s!=GUN_SKS          && d->ammo[GUN_SKS])            s = GUN_SKS;
        else if(s!=GUN_ARBALETE     && d->ammo[GUN_ARBALETE])       s = GUN_ARBALETE;
        else if(s!=GUN_AK47         && d->ammo[GUN_AK47])           s = GUN_AK47;
        else if(s!=GUN_GRAP1        && d->ammo[GUN_GRAP1])          s = GUN_GRAP1;
        else if(s!=GUN_ARTIFICE     && d->ammo[GUN_ARTIFICE])       s = GUN_ARTIFICE;
        else if(s!=GUN_GLOCK        && d->ammo[GUN_GLOCK])          s = GUN_GLOCK;

        else if(s!=GUN_S_NUKE       && d->ammo[GUN_S_NUKE])         s = GUN_S_NUKE;
        else if(s!=GUN_S_GAU8       && d->ammo[GUN_S_GAU8])         s = GUN_S_GAU8;
        else if(s!=GUN_S_ROQUETTES  && d->ammo[GUN_S_ROQUETTES])    s = GUN_S_ROQUETTES;
        else if(s!=GUN_S_CAMPOUZE   && d->ammo[GUN_S_CAMPOUZE])     s = GUN_S_CAMPOUZE;

        else if(s!=GUN_CAC349       && d->ammo[GUN_CAC349])         s = GUN_CAC349;
        else if(s!=GUN_CACMASTER    && d->ammo[GUN_CACMASTER])      s = GUN_CACMASTER;
        else if(s!=GUN_CACMARTEAU   && d->ammo[GUN_CACMARTEAU])     s = GUN_CACMARTEAU;
        else if(s!=GUN_CACFLEAU     && d->ammo[GUN_CACFLEAU])       s = GUN_CACFLEAU;
        gunselect(s, d);
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
                if(validgun(gun) && gun != player1->gunselect && player1->ammo[gun]) { gunselect(gun, player1); return; }
            } else { weaponswitch(player1); return; }
        }
        playsound(S_NOAMMO);
    });

    void offsetray(const vec &from, const vec &to, int spread, int nozoomspread, float range, vec &dest, gameent *d)
    {
        vec offset;
        do offset = vec(rndscale(1), rndscale(1), rndscale(1)).sub(0.5f);
        while(offset.squaredlen() > 0.5f*0.5f);

        if(d->champimillis)
        {
            spread/= (d->champimillis/1000);
            nozoomspread/= (d->champimillis/1000);
        }
        if(d->aptitude==APT_CAMPEUR && d->crouching)
        {
            spread/= 2;
            nozoomspread/= 2;
        }
        if(d->steromillis || d->ragemillis)
        {
            spread*=2;
            nozoomspread*=2;
        }

        spread = (spread*100)/aptitudes[d->aptitude].apt_precision;
        nozoomspread = (nozoomspread*100)/aptitudes[d->aptitude].apt_precision;

        if(d==player1)offset.mul((to.dist(from)/1024)*(zoom ? spread : nozoomspread));
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

    enum { BNC_GRENADE, BNC_KAMIKAZE, BNC_GIBS, BNC_DEBRIS, BNC_DOUILLES,};

    struct bouncer : physent
    {
        int lifetime, bounces;
        float lastyaw, roll;
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
            case BNC_GRENADE: case BNC_DOUILLES: bnc.collidetype = COLLIDE_ELLIPSE; break;
            case BNC_DEBRIS: bnc.variant = rnd(4); break;
            case BNC_GIBS: bnc.variant = rnd(3); break;
        }

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
        if(b->bouncetype == BNC_DOUILLES && b->bounces <= 2) {b->bounces++; playsound(S_CARTOUCHE, &b->o, 0, 0, 0 , 50, -1, 150); }
        if(b->bouncetype == BNC_GRENADE && b->bounces <= 3) {b->bounces++; playsound(S_RGRENADE, &b->o, 0, 0, 0 , 100, -1, 350); }
        b->bounces++;
        if(b->bouncetype != BNC_GIBS || b->bounces >= 3) return;
        addstain(STAIN_BLOOD, vec(b->o).sub(vec(surface).mul(b->radius)), surface, 2.96f/b->bounces, bvec(0x60, 0xFF, 0xFF), rnd(4));
    }

    void updatebouncers(int time)
    {
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            vec old(bnc.o);
            if(bnc.bouncetype==BNC_DOUILLES && bnc.vel.magnitude() > 50.0f)
            {
                vec pos(bnc.o);
                pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
                regular_particle_splash(PART_SMOKE, 1, 150, pos, 0x404040, 1.0f, 50, -20);
            }
            if(bnc.bouncetype==BNC_GRENADE && bnc.vel.magnitude() > 50.0f)
            {
                vec pos(bnc.o);
                pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
                regular_particle_splash(PART_SMOKE, 1, 150, pos, 0x404040, 2.4f, 50, -20);
            }
            if(bnc.bouncetype==BNC_KAMIKAZE && bnc.vel.magnitude() > 50.0f)
                {
                vec pos(bnc.o);
                pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
            }
            if(bnc.bouncetype==BNC_GIBS && bnc.vel.magnitude() > 10.0f)
            {
                vec pos(bnc.o);
                pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
                switch(rnd(16)) { case 1: regular_particle_splash(PART_BLOOD, 1, 9999, pos, 0x60FFFF  , 1.0f, 50);}
            }
            if(bnc.bouncetype==BNC_DEBRIS)
            {
                vec pos(bnc.o);
                regular_particle_splash(PART_SMOKE, 3, 250, pos, 0x222222, 2.0f, 50, -50);
            }

            bool stopped = false;
            if(bnc.bouncetype==BNC_GRENADE) stopped = bounce(&bnc, 0.6f, 0.5f, 0.8f) || (bnc.lifetime -= time)<0;
            else
            {
                for(int rtime = time; rtime > 0;)
                {
                    int qtime = min(30, rtime);
                    rtime -= qtime;
                    if((bnc.lifetime -= qtime)<0 || bounce(&bnc, qtime/1000.0f, 0.6f, 0.5f, 1)) { stopped = true; break; }
                }
            }

            if(stopped)
            {
                if(bnc.bouncetype==BNC_GRENADE)
                {
                    hits.setsize(0);
                    explode(bnc.local, bnc.owner, bnc.o, bnc.o, NULL, 1, ATK_M32_SHOOT); // 1 = qdam
                    if(bnc.local)
                        addmsg(N_EXPLODE, "rci3iv", bnc.owner, lastmillis-maptime, ATK_M32_SHOOT, bnc.id-maptime,
                                hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
                }
                if(bnc.bouncetype==BNC_KAMIKAZE)
                {
                    hits.setsize(0);
                    explode(bnc.local, bnc.owner, bnc.o, bnc.o, NULL, 1, ATK_KAMIKAZE_SHOOT); // 1 = qdam
                    if(bnc.local)
                        addmsg(N_EXPLODE, "rci3iv", bnc.owner, lastmillis-maptime, ATK_KAMIKAZE_SHOOT, bnc.id-maptime,
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
        int projchan, projsound;

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
        p.offsetmillis = OFFSETMILLIS;
        p.id = local ? lastmillis : id;
    }

    void removeprojectiles(gameent *owner)
    {
        // can't use loopv here due to strange GCC optimizer bug
        int len = projs.length();
        loopi(len) if(projs[i].owner==owner) { projs.remove(i--); len--; }
    }

    VARP(blood, 0, 1, 1);

    void spawnbouncer(const vec &p, const vec &vel, gameent *d, int type)
    {
        vec to(rnd(100)-50, rnd(100)-50, rnd(100)-50);
        if(to.iszero()) to.z += 1;
        to.normalize();
        to.add(p);
        newbouncer(p, to, true, 0, d, type, rnd(1000)+1000, rnd(100)+20);
    }

    void damageeffect(int damage, gameent *d, gameent *actor, bool thirdperson, int atk)
    {
        if(atk!=ATK_MEDIGUN_SHOOT) { if(actor!=d) if(isteam(d->team, actor->team)) return; } //

        vec p = d->o;
        p.z += 0.6f*(d->eyeheight + d->aboveeye) - d->eyeheight;
        if(d->armourtype!=A_MAGNET)
        {
            if(blood) particle_splash(PART_BLOOD, damage/10, 1000, p, 0x60FFFF, 2.96f);
            if(damage>=600) playsound(S_SANG, &d->o, 0, 0, 0 , 100, -1, 250);
            gibeffect(damage, vec(0,0,0), d);
        }
        if(thirdperson)
        {
            actor->steromillis > 0 ? damage*=actor->aptitude==13 ? 3 : 2 : damage+=0;
            damage = (((damage*aptitudes[actor->aptitude].apt_degats)/100)*100)/aptitudes[d->aptitude].apt_resistance;
            damage = damage/10.0f;

            if(actor->aptitude==3)
            {
                if(atk==ATK_CAC349_SHOOT || atk==ATK_CACFLEAU_SHOOT || atk==ATK_CACMARTEAU_SHOOT || atk==ATK_CACMASTER_SHOOT) particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*3.0f), PART_TEXT, 2000, 0xFF0000, actor==player1 ? 7.0f : 5.0f, -8);
                else particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*1.0f), PART_TEXT, 2000, damage<0 ? 0x22FF22 : actor->steromillis > 0 ? 0xFF0000: 0xFF4B19,  actor==player1 ?  7.0f : 3.0f, -8);
            }
            else if(actor->aptitude==9)
            {
                if(atk==ATK_SV98_SHOOT || atk==ATK_SKS_SHOOT || atk==ATK_ARBALETE_SHOOT || atk==ATK_CAMPOUZE_SHOOT) particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*2.0f), PART_TEXT, 2000, 0xFF0000, actor==player1 ? 7.0f : 5.0f, -8);
                else particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*1.0f), PART_TEXT, 2000, damage<0 ? 0x22FF22 : actor->steromillis > 0 ? 0xFF0000: 0xFF4B19,  actor==player1 ?  7.0f : 3.0f, -8);
            }
            else if(actor->aptitude==12)
            {
                if(actor->ragemillis) particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*1.5f), PART_TEXT, 2000, 0xAA0000, actor==player1 ? 10.0f : 7.0f, -8);
                else particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*1.0f), PART_TEXT, 2000, damage<0 ? 0x22FF22 : actor->steromillis > 0 ? 0xFF0000: 0xFF4B19,  actor==player1 ?  7.0f : 3.0f, -8);
            }
            else particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*1.0f), PART_TEXT, 2000, damage<0 ? 0x22FF22 : actor->steromillis > 0 ? 0xFF0000: 0xFF4B19, actor==player1 ? 7.0f : 3.0f, -8);
        }
    }

    void gibeffect(int damage, const vec &vel, gameent *d)
    {
        if(damage < 0) return;
        loopi(damage/300) spawnbouncer(d->o, vec(0,0,0), d, BNC_GIBS);
    }

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

        if(!m_mp(gamemode) || f==at) f->hitpush(damage, vel, at, atk);
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
                damageeffect(damage, f, at, true, atk);
                if(f==player1)
                {
                    if(atk==ATK_MEDIGUN_SHOOT)
                    {
                        regenblend(damage);
                        regencompass(damage, at ? at->o : f->o);
                    }
                    else
                    {
                        damageblend(damage);
                        damagecompass(damage, at ? at->o : f->o);
                        playsound(S_BALLECORPS);
                        switch(rnd(2)) {case 0: if(player1->armour>0)playsound(S_BALLEBOUCLIER); break; }
                        playsound(S_PAIN2);
                    }

                }
                else switch(rnd(2)) {case 0: if(f->armour>0)playsound(S_BALLEBOUCLIERENT, &f->o, 0, 0, 0 , 100, -1, 200); break; }
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
            float damage = attacks[atk].damage*(1-dist/EXP_DISTSCALE/attacks[atk].exprad);
            if(o==at) damage /= EXP_SELFDAMDIV;
            if(damage > 0) hit(max(int(damage), 1), o, at, dir, atk, dist);
            if((atk==ATK_ARTIFICE_SHOOT || atk==ATK_SMAW_SHOOT || atk==ATK_M32_SHOOT || atk==ATK_ROQUETTES_SHOOT || atk==ATK_KAMIKAZE_SHOOT) && dist<attacks[atk].exprad*2.0f && o==player1) execfile("config/shake.cfg");
        }
    }

    void explode(bool local, gameent *owner, const vec &v, const vec &vel, dynent *safe, int damage, int atk)
    {
        vec debrisvel = owner->o==v ? vec(0, 0, 0) : vec(owner->o).sub(v).normalize(), debrisorigin(v);

        switch(atk)
        {
            case ATK_PULSE_SHOOT:
            {
                playsound(S_IMPACTALIEN, &v, 0, 0, 0 , 100, -1, 250);
                particle_splash(PART_SPARK, 200, 300, v, 0xFF4400, 0.45f);
                particle_fireball(v, 1.15f*attacks[atk].exprad, PART_PULSE_BURST, int(attacks[atk].exprad*20), 0x221100, 2.5f);
                vec debrisorigin = vec(v).sub(vec(vel).mul(5));
                adddynlight(safe ? v : debrisorigin, 2*attacks[atk].exprad, vec(4.0f, 1.0f, 0.0f), 350, 40, 0, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
            }
            break;

            case ATK_GRAP1_SHOOT:
            {
                playsound(S_IMPACTGRAP1, &v, 0, 0, 0 , 100, -1, 250);
                particle_splash(PART_SPARK, 200, 100, v, 0xAA4466, 0.2f, 150);
                particle_fireball(v, 1.15f*attacks[atk].exprad, PART_PULSE_BURST, int(attacks[atk].exprad*20), 0x330011, 1.5f);
                vec debrisorigin = vec(v).sub(vec(vel).mul(5));
                adddynlight(safe ? v : debrisorigin, 2*attacks[atk].exprad, vec(3.0f, 0.0f, 2.0f), 350, 40, 0, attacks[atk].exprad/2, vec(3.0f, 0.0f, 2.0f));
            }
            break;

            case ATK_SPOCKGUN_SHOOT:
            {
                playsound(S_IMPACTALIEN, &v, 0, 0, 0 , 100, -1, 250);
                particle_splash(PART_SPARK, 200, 300, v, 0x22FF22, 0.45f);
                particle_fireball(v, 1.15f*attacks[atk].exprad, PART_PULSE_BURST, int(attacks[atk].exprad*20), 0x22FF22, 2.0f);
                vec debrisorigin = vec(v).sub(vec(vel).mul(5));
                adddynlight(safe ? v : debrisorigin, 2*attacks[atk].exprad, vec(0.0f, 4.0f, 0.0f), 350, 40, 0, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
            }
            break;
            case ATK_SMAW_SHOOT:
            {
                //particle_splash(PART_SPARK, 200, 300, v, 0xFF8800, 0.45f);
                playsound(S_EXPLOSION, &v, 0, 0, 0 , 100, -1, 350);
                if(camera1->o.dist(v) >= 300) playsound(S_EXPLOSIONLOIN, &v, NULL, 0, 0, 100, -1, 1200);
                vec debrisorigin = vec(v).sub(vec(vel).mul(5));
                adddynlight(safe ? v : debrisorigin, 7*attacks[atk].exprad, vec(8.0f, 4.0f, 0.0f), 80, 40, 0, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                particle_splash(PART_SMOKE,  9, 2000, v, 0x333333, 40.0f,  150,   300);
                particle_splash(PART_SMOKE,  9, 1300, v, 0x333333, 25.0f,  150,   600);
                particle_splash(PART_SPARK, 12,  150, v, 0xFFC864,  1.7f, 1500,  1500);
                particle_splash(PART_SPARK, 12,  200, v, 0xFFAA44,  1.7f, 2500,  2500);
                particle_splash(PART_SPARK, 12,  250, v, 0xFFAA44,  1.7f, 3500,  3500);
                particle_splash(PART_FLAME1+rnd(2),  15,  120, v, 0xCC8833, 17+rnd(5),  800, 800);
                particle_splash(PART_FLAME1+rnd(2),  15,  120, v, 0xCC7722, 17+rnd(5), 1000, 800);
                particle_splash(PART_FLAME1+rnd(2),  15,  120, v, 0xCC4422, 17+rnd(5), 1200, 800);
                particle_fireball(v, 350, PART_ONDECHOC, 300, 0xFF5500, 10.0f);
                particle_fireball(v, 350, PART_ONDECHOC, 300, 0xFFFFFF, 20.0f);
                loopi(5+rnd(3)) spawnbouncer(debrisorigin, debrisvel, owner, BNC_DEBRIS);
            }
            case ATK_ROQUETTES_SHOOT:
            {
                //particle_splash(PART_SPARK, 200, 300, v, 0xFF8800, 0.45f);
                playsound(S_EXPLOSION, &v, 0, 0, 0 , 100, -1, 350);
                if(camera1->o.dist(v) >= 300) playsound(S_EXPLOSIONLOIN, &v, NULL, 0, 0, 100, -1, 1200);
                vec debrisorigin = vec(v).sub(vec(vel).mul(5));
                adddynlight(safe ? v : debrisorigin, 7*attacks[atk].exprad, vec(8.0f, 4.0f, 0.0f), 80, 40, 0, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                particle_splash(PART_SMOKE,  6, 2000, v, 0x333333, 30.0f,  125,   200);
                particle_splash(PART_SMOKE,  6, 1300, v, 0x333333, 15.0f,  125,   400);
                particle_splash(PART_SPARK,  5,  150, v, 0xFFC864,  1.7f, 1000,  1000);
                particle_splash(PART_SPARK,  5,  200, v, 0xFFAA44,  1.7f, 1500,  1500);
                particle_splash(PART_SPARK,  5,  250, v, 0xFFAA44,  1.7f, 2000,  2000);
                particle_splash(PART_FLAME1+rnd(2),   7,  120, v, 0xCC8833, 17+rnd(5),  500, 500);
                particle_splash(PART_FLAME1+rnd(2),   7,  120, v, 0xCC7722, 17+rnd(5),  700, 500);
                particle_splash(PART_FLAME1+rnd(2),   7,  120, v, 0xCC4422, 17+rnd(5),  900, 500);
                particle_fireball(v, 350, PART_ONDECHOC, 300, 0xFF5500, 10.0f);
                particle_fireball(v, 350, PART_ONDECHOC, 300, 0xFFFFFF, 20.0f);
                loopi(5+rnd(3)) spawnbouncer(debrisorigin, debrisvel, owner, BNC_DEBRIS);
            }
            break;
            case ATK_KAMIKAZE_SHOOT:
            {
                //particle_splash(PART_SPARK, 200, 300, v, 0xFF8800, 0.45f);
                //playsound(S_KAMIKAZEBOOM, &v);
                if(camera1->o.dist(v) >= 300) playsound(S_EXPLOSIONLOIN, &v, NULL, 0, 0, 100, -1, 1200);
                vec debrisorigin = vec(v).sub(vec(vel).mul(5));
                adddynlight(safe ? v : debrisorigin, 7*attacks[atk].exprad, vec(8.0f, 4.0f, 0.0f), 80, 40, 0, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                particle_splash(PART_SMOKE,  6, 2000, v, 0x333333, 30.0f,  200,   400);
                particle_splash(PART_SMOKE,  6, 1300, v, 0x333333, 15.0f,  200,   800);
                particle_splash(PART_SPARK,  5,  150, v, 0xFFC864,  1.7f, 1750,  2000);
                particle_splash(PART_SPARK,  5,  200, v, 0xFFAA44,  1.7f, 2250,  3000);
                particle_splash(PART_SPARK,  5,  250, v, 0xFFAA44,  1.7f, 3000,  4000);
                particle_splash(PART_FLAME1+rnd(2),   7,  120, v, 0xCC8833, 17+rnd(5),  800, 700);
                particle_splash(PART_FLAME1+rnd(2),   7,  120, v, 0xCC7722, 17+rnd(5), 1000, 700);
                particle_splash(PART_FLAME1+rnd(2),   7,  120, v, 0xCC4422, 17+rnd(5), 1500, 700);
                particle_fireball(v, 350, PART_ONDECHOC, 300, 0xFF5500, 10.0f);
                particle_fireball(v, 350, PART_ONDECHOC, 300, 0xFFFFFF, 20.0f);
                loopi(5+rnd(3)) spawnbouncer(debrisorigin, debrisvel, owner, BNC_DEBRIS);
            }
            break;
            case ATK_NUKE_SHOOT:
            {
                //particle_splash(PART_SPARK, 200, 300, v, 0xFF8800, 0.45f);
                playsound(S_NUKE);
                adddynlight(safe ? v : debrisorigin, 7*attacks[atk].exprad, vec(8.0f, 4.0f, 0.0f), 80, 40, 0, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                particle_splash(PART_SMOKE, 50, 2000, v, 0x212121, 150.0f, 700, 70);
                particle_splash(PART_SMOKE, 50, 15000, v, 0x222222, 200.0f, 500, 300);
                particle_splash(PART_SMOKE, 100, 5000, v, 0x333333, 250.0f, 1000, 500);
                particle_splash(PART_FLAME1+rnd(2),  100,  800, v, 0xFFFF00, 75+rnd(50), 1500, 800);
                particle_splash(PART_FLAME1+rnd(2),  100,  800, v, 0x224400, 75+rnd(50), 1500, 800);
                particle_splash(PART_FLAME1+rnd(2),  100,  800, v, 0xFF2222, 75+rnd(50), 1200, 600);
                //particle_fireball(v, 350, PART_ONDECHOC, 300, 0xFF5500, 10.0f);
                //particle_fireball(v, 350, PART_ONDECHOC, 300, 0xFFFFFF, 20.0f);
                particle_fireball(v, 100, PART_EXPLOSION, 300, 0xFFEEDD, 1200.0f);
                particle_fireball(v, 350, PART_ONDECHOC, 300, 0xFFFFFF, 300.0f);
                //loopi(5+rnd(3)) spawnbouncer(debrisorigin, debrisvel, owner, BNC_DEBRIS);
            }
            break;
            case ATK_ARTIFICE_SHOOT:
            {
                //particle_fireball(v,  50, PART_EXPLOSION, 120, 0xFFFFFF, 30.0f);
                playsound(S_EXPLOSIONARTIFICE, &v, 0, 0, 0 , 100, -1, 350);
                if(camera1->o.dist(v) >= 300) playsound(S_ARTIFICELOIN, &v, NULL, 0, 0, 100, -1, 1000);
                particle_splash(PART_FLAME1+rnd(2),  5,  40, v, 0xFFC864, 20,  800, 1600);
                particle_splash(PART_GLOWSPARK, 12+rnd(10),  200, v, rnd(16777215),  0.75f+rnd(2),   500, 5000);
                particle_splash(PART_GLOWSPARK, 12+rnd(10),  250, v, rnd(16777215),  1.0f+rnd(2),    600, 6000);
                particle_splash(PART_GLOWSPARK, 12+rnd(10),  300, v, rnd(16777215),  1.25f+rnd(2),   700, 7000);
                particle_splash(PART_GLOWSPARK, 12+rnd(10),  350, v, rnd(16777215),  1.5f+rnd(2),    800, 8000);

                adddynlight(safe ? v : debrisorigin, 7*attacks[atk].exprad, vec(1.5f, 1.5f, 1.5f), 80, 40, 0, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
            }
            break;
            case ATK_M32_SHOOT:
            {
                playsound(S_EXPLOSIONGRENADE, &v, 0, 0, 0 , 100, -1, 350);
                if(camera1->o.dist(v) >= 300) playsound(S_EXPLOSIONLOIN, &v, NULL, NULL, 0, 0, 100, -1, 1200);
                particle_fireball(v, 40, PART_PLASMA, 300, 0xFFFFFF, 1.0f);
                particle_fireball(v, 60, PART_PLASMA, 300, 0xAAAAFF, 1.0f);
                particle_fireball(v, 80, PART_PLASMA, 300, 0x5555FF, 1.0f);
                adddynlight(v, 7*attacks[atk].exprad, vec(0.0f, 3.0f, 9.0f), 80, 40, 0, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                particle_splash(PART_SMOKE,  9, 2000, v, 0x555555, 40.0f,  150,   300);
                particle_splash(PART_SMOKE,  9, 1300, v, 0x555555, 25.0f,  250,   600);
                particle_splash(PART_SPARK, 10,  150, v, 0xFFFFFF,  2.0f, 1500,  1500);
                particle_splash(PART_SPARK, 10,  200, v, 0xFFFFFF,  2.0f, 2500,  2500);
                particle_splash(PART_SPARK, 10,  250, v, 0xFFFFFF,  2.0f, 3500,  3500);
                particle_splash(PART_FLAME1+rnd(2),   15,  120, v, 0x0000FF, 17+rnd(5), 1200, 800);
                particle_fireball(v, 350, PART_ONDECHOC, 300, 0xFF5500, 10.0f);
                particle_fireball(v, 350, PART_ONDECHOC, 300, 0xFFFFFF, 20.0f);
                vec debrisorigin = vec(v).sub(vec(vel).mul(5));
                loopi(5+rnd(3)) spawnbouncer(debrisorigin, debrisvel, owner, BNC_DEBRIS);
            }
            break;
            case ATK_MINIGUN_SHOOT:
            case ATK_AK47_SHOOT:
            {
                playsound(S_IMPACT, &v, 0, 0, 0 , 100, -1, 250);
                particle_splash(PART_SPARK,  8, 70,   v, 0xFF6600, 0.35f, 400, 150);
                particle_splash(PART_SMOKE,  3, 1500, v, 0x444444, 5.00f,  50, 100);
                particle_splash(PART_SMOKE,  6,  800, v, 0x442200, 4.00f,  30, 100);
                //particle_explodesplash(v, 15, PART_IMPACT, 0xFF8800, 10, 500, 7);
            }
            case ATK_UZI_SHOOT:
            case ATK_FAMAS_SHOOT:
            case ATK_MOSSBERG_SHOOT:
            case ATK_GLOCK_SHOOT:
            case ATK_HYDRA_SHOOT:
            {
                playsound(S_IMPACT, &v, 0, 0, 0 , 100, -1, 250);
                particle_splash(PART_SPARK,  8, 70,   v, 0xFF6600, 0.25f, 400, 150);
                particle_splash(PART_SMOKE,  3, 1500, v, 0x444444, 3.50f,  50, 100);
                particle_splash(PART_SMOKE,  6,  800, v, 0x442200, 2.00f,  30, 100);
                //particle_explodesplash(v, 15, PART_IMPACT, 0xFF8800, 10, 500, 7);
            }
            break;
            case ATK_SV98_SHOOT:
            case ATK_SKS_SHOOT:
            case ATK_GAU8_SHOOT:
            case ATK_CAMPOUZE_SHOOT:
            {
                //spawnbouncer(d->muzzle, d->muzzle, d, BNC_DOUILLES);
                playsound(S_IMPACTLOURDLOIN, &v, 0, 0, 0 , 700, -1, 1000);
                playsound(S_IMPACTSNIPE, &v, 0, 0, 0 , 100, -1, 300);
                particle_splash(PART_SPARK,  9,   70, v, 0xAA6600, 0.24f, 400, 150);
                particle_splash(PART_SMOKE,  4, 1500, v, 0x444444, 5.00f,  50, 175);
                particle_splash(PART_SMOKE,  4,  800, v, 0x442200, 4.00f,  30, 100);
            }
            break;
            case ATK_CAC349_SHOOT:
            case ATK_CACMARTEAU_SHOOT:
            case ATK_CACMASTER_SHOOT:
            case ATK_CACFLEAU_SHOOT:
                particle_splash(PART_SMOKE,  2, 1500, v, 0x444444, 2.50f,  50, 175);
                particle_splash(PART_SMOKE,  2,  800, v, 0x442200, 2.00f,  30, 100);
            break;
        }

#if 0
        int numdebris = maxdebris > MINDEBRIS ? rnd(maxdebris-MINDEBRIS)+MINDEBRIS : min(maxdebris, MINDEBRIS);
        if(numdebris)
        {
            vec debrisvel = vec(vel).neg();
            loopi(numdebris)
                spawnbouncer(debrisorigin, debrisvel, owner, BNC_DEBRIS);
        }
#endif
        if(!local) return;
        int numdyn = numdynents();
        loopi(numdyn)
        {
            dynent *o = iterdynents(i);
            if(o->o.reject(v, o->radius + attacks[atk].exprad) || o==safe) continue;
            radialeffect(o, v, vel, damage, owner, atk);
        }
    }

    void pulsestain(const projectile &p, const vec &pos)
    {
        vec dir = vec(p.dir).neg();
        float rad = attacks[p.atk].exprad*0.5f;
        addstain(STAIN_PULSE_SCORCH, pos, dir, rad);
        addstain(STAIN_PULSE_GLOW, pos, dir, rad, 0xAA3300);
    }

    void spockstain(const projectile &p, const vec &pos)
    {
        vec dir = vec(p.dir).neg();
        float rad = attacks[p.atk].exprad*0.5f;
        addstain(STAIN_PULSE_SCORCH, pos, dir, rad);
        addstain(STAIN_SPOCK_GLOW, pos, dir, rad, 0x22FF22);
    }

    void smawstain(const projectile &p, const vec &pos)
    {
        vec dir = vec(p.dir).neg();
        float rad = attacks[p.atk].exprad*0.75f;
        addstain(STAIN_PULSE_SCORCH, pos, dir, rad);
    }

    void flamestain(const projectile &p, const vec &pos)
    {
        vec dir = vec(p.dir).neg();
        addstain(STAIN_BRULAGE, pos, dir, 15.0f);
    }

    void bigballestain(const projectile &p, const vec &pos)
    {
        vec dir = vec(p.dir).neg();
        switch(rnd(3))
        {
            case 0: addstain(STAIN_BALLE_1, pos, dir, 1.5f+(rnd(2))); break;
            case 1: addstain(STAIN_BALLE_2, pos, dir, 1.5f+(rnd(2))); break;
            case 2: addstain(STAIN_BALLE_3, pos, dir, 1.5f+(rnd(2))); break;
        }
        addstain(STAIN_BALLE_GLOW, pos, dir, 2.0f+(rnd(2)), 0x883300);
    }

    void littleballestain(const projectile &p, const vec &pos)
    {
        vec dir = vec(p.dir).neg();
        switch(rnd(3))
        {
            case 0: addstain(STAIN_BALLE_1, pos, dir, 0.5f); break;
            case 1: addstain(STAIN_BALLE_2, pos, dir, 0.5f); break;
            case 2: addstain(STAIN_BALLE_3, pos, dir, 0.5f); break;
        }
        addstain(STAIN_BALLE_GLOW, pos, dir, 1.0f, 0x883300);
    }

    void grap1stain(const projectile &p, const vec &pos)
    {
        vec dir = vec(p.dir).neg();
        addstain(STAIN_RAIL_GLOW, pos, dir, 5.0f, 0xFF4455);
        adddynlight(vec(pos).madd(dir, 4), 30, vec(1.5f, 0.2f, 0.2f), 50, 75);
        addstain(STAIN_PULSE_SCORCH, pos, dir, 10);
    }

    void projsplash(projectile &p, const vec &v, dynent *safe)
    {
        explode(p.local, p.owner, v, p.dir, safe, attacks[p.atk].damage, p.atk);
        switch(p.atk)
        {
            case ATK_PULSE_SHOOT: pulsestain(p, v); break;
            case ATK_SMAW_SHOOT:
            case ATK_KAMIKAZE_SHOOT:
            case ATK_ROQUETTES_SHOOT: smawstain(p, v); break;
            case ATK_MINIGUN_SHOOT:
            case ATK_SV98_SHOOT:
            case ATK_AK47_SHOOT:
            case ATK_GAU8_SHOOT:
            case ATK_CAMPOUZE_SHOOT:
            case ATK_SKS_SHOOT: bigballestain(p, v); break;
            case ATK_SPOCKGUN_SHOOT: spockstain(p, v); break;
            case ATK_UZI_SHOOT:
            case ATK_MOSSBERG_SHOOT:
            case ATK_HYDRA_SHOOT:
            case ATK_ARBALETE_SHOOT:
            case ATK_GLOCK_SHOOT:
            case ATK_FAMAS_SHOOT: littleballestain(p, v); break;
            case ATK_GRAP1_SHOOT: grap1stain(p, v); break;
            case ATK_LANCEFLAMMES_SHOOT: flamestain(p, v); break;
        }

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
                switch(atk)
                {
                    case ATK_PULSE_SHOOT: pulsestain(p, pos); break;
                    case ATK_SMAW_SHOOT:
                    case ATK_KAMIKAZE_SHOOT:
                    case ATK_ROQUETTES_SHOOT: smawstain(p, pos); break;
                    case ATK_MINIGUN_SHOOT:
                    case ATK_SV98_SHOOT:
                    case ATK_AK47_SHOOT:
                    case ATK_GAU8_SHOOT:
                    case ATK_CAMPOUZE_SHOOT:
                    case ATK_SKS_SHOOT: bigballestain(p, pos); break;
                    case ATK_ARBALETE_SHOOT:
                    case ATK_UZI_SHOOT:
                    case ATK_MOSSBERG_SHOOT:
                    case ATK_HYDRA_SHOOT:
                    case ATK_GLOCK_SHOOT:
                    case ATK_FAMAS_SHOOT: littleballestain(p, pos); break;
                    case ATK_GRAP1_SHOOT: grap1stain(p, pos); break;
                    case ATK_LANCEFLAMMES_SHOOT: flamestain(p, pos); break;
                    default: break;
                }
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
        if(projs.empty()) return;
        //gameent *noside = hudplayer();
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
            if(p.local)
            {
                vec halfdv = vec(dv).mul(0.5f), bo = vec(p.o).add(halfdv);
                float br = max(fabs(halfdv.x), fabs(halfdv.y)) + 1 + attacks[p.atk].margin;
                loopj(numdynents())
                {
                    dynent *o = iterdynents(j);
                    if(p.owner==o || o->o.reject(bo, o->radius + br)) continue;
                    if(projdamage(o, p, v)) { exploded = true; break; }
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
                    projsplash(p, v, NULL);
                    exploded = true;
                }
                else
                {
                    vec pos = vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)).add(v);

                    float len = min(20.0f, vec(p.offset).add(p.from).dist(pos));
                        vec dir = vec(dv).normalize(),
                        tail = vec(dir).mul(-len).add(pos),
                        head = vec(dir).mul(2.4f).add(pos);

                    bool canplaysound = false;

                    switch(p.atk)
                    {
                        case ATK_PULSE_SHOOT:
                            particle_splash(PART_PULSE_FRONT, 1, 1, pos, 0xFF2200, 2.4f, 150, 20);
                            if(p.owner->steromillis) loopi(2)particle_flare(tail, head, 1, PART_PULSE_SIDE, 0xFF0000, 2.5f);
                            else particle_flare(tail, head, 1, PART_PULSE_SIDE, 0xFF2200, 2.0f);
                            p.projsound = S_FLYBYALIEN;
                            canplaysound = true;
                            break;
                        case ATK_GRAP1_SHOOT:
                            particle_splash(PART_PULSE_FRONT, 1, 1, pos, 0xFF0033, 3.0f, 150, 20);
                            if(p.owner->steromillis) loopi(2)particle_flare(tail, head, 1, PART_PULSE_SIDE, 0xFF0000, 2.5f);
                            else particle_flare(tail, head, 1, PART_PULSE_SIDE, 0xCC0011, 3.0f);
                            regular_particle_splash(PART_SMOKE, 1, 500, pos, 0xAAAAAA, 4.0f, 25, 250);
                            p.projsound = S_FLYBYGRAP1;
                            canplaysound = true;
                            break;
                        case ATK_SPOCKGUN_SHOOT:
                            particle_splash(PART_SPOCK_FRONT, 1, 1, pos, 0x00AA00, 6.0f, 150, 20);
                            if(p.owner->steromillis) loopi(2)particle_flare(tail, head, 1, PART_PULSE_SIDE, 0xFF0000, 3.5f);
                            else particle_flare(tail, head, 1, PART_PULSE_SIDE, 0x22FF22, 2.5f);
                            p.projsound = S_FLYBYALIEN;
                            canplaysound = true;
                            break;
                        case ATK_MINIGUN_SHOOT:
                        case ATK_AK47_SHOOT:
                            if(p.owner->steromillis) loopi(2)particle_flare(tail, head, 1, PART_BALLE_SIDE, 0xFF0000, 3.0f);
                            else particle_flare(tail, head, 1, PART_BALLE_SIDE, 0xFF7722, 2.5f);
                            break;
                        case ATK_SV98_SHOOT:
                        case ATK_SKS_SHOOT:
                        case ATK_CAMPOUZE_SHOOT:
                        case ATK_GAU8_SHOOT:
                            //particle_splash(PART_PULSE_FRONT, 1, 1, pos, 0xFF7722, 0.4f, 150, 20);
                            //particle_trail(PART_SMOKE, 500, pos, to, 0x404040, 0.6f, 20);
                            particle_flare(tail, head, 200+(rnd(800)), PART_SMOKE, 0x404040, 1.0f+(rnd(2)));
                            if(p.owner->steromillis) loopi(2)particle_flare(tail, head, 1, PART_BALLE_SIDE, 0xFF0000,  ATK_GAU8_SHOOT ? 7.0f : 3.5f);
                            else particle_flare(tail, head, 1, PART_BALLE_SIDE, 0xFF7722, ATK_GAU8_SHOOT ? 6.0f : 3.0f);
                            break;
                        case ATK_UZI_SHOOT:
                        case ATK_MOSSBERG_SHOOT:
                        case ATK_HYDRA_SHOOT:
                        case ATK_GLOCK_SHOOT:
                        case ATK_FAMAS_SHOOT:
                            if(p.owner->steromillis) loopi(2)particle_flare(tail, head, 1, PART_BALLE_SIDE, 0xFF0000, 2.75f);
                            else particle_flare(tail, head, 1, PART_BALLE_SIDE, 0xFF7722, 2.00f);
                            break;
                        case ATK_SMAW_SHOOT:
                            p.projsound = S_MISSILE;
                            canplaysound = true;
                            break;
                        case ATK_ROQUETTES_SHOOT:
                            p.projsound = S_MINIMISSILE;
                            canplaysound = true;
                            break;
                        case ATK_NUKE_SHOOT:
                            p.projsound = S_MISSILENUKE;
                            canplaysound = true;
                            break;
                        case ATK_ARTIFICE_SHOOT:
                            p.projsound = S_ARTIFICE;
                            canplaysound = true;
                            break;
                    }
                    if(canplaysound) p.projchan = playsound(p.projsound, &pos, NULL, 0, -1, 128, p.projchan, 400);
                }
            }
            if(exploded)
            {
                if(p.local)
                    addmsg(N_EXPLODE, "rci3iv", p.owner, lastmillis-maptime, p.atk, p.id-maptime,
                            hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
                projs.remove(i--);
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
            //vec tmp = vec(float(rnd(11)-5), float(rnd(11)-5), float(rnd(11)-5));
            if(camera1->o.dist(p) <= 32 && !soundused)
            {
                playsound(sound, &p, 0, 0, 0 , 100, -1, 200);
                soundused = true;
            }
        }
    }

    void railhit(const vec &from, const vec &to, bool stain = true)
    {
        vec dir = vec(from).sub(to).safenormalize();
        if(stain)
        {
            addstain(STAIN_RAIL_HOLE, to, dir, 2.0f);
            addstain(STAIN_RAIL_GLOW, to, dir, 1.5f, 0xFF2200);
            addstain(STAIN_RAIL_GLOW, to, dir, 2.5f, 0xFF8800);
        }
        adddynlight(vec(to).madd(dir, 4), 10, vec(1.00f, 0.5f, 0.0f), 225, 75);
    }

    bool looped;

    void shoteffects(int atk, const vec &from, const vec &to, gameent *d, bool local, int id, int prevaction)     // create visual effect from a shot
    {
        int gun = attacks[atk].gun;
        int sound = attacks[atk].sound;
        //int soundwater = attacks[atk]].soundwater;

        switch(atk)
        {
            case ATK_PULSE_SHOOT:
                particle_flare(d->muzzle, d->muzzle, 140, PART_PULSE_MUZZLE_FLASH, d->steromillis ? 0xFF0000 : 0xFF4400, zoom ? 1.00f : 3.50f, d);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 140, PART_PULSE_MUZZLE_FLASH, 0xFF0000, zoom ? 1.50f : 5.50f, d);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d==player1) mousemove(-5+rnd(6),-5+rnd(6));
                break;

            case ATK_SPOCKGUN_SHOOT:
                particle_flare(d->muzzle, d->muzzle, 140, PART_PULSE_MUZZLE_FLASH, d->steromillis ? 0xFF0000 : 0x22FF22, zoom ? 1.0f : 2.25f, d);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 140, PART_PULSE_MUZZLE_FLASH, 0xFF0000, zoom ? 1.5f : 4.00f, d);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                break;

            case ATK_RAIL_SHOOT:
                playsound(S_IMPACTELEC, &to, 0, 0, 0 , 100, -1, 250);
                if(d!=hudplayer()) sound_nearmiss(S_FLYBYELEC, from, to);

                particle_splash(PART_SPARK, 200, 250, to, 0xFF4400, 0.45f);
                //particle_flare(hudgunorigin(gun, from, to, d), to, 1, PART_LIGHTNING, 0xFFFF00);
                particle_flare(d->muzzle, to,  50, PART_LIGHTNING, 0x8888FF, 2.0f);
                particle_flare(d->muzzle, to,  80, PART_LIGHTNING, 0x8888FF, 2.0f);
                particle_flare(d->muzzle, to, 100, PART_LIGHTNING, 0x0000FF, 3.0f);
                particle_flare(d->muzzle, d->muzzle, 140, PART_RAIL_MUZZLE_FLASH, d->steromillis ? 0xFF0000 : 0x50CFFF, zoom ? 1.75f : 3.0f, d);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 140, PART_RAIL_MUZZLE_FLASH, 0xFF0000, zoom ? 2.5f : 5.5f, d);
                adddynlight(hudgunorigin(gun, d->o, to, d), 35, vec(0.25f, 0.75f, 2.0f), 75, 75, DL_FLASH, 0, vec(0, 0, 0), d);
                if(!local) railhit(from, to);
                break;

            case ATK_SMAW_SHOOT:
            case ATK_ROQUETTES_SHOOT:
            case ATK_NUKE_SHOOT:
                particle_flare(d->muzzle, d->muzzle, 250, PART_NORMAL_MUZZLE_FLASH, d->steromillis ? 0xFF0000 : 0xFF7700, ATK_ROQUETTES_SHOOT ? 3.0f : 7.00f, d);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 250, PART_NORMAL_MUZZLE_FLASH, 0xFF0000, ATK_ROQUETTES_SHOOT ? 5.0f : 12.00f, d);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                break;

            case ATK_ARTIFICE_SHOOT:
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d->ragemillis) particle_splash(PART_SPARK,  6, 500, d->muzzle, 0xFF0000, 1.0f,  50,   200);
                break;

            case ATK_MINIGUN_SHOOT:
            case ATK_AK47_SHOOT:
                spawnbouncer(d->muzzle, d->muzzle, d, BNC_DOUILLES);
                particle_flare(d->muzzle, d->muzzle, 100, PART_MINIGUN_MUZZLE_FLASH, d->steromillis ? 0xFF0000 : 0xFF7722, zoom ? 1.5f : 3.5f, d);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 100, PART_MINIGUN_MUZZLE_FLASH, 0xFF0000, zoom ? 2.0f : 5.5f, d);
                particle_splash(PART_SMOKE,  4, 500, d->muzzle, 0x444444, 3.5f, 20, 500);
                adddynlight(hudgunorigin(gun, d->o, to, d), 15, vec(1.0f, 0.75f, 0.5f), 30, 100, DL_FLASH, 0, vec(0, 0, 0), d);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d!=hudplayer()) sound_nearmiss(S_FLYBY, from, to);
                if(d==player1) mousemove(ATK_MINIGUN_SHOOT ? -7+rnd(15) : -3+rnd(4), ATK_MINIGUN_SHOOT ? -7+rnd(15) :  -3+rnd(4));
                break;
            case ATK_GAU8_SHOOT:
                if(d->type==ENT_PLAYER) sound = S_GAU8;
                spawnbouncer(d->muzzle, d->muzzle, d, BNC_DOUILLES);
                particle_flare(d->muzzle, d->muzzle, 100, PART_MINIGUN_MUZZLE_FLASH, d->steromillis ? 0xFF0000 : 0xFF7722, zoom ? 1.5f : 3.0f, d);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 100, PART_MINIGUN_MUZZLE_FLASH, 0xFF0000, zoom ? 5.0f : 12.0f, d);
                particle_splash(PART_SMOKE,  4, 500, d->muzzle, 0x444444, 3.5f, 20, 500);
                adddynlight(hudgunorigin(gun, d->o, to, d), 15, vec(1.0f, 0.75f, 0.5f), 30, 100, DL_FLASH, 0, vec(0, 0, 0), d);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d!=hudplayer()) sound_nearmiss(S_FLYBYSNIPE, from, to);
                if(d==player1) mousemove(-12+rnd(25), -12+rnd(25));
                break;
            case ATK_MOSSBERG_SHOOT:
            case ATK_HYDRA_SHOOT:
                loopi(ATK_HYDRA_SHOOT ? 3 : 2)spawnbouncer(d->muzzle, d->muzzle, d, BNC_DOUILLES);
                particle_flare(d->muzzle, d->muzzle, 140, PART_NORMAL_MUZZLE_FLASH, d->steromillis ? 0xFF0000 : 0xFF7722, zoom ? 1.25f : 3.50f, d);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 140, PART_NORMAL_MUZZLE_FLASH, 0xFF0000, zoom ? 2.00f : 5.50f, d);
                particle_splash(PART_SMOKE,  4, 500, d->muzzle, 0x444444, 3.5f, 20, 500);
                adddynlight(hudgunorigin(gun, d->o, to, d), 15, vec(1.0f, 0.75f, 0.5f), 30, 100, DL_FLASH, 0, vec(0, 0, 0), d);
                loopi(attacks[atk].rays)
                {
                    newprojectile(from, rays[i], attacks[atk].projspeed, local, id, d, atk);
                    if(d!=hudplayer()) sound_nearmiss(S_FLYBY, from, rays[i]);
                }
                break;
            case ATK_SV98_SHOOT:
            case ATK_SKS_SHOOT:
            case ATK_CAMPOUZE_SHOOT:
                spawnbouncer(d->muzzle, d->muzzle, d, BNC_DOUILLES);
                particle_splash(PART_SMOKE,  4,  600,   d->muzzle, 0x444444, 2.0f, 20, 500);
                particle_flare(d->muzzle, d->muzzle, 100, PART_NORMAL_MUZZLE_FLASH, d->steromillis ? 0xFF0000 : 0xFFFFFF, 1.25f, d);
                particle_flare(d->muzzle, d->muzzle, 100, PART_SNIPE_MUZZLE_FLASH, d->steromillis ? 0xFF0000 : 0xFFFFFF, 5.0f, d);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 75, PART_SNIPE_MUZZLE_FLASH, 0xFF0000, 8.0f, d);
                adddynlight(hudgunorigin(gun, d->o, to, d), 25, vec(1.0f, 0.75f, 0.5f), 75, 75, DL_FLASH, 0, vec(0, 0, 0), d);
                if(atk==ATK_CAMPOUZE_SHOOT)
                {
                    loopi(attacks[atk].rays)
                    {
                        newprojectile(from, rays[i], attacks[atk].projspeed, local, id, d, atk);
                        if(d!=hudplayer()) sound_nearmiss(S_FLYBY, from, rays[i]);
                    }
                }
                else newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d!=hudplayer()) sound_nearmiss(S_FLYBYSNIPE, from, to);
                break;
            case ATK_ARBALETE_SHOOT:
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d->ragemillis) particle_splash(PART_SPARK,  8, 500, d->muzzle, 0xFF0000, 1.0f,  50,   200);
                if(d!=hudplayer()) sound_nearmiss(S_FLECHE, from, to);
                break;
            case ATK_UZI_SHOOT:
            case ATK_FAMAS_SHOOT:
            case ATK_GLOCK_SHOOT:
                spawnbouncer(d->muzzle, d->muzzle, d, BNC_DOUILLES);
                particle_flare(d->muzzle, d->muzzle, 140, PART_MINIGUN_MUZZLE_FLASH, d->steromillis ? 0xFF0000 : 0xFF7722, zoom ? 0.75f : 2.00f, d);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 80, PART_MINIGUN_MUZZLE_FLASH, 0xFF00002, zoom ? 1.5f : 5.00f, d);
                particle_splash(PART_SMOKE,  4, 500, d->muzzle, 0x444444, 2.0f, 20, 500);
                adddynlight(hudgunorigin(gun, d->o, to, d), 15, vec(1.0f, 0.75f, 0.5f), 30, 100, DL_FLASH, 0, vec(0, 0, 0), d);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d!=hudplayer()) sound_nearmiss(S_FLYBY, from, to);
                if(d==player1) {if(atk!=ATK_GLOCK_SHOOT) mousemove(-4+rnd(5), -4+rnd(5));}
                break;
            case ATK_MEDIGUN_SHOOT:
            {
                if(d->type==ENT_PLAYER) sound = S_MEDIGUN;
                loopi(attacks[atk].rays)
                {
                    vec irays(rays[i]);
                    irays.sub(d->muzzle);
                    irays.normalize().mul(1300.0f);
                    switch(rnd(2)) { case 1:  particle_flying_flare(d->muzzle, irays, 400, PART_SANTE, 0xFFFFFF, 0.5f+rnd(3), 100); }
                    newprojectile(from, rays[i], attacks[atk].projspeed, local, id, d, atk);
                }
                adddynlight(hudgunorigin(gun, d->o, to, d), 15, vec(1.0f, 0.75f, 0.5f), 100, 100, DL_FLASH, 0, vec(0, 0, 0), d);
                break;
            }
            case ATK_LANCEFLAMMES_SHOOT:
            {
                if(d->type==ENT_PLAYER) sound = S_FLAMEATTACK;
                loopi(attacks[atk].rays)
                {
                    vec irays(rays[i]);
                    irays.sub(d->muzzle);
                    irays.normalize().mul(1300.0f);
                    switch(rnd(3)) { case 1:  particle_flying_flare(d->muzzle, irays, 700, PART_FLAME1+rnd(2), d->steromillis ? 0xAA0000 :  0x994422, 6, 100); }
                    switch(rnd(3)) { case 1:  particle_flying_flare(d->muzzle, irays, 700, PART_FLAME1+rnd(2), d->steromillis ? 0x990000 :  0xBB0011, 6, 100); }
                    switch(rnd(3)) { case 1:  particle_flying_flare(d->muzzle, irays, 700, PART_FLAME1+rnd(2), d->steromillis ? 0xBB0000 :  0x991100, 6, 100); }
                    //switch(rnd(20)) { case 1:  particle_splash(PART_FLAME1+rnd(4),  1*lodparticules, 1500, to, arprecision ? rndcolor : 0x444444, 1.00f,  50, 175, NULL, 1, false, 1); }
                    particle_flying_flare(d->muzzle, irays, 900, PART_SMOKE, 0x111111, 7, 120);
                    newprojectile(from, rays[i], attacks[atk].projspeed, local, id, d, atk);
                }
                adddynlight(hudgunorigin(gun, d->o, to, d), 40, vec(1.0f, 0.75f, 0.5f), 20, 100, DL_FLASH, 0, vec(0, 0, 0), d);
                if(d->ragemillis) particle_splash(PART_SPARK,  3, 500, d->muzzle, 0xFF0000, 1.0f,  50,   200);
                break;
            }
            case ATK_GRAP1_SHOOT:
            {
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                particle_flare(d->muzzle, d->muzzle, 150, PART_RAIL_MUZZLE_FLASH, d->steromillis ? 0xFF0000 : 0xFF00FF, 2.5f, d);
                if(d->ragemillis) particle_splash(PART_SPARK,  3, 500, d->muzzle, 0xFF0000, 1.0f,  50,   200);
                adddynlight(hudgunorigin(gun, d->o, to, d), 40, vec(1.0f, 0.0f, 1.0f), 100, 100, DL_FLASH, 0, vec(0, 0, 0), d);
                break;
            }
            case ATK_M32_SHOOT:
            {
                float dist = from.dist(to);
                vec up = to;
                up.z += dist/8;
                particle_splash(PART_SMOKE,  10, 600, d->muzzle, 0x444444, 4.0f, 20, 500);
                if(d->ragemillis) particle_splash(PART_SPARK,  8, 500, d->muzzle, 0xFF0000, 1.0f,  50,   200);
                newbouncer(d==player1 ? d->muzzle : hudgunorigin(gun, d->o, to, d), up, local, id, d, BNC_GRENADE, attacks[atk].ttl, attacks[atk].projspeed);
                break;
            }
            case ATK_KAMIKAZE_SHOOT:
            {
                float dist = from.dist(to);
                vec up = to;
                up.z += dist/8;
                particle_splash(PART_SMOKE,  10, 600, d->muzzle, 0x444444, 4.0f, 20, 500);
                newbouncer(d==player1 ? d->muzzle : hudgunorigin(gun, d->o, to, d), up, local, id, d, BNC_KAMIKAZE, attacks[atk].ttl, attacks[atk].projspeed);
                break;
            }
            case ATK_CAC349_SHOOT:
            case ATK_CACMARTEAU_SHOOT:
            case ATK_CACMASTER_SHOOT:
            case ATK_CACFLEAU_SHOOT:
                loopi(attacks[atk].rays)
                {
                    if(d!=hudplayer()) sound_nearmiss(S_EPEEATTACK, from, rays[i]);
                }
                break;

            default:
                break;
        }

        looped = false;

        if(d->attacksound >= 0 && d->attacksound != sound) d->stopattacksound();

        switch(sound)
        {
            case S_FLAMEATTACK:
            case S_MEDIGUN:
            case S_GAU8:
                if(d->attacksound >= 0) looped = true;
                d->attacksound = sound;
                d->attackchan = playsound(sound, d==hudplayer() ? NULL : &d->o, NULL, 0, -1, 100, d->attackchan, 300);
                return;
            default:
                {
                    if(d==hudplayer()) playsound(attacks[atk].hudsound, NULL);
                    else playsound(attacks[atk].sound, &d->o, NULL, 0, 0 , 50, -1, 350);
                }
        }

        if(camera1->o.dist(hudgunorigin(gun, d->o, to, d)) >= 250) playsound(attacks[atk].farsound1, &d->muzzle, NULL, 0, 0 , 200, -1, 600);
        else if(camera1->o.dist(hudgunorigin(gun, d->o, to, d)) >= 500 && camera1->o.dist(hudgunorigin(gun, d->o, to, d)) <= 850) playsound(attacks[atk].farsound2, &d->muzzle, NULL, 0, 0 , 200, -1, 800);
    }

    void particletrack(physent *owner, vec &o, vec &d)
    {
        if(owner->type!=ENT_PLAYER) return;
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
        if(owner->type!=ENT_PLAYER) return;
        gameent *pl = (gameent *)owner;
        if(pl->muzzle.x < 0 || pl->lastattack < 0 || attacks[pl->lastattack].gun != pl->gunselect) return;
        o = pl->muzzle;
        hud = owner == hudplayer() ? vec(pl->o).add(vec(0, 0, 2)) : pl->muzzle;
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
        int maxrays = attacks[atk].rays, margin = attacks[atk].margin;
        if(attacks[atk].rays > 1)
        {
            dynent *hits[MAXRAYS];
            loopi(maxrays)
            {
                if((hits[i] = intersectclosest(from, rays[i], d, margin, dist)))
                {
                    shorten(from, rays[i], dist);
                    if(atk==ATK_RAIL_SHOOT) railhit(from, rays[i], false);
                }
                else if(atk==ATK_RAIL_SHOOT) railhit(from, rays[i]);
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
            if(atk==ATK_RAIL_SHOOT) railhit(from, to, false);

            hitpush(attacks[atk].damage, o, d, from, to, atk, 1);
        }
        else if(atk==ATK_RAIL_SHOOT) railhit(from, to);
    }

    float kickfactor = 2.5f;

    void shoot(gameent *d, const vec &targ)
    {
        int prevaction = d->lastaction, attacktime = lastmillis-prevaction;
        if(attacktime<d->gunwait) return;
        d->gunwait = 0;
        if(!d->attacking) return;
        int gun = d->gunselect, act = d->attacking, atk = guns[gun].attacks[act];
        d->lastaction = lastmillis;
        d->lastattack = atk;
        if(!d->ammo[gun])
        {
            if(d==player1)
            {
                msgsound(S_NOAMMO, d);
                d->gunwait = 600;
                d->lastattack = -1;
                weaponswitch(d);
            }
            return;
        }
        //d->ammo[gun] -= attacks[atk].use;
        if(!m_random)
        {
            if(atk==ATK_CAC349_SHOOT || atk==ATK_CACMARTEAU_SHOOT || atk==ATK_CACMASTER_SHOOT || atk==ATK_CACFLEAU_SHOOT);
            else d->ammo[gun]--;
        }
        else if(atk==ATK_GAU8_SHOOT || atk==ATK_NUKE_SHOOT || atk==ATK_CAMPOUZE_SHOOT ||atk==ATK_ROQUETTES_SHOOT) d->ammo[gun]--;

        vec from = d->o, to = targ, dir = vec(to).sub(from).safenormalize();
        float dist = to.dist(from);
        if(!(d->physstate >= PHYS_SLOPE && d->crouching && d->crouched()))
        {
            switch (d->aptitude)
            {
                case 2: kickfactor = 0; break;
                case 3: if(d->gunselect==GUN_CAC349 || d->gunselect==GUN_CACFLEAU || d->gunselect==GUN_CACMARTEAU || d->gunselect==GUN_CACMASTER) kickfactor = 15.0f; break;
                default: kickfactor = 2.5f;
            }
            vec kickback = vec(dir).mul(attacks[atk].kickamount*-kickfactor);
            d->vel.add(kickback);
        }
        float shorten = attacks[atk].range && dist > attacks[atk].range ? attacks[atk].range : 0,
              barrier = raycube(d->o, dir, dist, RAY_CLIPMAT|RAY_ALPHAPOLY);
        if(barrier > 0 && barrier < dist && (!shorten || barrier < shorten))
            shorten = barrier;
        if(shorten) to = vec(dir).mul(shorten).add(from);

        if(attacks[atk].rays > 1) createrays(atk, from, to, d);
        else if(attacks[atk].spread) offsetray(from, to, attacks[atk].spread, attacks[atk].nozoomspread, attacks[atk].range, to, d);

        hits.setsize(0);

        if(!attacks[atk].projspeed)
        {
            if(attacks[atk].rays>1) {loopi(attacks[atk].rays) raydamage(from, rays[i], d, atk);}
            else raydamage(from, to, d, atk);
        }

        shoteffects(atk, from, to, d, true, 0, prevaction);

        if(d==player1 || d->ai)
        {
            addmsg(N_SHOOT, "rci2i6iv", d, lastmillis-maptime, atk,
                   (int)(from.x*DMF), (int)(from.y*DMF), (int)(from.z*DMF),
                   (int)(to.x*DMF), (int)(to.y*DMF), (int)(to.z*DMF),
                   hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
        }

        d->gunwait = attacks[atk].attackdelay;
        if(d->ai) d->gunwait += int(d->gunwait*(((101-d->skill)+rnd(111-d->skill))/100.f));
        d->steromillis ? d->totalshots += (attacks[atk].damage*attacks[atk].rays)*2: d->totalshots += attacks[atk].damage*attacks[atk].rays;
    }

    void adddynlights()
    {
        loopv(projs)
        {
            projectile &p = projs[i];
            vec pos(p.o);
            pos.add(vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)));
            switch(p.atk)
            {
                case ATK_PULSE_SHOOT: adddynlight(pos, 30, vec(1.00f, 0.75f, 0.0f)); break;
                case ATK_LANCEFLAMMES_SHOOT: switch(rnd(2)) {case 0: adddynlight(pos, 50, vec(0.60f, 0.30f, 0.0f));} break;
                case ATK_SPOCKGUN_SHOOT: adddynlight(pos, 30, vec(0.00f, 1.00f, 0.0f)); break;
                case ATK_GRAP1_SHOOT: adddynlight(pos, 50, vec(0.3f, 0.00f, 0.2f)); break;
                case ATK_ARTIFICE_SHOOT:
                case ATK_SMAW_SHOOT:
                case ATK_ROQUETTES_SHOOT: adddynlight(pos, 50, vec(1.2f, 0.75f, 0.0f)); break;
                case ATK_NUKE_SHOOT: adddynlight(pos, 100, vec(1.2f, 0.75f, 0.0f)); break;
            }
        }
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            if(bnc.bouncetype!=BNC_GRENADE) continue;
            vec pos(bnc.o);
            pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
            adddynlight(pos, 40, vec(0.5f, 0.5f, 2.0f));
        }
    }

    static const char * const gibnames[3] = { "pixels/noir", "pixels/jaune", "pixels/rouge" };
    static const char * const douillesnames[1] = { "douille" };
    static const char * const debrisnames[1] = { "pixels/noir" };

    void preloadbouncers()
    {
        loopi(sizeof(gibnames)/sizeof(gibnames[0])) preloadmodel(gibnames[i]);
        loopi(sizeof(debrisnames)/sizeof(debrisnames[0])) preloadmodel(debrisnames[i]);
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
            if(vel.magnitude() <= 25.0f) yaw = bnc.lastyaw;
            else
            {
                vectoyawpitch(vel, yaw, pitch);
                yaw += 90;
                bnc.lastyaw = yaw;
            }
            pitch = -bnc.roll;

            if(bnc.bouncetype==BNC_GRENADE)
                rendermodel("projectiles/grenade", ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, MDL_CULL_VFC|MDL_CULL_OCCLUDED);
            else
            {
                const char *mdl = NULL;
                int cull = MDL_CULL_VFC|MDL_CULL_DIST|MDL_CULL_OCCLUDED;
                float fade = 1;
                if(bnc.lifetime < 250) fade = bnc.lifetime/250.0f;
                switch(bnc.bouncetype)
                {
                    case BNC_GIBS: mdl = gibnames[bnc.variant]; break;
                    case BNC_DEBRIS: mdl = debrisnames[bnc.variant]; break;
                    case BNC_DOUILLES: mdl = douillesnames[bnc.variant]; break;
                    default: continue;
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
                //if(lookupmaterial(pos)==MAT_WATER) regular_particle_splash(PART_BULLE, 4, 2000, pos, 0x666666, rnd(2)+2.0f, 50, 50);
                //else
                rendermodel(p.atk==ATK_SMAW_SHOOT ? "projectiles/missile" : p.atk==ATK_ARTIFICE_SHOOT ? "projectiles/feuartifice" : p.atk==ATK_ROQUETTES_SHOOT ? "projectiles/minimissile" : p.atk==ATK_NUKE_SHOOT ? "projectiles/missilenuke" :"projectiles/fleche", ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, MDL_CULL_VFC|MDL_CULL_OCCLUDED);
                switch(p.atk)
                {
                    case ATK_NUKE_SHOOT:
                        particle_flare(pos, pos, 1, PART_NORMAL_MUZZLE_FLASH, 0xFFC864, 3.0f+rndscale(9), NULL);
                        regular_particle_splash(PART_SMOKE, 3, 8000, pos, 0x111111, 7.0f, 25, 100);
                        regular_particle_splash(PART_FLAME1+rnd(2), 1, 100, pos, 0xFF6600, 1.0f+rndscale(4), 50, 500);
                        break;
                    case ATK_SMAW_SHOOT:
                    case ATK_ROQUETTES_SHOOT:
                        regular_particle_splash(PART_SMOKE, 1, 2000, pos, 0x666666, 6.0f, 25, 250);
                        particle_flare(pos, pos, 1, PART_NORMAL_MUZZLE_FLASH, 0xFFC864, 0.5f+rndscale(5), NULL);
                        break;
                    case ATK_ARTIFICE_SHOOT:
                        regular_particle_splash(PART_SPARK, 8, 100, pos, 0xFFC864, 0.4f, 50, 500);
                        regular_particle_splash(PART_FLAME1+rnd(2), 2, 100, pos, 0xFFFFFF, 0.8f, 50, 500);
                        particle_flare(pos, pos, 1, PART_NORMAL_MUZZLE_FLASH, 0xFFC864, 0.5f+rndscale(2), NULL);
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
            case S_FLAMEATTACK:
                atk = ATK_LANCEFLAMMES_SHOOT;
                break;
            case S_MEDIGUN:
                atk = ATK_MEDIGUN_SHOOT;
                break;
            case S_GAU8:
                atk = ATK_GAU8_SHOOT;
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
        else d->stopattacksound();
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
        loopv(players)
        {
            gameent *d = players[i];
            checkattacksound(d, d==following);
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
