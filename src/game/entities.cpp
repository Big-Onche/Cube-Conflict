#include "game.h"
#include "engine.h"
#include "ccheader.h"
#include "stats.h"

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

    const char *entmdlname(int type)
    {
        static const char * const entmdlnames[MAXENTTYPES] =
        {
            NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
            //Armes
            "munitions/fusilelectrique", "munitions/fusilplasma", "munitions/smaw", "munitions/minigun", "munitions/spockgun", "munitions/M32",
            "munitions/lanceflammes", "munitions/uzi", "munitions/famas", "munitions/mossberg500", "munitions/hydra", "munitions/SV_98",
            "munitions/sks", "munitions/arbalete", "munitions/ak47", "munitions/GRAP1", "munitions/feuartifice", "munitions/glock",
            "munitions/supercaisse", NULL, "munitions/supercaisse", "munitions/supercaisse",
            //Objets
            "objets/panachay", "objets/cochongrillay", "objets/steroides", "objets/champis", "objets/epo", "objets/joint",
            "objets/bouclierbois", "objets/bouclierfer", "objets/bouclieror", "objets/boucliermagnetique", "objets/armureassistee",
            "objets/mana",

            "objets/teleporteur",

            NULL, NULL, NULL,
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
                    if(!e.spawned() || e.type < I_RAIL || e.type > I_MANA) continue;
                    break;
            }
            const char *mdlname = entmodel(e);
            const char *secmdlname = e.type==I_BOUCLIERBOIS || e.type==I_BOUCLIERFER || e.type==I_BOUCLIEROR || e.type==I_BOUCLIERMAGNETIQUE ? "objets/piecerobotique" : entmodel(e);
            if(mdlname)
            {
                vec p = e.o;
                p.z += 1+sinf(lastmillis/100.0+e.o.x+e.o.y)/20;
                rendermodel(game::player1->armourtype==A_ASSIST && game::player1->armour>0 ? secmdlname : mdlname, ANIM_MAPMODEL|ANIM_LOOP, p, lastmillis/(float)revs, 0, 0, MDL_CULL_VFC | MDL_CULL_DIST | MDL_CULL_OCCLUDED);
            }
        }
    }

    VAR(autowield, -1, 0, 1);

    void addammo(int type, int &v, bool local)
    {
        itemstat &is = itemstats[type-I_RAIL];
        v += is.add;
        if(v>is.max) v = is.max;
        if(local) msgsound(is.sound);
    }

    void repammo(gameent *d, int type, bool local)
    {
        addammo(type, d->ammo[type-I_RAIL+GUN_RAIL], local);
    }

    // these two functions are called when the server acknowledges that you really
    // picked up the item (in multiplayer someone may grab it before you).

    void pickupeffects(int n, gameent *d, int rndsweap)
    {
        if(!ents.inrange(n)) return;
        extentity *e = ents[n];
        int type = e->type;
        if(type<I_RAIL || type>I_MANA) return;
        e->clearspawned();
        e->clearnopickup();
        if(!d) return;
        d->pickup(type, d->aptitude, d->aptisort1, d->armourtype, rndsweap);
        if(type==I_SUPERARME) conoutf("%d N, %d G, %d, C %d, R, RND %d", d->ammo[GUN_S_NUKE], d->ammo[GUN_S_GAU8], d->ammo[GUN_S_CAMPOUZE], d->ammo[GUN_S_ROQUETTES], rndsweap);

        if(type>=I_RAIL && type<=I_SUPERARME)
        {
            if(d!=player1) gunselect(type-9+rndsweap, d);
            else if(autowield==1) gunselect(type-9+rndsweap, player1);
        }

        if(d->aptisort1 && d->aptitude==APT_PRETRE)
        {
            adddynlight(d->o, 20, vec(1.5f, 1.5f, 0.0f), 300, 50, L_NOSHADOW|L_VOLUMETRIC);
            playsound(S_SORTPRETRE1, d==hudplayer() ? NULL : &d->o, NULL, 0, 0, d==hudplayer() ? 0 : 150, -1, 300);
        }

        playsound((type==I_BOUCLIERBOIS || type==I_BOUCLIERFER || type == I_BOUCLIERMAGNETIQUE || type==I_BOUCLIEROR) && d->armourtype==A_ASSIST ? S_ITEMPIECEROBOTIQUE : itemstats[type-I_RAIL].sound, d==hudplayer() ? NULL : &d->o, NULL, 0, 0, d==hudplayer() ? 0 : 150, -1, 300);

        if(d==player1) switch(type)
        {
            case I_BOOSTPV:
                playsound(S_COCHON, NULL, NULL, 0, 0, 0, -1, 0, 0);
                break;
            case I_BOOSTDEGATS:
                playsound(S_ITEMSTEROS, NULL, NULL, 0, 0, 0, -1, 0);
                break;
            case I_BOOSTPRECISION:
                addpostfx("sobel", 1, 1, 1, 1, vec4(1, 1, 1, 1));
                fullbrightmodels = 200;
                playsound(S_ITEMCHAMPIS, NULL, NULL, 0, 0, 0, -1, 0);
                break;
            case I_BOOSTVITESSE:
                playsound(S_ITEMEPO, NULL, NULL, 0, 0, 0, -1, 0);
                break;
            case I_BOOSTGRAVITE:
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
                gameent *h = followingplayer(player1);
                playsound(snd, d==h ? NULL : &e.o, NULL, flags);
                if(d!=h && ents.inrange(td) && ents[td]->type == TELEDEST) playsound(snd, &ents[td]->o, NULL, flags);

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
                playsound(snd, d == followingplayer(player1) ? NULL : &e.o, NULL, flags);
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
        extentity *e = ents[n];
        switch(e->type)
        {
            default:
                if(d->canpickup(e->type, d->aptitude, d->armourtype))
                {
                    addmsg(N_ITEMPICKUP, "rci", d, n);
                    e->setnopickup(); // even if someone else gets it first

                    if(d==player1)
                    {
                        switch(e->type)
                        {
                            case I_SANTE: addstat(1, STAT_PANACHAY); addxpandcc(1); break;
                            case I_MANA:  addstat(1, STAT_MANA); addxpandcc(1); break;
                            case I_BOUCLIERBOIS:
                            case I_BOUCLIERFER:
                            case I_BOUCLIEROR:
                            case I_BOUCLIERMAGNETIQUE:
                                if(player1->armourtype==A_ASSIST) addstat(1, STAT_REPASSIST);
                                else
                                {
                                    addstat(1, e->type==I_BOUCLIERBOIS ? STAT_BOUCLIERBOIS : e->type==I_BOUCLIERFER ? STAT_BOUCLIERFER : e->type==I_BOUCLIERMAGNETIQUE ? STAT_BOUCLIERMAGNETIQUE : STAT_BOUCLIEROR);
                                    addxpandcc(e->type==I_BOUCLIERBOIS ? 2 : e->type==I_BOUCLIERFER ? 3 : 4);
                                }
                                break;
                            case I_ARMUREASSISTEE:  addstat(1, STAT_ARMUREASSIST); addxpandcc(5); break;
                            case I_BOOSTPV:         addstat(1, STAT_COCHON); addxpandcc(5); break;
                            case I_BOOSTDEGATS:     addstat(1, STAT_STEROS); addxpandcc(5); break;
                            case I_BOOSTVITESSE:    addstat(1, STAT_EPO); addxpandcc(5); break;
                            case I_BOOSTGRAVITE:    addstat(1, STAT_JOINT); addxpandcc(5); break;
                            case I_BOOSTPRECISION:  addstat(1, STAT_CHAMPIS); addxpandcc(5); break;
                            case I_SUPERARME:       addstat(1, STAT_SUPERARMES); addxpandcc(7); break;
                            default: if(e->type>=I_RAIL && e->type<=I_GLOCK) {addstat(1, STAT_ARMES); addxpandcc(1);}
                        }
                    }
                }
                break;

            case TELEPORT:
            {
                if(d->lastpickup==e->type && lastmillis-d->lastpickupmillis<500) break;
                if(!teleteam && m_teammode) break;
                if(e->attr3 > 0)
                {
                    defformatstring(hookname, "can_teleport_%d", e->attr3);
                    if(!execidentbool(hookname, true)) break;
                }
                d->lastpickup = e->type;
                d->lastpickupmillis = lastmillis;
                teleport(n, d);
                break;
            }

            case JUMPPAD:
            {
                if(d->lastpickup==e->type && lastmillis-d->lastpickupmillis<300) break;
                d->lastpickup = e->type;
                d->lastpickupmillis = lastmillis;
                jumppadeffects(d, n, true);
                if(d->ai) d->ai->becareful = true;
                d->falling = vec(0, 0, 0);
                d->physstate = PHYS_FALL;
                d->timeinair = 1;
                d->vel = vec(e->attr3*10.0f, e->attr2*10.0f, e->attr1*12.5f);
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
            if((!e.spawned() || e.nopickup()) && e.type!=TELEPORT && e.type!=JUMPPAD) continue;
            float dist = e.o.dist(o);
            if(dist<(e.type==TELEPORT ? 16 : 12)) trypickup(i, d);
        }
    }

    void checkboosts(int time, gameent *d)
    {
        if(d->steromillis && (d->steromillis -= time)<=0)
        {
            d->steromillis = 0;
            //playsound(S_PUPOUT, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 300);
            if(d==player1) conoutf(CON_GAMEINFO, langage ? "\f8The steroid cycle is over." : "\f8La cure de stéros est terminée.");
        }
        if((d->epomillis -= time)<=0)
        {
            d->epomillis = 0;
            //playsound(S_PUPOUT, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 300);
            if(d==player1) conoutf(CON_GAMEINFO, langage ? "\f8EPO no longer works..." : "\f8L'EPO ne fait plus effet.");
        }
        if(d->champimillis && (d->champimillis -= time)<=0)
        {
            d->champimillis = 0;
            //playsound(S_PUPOUT, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 300);
            fullbrightmodels = 0;
            clearpostfx();
            if(d==player1) conoutf(CON_GAMEINFO, langage ? "\f8The mushrooms have been digested." : "\f8Les champignons sont digérés.");
        }
        if(d->jointmillis && (d->jointmillis -= time)<=0)
        {
            d->jointmillis = 0;
            //playsound(S_PUPOUT, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 300);
            if(d==player1) conoutf(CON_GAMEINFO, langage ? "\f8You no longer feel the effect of weed" : "\f8La weed ne fait plus effet !");
        }
    }

    void checkaptiskill(int time, gameent *d)
    {
        if(d->ragemillis && (d->ragemillis -= time)<=0) d->ragemillis = 0;
        if(d->vampimillis && (d->vampimillis -= time)<=0) d->vampimillis = 0;
        if(d->aptisort1 && (d->aptisort1 -= time)<=0) d->aptisort1 = 0;
        if(d->aptisort2 && (d->aptisort2 -= time)<=0) d->aptisort2 = 0;
        if(d->aptisort3 && (d->aptisort3 -= time)<=0) d->aptisort3 = 0;
    }

    void putitems(packetbuf &p)            // puts items in network stream and also spawns them locally
    {
        putint(p, N_ITEMLIST);
        loopv(ents) if(ents[i]->type>=I_RAIL && ents[i]->type<=I_MANA && (!m_noammo || ents[i]->type<I_RAIL || ents[i]->type>I_GLOCK))
        {
            putint(p, i);
            putint(p, ents[i]->type);
        }
        putint(p, -1);
    }

    void resetspawns() { loopv(ents) { extentity *e = ents[i]; e->clearspawned(); e->clearnopickup(); } }

    void spawnitems(bool force)
    {
        loopv(ents)
        {
            extentity *e = ents[i];
            if(e->type>=I_RAIL && e->type<=I_MANA && (!m_noammo || e->type<I_RAIL || e->type>I_GLOCK))
            {
                e->setspawned(force || !server::delayspawn(e->type));
                e->clearnopickup();
            }
        }
    }

    void setspawn(int i, bool on) { if(ents.inrange(i)) { extentity *e = ents[i]; e->setspawned(on); e->clearnopickup(); } }

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
            "supercaisse", "base", "NULL", "NULL",

            "panache", "cochon_grille", "steroides", "champis", "epo", "joint",
            "bouclier_bois", "bouclier_fer", "bouclier_or", "bouclier_magnetique", "armure_assistee",
            "mana",

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
