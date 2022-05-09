// weapon.cpp: all shooting and effects code, projectile management
#include "game.h"
#include "engine.h"
#include "cubedef.h"

int lastshoot;

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

    void gunselect(int gun, gameent *d, bool force)
    {
        if(gun==GUN_ASSISTXPL && !force) return;
        if(gun!=d->gunselect)
        {
            addmsg(N_GUNSELECT, "rci", d, gun);
            playsound(S_WEAPLOAD, d==hudplayer() ? NULL : &d->o, 0, 0, 0 , 50, -1, 150);
        }
        d->gunselect = gun;
    }

    int oldweap = 0;

    ICOMMAND(getgrenade, "", (),
    {
         if(m_identique) return;
         oldweap = player1->gunselect;
         if(player1->ammo[GUN_M32]>0) gunselect(GUN_M32, player1);
    });

    void getcac()
    {
         if(m_identique && player1->aptitude!=APT_NINJA) return;
         oldweap = player1->gunselect;
         int gun = 0;
         loopi(4) if(player1->ammo[GUN_CAC349+i]>0) gun = GUN_CAC349+i;
         gunselect(gun, player1);
    }
    ICOMMAND(getcac, "", (), getcac(););

    ICOMMAND(getoldweap, "", (),
    {
         if(player1->aptitude != APT_NINJA && m_identique) return;
         gunselect(oldweap, player1);
    });

    void getsweap()
    {
         if(!m_identique) return;
         int gun = cnidentiquearme;
         loopi(4) if(player1->ammo[GUN_S_CAMPOUZE-i]>0) gun = GUN_S_CAMPOUZE-i;
         gunselect(gun, player1);
    }

    void nextweapon(int dir, bool force = false)
    {
        if(player1->state!=CS_ALIVE) return;
        if(m_identique)
        {
            switch(player1->aptitude)
            {
                case APT_KAMIKAZE:
                    if(player1->gunselect==cnidentiquearme) {dir-1 ? gunselect(GUN_KAMIKAZE, player1) : getsweap();}
                    else gunselect(cnidentiquearme, player1);
                    return;
                case APT_NINJA:
                    if(player1->gunselect==cnidentiquearme){dir-1 ? gunselect(GUN_CACNINJA, player1) : getsweap();}
                    else gunselect(cnidentiquearme, player1);
                    return;
                default:
                    if(player1->gunselect==cnidentiquearme) getsweap();
                    else gunselect(cnidentiquearme, player1);
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

        else if(s!=GUN_KAMIKAZE     && d->ammo[GUN_KAMIKAZE])       s = GUN_KAMIKAZE;
        else if(s!=GUN_CACNINJA     && d->ammo[GUN_CACNINJA])       s = GUN_CACNINJA;
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

        if(d->champimillis>0)
        {
            spread/= 1.5f;
            nozoomspread*= 1.5f;
        }
        if(d->aptitude==APT_MAGICIEN && d->aptisort2)
        {
            spread/=3;
            nozoomspread/=3;
        }
        if(d->crouching)
        {
            spread/=d->aptitude==APT_CAMPEUR ? 2.5f : 1.333f;
            nozoomspread/=d->aptitude==APT_CAMPEUR ? 2.5f : 1.333f;
        }
        if(d->steromillis || d->ragemillis)
        {
            spread*=1.75f;
            nozoomspread*=1.75f;
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

    VARP(bnclifetime, 1000, 5000, 30000);
    enum { BNC_GRENADE, BNC_KAMIKAZE, BNC_GIBS, BNC_DEBRIS, BNC_DOUILLES, BNC_BIGDOUILLES, BNC_CARTOUCHES, BNC_DOUILLESUZI, BNC_ASSISTEXPL, BNC_LIGHT};

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
            case BNC_GIBS: bnc.variant = rnd(3); break;
            default:  bnc.collidetype = COLLIDE_ELLIPSE;
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
        if(b->bouncetype == BNC_GIBS && b->bounces < 2) {switch(rnd(3)) {case 0: addstain(STAIN_BLOOD, vec(b->o).sub(vec(surface).mul(b->radius)), surface, 2.96f/b->bounces, bvec(0x60, 0xFF, 0xFF), rnd(4));}}
        if(b->bouncetype == BNC_GRENADE) addstain(STAIN_PULSE_GLOW, vec(b->o).sub(vec(surface).mul(b->radius)), surface, 4.f, 0x0000FF);
    }

    void updatebouncers(int time)
    {
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            vec old(bnc.o);

            if(bnc.vel.magnitude() > 25.0f)
            {
                vec pos(bnc.o);
                pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));

                switch(bnc.bouncetype)
                {
                    case BNC_DOUILLES: case BNC_DOUILLESUZI: case BNC_BIGDOUILLES: case BNC_CARTOUCHES: regular_particle_splash(PART_SMOKE, 1, 150, pos, 0x404040, BNC_DOUILLES==1 ? 1.0f : BNC_DOUILLESUZI==1 ? 0.75 : 1.75f, 50, -20); break;
                    case BNC_GRENADE: regular_particle_splash(PART_SMOKE, 1, 150, pos, 0x404040, 2.5f, 50, -20); break;
                    case BNC_DEBRIS:
                        regular_particle_splash(lookupmaterial(pos)&MAT_WATER ? PART_BULLE : PART_SMOKE, lookupmaterial(pos)&MAT_WATER ? 1 : 3, 250, pos, 0x222222, 2.5f, 50, -50);
                        regular_particle_splash(PART_FLAME1+rnd(2), 2, 75, pos, 0x994400, 0.7f, 30, -30);

                        break;
                    case BNC_GIBS: {switch(rnd(40)) {case 1: regular_particle_splash(PART_BLOOD, 1, 9999, pos, 0x60FFFF, 1.0f, 50);}}
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
                if(bnc.bouncetype==BNC_ASSISTEXPL)
                {
                    hits.setsize(0);
                    explode(bnc.local, bnc.owner, bnc.o, bnc.o, NULL, 1, ATK_ASSISTXPL_SHOOT); // 1 = qdam
                    if(bnc.local)
                        addmsg(N_EXPLODE, "rci3iv", bnc.owner, lastmillis-maptime, ATK_ASSISTXPL_SHOOT, bnc.id-maptime,
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
            case ATK_ARBALETE_SHOOT: p.lifetime=attacks[atk].ttl*decoration_lod; break;
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

    void spawnbouncer(const vec &p, const vec &vel, gameent *d, int type, int lifetime = rnd(1000)+1000)
    {
        vec to(0, 0, 0);

        switch(type)
        {
            case BNC_GIBS: to.add(vec(rnd(100)-50, rnd(100)-50, rnd(100)-50)); break;
            case BNC_DEBRIS: to.add(vec(rnd(100)-50, rnd(100)-50, rnd(100)-50)); break;
            case BNC_DOUILLESUZI: to.add(vec(0, 0, -1)); break;
            default: to.add(vec(0, 0, 1));
        }
        if(to.iszero()) to.z += 1;
        to.normalize();
        to.add(p);
        newbouncer(p, to, true, 0, d, type, lifetime, rnd(100)+20);
    }

    void damageeffect(int damage, gameent *d, gameent *actor, bool thirdperson, int atk)
    {
        if(d==player1) return;
        vec p = d->o;
        p.z += 0.6f*(d->eyeheight + d->aboveeye) - d->eyeheight;
        if(d->armourtype!=A_MAGNET)
        {
            if(blood) particle_splash(PART_BLOOD, damage > 300 ? 3 : damage/100, 1000, p, 0x60FFFF, 2.96f);
            if(damage>=600) playsound(S_SANG, &d->o, 0, 0, 0 , 100, -1, 250);
            gibeffect(damage, vec(0,0,0), d);
        }

        damage = (damage*aptitudes[actor->aptitude].apt_degats)/(aptitudes[d->aptitude].apt_resistance); //Dégats de base
        actor->steromillis > 0 ? damage*=actor->aptitude==13 ? 3 : 2 : damage+=0; //Stéros ou non
        if(d->aptisort3>0 && d->aptitude==APT_MAGICIEN) damage = damage/5.0f;
        damage = damage/10.f;

        if(isteam(d->team, actor->team) && actor!=d && actor->aptitude!=APT_MEDECIN)
        {
            damage/=(actor->aptitude==APT_JUNKIE ? 1.5f : 3.f); //Divisé si allié sauf sois-même
            particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*1.0f), PART_TEXT, actor->steromillis > 0 ? 2500 : 1500, 0x666666, actor==player1 ? 5.0f : 2.2f, -8);
            return;
        }

        bool normaldamage = true;

        switch(actor->aptitude)
        {
            case APT_AMERICAIN:
                if(atk==ATK_NUKE_SHOOT || atk==ATK_GAU8_SHOOT || atk==ATK_ROQUETTES_SHOOT || atk==ATK_CAMPOUZE_SHOOT)
                    {particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*2.f), PART_TEXT, 2500, 0xFF5500, actor==player1 ? 5.5f : 4.0f, -8); normaldamage = false; }
            case APT_NINJA:
                if(atk==ATK_CACNINJA_SHOOT) {particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*3.0f), PART_TEXT, 2500, 0xFF0000, actor==player1 ? 7.0f : 5.0f, -8);  normaldamage = false; }
                break;
            case APT_MAGICIEN:
                if(actor->aptisort2) {particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*1.3333f), PART_TEXT, 2500, 0xFF00FF, actor==player1 ? 5.5f : 4.0f, -8); normaldamage = false; }
            case APT_CAMPEUR:
                {
                    float campdeg = (actor->o.dist(d->o)/1800.f)+1.f;
                    particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*campdeg), PART_TEXT, actor->steromillis > 0 ? 2500 : 1500, damage<0 ? 0x22FF22 : actor->steromillis > 0 ? 0xFF0000: 0xFF4B19, actor==player1 ? 7.0f : 3.0f, -8);  normaldamage = false;
                    break;
                }
            case APT_VIKING:
                if(actor->ragemillis) {particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*1.5f), PART_TEXT, 2500, 0xAA0000, actor==player1 ? 10.0f : 7.0f, -8); normaldamage = false; }
                break;
            case APT_PRETRE:
                if(d->aptisort2)
                {
                    adddynlight(d->o, 25, vec(1.0f, 0.0f, 1.0f), 300, 50, L_NOSHADOW|L_VOLUMETRIC);
                    playsound(S_SORTPRETRE1, d!=player1 ? &d->o : NULL, NULL, 0, 0, 0, -1, 150, 300);
                    particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*1.0f), PART_TEXT, 1500, 0xAA00AA, actor==player1 ? 7.0f : 3.0f, -8);
                    normaldamage = false;
                }
                break;
            case APT_SHOSHONE:
                if(d->aptisort1) {particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage/1.3f), PART_TEXT, 2500, 0xAAAA00, actor==player1 ? 10.0f : 7.0f, -8); normaldamage = false; }
                if(actor->aptisort3) {particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*1.3f), PART_TEXT, 2500, 0xFF3333, actor==player1 ? 10.0f : 7.0f, -8); normaldamage = false; }
                break;
        }
        if(normaldamage) particle_textcopy(d->abovehead(), tempformatstring("%.1f", damage*1.0f), PART_TEXT, actor->steromillis > 0 ? 2500 : 1500, actor->steromillis > 0 ? 0xFF0000 : 0xFF4400, actor==player1 ? 7.0f : 3.0f, -8);
    }

    void regeneffect(int damage, gameent *d, gameent *actor, bool thirdperson)
    {
        vec p = d->o;
        p.z += 0.6f*(d->eyeheight + d->aboveeye) - d->eyeheight;

        if(thirdperson && actor->health<actor->maxhealth)
        {
            actor->steromillis > 0 ? damage*=actor->aptitude==13 ? 3 : 2 : damage+=0;
            damage = (damage*aptitudes[actor->aptitude].apt_degats)/(aptitudes[d->aptitude].apt_resistance);
            if(d->aptisort3>0 && d->aptitude==APT_MAGICIEN) damage = damage/5.0f;
            damage = damage/10.0f;

            particle_textcopy(actor->abovehead(), tempformatstring("%.1f", damage*1.0f), PART_TEXT, actor->steromillis > 0 ? 2500 : 1500, 0xBBDDBB, 3.5f, -8);
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

        if(!m_mp(gamemode) || f==at) f->hitpush(damage, vel, at, atk, f);
        if(!m_mp(gamemode)) damaged(damage, f, at, false, atk);
        else
        {
            hitmsg &h = hits.add();
            h.target = f->clientnum;
            h.lifesequence = f->lifesequence;
            h.info1 = int(info1*DMF);
            h.info2 = info2;
            h.dir = f==at ? ivec(0, 0, 0) : ivec(vec(vel).mul(DNF));

            int armoursound = f->armourtype == A_BLUE ? S_BALLEBOUCLIERBOIS : f->armourtype == A_GREEN ? S_BALLEBOUCLIERFER : f->armourtype == A_YELLOW ? S_BALLEBOUCLIEROR : f->armourtype == A_ASSIST ? S_BALLEARMUREASSISTENT : S_BALLEBOUCLIERMAGNETIQUE;

            if(at==player1)
            {
                damageeffect(damage, f, at, true, atk);
                if(f==player1)
                {
                    damageblend(damage);
                    damagecompass(damage, at ? at->o : f->o);
                    switch(rnd(2))
                    {
                        case 0:
                            if(player1->aptitude==APT_PHYSICIEN && player1->aptisort1 && player1->armour>0) playsound(S_SORTPHY1);
                            else if(player1->armour>0 && atk!=ATK_LANCEFLAMMES_SHOOT) playsound(armoursound);
                            break;
                        case 1:
                            playsound(S_BALLECORPS);
                    }
                }
            }
            else
            {
                switch(rnd(2))
                {
                    case 0:
                        if(f->aptitude==APT_PHYSICIEN && f->aptisort1 && f->armour>0) playsound(S_SORTPHY1, &f->o, 0, 0, 0 , 100, -1, 200);
                        else if(f->armour>0 && atk!=ATK_LANCEFLAMMES_SHOOT) playsound(armoursound, &f->o, 0, 0, 0 , 100, -1, 200);
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
            float damage = attacks[atk].damage*(1-dist/EXP_DISTSCALE/attacks[atk].exprad);
            if(o==at && atk==ATK_ASSISTXPL_SHOOT) damage = 0;
            if(damage > 0) hit(max(int(damage), 1), o, at, dir, atk, dist);
            if((atk==ATK_ARTIFICE_SHOOT || atk==ATK_SMAW_SHOOT || atk==ATK_M32_SHOOT || atk==ATK_ROQUETTES_SHOOT || atk==ATK_KAMIKAZE_SHOOT || atk==ATK_ASSISTXPL_SHOOT) && dist<attacks[atk].exprad*2.0f && o==player1) execfile("config/shake.cfg");
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
                vec debrisorigin = vec(v).sub(vec(vel).mul(6));
                particle_splash(PART_SPARK, 20, 150, v, owner->steromillis ? 0xFF0000 : 0xFF4400, 0.45f, 150, 150, 10, player1->champimillis ? true : false);
                adddynlight(safe ? v : debrisorigin, 2*attacks[atk].exprad, vec(1.5f, 0.75f, 0.0f), 150, 50, 0, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                particle_fireball(v, 0.6f*attacks[atk].exprad, PART_PULSE_BURST, int(attacks[atk].exprad*15), owner->steromillis ? 0xFF0000 : 0xFF9900, 2.5f, player1->champimillis ? true : false);
            }
            break;

            case ATK_GRAP1_SHOOT:
            {
                playsound(S_IMPACTGRAP1, &v, 0, 0, 0 , 100, -1, 250);
                vec debrisorigin = vec(v).sub(vec(vel).mul(6));
                adddynlight(safe ? v : debrisorigin, 2*attacks[atk].exprad, vec(1.5f, 0.0f, 1.5f), 300, 100, L_NODYNSHADOW, attacks[atk].exprad/2, vec(0.5f, 0.0f, 0.5f));
                particle_splash(PART_SPARK, 30, 175, v, owner->steromillis ? 0xFF0000 : 0xAA4466, 0.5f, 200, 200, 0, player1->champimillis ? true : false);
                loopi(3)particle_fireball(v, 0.8f*attacks[atk].exprad, PART_EXPLOSION, int(attacks[atk].exprad*5), owner->steromillis ? 0xFF0000 : 0x550055, 1.5f, player1->champimillis ? true : false);
            }
            break;

            case ATK_SPOCKGUN_SHOOT:
            {
                playsound(S_IMPACTALIEN, &v, 0, 0, 0 , 100, -1, 250);
                particle_splash(PART_SPOCK_FRONT, 30, 150, v, owner->steromillis ? 0xFF0000 : 0x22FF22, 1.25f, 150, 150, 200, player1->champimillis ? true : false);
                vec debrisorigin = vec(v).sub(vec(vel).mul(6));
                adddynlight(safe ? v : debrisorigin, 2*attacks[atk].exprad, vec(0.0f, 2.5f, 0.0f), 250, 40, 0, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
            }
            break;

            case ATK_SMAW_SHOOT:
            case ATK_ROQUETTES_SHOOT:
            {
                playsound(S_EXPLOSION, &v, 0, 0, 0 , 100, -1, 350);
                if(camera1->o.dist(v) >= 300) playsound(S_EXPLOSIONLOIN, &v, NULL, 0, 0, 100, -1, 1200);

                adddynlight(safe ? v : debrisorigin, 7*attacks[atk].exprad, vec(8.0f, 4.0f, 0.0f), 80, 40, L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                adddynlight(safe ? v : debrisorigin, 4*attacks[atk].exprad, vec(1.2f, 0.4f, 0.0f), 80, 40, L_VOLUMETRIC|L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(0.0f, 0.0f, 1.5f));
                particle_splash(PART_SMOKE, atk==ATK_ROQUETTES_SHOOT ? 5 : 9, 2000, v, 0x333333, 40.0f, 150+rnd(50), 300+rnd(100), 0, player1->champimillis ? true : false);
                particle_splash(PART_SMOKE, atk==ATK_ROQUETTES_SHOOT ? 5 : 9, 1300, v, 0x333333, 25.0f, 150+rnd(50), 600+rnd(100), 0, player1->champimillis ? true : false);
                particle_splash(PART_SPARK, atk==ATK_ROQUETTES_SHOOT ? 7 : 10, 300, v, owner->steromillis ? 0xFF4444 : 0xFFBB55,  1.7f+rnd(2), 3500, 3500, 0, player1->champimillis ? true : false);
                loopi(lookupmaterial(v)&MAT_WATER ? 1 : 3) particle_splash(PART_FLAME1+rnd(2), atk==ATK_ROQUETTES_SHOOT ? 9 : 17, 80+rnd(40), v, owner->steromillis ? 0xFF4444 : i==0 ? 0x383838 : i==1 ? 0x474747: 0x604930, 9.f+rnd(6), atk==ATK_ROQUETTES_SHOOT ? 300+rnd(150) : 400+rnd(200), 800, 1, player1->champimillis ? true : false);

                particle_fireball(v, 350, PART_ONDECHOC, 300, owner->steromillis ? 0xFF0000 : 0xFF5500, 10.0f, player1->champimillis ? true : false);
                particle_fireball(v, 350, PART_ONDECHOC, 300, owner->steromillis ? 0xFF0000 : 0xFFFFFF, 20.0f, player1->champimillis ? true : false);

                vec debrisorigin = vec(v).sub(vec(vel).mul(15));
                loopi(5+rnd(3)) spawnbouncer(debrisorigin, debrisvel, owner, BNC_DEBRIS, 5000);

                if((lookupmaterial(v)&MAT_WATER) == MAT_WATER)
                {
                    particle_splash(PART_EAU, 50, 200, v, 0x18181A, 12.0f+rnd(14), 600, 300);
                    particle_splash(PART_BULLE, 15, 150, v, 0x18181A, 4.0f+rnd(8), 300, 150);
                    playsound(S_EXPLOSIONAVECEAU, &v, 0, 0, 0 , 100, -1, 350);
                }
            }
            break;

            case ATK_KAMIKAZE_SHOOT:
            case ATK_ASSISTXPL_SHOOT:
            {
                if(camera1->o.dist(v) >= 300) playsound(S_EXPLOSIONLOIN, &v, NULL, 0, 0, 100, -1, 1200);

                loopi(10)
                {
                    vec pos = vec(v).add(vec(rnd(35)-rnd(70), rnd(35)-rnd(70), rnd(35)-rnd(70)));
                    particle_splash(PART_SMOKE, 4, atk==ATK_KAMIKAZE_SHOOT ? 5000 : 3000, pos, 0x333333, 60.f, 200+rnd(75), 100, 0, player1->champimillis ? true : false);
                    particle_splash(PART_SMOKE, 3, atk==ATK_KAMIKAZE_SHOOT ? 3000 : 2000, pos, 0x151515, 40.f, 200+rnd(75), 250, 0, player1->champimillis ? true : false);
                    particle_splash(PART_SPARK, 6, 300, pos, owner->steromillis ? 0xFF4444 : 0xFFBB55,  1.7f+rnd(2), 3500, 3500, 0, player1->champimillis ? true : false);
                    loopi(lookupmaterial(v)&MAT_WATER ? 1 : 3) particle_splash(PART_FLAME1+rnd(2), 6, 130+rnd(50), pos, owner->steromillis ? 0xFF4444 : i==0 ? 0x383838: i==1 ? 0x474747 : 0x6A4A3A, 9.f+rnd(6), 1200+rnd(700), 1200, 1, player1->champimillis ? true : false);
                }

                particle_fireball(v, 350, PART_ONDECHOC, 300, owner->steromillis ? 0xFF0000 : 0xFF5500, 10.0f, player1->champimillis ? true : false);
                particle_fireball(v, 350, PART_ONDECHOC, 300, owner->steromillis ? 0xFF0000 : 0xFFFFFF, 20.0f, player1->champimillis ? true : false);

                vec debrisorigin = vec(v).sub(vec(vel).mul(15));
                loopi(5+rnd(3)) spawnbouncer(debrisorigin, debrisvel, owner, BNC_DEBRIS, 5000);
                adddynlight(safe ? v : debrisorigin, 7*attacks[atk].exprad, vec(8.0f, 4.0f, 0.0f), 80, 40, L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                adddynlight(safe ? v : debrisorigin, 4*attacks[atk].exprad, vec(1.2f, 0.4f, 0.0f), 80, 40, L_VOLUMETRIC|L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(0.0f, 0.0f, 1.5f));

                if((lookupmaterial(v)&MAT_WATER) == MAT_WATER)
                {
                    particle_splash(PART_EAU, 50, 200, v, 0x18181A, 12.0f+rnd(14), 600, 300);
                    particle_splash(PART_BULLE, 15, 150, v, 0x18181A, 4.0f+rnd(8), 300, 150);
                    playsound(S_EXPLOSIONAVECEAU, &v, 0, 0, 0 , 100, -1, 350);
                }
            }
            break;

            case ATK_NUKE_SHOOT:
            {
                playsound(S_NUKE);

                loopi(50)
                {
                    vec pos = vec(v).add(vec(rnd(200)-rnd(400), rnd(200)-rnd(400), rnd(200)-rnd(400)));
                    particle_splash(PART_SMOKE, 1, 2000,  pos, 0x212121, 150.0f, 700, 70, 0, player1->champimillis ? true : false);
                    particle_splash(PART_SMOKE, 1, 15000, pos, 0x222222, 200.0f,  35, 300, 0, player1->champimillis ? true : false);
                    particle_splash(PART_SMOKE, 2, 5000,  pos, 0x333333, 250.0f, 1000, 500, 0, player1->champimillis ? true : false);
                    particle_splash(PART_FLAME1+rnd(2),  3, 1000, pos, owner->steromillis ? 0xFF0000 : 0xFFFF00, 35+rnd(15), 800, 300, 0, player1->champimillis ? true : false);
                    particle_splash(PART_FLAME1+rnd(2),  3, 1000, pos, owner->steromillis ? 0xFF0000 : 0x224400, 35+rnd(15), 800, 300, 0, player1->champimillis ? true : false);
                    particle_splash(PART_FLAME1+rnd(2),  3, 1000, pos, owner->steromillis ? 0xFF0000 : 0xFF2222, 35+rnd(15), 800, 300, 0, player1->champimillis ? true : false);
                }

                loopi(3) particle_fireball(v, 350, PART_ONDECHOC, 300, 0xFFCCAA, 800.0f, player1->champimillis ? true : false);

                vec debrisorigin = vec(v).sub(vec(vel).mul(15));
                adddynlight(safe ? v : debrisorigin, 7*attacks[atk].exprad, vec(8.0f, 4.0f, 0.0f), 300, 40, L_NOSHADOW, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                adddynlight(safe ? v : debrisorigin, 5*attacks[atk].exprad, vec(8.0f, 4.0f, 0.0f), 300, 40, L_NOSHADOW|L_VOLUMETRIC, attacks[atk].exprad/2, vec(0.0f, 0.0f, 1.5f));
            }
            break;

            case ATK_ARTIFICE_SHOOT:
            {
                playsound(S_EXPLOSIONARTIFICE, &v, 0, 0, 0 , 100, -1, 350);
                if(camera1->o.dist(v) >= 300) playsound(S_ARTIFICELOIN, &v, NULL, 0, 0, 100, -1, 1000);

                particle_splash(PART_FLAME1+rnd(2), 5, 40, v, 0xFFC864, 5, 800, 1600, 0, player1->champimillis ? true : false);
                loopi(4) particle_splash(PART_GLOWSPARK, 16+rnd(10), 200+rnd(200), v, owner->steromillis ? 0xFF0000 : rnd(16777215),  0.4f+(rnd(5)/10.f), 500+rnd(300), 5000+rnd(3000), 2, player1->champimillis ? true : false);

                vec debrisorigin = vec(v).sub(vec(vel).mul(5));
                adddynlight(safe ? v : debrisorigin, 7*attacks[atk].exprad, vec(1+rnd(4), 1+rnd(4), 1+rnd(4)), 80, 40, 0, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                adddynlight(safe ? v : debrisorigin, 4*attacks[atk].exprad, vec(rnd(15)/10.0f, rnd(15)/10.0f, rnd(15)/10.0f), 80, 40, L_VOLUMETRIC|L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(0.0f, 0.0f, 1.5f));

                if(lookupmaterial(v)&MAT_WATER)
                {
                    particle_splash(PART_EAU, 50, 200, v, 0x18181A, 12.0f+rnd(14), 600, 300);
                    particle_splash(PART_BULLE, 15, 150, v, 0x18181A, 4.0f+rnd(8), 300, 150);
                    playsound(S_EXPLOSIONAVECEAU, &v, 0, 0, 0 , 100, -1, 350);
                }
            }
            break;

            case ATK_M32_SHOOT:
            {
                playsound(S_EXPLOSIONGRENADE, &v, 0, 0, 0 , 100, -1, 350);
                if(camera1->o.dist(v) >= 300) playsound(S_EXPLOSIONLOIN, &v, NULL, NULL, 0, 0, 100, -1, 1200);

                loopi(3) particle_fireball(v, 40+rnd(50), PART_PLASMA, 300, owner->steromillis ? 0xFF0000 : 0xFFFFFF, 1.0f, player1->champimillis ? true : false);
                loopi(3) particle_splash(PART_SPARK, 8, 150+rnd(150), v, owner->steromillis ? 0xFF0000 : 0xFFFFFF,  2.0f+rnd(2), 1500+rnd(2250), 1500+rnd(2250), 0, player1->champimillis ? true : false);
                loopi(2) particle_splash(PART_SMOKE, 7, 1300+rnd(800), v, 0x555555, 40.0f, 150+rnd(150), 300+rnd(700), 0, player1->champimillis ? true : false);
                particle_fireball(v, 350, PART_ONDECHOC, 300, owner->steromillis ? 0xFF0000 : 0xFF5500, 10.0f, player1->champimillis ? true : false);
                particle_fireball(v, 350, PART_ONDECHOC, 300, owner->steromillis ? 0xFF0000 : 0xFFFFFF, 20.0f, player1->champimillis ? true : false);

                vec debrisorigin = vec(v).sub(vec(vel).mul(5));
                adddynlight(v, 7*attacks[atk].exprad, vec(0.0f, 3.0f, 9.0f), 80, 40, L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
                adddynlight(v, 5*attacks[atk].exprad, vec(0.0f, 0.4f, 1.2f), 80, 40, L_VOLUMETRIC|L_NODYNSHADOW|DL_FLASH, attacks[atk].exprad/2, vec(0.0f, 0.0f, 1.5f));
                loopi(5+rnd(3)) spawnbouncer(debrisorigin, debrisvel, owner, BNC_DEBRIS, 5000);

                if(lookupmaterial(v)&MAT_WATER)
                {
                    particle_splash(PART_EAU, 50, 200, v, 0x18181A, 12.0f+rnd(14), 600, 300);
                    particle_splash(PART_BULLE, 15, 150, v, 0x18181A, 4.0f+rnd(8), 300, 150);
                    playsound(S_EXPLOSIONAVECEAU, &v, 0, 0, 0 , 100, -1, 350);
                }
            }
            break;

            case ATK_MINIGUN_SHOOT:
            case ATK_AK47_SHOOT:
            case ATK_UZI_SHOOT:
            case ATK_FAMAS_SHOOT:
            case ATK_GLOCK_SHOOT:
            {
                if(!(lookupmaterial(v)&MAT_WATER)) playsound(S_IMPACT, &v, 0, 0, 0 , 100, -1, 250);
                particle_splash(PART_SPARK, ATK_MINIGUN_SHOOT==1 || ATK_AK47_SHOOT==1 ? 12 : 9, 60, v, owner->steromillis ? 0xFF0000 : 0xFF6600, ATK_MINIGUN_SHOOT==1 || ATK_AK47_SHOOT==1 ? 0.4 : 0.3f, 150, 100, 0, player1->champimillis ? true : false);
                particle_splash(PART_SMOKE, 3, 600+rnd(300), v, 0x565656, ATK_MINIGUN_SHOOT==1 || ATK_AK47_SHOOT==1 ? 0.35f : 0.3f, 25, 300, 2, player1->champimillis ? true : false);
                particle_splash(PART_SMOKE, 6, 350+rnd(300), v, 0x552900, ATK_MINIGUN_SHOOT==1 || ATK_AK47_SHOOT==1 ? 0.35f : 0.3f, 15, 300, 2, player1->champimillis ? true : false);
            }
            case ATK_ARBALETE_SHOOT:
            {
                //playsound(S_IMPACT, &v, 0, 0, 0 , 100, -1, 250);
                particle_splash(PART_SMOKE, 3, 600+rnd(300), v, 0x565656, 0.3f, 25, 300, 2, player1->champimillis ? true : false);
                particle_splash(PART_SMOKE, 6, 350+rnd(300), v, 0x552900, 0.3f, 15, 300, 2, player1->champimillis ? true : false);
            }
            break;
            case ATK_SV98_SHOOT:
            case ATK_SKS_SHOOT:
            case ATK_GAU8_SHOOT:
            case ATK_CAMPOUZE_SHOOT:
            {
                if(!(lookupmaterial(v)&MAT_WATER))
                {
                    playsound(S_IMPACTLOURDLOIN, &v, 0, 0, 0 , 700, -1, 1000);
                    playsound(S_IMPACTSNIPE, &v, 0, 0, 0 , 100, -1, 300);
                }
                particle_splash(PART_SPARK, 12, 85, v, owner->steromillis ? 0xFF0000 : 0xFF5533, 0.7f,  400, 200, 0, player1->champimillis ? true : false);
                particle_splash(PART_SMOKE,  4, 800+rnd(300), v, 0x414141, 0.4f, 25, 300, 2, player1->champimillis ? true : false);
                particle_splash(PART_SMOKE,  4, 500+rnd(300), v, 0x442200, 0.4f, 15, 300, 2, player1->champimillis ? true : false);
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
        float rad = attacks[p.atk].exprad*0.35f;
        addstain(STAIN_EXPL_SCORCH, pos, dir, rad);
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
            case ATK_ASSISTXPL_SHOOT:
            case ATK_ROQUETTES_SHOOT: smawstain(p, v); break;
            case ATK_MINIGUN_SHOOT:
            case ATK_SV98_SHOOT:
            case ATK_AK47_SHOOT:
            case ATK_GAU8_SHOOT:
            case ATK_CAMPOUZE_SHOOT:
            case ATK_SKS_SHOOT: bigballestain(p, v); break;
            case ATK_SPOCKGUN_SHOOT: spockstain(p, v); break;
            case ATK_UZI_SHOOT:
            case ATK_ARBALETE_SHOOT:
            case ATK_GLOCK_SHOOT:
            case ATK_FAMAS_SHOOT: littleballestain(p, v); break;
            case ATK_GRAP1_SHOOT: grap1stain(p, v); break;
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
                    case ATK_ASSISTXPL_SHOOT:
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

            if((p.lifetime -= time)<0 && (p.atk==ATK_ARTIFICE_SHOOT || p.atk==ATK_NUKE_SHOOT))
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

                    float len = min(20.0f, vec(p.offset).add(p.from).dist(pos));
                        vec dir = vec(dv).normalize(),
                        tail = vec(dir).mul(-len).add(pos),
                        head = vec(dir).mul(2.4f).add(pos);

                    bool canplaysound = false;
                    if(!p.inwater && lookupmaterial(pos)&MAT_WATER)
                    {
                        p.inwater = true;
                        particle_splash(PART_EAU, 15, 100, v, 0x28282A, 0.75f, 50, -300, 1, player1->champimillis ? true : false);
                        playsound(S_IMPACTEAU, &v, 0, 0, 0 , 100, -1, 250);
                    }

                    switch(p.atk)
                    {
                        case ATK_PULSE_SHOOT:
                            particle_splash(PART_PULSE_FRONT, 1, 1, pos, p.owner->steromillis ? 0xFF4444 : 0xFF6600, 2.4f, 150, 20, 0, player1->champimillis ? true : false);
                            particle_flare(tail, head, 1, PART_PULSE_SIDE, p.owner->steromillis ? 0xFF4444 : 0xFF6600, 2.0f, p.owner, player1->champimillis ? true : false);
                            if(lookupmaterial(pos)&MAT_WATER) particle_splash(PART_BULLE, 1, 150, v, 0x18181A, 2.0f+rnd(2), 20, -30);
                            p.projsound = S_FLYBYALIEN;
                            canplaysound = true;
                            break;
                        case ATK_GRAP1_SHOOT:
                            particle_splash(PART_PULSE_FRONT, 1, 1, pos, p.owner->steromillis ? 0xFF4444 : 0xFF33BB, 3.0f, 150, 20, 0, player1->champimillis ? true : false);
                            particle_flare(tail, head, 1, PART_PULSE_SIDE, p.owner->steromillis ? 0xFF4444 : 0xEE22AA, 3.0f, p.owner, player1->champimillis ? true : false);
                            particle_splash(lookupmaterial(pos)&MAT_WATER ? PART_BULLE : PART_SMOKE, 1, lookupmaterial(pos)&MAT_WATER ? 150 : 300, pos, lookupmaterial(pos)&MAT_WATER ? 0x18181A : 0xAAAAAA, 4.0f, 25, 250, 0, player1->champimillis ? true : false);
                            p.projsound = S_FLYBYGRAP1;
                            canplaysound = true;
                            break;
                        case ATK_SPOCKGUN_SHOOT:
                            particle_splash(PART_SPOCK_FRONT, 1, 1, pos, p.owner->steromillis ? 0xFF4444 : 0x00FF00, 6.0f, 150, 20, 0, player1->champimillis ? true : false);
                            particle_flare(tail, head, 1, PART_PULSE_SIDE, p.owner->steromillis ? 0xFF4444 : 0x22FF22, 2.5f, p.owner, player1->champimillis ? true : false);
                            p.projsound = S_FLYBYALIEN;
                            canplaysound = true;
                            break;
                        case ATK_SV98_SHOOT:
                        case ATK_SKS_SHOOT:
                        case ATK_CAMPOUZE_SHOOT:
                        case ATK_GAU8_SHOOT:
                            if(lookupmaterial(pos)&MAT_WATER) particle_splash(PART_BULLE, 1, 150, v, 0x18181A, 2.0f+rnd(2), 20, -30);
                            particle_flare(tail, head, 1, PART_BALLE_SIDE, p.owner->steromillis ? 0xFF4444 : 0xFFBB88, ATK_GAU8_SHOOT==1 ? 0.8f : 0.65f, p.owner, player1->champimillis ? true : false);
                            particle_splash(PART_PULSE_FRONT, 1, 1, pos, p.owner->steromillis ? 0xFF4444 : 0xFFBB88,  p.owner==player1 ? 0.8f : ATK_GAU8_SHOOT==1 ? 0.65f : 0.45f, 150, 20, 0, player1->champimillis ? true : false);
                            break;
                        case ATK_MINIGUN_SHOOT:
                        case ATK_AK47_SHOOT:
                        case ATK_UZI_SHOOT:
                        case ATK_GLOCK_SHOOT:
                        case ATK_FAMAS_SHOOT:
                            if(lookupmaterial(pos)&MAT_WATER) particle_splash(PART_BULLE, 1, 150, v, 0x18181A, 1.0f+rnd(2), 20, -30);
                            particle_flare(tail, head, 1, PART_BALLE_SIDE, p.owner->steromillis ? 0xFF4444 : 0xFFBB88, ATK_MINIGUN_SHOOT==1 || ATK_AK47_SHOOT==1 ? 0.55f : 0.45f, p.owner, player1->champimillis ? true : false);
                            particle_splash(PART_PULSE_FRONT, 1, 1, pos, p.owner->steromillis ? 0xFF4444 : 0xFFBB88, p.owner==player1 ? 0.65f : ATK_MINIGUN_SHOOT==1 || ATK_AK47_SHOOT==1 ? 0.4f : 0.3f, 150, 20, 0, player1->champimillis ? true : false);
                            break;
                        case ATK_ARBALETE_SHOOT:
                            if(!p.exploded)
                            {
                                if(lookupmaterial(pos)&MAT_WATER) particle_splash(PART_BULLE, 1, 150, v, 0x888888, 1.0f+rnd(2), 20, -30);
                                particle_splash(PART_SMOKE, 2, 100, pos, 0xAAAAAA, 0.8f, 25, 250, 0, player1->champimillis ? true : false);
                                if(p.owner->steromillis)
                                {
                                    particle_flare(tail, head, 1, PART_BALLE_SIDE, 0xFF4444, 0.30f, p.owner, player1->champimillis ? true : false);
                                    particle_splash(PART_PULSE_FRONT, 1, 1, pos, 0xFF4444, p.owner==player1 ? 0.5f : 0.25f, 150, 20, 0, player1->champimillis ? true : false);
                                }
                            }
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
            //vec tmp = vec(float(rnd(11)-5), float(rnd(11)-5), float(rnd(11)-5));
            if(camera1->o.dist(p) <= 32 && !soundused)
            {
                playsound(sound, &p, 0, 0, 0 , 100, -1, 200);
                soundused = true;
            }
        }
    }

    void railhit(const vec &from, const vec &to, const vec &muzzle, bool stain = false)
    {
        vec dir = vec(from).sub(to).safenormalize();
        loopi(3)particle_flare(muzzle, to,  50+rnd(50), PART_LIGHTNING, 0x8888FF, 1.5f+rnd(2), NULL, player1->champimillis ? true : false);
        if(stain)
        {
            addstain(STAIN_RAIL_HOLE, to, dir, 2.0f);
            addstain(STAIN_RAIL_GLOW, to, dir, 1.5f, 0xFF2200);
            addstain(STAIN_RAIL_GLOW, to, dir, 2.5f, 0xFF8800);
        }
        adddynlight(vec(to).madd(dir, 4), 10, vec(1.00f, 0.5f, 0.0f), 225, 75);
        particle_splash(PART_SPARK, 50, 150, to, 0xFF4400, 0.45f, 300, 30, 0, player1->champimillis ? true : false);
    }

    void pompehit(const vec &from, const vec &to, bool stain = true)
    {
        vec dir = vec(from).sub(to).safenormalize();
        if(stain)
        {
            switch(rnd(3))
            {
                case 0: addstain(STAIN_BALLE_1, to, dir, 0.4f); break;
                case 1: addstain(STAIN_BALLE_2, to, dir, 0.4f); break;
                case 2: addstain(STAIN_BALLE_3, to, dir, 0.4f); break;
            }
            addstain(STAIN_BALLE_GLOW, to, dir, 0.8f, 0x991100);
        }
    }

    void flamehit(const vec &from, const vec &to, bool stain = true)
    {
        vec dir = vec(from).sub(to).safenormalize();
        if(stain)
        {
            switch(rnd(2)){case 0: addstain(STAIN_BRULAGE, to, dir, 20.0f);}
        }
    }

    bool looped;

    void shoteffects(int atk, const vec &from, const vec &to, gameent *d, bool local, int id, int prevaction)     // create visual effect from a shot
    {
        int gun = attacks[atk].gun;
        int sound = attacks[atk].sound;
        //int soundwater = attacks[atk]].soundwater;
        if(player1->aptisort2 && d==player1 && player1->aptitude==APT_MAGICIEN) playsound(S_SORTMAGE2);
        if(d->aptisort3 && d->aptitude==APT_PRETRE)adddynlight(d->muzzle, 6, vec(1.5f, 1.5f, 0.0f), 80, 40, L_NOSHADOW|L_VOLUMETRIC|DL_FLASH);

        int movefactor = 1;
        if(game::player1->aptitude==APT_SOLDAT) movefactor = 2;

        switch(atk)
        {
            case ATK_PULSE_SHOOT:
                if(d->type==ENT_PLAYER) sound = S_DEMARRAGEPLASMA;
                particle_flare(d->muzzle, d->muzzle, 140, PART_PULSE_MUZZLE_FLASH, d->steromillis ? 0xFF4444 : d->aptisort2 && d->aptitude==APT_MAGICIEN ? 0xFF44FF : 0xFF7911, d==hudplayer() ? zoom ? 1.0f : 2.25f : 3.f, d, player1->champimillis ? true : false);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 140, PART_PULSE_MUZZLE_FLASH, 0xFF4444, zoom ? 1.50f : 5.50f, d, player1->champimillis ? true : false);
                adddynlight(hudgunorigin(gun, d->o, to, d), 100, vec(1.25f, 0.2f, 0.0f), 40, 2, DL_FLASH, 0, vec(1.25f, 0.2f, 0.0f), d);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d==player1) mousemove((-5+rnd(11))/movefactor, (-5+rnd(11))/movefactor, player1->aptisort2 && player1->aptitude==APT_MAGICIEN ? true : false);
                break;

            case ATK_SPOCKGUN_SHOOT:
                particle_flare(d->muzzle, d->muzzle, 140, PART_PULSE_MUZZLE_FLASH, d->steromillis ? 0xFF2222 : d->aptisort2 && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0x22FF22, zoom ? 1.0f : 2.25f, d, player1->champimillis ? true : false);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 140, PART_PULSE_MUZZLE_FLASH, 0xFF2222, zoom ? 1.5f : 4.00f, d, player1->champimillis ? true : false);
                adddynlight(hudgunorigin(gun, d->o, to, d), 75, vec(0.0f, 1.25f, 0.0f), 35, 2, DL_FLASH, 0, vec(0.0f, 1.25f, 0.0f), d);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                break;

            case ATK_RAIL_SHOOT:
                playsound(S_IMPACTELEC, &to, 0, 0, 0 , 100, -1, 250);
                if(d!=hudplayer()) sound_nearmiss(S_FLYBYELEC, from, to);

                particle_flare(d->muzzle, to, 30, PART_POMPE_SIDE, d->steromillis ? 0xFF2222 : 0xFFFF22, 0.2f, d, player1->champimillis ? true : false);
                particle_flare(d->muzzle, d->muzzle, 140, PART_RAIL_MUZZLE_FLASH, d->steromillis ? 0xFF2222 : d->aptisort2 && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0x50CFFF, zoom ? 1.75f : 3.0f, d, player1->champimillis ? true : false);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 140, PART_RAIL_MUZZLE_FLASH, 0xFF2222, zoom ? 2.5f : 5.5f, d, player1->champimillis ? true : false);
                adddynlight(hudgunorigin(gun, d->o, to, d), 100, vec(0.25f, 0.75f, 2.0f), 40, 2, DL_FLASH, 0, vec(0.25f, 0.75f, 2.0f), d);
                break;

            case ATK_SMAW_SHOOT:
            case ATK_ROQUETTES_SHOOT:
            case ATK_NUKE_SHOOT:
                if(d==player1 && atk==ATK_NUKE_SHOOT && conserveurofficiel) unlockachievement(ACH_ATOME);
                particle_flare(d->muzzle, d->muzzle, 250, PART_NORMAL_MUZZLE_FLASH, d->steromillis ? 0xFF2222 : d->aptisort2 && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0xFF7700, atk==ATK_ROQUETTES_SHOOT ? 2.5f : 7.00f, d, player1->champimillis ? true : false);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 250, PART_NORMAL_MUZZLE_FLASH, 0xFF2222, atk==ATK_ROQUETTES_SHOOT ? 4.0f : 3.00f, d, player1->champimillis ? true : false);
                adddynlight(hudgunorigin(gun, d->o, to, d), 100, vec(1.25f, 0.75f, 0.3f), 75, 2, DL_FLASH, 0, vec(1.25f, 0.75f, 0.3f), d);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d==player1 && atk==ATK_ROQUETTES_SHOOT) mousemove((-7+rnd(15))/movefactor, (-7+rnd(15))/movefactor, player1->aptisort2 && player1->aptitude==APT_MAGICIEN ? true : false);
                break;
            case ATK_ARTIFICE_SHOOT:
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d->ragemillis) particle_splash(PART_SPARK,  6, 500, d->muzzle, 0xFF5555, 1.0f, 50, 200, 0, player1->champimillis ? true : false);
                break;
            case ATK_MINIGUN_SHOOT:
            case ATK_AK47_SHOOT:
                spawnbouncer(d->balles, d->balles, d, BNC_DOUILLES, bnclifetime+rnd(bnclifetime));
                particle_flare(d->muzzle, d->muzzle, 100, PART_MINIGUN_MUZZLE_FLASH, d->steromillis ? 0xFF2222 : d->aptisort2 && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0xCCAAAA, d==hudplayer() ? zoom ? 1.3f : 3.f : 3.6f, d, player1->champimillis ? true : false);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 100, PART_MINIGUN_MUZZLE_FLASH, 0xFF2222, d==hudplayer() ? zoom ? 2.f : 5.5f : 6.5f, d, player1->champimillis ? true : false);
                particle_splash(PART_SMOKE, d==hudplayer() ? 3 : 5, d==hudplayer() ? 350 : 500, d->muzzle, 0x444444, d==hudplayer() ? 3.5f : 4.5f, 20, 500, 0, player1->champimillis ? true : false);
                particle_splash(PART_SPARK, d==hudplayer() ? 3 : 5, 35, d->muzzle, 0xFF4400, 0.35f, 300, 500, 0, player1->champimillis ? true : false);
                adddynlight(hudgunorigin(gun, d->o, to, d), 75, vec(1.25f, 0.75f, 0.3f), 35, 2, DL_FLASH, 0, vec(1.25f, 0.75f, 0.3f), d);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d!=hudplayer()) sound_nearmiss(S_FLYBY, from, to);
                if(d==player1) mousemove(ATK_MINIGUN_SHOOT==1 ? (-7+rnd(15))/movefactor : (-3+rnd(7))/movefactor, ATK_MINIGUN_SHOOT==1 ? (-7+rnd(15))/movefactor : (-3+rnd(7))/movefactor, player1->aptisort2 && player1->aptitude==APT_MAGICIEN ? true : false);
                break;
            case ATK_GAU8_SHOOT:
                if(d->type==ENT_PLAYER) sound = S_GAU8;
                if(d==player1 && player1->aptitude==APT_PRETRE && player1->champimillis && player1->aptisort3) unlockachievement(ACH_CADENCE);
                spawnbouncer(d->balles, d->balles, d, BNC_BIGDOUILLES,  bnclifetime+rnd(bnclifetime));
                particle_flare(d->muzzle, d->muzzle, 100, PART_MINIGUN_MUZZLE_FLASH, d->steromillis ? 0xFF2222 : d->aptisort2 && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0xFFFFFF, d==hudplayer() ? zoom ? 1.5f : 2.0f : 5.f, d, player1->champimillis ? true : false);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 100, PART_MINIGUN_MUZZLE_FLASH, 0xFF2222, d==hudplayer() ? zoom ? 1.5f : 2.5f : 6.f, d, player1->champimillis ? true : false);
                switch(rnd(7))
                {
                    case 0:
                        particle_splash(PART_SMOKE, d==hudplayer() ? 4 : 6, d==hudplayer() ? 350 : 600, d->muzzle, 0x222222, d==hudplayer() ? 3.5f : 6.5f, 40, 500, 0, player1->champimillis ? true : false);
                        particle_splash(PART_SPARK, d==hudplayer() ? 4 : 7, 40, d->muzzle, 0xFF4400, 0.5f, 300, 500, 0, player1->champimillis ? true : false);
                }
                adddynlight(hudgunorigin(gun, d->o, to, d), 100, vec(1.25f, 0.75f, 0.3f), 30, 2, DL_FLASH, 0, vec(1.25f, 0.75f, 0.3f), d);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d!=hudplayer()) sound_nearmiss(S_FLYBYSNIPE, from, to);
                if(d==player1) mousemove((-9+rnd(19))/movefactor, (-9+rnd(19))/movefactor, player1->aptisort2 && player1->aptitude==APT_MAGICIEN ? true : false);
                break;
            case ATK_MOSSBERG_SHOOT:
            case ATK_HYDRA_SHOOT:
                {
                    if(!local) createrays(gun, from, to, d);
                    loopi(atk==ATK_HYDRA_SHOOT ? 3 : 2) spawnbouncer(d->balles, d->balles, d, BNC_CARTOUCHES, bnclifetime+rnd(bnclifetime));
                    particle_flare(d->muzzle, d->muzzle, 140, PART_NORMAL_MUZZLE_FLASH, d->steromillis ? 0xFF2222 : d->aptisort2 && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0xCCAAAA,  d==hudplayer() ? zoom ? 1.25f : 3.50f : 4.5f, d, player1->champimillis ? true : false);
                    if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 140, PART_NORMAL_MUZZLE_FLASH, 0xFF2222, d==hudplayer() ? zoom ? 2.00f : 5.5f : 6.5f, d, player1->champimillis ? true : false);
                    particle_splash(PART_SMOKE,  4, 500, d->muzzle, 0x443333, 3.5f, 20, 500, 0, player1->champimillis ? true : false);
                    particle_splash(PART_SPARK, d==hudplayer() ? 4 : 7, 40, d->muzzle, 0xFF2200, 0.5f, 300, 500, 0, player1->champimillis ? true : false);
                    adddynlight(hudgunorigin(gun, d->o, to, d), 75, vec(1.25f, 0.25f, 0.f), 40, 2, DL_FLASH, 0, vec(1.25f, 0.25f, 0.f), d);
                    loopi(attacks[atk].rays)
                    {
                        if(!(lookupmaterial(rays[i])&MAT_WATER)) playsound(S_IMPACT, &rays[i], 0, 0, 0 , 100, -1, 250);
                        particle_splash(PART_SPARK, 9, 60, rays[i], d->steromillis ? 0xFF2222 : 0xAA1100, 0.4, 150, 100, 0, player1->champimillis ? true : false);
                        particle_splash(PART_SMOKE, 3, 500+rnd(300), rays[i], 0x797979, 0.2f, 35, 300, 2, player1->champimillis ? true : false);
                        particle_splash(PART_SMOKE, 3, 275+rnd(275), rays[i], 0x553915, 0.15f, 35, 300, 2, player1->champimillis ? true : false);
                        particle_flare(d->muzzle, rays[i], 30, PART_POMPE_SIDE, d->steromillis ? 0xFF2222 : 0xFFFF22, 0.2f, d, player1->champimillis ? true : false);
                        pompehit(from, rays[i]);
                        if(d!=hudplayer()) sound_nearmiss(S_FLYBY, from, rays[i]);
                    }
                }
                break;
            case ATK_SV98_SHOOT:
            case ATK_SKS_SHOOT:
            case ATK_CAMPOUZE_SHOOT:
                if(atk==ATK_CAMPOUZE_SHOOT) {loopi(3)spawnbouncer(d->balles, d->muzzle, d, BNC_BIGDOUILLES, bnclifetime+rnd(bnclifetime));}
                else spawnbouncer(d->balles, d->muzzle, d, BNC_BIGDOUILLES, bnclifetime+rnd(bnclifetime));
                particle_splash(PART_SMOKE, d==hudplayer() ? 4 : 6, d==hudplayer() ? 350 : 600, d->muzzle, 0x222222, d==hudplayer() ? 3.5f : 6.5f, 40, 500, 0, player1->champimillis ? true : false);
                particle_splash(PART_SPARK, d==hudplayer() ? 4 : 7, 40, d->muzzle, 0xFFFFFF, 0.5f, 300, 500, 0, player1->champimillis ? true : false);
                particle_flare(d->muzzle, d->muzzle, 100, PART_NORMAL_MUZZLE_FLASH, d->steromillis ? 0xFF2222 : d->aptisort2 && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0xFFFFFF, 1.25f, d, player1->champimillis ? true : false);
                particle_flare(d->muzzle, d->muzzle, 100, PART_SNIPE_MUZZLE_FLASH, d->steromillis ? 0xFF2222 : d->aptisort2 && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0xFFFFFF, 5.0f, d, player1->champimillis ? true : false);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 75, PART_SNIPE_MUZZLE_FLASH, 0xFF2222, 8.0f, d, player1->champimillis ? true : false);
                adddynlight(hudgunorigin(gun, d->o, to, d), 50, vec(1.25f, 0.75f, 0.3f), 37, 2, DL_FLASH, 0, vec(1.25f, 0.75f, 0.3f), d);
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
                if(d->ragemillis) particle_splash(PART_SPARK,  8, 500, d->muzzle, 0xFF2222, 1.0f,  50,   200, 0, player1->champimillis ? true : false);
                if(d!=hudplayer()) sound_nearmiss(S_FLECHE, from, to);
                break;
            case ATK_UZI_SHOOT:
            case ATK_FAMAS_SHOOT:
            case ATK_GLOCK_SHOOT:
                spawnbouncer(d->balles, d->balles, d, atk==ATK_UZI_SHOOT ? BNC_DOUILLESUZI : BNC_DOUILLES, bnclifetime+rnd(bnclifetime));
                particle_flare(d->muzzle, d->muzzle, 125, PART_NORMAL_MUZZLE_FLASH, d->steromillis ? 0xFF2222 : d->aptisort2 && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0xFFFFFF, d==hudplayer() ? zoom ? 0.5f : 0.75f : 1.75f, d, player1->champimillis ? true : false);
                particle_flare(d->muzzle, d->muzzle, 75, PART_MINIGUN_MUZZLE_FLASH, d->steromillis ? 0xFF2222 : d->aptisort2 && d->aptitude==APT_MAGICIEN ? 0xFF22FF : 0xFFAA55, d==hudplayer() ? zoom ? 0.75f : 2.f : 3.f, d, player1->champimillis ? true : false);
                if(d->ragemillis) particle_flare(d->muzzle, d->muzzle, 80, PART_MINIGUN_MUZZLE_FLASH, 0xFF2222, d==hudplayer() ? zoom ? 1.5f : 4.f : 5.f, d, player1->champimillis ? true : false);
                particle_splash(PART_SMOKE, d==hudplayer() ? 3 : 5, d==hudplayer() ? 350 : 500, d->muzzle, 0x444444, d==hudplayer() ? 3.5f : 4.5f, 20, 500, 0, player1->champimillis ? true : false);
                particle_splash(PART_SPARK, d==hudplayer() ? 3 : 5, 35, d->muzzle, 0xFF4400, 0.35f, 300, 500, 0, player1->champimillis ? true : false);
                adddynlight(hudgunorigin(gun, d->o, to, d), 60, vec(1.25f, 0.75f, 0.3f), 30, 2, DL_FLASH, 0, vec(1.25f, 0.75f, 0.3f), d);
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                if(d!=hudplayer()) sound_nearmiss(S_FLYBY, from, to);
                if(d==player1) {if(atk!=ATK_GLOCK_SHOOT) mousemove((-3+rnd(7))/movefactor, (-3+rnd(7))/movefactor, player1->aptisort2 && player1->aptitude==APT_MAGICIEN ? true : false);}
                break;
            case ATK_LANCEFLAMMES_SHOOT:
            {
                if(!local) createrays(gun, from, to, d);
                if(d->type==ENT_PLAYER) sound = S_FLAMEATTACK;
                loopi(attacks[atk].rays)
                {
                    vec irays(rays[i]);
                    irays.sub(d->muzzle);
                    irays.normalize().mul(1550.0f);

                    switch(rnd(5))
                    {
                        case 0: particle_flying_flare(d->muzzle, irays, 700, PART_FLAME1+rnd(2), d->steromillis ? 0x881111 :  0x604930, (7.f+rnd(16))/10.f, 100, 1, player1->champimillis ? true : false); flamehit(from, rays[i]); break;
                        case 1: particle_flying_flare(d->muzzle, irays, 700, PART_FLAME1+rnd(2), d->steromillis ? 0x770000 :  0x474747, (7.f+rnd(16))/10.f, 100, 1, player1->champimillis ? true : false); flamehit(from, rays[i]); break;
                        case 2: particle_flying_flare(d->muzzle, irays, 700, PART_FLAME1+rnd(2), d->steromillis ? 0x991111 :  0x383838, (7.f+rnd(16))/10.f, 100, 1, player1->champimillis ? true : false); flamehit(from, rays[i]); break;
                        default:
                            particle_flying_flare(d->muzzle, irays, 1100, PART_SMOKE, 0x111111, (10.f+rnd(18))/10.f, -20, 1, player1->champimillis ? true : false);
                            adddynlight(hudgunorigin(gun, d->o, irays, d), 50, vec(0.40f, 0.2f, 0.1f), 100, 100, L_NODYNSHADOW, 10, vec(0.50f, 0, 0), d);
                            switch(rnd(2)){case 0: if(d!=hudplayer()) sound_nearmiss(S_FLYBYFLAME, from, rays[i]);}
                    }
                }
                if(d->ragemillis) particle_splash(PART_SPARK,  3, 500, d->muzzle, 0xFF2222, 1.0f,  50,   200, 0, player1->champimillis ? true : false);
                particle_flare(d->muzzle, d->muzzle, 120, PART_NORMAL_MUZZLE_FLASH, d->steromillis ? 0x880000 : d->aptisort2 && d->aptitude==APT_MAGICIEN ? 0x440044 : 0x663311,  d==hudplayer() ? zoom ? 2.00f : 3.5f : 4.5f, d, player1->champimillis ? true : false);

                float dist = from.dist(to);
                vec up = to;
                up.z += dist/8;
                newbouncer(d==player1 && !thirdperson ? d->muzzle : hudgunorigin(gun, d->o, to, d), up, local, id, d, BNC_LIGHT, 650, 400);

                break;
            }
            case ATK_GRAP1_SHOOT:
            {
                newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
                particle_flare(d->muzzle, d->muzzle, 150, PART_RAIL_MUZZLE_FLASH, d->steromillis ? 0xFF4444 : d->aptisort2 && d->aptitude==APT_MAGICIEN ? 0xFF00FF : 0xFF55FF, 2.5f, d, player1->champimillis ? true : false);
                if(d->ragemillis) particle_splash(PART_SPARK,  3, 500, d->muzzle, 0xFF4444, 1.0f,  50,   200, 0, player1->champimillis ? true : false);
                adddynlight(hudgunorigin(gun, d->o, to, d), 70, vec(1.0f, 0.0f, 1.0f), 80, 100, DL_FLASH, 0, vec(0, 0, 0), d);
                break;
            }
            case ATK_M32_SHOOT:
            {
                float dist = from.dist(to);
                vec up = to;
                up.z += dist/8;
                particle_splash(PART_SMOKE,  10, 600, d->muzzle, d->aptisort2 && d->aptitude==APT_MAGICIEN ? 0x550044 : 0x444444, 4.0f, 20, 500, 0, player1->champimillis ? true : false);
                if(d->ragemillis) particle_splash(PART_SPARK,  8, 500, d->muzzle, 0xFF4444, 1.0f,  50,   200, 0, player1->champimillis ? true : false);
                newbouncer(d==player1 && !thirdperson ? d->muzzle : hudgunorigin(gun, d->o, to, d), up, local, id, d, BNC_GRENADE, attacks[atk].ttl, attacks[atk].projspeed);
                break;
            }
            case ATK_KAMIKAZE_SHOOT:
            case ATK_ASSISTXPL_SHOOT:
            {
                float dist = from.dist(to);
                vec up = to;
                up.z += dist/8;
                newbouncer(d==player1 && !thirdperson ?  d->muzzle : hudgunorigin(gun, d->o, to, d), up, local, id, d, atk==ATK_KAMIKAZE_SHOOT ? BNC_KAMIKAZE : BNC_ASSISTEXPL, attacks[atk].ttl, attacks[atk].projspeed);
                break;
            }
            default:
                break;
        }

        if(d->steromillis>0)
        {
            switch(rnd(attacks[atk].specialsounddelay))
            {
                case 0:
                {
                    playsound(S_STEROSTIR, d==hudplayer() ? NULL : &d->o, NULL, 0, 0 , 300, -1, 500);
                    if(camera1->o.dist(hudgunorigin(gun, d->o, to, d)) >= 400 && d!=player1) playsound(S_STEROTIRLOIN, &d->o, NULL, 0, 0 , 600, -1, 800);
                }
            }
        }

        looped = false;
        if(d->attacksound >= 0 && d->attacksound != sound) d->stopattacksound(d);
        switch(sound)
        {
            case S_FLAMEATTACK:
            case S_GAU8:
            case S_DEMARRAGEPLASMA:
                if(d->attacksound >= 0) looped = true;
                d->attacksound = sound;
                d->attackchan = playsound(sound, d==hudplayer() ? NULL : &d->o, NULL, 0, -1, atk==ATK_GAU8_SHOOT ? 75 : 50, d->attackchan, atk==ATK_GAU8_SHOOT ? 600 : 350);
                if(sound==S_GAU8 || sound==S_FLAMEATTACK) return;
            default:
                {
                    if(d==hudplayer()) playsound(attacks[atk].sound, NULL);
                    else playsound(attacks[atk].sound, &d->o, NULL, 0, 0, atk==ATK_ASSISTXPL_SHOOT || atk==ATK_KAMIKAZE_SHOOT ? 75 : 50, -1, atk==ATK_ASSISTXPL_SHOOT || atk==ATK_KAMIKAZE_SHOOT ? 600 : 350);
                }
        }

        if(camera1->o.dist(hudgunorigin(gun, d->o, to, d)) >= 300) playsound(attacks[atk].farsound1, &d->muzzle, NULL, 0, 0 , 200, -1, 500);
        else if(camera1->o.dist(hudgunorigin(gun, d->o, to, d)) >= 450 && camera1->o.dist(hudgunorigin(gun, d->o, to, d)) <= 650) playsound(attacks[atk].farsound2, &d->muzzle, NULL, 0, 0 , 200, -1, 700);
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
            if(atk==ATK_RAIL_SHOOT) railhit(from, to, d->muzzle);

            hitpush(attacks[atk].damage, o, d, from, to, atk, 1);
        }
        else if(atk==ATK_RAIL_SHOOT) railhit(from, to, d->muzzle, true);
    }

    float kickfactor = 2.5f;

    void shoot(gameent *d, const vec &targ)
    {
        int prevaction = d->lastaction, attacktime = lastmillis-prevaction;

        if(d->aitype==AI_BOT && (d->gunselect==GUN_GLOCK || d->gunselect==GUN_SPOCKGUN || d->gunselect==GUN_HYDRA || d->gunselect==GUN_SKS || d->gunselect==GUN_S_CAMPOUZE))
        {
            switch(rnd(d->gunselect==GUN_GLOCK || d->gunselect==GUN_SPOCKGUN || d->gunselect==GUN_HYDRA ? 5 : 15)) {case 0: d->gunwait+=(d->aptitude==APT_PRETRE && d->aptisort3 ? 500 : 1200)/nbfps;}
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

        if(attacktime < d->gunwait + d->gunaccel*(d->gunselect==GUN_PULSE ? 50 : d->gunselect==GUN_S_ROQUETTES ? 150 : 8) + (d==player1 ? 0 : attacks[d->gunselect].attackdelay)) return;
        d->gunwait = 0;

        if(d->aptitude==APT_KAMIKAZE)
        {
            if(d->aptisort2>0 && d->aptisort2<2000 && d->ammo[GUN_KAMIKAZE]>0 && !d->playerexploded)
            {
                gunselect(GUN_KAMIKAZE, d);
                d->attacking = ACT_SHOOT;
                d->lastattack = -1;
                d->playerexploded = true;
            }
        }

        if(d->armour==0 && !d->playerexploded && d->ammo[GUN_ASSISTXPL]>0)
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

        switch(atk)
        {
            case ATK_GAU8_SHOOT:
            case ATK_NUKE_SHOOT:
            case ATK_CAMPOUZE_SHOOT:
            case ATK_ROQUETTES_SHOOT:
            case ATK_KAMIKAZE_SHOOT:
            case ATK_ASSISTXPL_SHOOT:
                d->ammo[gun]--;
                break;
            case ATK_CAC349_SHOOT:
            case ATK_CACMARTEAU_SHOOT:
            case ATK_CACMASTER_SHOOT:
            case ATK_CACFLEAU_SHOOT:
            case ATK_CACNINJA_SHOOT:
                break;
            default:
                if(!m_muninfinie) d->ammo[gun]--;
        }

        vec from = d->o, to = targ, dir = vec(to).sub(from).safenormalize();
        float dist = to.dist(from);
        if(!(d->physstate >= PHYS_SLOPE && d->crouching && d->crouched()))
        {
            switch (d->aptitude)
            {
                case 2: kickfactor = 0; break;
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

        if(!attacks[atk].projspeed) raydamage(from, to, d, atk);

        shoteffects(atk, from, to, d, true, 0, prevaction);

        if(d==player1 || d->ai)
        {
            addmsg(N_SHOOT, "rci2i6iv", d, lastmillis-maptime, atk,
                   (int)(from.x*DMF), (int)(from.y*DMF), (int)(from.z*DMF),
                   (int)(to.x*DMF), (int)(to.y*DMF), (int)(to.z*DMF),
                   hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
        }
        float waitfactor = 1;
        if(d->aptitude==APT_PRETRE && d->aptisort3) waitfactor = 2.5f;
        if(d->champimillis>0) waitfactor*=1.25f;
        d->gunwait = attacks[atk].attackdelay/waitfactor;
        //if(d->ai) d->gunwait += int(d->gunwait*(((101-d->skill)+rnd(111-d->skill))/100.f));
        d->steromillis ? d->totalshots += (attacks[atk].damage*attacks[atk].rays)*2: d->totalshots += attacks[atk].damage*attacks[atk].rays;

        if(d->playerexploded){d->attacking = ACT_IDLE; d->playerexploded = false; weaponswitch(d);}
        if(atk==ATK_GLOCK_SHOOT || atk==ATK_SPOCKGUN_SHOOT || atk==ATK_HYDRA_SHOOT || d->gunselect==GUN_SKS || d->gunselect==GUN_S_CAMPOUZE) d->attacking = ACT_IDLE;
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
            if(bnc.bouncetype==BNC_GRENADE || bnc.bouncetype==BNC_LIGHT)
            {
                vec pos(bnc.o);
                pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
                if(bnc.bouncetype==BNC_GRENADE) adddynlight(pos, 40, vec(0.5f, 0.5f, 2.0f));
                else adddynlight(pos, 80+rnd(15), vec(0.2f, 0.09f, 0.0f), 0, 0, L_VOLUMETRIC|L_NODYNSHADOW);
            }
        }
    }

    static const char * const gibnames[3] = { "pixels/noir", "pixels/jaune", "pixels/rouge" };
    static const char * const douillesnames[1] = { "douilles/normale" };
    static const char * const bigdouillesnames[1] = { "douilles/grosse" };
    static const char * const cartouchessnames[1] = { "douilles/cartouche" };
    static const char * const debrisnames[1] = { "pixels/noir" };

    void preloadbouncers()
    {
        loopi(sizeof(gibnames)/sizeof(gibnames[0])) preloadmodel(gibnames[i]);
        loopi(sizeof(douillesnames)/sizeof(douillesnames[0])) preloadmodel(douillesnames[i]);
        loopi(sizeof(bigdouillesnames)/sizeof(bigdouillesnames[0])) preloadmodel(bigdouillesnames[i]);
        loopi(sizeof(cartouchessnames)/sizeof(cartouchessnames[0])) preloadmodel(cartouchessnames[i]);
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
            pitch = -bnc.roll;
            if(vel.magnitude() <= 25.0f) {yaw = bnc.lastyaw; pitch = bnc.lastpitch;}
            else
            {
                vectoyawpitch(vel, yaw, pitch);
                yaw += 90;
                bnc.lastyaw = yaw;
                bnc.lastpitch = pitch;
            }
			int cull = MDL_CULL_VFC|MDL_CULL_DIST|MDL_CULL_OCCLUDED;

            if(bnc.bouncetype==BNC_GRENADE) rendermodel("projectiles/grenade", ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, cull);
            else
            {
                const char *mdl = NULL;
                float fade = 1;
                if(bnc.lifetime < 250) fade = bnc.lifetime/250.0f;
                switch(bnc.bouncetype)
                {
                    case BNC_GIBS: mdl = gibnames[bnc.variant]; break;
                    case BNC_DEBRIS: mdl = "pixels/noir"; break;
                    case BNC_DOUILLES: case BNC_DOUILLESUZI: mdl = "douilles/normale"; break;
                    case BNC_BIGDOUILLES: mdl = "douilles/grosse"; break;
                    case BNC_CARTOUCHES: mdl = "douilles/cartouche"; break;
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
                switch(p.atk)
                {
                    case ATK_NUKE_SHOOT:
                        particle_flare(pos, pos, 1, PART_NORMAL_MUZZLE_FLASH, p.owner->steromillis ? 0xFF0000 : 0xFFC864, 6.0f+rndscale(6), NULL, player1->champimillis ? true : false);
                        particle_splash(lookupmaterial(pos)&MAT_WATER ? PART_BULLE : PART_SMOKE, 3, lookupmaterial(pos)&MAT_WATER ? 2000 : 8000, pos, lookupmaterial(pos)&MAT_WATER ? 0x18181A : 0x111111, 7.0f, 25, 100, 0, player1->champimillis ? true : false);
                        particle_splash(PART_FLAME1+rnd(2), 1, 100, pos, p.owner->steromillis ? 0xFF0000 : 0xFF6600, 1.0f+rndscale(4), 50, 500, 0, player1->champimillis ? true : false);
                        break;
                    case ATK_SMAW_SHOOT:
                    case ATK_ROQUETTES_SHOOT:
                        particle_splash(PART_SMOKE, 1, 2000, pos, 0x666666, 6.0f, 25, 250, 0, player1->champimillis ? true : false);
                        particle_flare(pos, pos, 1, PART_NORMAL_MUZZLE_FLASH, p.owner->steromillis ? 0xFF0000 : 0xFFC864, 3.0f+rndscale(2), NULL, player1->champimillis ? true : false);
                        break;
                    case ATK_ARTIFICE_SHOOT:
                        if(lookupmaterial(pos)&MAT_WATER)  particle_splash(PART_BULLE, 3, 200, pos, 0x18181A, 2.5f, 25, 100, 0, player1->champimillis ? true : false);
                        particle_splash(PART_SPARK, 8, 100, pos, p.owner->steromillis ? 0xFF0000 : 0xFFC864, 0.4f, 50, 500, 0, player1->champimillis ? true : false);
                        particle_splash(PART_FLAME1+rnd(2), 2, 100, pos, p.owner->steromillis ? 0xFF0000 : 0xFFFFFF, 0.8f, 50, 500, 0, player1->champimillis ? true : false);
                        particle_flare(pos, pos, 1, PART_NORMAL_MUZZLE_FLASH, p.owner->steromillis ? 0xFF0000 : 0xFFC864, 0.5f+rndscale(2), NULL, player1->champimillis ? true : false);
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
            case S_GAU8:
                atk = ATK_GAU8_SHOOT;
                break;
            case S_DEMARRAGEPLASMA:
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
        if(d->clientnum >= 0 && d->state == CS_ALIVE)
        {
            d->dansechan = playsound(S_CGCORTEX+(d->customdanse), local ? NULL : &d->o, NULL, 0, -1, -1, d->dansechan, 400);
            if(d->dansechan < 0) d->dansechan = -1;
        }
        else d->stopdansesound(d);
    }

    void checksortsound(gameent *d, bool local)
    {
        if(d->clientnum >= 0 && d->state == CS_ALIVE)
        {
            int neededdata = 0;
            switch(d->aptitude) {case APT_PHYSICIEN: neededdata++; break; case APT_PRETRE: neededdata+=2; break; case APT_SHOSHONE: neededdata+=3; break; case APT_ESPION: neededdata+=4; break; case APT_KAMIKAZE: neededdata+=5;}

            d->sortchan = playsound(d->aptisort1 ? sorts[neededdata].sound1 : d->aptisort2 ? sorts[neededdata].sound2 : sorts[neededdata].sound3, local ? NULL : &d->o, NULL, 0, -1, -1, d->sortchan, 300);
            if(d->sortchan < 0) d->sortchan = -1;
        }
        else d->stopsortsound(d);
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
            checksortsound(d, d==following);
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
