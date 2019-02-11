#include "game.h"

namespace entities
{
    using namespace game;

    int extraentinfosize() { return 0; }       // size in bytes of what the 2 methods below read/write... so it can be skipped by other games

    void writeent(entity &e, char *buf)   // write any additional data to disk (except for ET_ ents)
    {
    }

    void readent(entity &e, char *buf, int ver)     // read from disk, and init
    {
    }

#ifndef STANDALONE
    vector<extentity *> ents;

    vector<extentity *> &getents() { return ents; }

    bool mayattach(extentity &e) { return false; }
    bool attachent(extentity &e, extentity &a) { return false; }

    const char *itemname(int i)
    {
        int t = ents[i]->type;
        if(t<I_RAIL || t>I_BOUCLIERMAGNETIQUE) return NULL;
        return itemstats[t-I_RAIL].name;
    }

    int itemicon(int i)
    {
        int t = ents[i]->type;
        if(t<I_RAIL || t>I_BOUCLIERMAGNETIQUE) return -1;
        return itemstats[t-I_RAIL].icon;
    }

    const char *entmdlname(int type)
    {
        static const char *entmdlnames[] =
        {
            NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
            "munitions/fusilelectrique", "munitions/fusilplasma", "munitions/smaw", "munitions/minigun", "munitions/spockgun", "munitions/M32",
            "munitions/lanceflammes", "munitions/uzi", "munitions/famas", "munitions/mossberg500", "munitions/hydra", "munitions/SV_98",
            "munitions/sks", "munitions/arbalete", "munitions/ak47", "munitions/GRAP1", "munitions/feuartifice", "munitions/glock",
            "munitions/supercaisse", NULL, NULL, NULL,

            "objets/panachay", "objets/cochongrillay", "objets/steroides", "objets/champis", "objets/epo", "objets/joint",
            "objets/bouclierbois", "objets/bouclierfer", "objets/bouclieror", "objets/boucliermagnetique",
            "objets/teleporteur",

            NULL, NULL, NULL, NULL,
        };
        return entmdlnames[type];
    }

    const char *entmodel(const entity &e)
    {
        if(e.type == TELEPORT)
        {
            if(e.attr2 > 0) return mapmodelname(e.attr2);
            if(e.attr2 < 0) return NULL;
        }
        return e.type < MAXENTTYPES ? entmdlname(e.type) : NULL;
    }

    void preloadentities()
    {
        loopi(MAXENTTYPES)
        {
            switch(i)
            {
                case I_RAIL: case I_PULSE: case I_SMAW: case I_MINIGUN: case I_SPOCKGUN: case I_M32:
                case I_LANCEFLAMMES: case I_UZI: case I_FAMAS: case I_MOSSBERG: case I_HYDRA: case I_SV98:
                case I_SKS: case I_ARBALETE: case I_AK47: case I_GRAP1: case I_ARTIFICE: case I_GLOCK:
                case I_S_NUKE: case I_S_GAU8: case I_S_ROQUETTES: case I_S_CAMPOUZE:
                    //if(m_noammo) continue;
                    break;
                case I_SANTE: case I_BOOSTPV: case I_BOOSTDEGATS: case I_BOOSTPRECISION: case I_BOOSTVITESSE: case I_BOOSTGRAVITE:
                case I_BOUCLIERBOIS: case I_BOUCLIERFER: case I_BOUCLIEROR: case I_BOUCLIERMAGNETIQUE:
                    break;
            }
            const char *mdl = entmdlname(i);
            if(!mdl) continue;
            preloadmodel(mdl);
        }
        loopv(ents)
        {
            extentity &e = *ents[i];
            switch(e.type)
            {
                case TELEPORT:
                    if(e.attr2 > 0) preloadmodel(mapmodelname(e.attr2));
                case JUMPPAD:
                    if(e.attr4 > 0) preloadmapsound(e.attr4);
                    break;
            }
        }
    }

    void renderentities()
    {
        loopv(ents)
        {
            extentity &e = *ents[i];
            int revs = 10;
            switch(e.type)
            {
                case TELEPORT:
                    if(e.attr2 < 0) continue;
                    break;
                default:
                    if(!e.spawned() || e.type < I_RAIL || e.type > I_BOUCLIERMAGNETIQUE) continue;
                    break;
            }
            const char *mdlname = entmodel(e);
            if(mdlname)
            {
                vec p = e.o;
                p.z += 1+sinf(lastmillis/100.0+e.o.x+e.o.y)/20;
                rendermodel(mdlname, ANIM_MAPMODEL|ANIM_LOOP, p, lastmillis/(float)revs, 0, 0, MDL_CULL_VFC | MDL_CULL_DIST | MDL_CULL_OCCLUDED);
            }
        }
    }

    void addammo(int type, int &v, bool local, gameent *d)
    {
        itemstat &is = itemstats[type-I_RAIL];
        v += is.add;
        if(v>is.max) v = is.max;
        if(local) msgsound(is.sound);
    }

    void repammo(gameent *d, int type, bool local)
    {
        addammo(type, d->ammo[type-I_RAIL+GUN_RAIL], local, d);
    }

    // these two functions are called when the server acknowledges that you really
    // picked up the item (in multiplayer someone may grab it before you).

    void pickupeffects(int n, gameent *d)
    {
        if(!ents.inrange(n)) return;
        int type = ents[n]->type;
        if(type<I_RAIL || type>I_BOUCLIERMAGNETIQUE) return;
        ents[n]->clearspawned();
        if(!d) return;
        itemstat &is = itemstats[type-I_RAIL];
        if(d!=player1 || isthirdperson())
        {
            particle_icon(d->abovehead(), is.icon%4, is.icon/4, PART_HUD_ICON_GREY, 2000, 0xFFFFFF, 2.0f, -8);
        }
        d->pickup(type, d->aptitude);
        //d->pickup(type, d->classe, d->sortmiracle, gamemode);
        if(d==player1) playsound(itemstats[type-I_RAIL].sound, d!=player1 ? &d->o : NULL, NULL, 0, 0, 0, -1, 0, 1500);
        if(d==player1) switch(type)
        {
            case I_BOOSTPV:
                conoutf(CON_GAMEINFO, "\f8Le cochon grillay te donne 75 points de vie supplémentaires !");
                playsound(S_COCHON, NULL, NULL, 0, 0, 0, -1, 0, 0);
                break;
            case I_BOOSTDEGATS:
                conoutf(CON_GAMEINFO, "\f8C'est l'heure de la R-R-R-Roid Rage");
                playsound(S_ITEMSTEROS, NULL, NULL, 0, 0, 0, -1, 0);
                break;
            case I_BOOSTPRECISION:
                //arprecision = true;
                //string hallu;
                //switch(rnd(3)) {
                // case 0: copystring(hallu, "sobel"); break;
                // case 1: copystring(hallu, "rotoscope"); break;
                // case 2: copystring(hallu, "gbr"); break;
                //}
                //addpostfx(hallu, 1, 1, 1, 1, vec4(1, 1, 1, 1));
                conoutf(CON_GAMEINFO, "\f8Les psilos commencent à te petay à laggle !");
                playsound(S_ITEMCHAMPIS, NULL, NULL, 0, 0, 0, -1, 0);
                break;
            case I_BOOSTVITESSE:
                switch(rnd(10))
                {
                    case 0:
                        suicide(player1);
                        conoutf(CON_GAMEINFO, "\f8L'EPO vient de faire claquer ton coeur. Ca t'apprendra à te doper sale raclure !");
                        playsound(S_ITEMEPO, NULL, NULL, 0, 0, 0, -1, 0, 750);
                        break ;
                    default:
                        conoutf(CON_GAMEINFO, "\f8L'EPO commence à te booster...");
                        playsound(S_ITEMEPO, NULL, NULL, 0, 0, 0, -1, 0);
                }
                break;
            case I_BOOSTGRAVITE:
                conoutf(CON_GAMEINFO, "\f8Ce putain de royal te rend complètement high :bave:");
                playsound(S_ITEMJOINT, NULL, NULL, 0, 0, 0, -1, 0);
                break;
        }
    }

    // these functions are called when the client touches the item

    void teleporteffects(gameent *d, int tp, int td, bool local)
    {
        if(ents.inrange(tp) && ents[tp]->type == TELEPORT)
        {
            extentity &e = *ents[tp];
            if(e.attr4 >= 0)
            {
                int snd = S_TELEPORTEUR, flags = 0;
                if(e.attr4 > 0) { snd = e.attr4; flags = SND_MAP; }
                if(d == player1) playsound(snd, NULL, NULL, flags);
                else
                {
                    playsound(snd, &e.o, NULL, flags);
                    if(ents.inrange(td) && ents[td]->type == TELEDEST) playsound(snd, &ents[td]->o, NULL, flags);
                }
            }
        }
        if(local && d->clientnum >= 0)
        {
            sendposition(d);
            packetbuf p(32, ENET_PACKET_FLAG_RELIABLE);
            putint(p, N_TELEPORT);
            putint(p, d->clientnum);
            putint(p, tp);
            putint(p, td);
            sendclientpacket(p.finalize(), 0);
            flushclient();
        }
    }

    void jumppadeffects(gameent *d, int jp, bool local)
    {
        if(ents.inrange(jp) && ents[jp]->type == JUMPPAD)
        {
            extentity &e = *ents[jp];
            if(e.attr4 >= 0)
            {
                int snd = S_JUMPPAD, flags = 0;
                if(e.attr4 > 0) { snd = e.attr4; flags = SND_MAP; }
                if(d == player1) playsound(snd, NULL, NULL, flags);
                else playsound(snd, &e.o, NULL, flags);
            }
        }
        if(local && d->clientnum >= 0)
        {
            sendposition(d);
            packetbuf p(16, ENET_PACKET_FLAG_RELIABLE);
            putint(p, N_JUMPPAD);
            putint(p, d->clientnum);
            putint(p, jp);
            sendclientpacket(p.finalize(), 0);
            flushclient();
        }
    }

    void teleport(int n, gameent *d)     // also used by monsters
    {
        int e = -1, tag = ents[n]->attr1, beenhere = -1;
        for(;;)
        {
            e = findentity(TELEDEST, e+1);
            if(e==beenhere || e<0) { conoutf(CON_WARN, "no teleport destination for tag %d", tag); return; }
            if(beenhere<0) beenhere = e;
            if(ents[e]->attr2==tag)
            {
                teleporteffects(d, n, e, true);
                d->o = ents[e]->o;
                d->yaw = ents[e]->attr1;
                if(ents[e]->attr3 > 0)
                {
                    vec dir;
                    vecfromyawpitch(d->yaw, 0, 1, 0, dir);
                    float speed = d->vel.magnitude2();
                    d->vel.x = dir.x*speed;
                    d->vel.y = dir.y*speed;
                }
                else d->vel = vec(0, 0, 0);
                entinmap(d);
                updatedynentcache(d);
                ai::inferwaypoints(d, ents[n]->o, ents[e]->o, 16.f);
                break;
            }
        }
    }

    VARR(teleteam, 0, 1, 1);

    void trypickup(int n, gameent *d)
    {
        switch(ents[n]->type)
        {
            default:
                if(d->canpickup(ents[n]->type, d->aptitude))
                {
                    addmsg(N_ITEMPICKUP, "rci", d, n);
                    ents[n]->clearspawned(); // even if someone else gets it first
                }
                break;

            case TELEPORT:
            {
                if(d->lastpickup==ents[n]->type && lastmillis-d->lastpickupmillis<500) break;
                if(!teleteam && m_teammode) break;
                if(ents[n]->attr3 > 0)
                {
                    defformatstring(hookname, "can_teleport_%d", ents[n]->attr3);
                    if(!execidentbool(hookname, true)) break;
                }
                d->lastpickup = ents[n]->type;
                d->lastpickupmillis = lastmillis;
                teleport(n, d);
                break;
            }

            case JUMPPAD:
            {
                if(d->lastpickup==ents[n]->type && lastmillis-d->lastpickupmillis<300) break;
                d->lastpickup = ents[n]->type;
                d->lastpickupmillis = lastmillis;
                jumppadeffects(d, n, true);
                if(d->ai) d->ai->becareful = true;
                d->falling = vec(0, 0, 0);
                d->physstate = PHYS_FALL;
                d->timeinair = 1;
                d->vel = vec(ents[n]->attr3*10.0f, ents[n]->attr2*10.0f, ents[n]->attr1*12.5f);
                break;
            }
        }
    }

    void checkitems(gameent *d)
    {
        if(d->state!=CS_ALIVE) return;
        vec o = d->feetpos();
        loopv(ents)
        {
            extentity &e = *ents[i];
            if(e.type==NOTUSED) continue;
            if(!e.spawned() && e.type!=TELEPORT && e.type!=JUMPPAD) continue;
            float dist = e.o.dist(o);
            if(dist<(e.type==TELEPORT ? 16 : 12)) trypickup(i, d);
        }
    }

    void checkstero(int time, gameent *d)
    {
        if(d->steromillis && (d->steromillis -= time)<=0)
        {
            d->steromillis = 0;
            //playsound(S_PUPOUT, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 300);
            if(d==player1) conoutf(CON_GAMEINFO, "\f8La cure de stéros est terminée !");
        }
    }

    void checkepo(int time, gameent *d)
    {
        if(d->epomillis && (d->epomillis -= time)<=0)
        {
            d->epomillis = 0;
//            playsound(S_PUPOUT, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 300);
            if(d==player1) conoutf(CON_GAMEINFO, "\f8L'EPO ne fait plus effet !");
        }
    }

    void checkjoint(int time, gameent *d)
    {
        if(d->jointmillis && (d->jointmillis -= time)<=0)
        {
            d->jointmillis = 0;
//            playsound(S_PUPOUT, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 300);
            if(d==player1) conoutf(CON_GAMEINFO, "\f8La weed ne fait plus effet, il est temps de fumer un autre pétard !");
        }
    }

    void checkchampi(int time, gameent *d)
    {
        if(d->champimillis && (d->champimillis -= time)<=0)
        {
            d->champimillis = 0;
//            playsound(S_PUPOUT, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 300);
            if(d==player1) conoutf(CON_GAMEINFO, "\f8Les champignons sont digérés, il est temps d'en reprendre !");
        }
    }

    void checkrage(int time, gameent *d)
    {
        if(d->ragemillis && (d->ragemillis -= time)<=0)
        {
            d->ragemillis = 0;
            //playsound(S_PUPOUT, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 300);
        }
    }

    void putitems(packetbuf &p)            // puts items in network stream and also spawns them locally
    {
        putint(p, N_ITEMLIST);
        loopv(ents) if(ents[i]->type>=I_RAIL && ents[i]->type<=I_BOUCLIERMAGNETIQUE) // && (!m_noammo || ents[i]->type<I_RAIL || ents[i]->type>I_GLOCK)
        {
            putint(p, i);
            putint(p, ents[i]->type);
        }
        putint(p, -1);
    }

    void resetspawns() { loopv(ents) ents[i]->clearspawned(); }

    void spawnitems(bool force)
    {
        //if(m_noitems) return;
        loopv(ents) if(ents[i]->type>=I_RAIL && ents[i]->type<=I_BOUCLIERMAGNETIQUE) //&& (!m_noammo || ents[i]->type<I_SHELLS || ents[i]->type>I_CARTRIDGES)
        {
            ents[i]->setspawned(force || !server::delayspawn(ents[i]->type));
        }
    }

    void setspawn(int i, bool on) { if(ents.inrange(i)) ents[i]->setspawned(on); }

    extentity *newentity() { return new gameentity(); }
    void deleteentity(extentity *e) { delete (gameentity *)e; }

    void clearents()
    {
        while(ents.length()) deleteentity(ents.pop());
    }

    void animatemapmodel(const extentity &e, int &anim, int &basetime)
    {
    }

    void fixentity(extentity &e)
    {
        switch(e.type)
        {
            case FLAG:
                e.attr5 = e.attr4;
                e.attr4 = e.attr3;
            case TELEDEST:
                e.attr3 = e.attr2;
                e.attr2 = e.attr1;
                e.attr1 = (int)player1->yaw;
                break;
        }
    }

    void entradius(extentity &e, bool color)
    {
        switch(e.type)
        {
            case TELEPORT:
                loopv(ents) if(ents[i]->type == TELEDEST && e.attr1==ents[i]->attr2)
                {
                    renderentarrow(e, vec(ents[i]->o).sub(e.o).normalize(), e.o.dist(ents[i]->o));
                    break;
                }
                break;

            case JUMPPAD:
                renderentarrow(e, vec((int)(char)e.attr3*10.0f, (int)(char)e.attr2*10.0f, e.attr1*12.5f).normalize(), 4);
                break;

            case FLAG:
            case TELEDEST:
            {
                vec dir;
                vecfromyawpitch(e.attr1, 0, 1, 0, dir);
                renderentarrow(e, dir, 4);
                break;
            }
        }
    }

    bool printent(extentity &e, char *buf, int len)
    {
        return false;
    }

    const char *entnameinfo(entity &e) { return ""; }
    const char *entname(int i)
    {
        static const char * const entnames[MAXENTTYPES] =
        {
            "none?", "light", "mapmodel", "playerstart", "envmap", "particles", "sound", "spotlight", "decal",

            "fusil_electrique", "fusil_plasma", "smaw", "minigun", "spockgun", "m32",
            "lanceflammes", "uzi", "famas", "mossberg", "hydra", "SV98",
            "sks", "arbalete", "ak47", "grap1", "feu_artifice", "glock",
            "bombe_nucleaire", "gau8", "miniroquettes", "campouze2000",

            "panache", "cochon_grille", "steroides", "champis", "epo", "joint",
            "bouclier_bois", "bouclier_fer", "bouclier_or", "bouclier_magnetique",

            "teleport", "teledest", "jumppad", "flag",
        };
        return i>=0 && size_t(i)<sizeof(entnames)/sizeof(entnames[0]) ? entnames[i] : "";
    }

    void editent(int i, bool local)
    {
        extentity &e = *ents[i];
        //e.flags = 0;
        if(local) addmsg(N_EDITENT, "rii3ii5", i, (int)(e.o.x*DMF), (int)(e.o.y*DMF), (int)(e.o.z*DMF), e.type, e.attr1, e.attr2, e.attr3, e.attr4, e.attr5);
    }

    float dropheight(entity &e)
    {
        if(e.type==FLAG) return 0.0f;
        return 4.0f;
    }
#endif
}

