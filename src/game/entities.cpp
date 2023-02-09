#include "gfx.h"
#include "stats.h"
#include "engine.h"

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

            NULL, NULL, NULL, NULL, NULL, "objets/respawn", NULL, NULL,
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
            int revs = hudplayer()->boostmillis[B_SHROOMS] ? 2 : 10;
            switch(e.type)
            {
                case TELEPORT:
                case RESPAWNPOINT:
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
                rendermodel(game::player1->armourtype==A_ASSIST && game::player1->armour>0 ? secmdlname : mdlname, ANIM_MAPMODEL|ANIM_LOOP, p, lastmillis/(float)revs, 0, 0, MDL_CULL_VFC | MDL_CULL_EXTDIST | MDL_CULL_OCCLUDED | MDL_FORCESHADOW);
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
        int type = ents[n]->type;
        if(type<I_RAIL || type>I_MANA) return;
        ents[n]->clearspawned();
        if(!d) return;
        d->pickupitem(type, d->aptitude, d->abilitymillis[ABILITY_1], d->armourtype==A_ASSIST, rndsweap);

        if(type>=I_RAIL && type<=I_SUPERARME)
        {
            if(d!=player1) gunselect(type-9+rndsweap, d);
            else if(autowield==1) gunselect(type-9+rndsweap, player1);
        }

        if(d->abilitymillis[ABILITY_1] && d->aptitude==APT_PRETRE)
        {
            adddynlight(d->o, 20, vec(1.5f, 1.5f, 0.0f), 300, 50, L_NOSHADOW|L_VOLUMETRIC);
            playsound(S_PRI_1, d==hudplayer() ? NULL : &d->o, NULL, 0, 0, d==hudplayer() ? 0 : 150, -1, 300);
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
            if(e.attr4 >= 0) playsound(S_TELEPORT, d==hudplayer() ? NULL : &e.o, 0, 0, 0 , 100, -1, 350);

            if(d==hudplayer()) { defformatstring(cmd, "%s %d", "screenshake", 2); int sleep = 0; loopi(16){addsleep(&sleep, cmd); sleep +=25;} }
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
            playsound(S_JUMPPAD, d==hudplayer() ? NULL : &e.o, 0, 0, 0 , 100, -1, 350);
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
                if(d->canpickupitem(ents[n]->type, d->aptitude, d->armourtype==A_ASSIST))
                {
                    addmsg(N_ITEMPICKUP, "rci", d, n);
                    ents[n]->clearspawned(); // even if someone else gets it first

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

            case RESPAWNPOINT:
                if(d!=player1 || n==respawnent) break;
                if(m_classicsp || m_tutorial)
                {
                    respawnent = n;
                    conoutf(CON_GAMEINFO, GAME_LANG ? "\f2Respawn point set!" : "\f2Point de réapparition mis à jour !");
                    //playsound(-1);
                }
                break;

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
            if((!e.spawned() || e.nopickup()) && e.type!=TELEPORT && e.type!=JUMPPAD && e.type!=RESPAWNPOINT) continue;
            float dist = e.o.dist(o);
            if(dist<(e.type==TELEPORT || e.type==I_SUPERARME ? 20 : 16)) trypickup(i, d);
        }
    }

    bool playendshrooms;

    void checkboosts(int time, gameent *d)
    {
        if(d->boostmillis[B_ROIDS] && (d->boostmillis[B_ROIDS] -= time)<=0)
        {
            d->boostmillis[B_ROIDS] = 0;
            playsound(S_ROIDS_PUPOUT, d==hudplayer() ? NULL : &d->o, 0, 0, 0 , 100, -1, 300);
            if(d==player1) conoutf(CON_GAMEINFO, GAME_LANG ? "\f8The steroid cycle is over." : "\f8La cure de stéros est terminée.");
        }

        if(d->boostmillis[B_EPO] && (d->boostmillis[B_EPO] -= time)<=0)
        {
            d->boostmillis[B_EPO] = 0;
            playsound(S_EPO_PUPOUT, d==player1 ? NULL : &d->o, 0, 0, 0 , 100, -1, 300);
            if(d==player1) conoutf(CON_GAMEINFO, GAME_LANG ? "\f8EPO no longer works..." : "\f8L'EPO ne fait plus effet.");
        }

        if(d->boostmillis[B_JOINT] && (d->boostmillis[B_JOINT] -= time)<=0)
        {
            d->boostmillis[B_JOINT] = 0;
            if(d==player1) conoutf(CON_GAMEINFO, GAME_LANG ? "\f8You smoked the joint to the filter!" : "\f8Tu as fumé le joint jusqu'au carton !");
        }

        if(d==hudplayer() && d->boostmillis[B_SHROOMS] > 4000) playendshrooms = false;
        else if(!playendshrooms && d==hudplayer() && d->boostmillis[B_SHROOMS] && d->boostmillis[B_SHROOMS] < 4000)
        {
            playsound(S_SHROOMS_PUPOUT, NULL);
            playendshrooms = true;
        }

        if(d->boostmillis[B_SHROOMS] && (d->boostmillis[B_SHROOMS] -= time)<=0)
        {
            d->boostmillis[B_SHROOMS] = 0;
            gfx::resetshroomsgfx();
            if(d==player1) conoutf(CON_GAMEINFO, GAME_LANG ? "\f8The mushrooms have been digested." : "\f8Les champignons sont digérés.");
        }
    }

    void checkaptiskill(int time, gameent *d)
    {
        if(d->ragemillis && (d->ragemillis -= time)<=0) d->ragemillis = 0;
        if(d->vampimillis && (d->vampimillis -= time)<=0) d->vampimillis = 0;
        loopi(NUMABILITIES) { if(d->abilitymillis[i] && (d->abilitymillis[i] -= time)<=0) d->abilitymillis[i] = 0; }
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
            if(m_noitems && ((e->type!=I_SUPERARME) || (e->type>I_BOOSTPV && e->type<I_BOOSTGRAVITE))) return;
            if(e->type>=I_RAIL && e->type<=I_MANA && (!m_noammo || e->type<I_RAIL || e->type>I_GLOCK))
            {
                e->setspawned(force || m_tutorial || !server::delayspawn(e->type));
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

    enum
    {
        TRIG_COLLIDE    = 1<<0,
        TRIG_TOGGLE     = 1<<1,
        TRIG_ONCE       = 0<<2,
        TRIG_MANY       = 1<<2,
        TRIG_DISAPPEAR  = 1<<3,
        TRIG_AUTO_RESET = 1<<4,
        TRIG_RUMBLE     = 1<<5,
        TRIG_LOCKED     = 1<<6,
        TRIG_LOOP       = 1<<7,
        TRIG_ENDSP      = 1<<8
    };

    static const int NUMTRIGGERTYPES = 32;

    static const int triggertypes[NUMTRIGGERTYPES] =
    {
        -1,
        TRIG_ONCE,                    // 1
        TRIG_RUMBLE,                  // 2
        TRIG_TOGGLE,                  // 3
        TRIG_TOGGLE | TRIG_RUMBLE,    // 4
        TRIG_MANY | TRIG_LOOP,        // 5
        TRIG_MANY,                    // 6
        TRIG_MANY | TRIG_TOGGLE,      // 7
        TRIG_MANY | TRIG_TOGGLE | TRIG_RUMBLE,    // 8
        TRIG_COLLIDE | TRIG_TOGGLE | TRIG_RUMBLE, // 9
        TRIG_COLLIDE | TRIG_TOGGLE | TRIG_AUTO_RESET | TRIG_RUMBLE, // 10
        TRIG_COLLIDE | TRIG_TOGGLE | TRIG_LOCKED | TRIG_RUMBLE,     // 11
        TRIG_DISAPPEAR,               // 12
        TRIG_DISAPPEAR | TRIG_RUMBLE, // 13
        TRIG_DISAPPEAR | TRIG_COLLIDE | TRIG_LOCKED, // 14
        TRIG_AUTO_RESET,              // 15
        -1 /* reserved 16 */,
        -1 /* reserved 17 */,
        -1 /* reserved 18 */,
        -1 /* reserved 19 */,
        -1 /* reserved 20 */,
        -1 /* reserved 21 */,
        -1 /* reserved 22 */,
        -1 /* reserved 23 */,
        -1 /* reserved 24 */,
        -1 /* reserved 25 */,
        -1 /* reserved 26 */,
        -1 /* reserved 27 */,
        -1 /* reserved 28 */,
        TRIG_DISAPPEAR | TRIG_RUMBLE | TRIG_ENDSP, // 29
        -1 /* reserved 30 */,
        -1 /* reserved 31 */,
    };

    #define validtrigger(type) (triggertypes[(type) & (NUMTRIGGERTYPES-1)]>=0)
    #define checktriggertype(type, flag) (triggertypes[(type) & (NUMTRIGGERTYPES-1)] & (flag))

    static inline void cleartriggerflags(extentity &e)
    {
        e.flags &= ~(EF_ANIM | EF_NOVIS | EF_NOSHADOW | EF_NOCOLLIDE);
    }

    static inline void setuptriggerflags(gameentity &e)
    {
        cleartriggerflags(e);
        e.flags = EF_ANIM;
        if(checktriggertype(e.attr2, TRIG_COLLIDE|TRIG_DISAPPEAR)) e.flags |= EF_NOSHADOW;
        if(!checktriggertype(e.attr2, TRIG_COLLIDE)) e.flags |= EF_NOCOLLIDE;
        switch(e.triggerstate)
        {
            case TRIGGERING:
                if(checktriggertype(e.attr2, TRIG_COLLIDE) && lastmillis-e.lasttrigger >= 500) e.flags |= EF_NOCOLLIDE;
                break;
            case TRIGGERED:
                if(checktriggertype(e.attr2, TRIG_COLLIDE)) e.flags |= EF_NOCOLLIDE;
                break;
            case TRIGGER_DISAPPEARED:
                e.flags |= EF_NOVIS | EF_NOCOLLIDE;
                break;
        }
    }

    void resettriggers()
    {
        loopv(ents)
        {
            gameentity &e = *(gameentity *)ents[i];
            if(e.type != TRIGGER_ZONE || !validtrigger(e.attr2)) continue;
            e.triggerstate = TRIGGER_RESET;
            e.lasttrigger = 0;
            setuptriggerflags(e);
        }
    }

    void unlocktriggers(int tag, int oldstate = TRIGGER_RESET, int newstate = TRIGGERING)
    {
        loopv(ents)
        {
            gameentity &e = *(gameentity *)ents[i];
            if(e.type != TRIGGER_ZONE || !validtrigger(e.attr2)) continue;
            if(e.attr1 == tag && e.triggerstate == oldstate && checktriggertype(e.attr2, TRIG_LOCKED))
            {
                if(newstate == TRIGGER_RESETTING && checktriggertype(e.attr2, TRIG_COLLIDE) && overlapsdynent(e.o, 20)) continue;
                e.triggerstate = newstate;
                e.lasttrigger = lastmillis;
                //if(checktriggertype(e.attr2, TRIG_RUMBLE)) playsound(-1, &e.o);
            }
        }
    }

    ICOMMAND(trigger, "ii", (int *tag, int *state),
    {
        if(*state) unlocktriggers(*tag);
        else unlocktriggers(*tag, TRIGGERED, TRIGGER_RESETTING);
    });

    VAR(triggerstate, -1, 0, 1);

    void doleveltrigger(int trigger, int state)
    {
        defformatstring(aliasname, "level_trigger_%d", trigger);
        if(identexists(aliasname))
        {
            triggerstate = state;
            execute(aliasname);
        }
    }

    void checktriggers()
    {
        if(player1->state != CS_ALIVE) return;
        vec o = player1->feetpos();
        loopv(ents)
        {
            gameentity &e = *(gameentity *)ents[i];
            if(e.type != TRIGGER_ZONE || !validtrigger(e.attr2)) continue;
            switch(e.triggerstate)
            {
                case TRIGGERING:
                    if(checktriggertype(e.attr2, TRIG_LOOP) && e.o.dist(o)-player1->radius<=(checktriggertype(e.attr2, TRIG_COLLIDE) ? 20 : e.attr3))
                    {
                        doleveltrigger(e.attr1, 1);
                        break;
                    }
                case TRIGGER_RESETTING:
                    if(lastmillis-e.lasttrigger>=500)
                    {
                        if(e.attr1)
                        {
                            if(e.triggerstate == TRIGGERING) unlocktriggers(e.attr1);
                            else unlocktriggers(e.attr1, TRIGGERED, TRIGGER_RESETTING);
                        }
                        if(checktriggertype(e.attr2, TRIG_DISAPPEAR)) e.triggerstate = TRIGGER_DISAPPEARED;
                        else if(e.triggerstate==TRIGGERING && checktriggertype(e.attr2, TRIG_TOGGLE)) e.triggerstate = TRIGGERED;
                        else e.triggerstate = TRIGGER_RESET;
                    }
                    setuptriggerflags(e);
                    break;
                case TRIGGER_RESET:
                    if(e.lasttrigger)
                    {
                        if(checktriggertype(e.attr2, TRIG_AUTO_RESET|TRIG_MANY|TRIG_LOCKED) && e.o.dist(o)-player1->radius>=(checktriggertype(e.attr2, TRIG_COLLIDE) ? 20 : e.attr3))
                            e.lasttrigger = 0;
                        if(e.o.dist(o)-player1->radius>=(checktriggertype(e.attr2, TRIG_COLLIDE) ? 20 : e.attr3))
                        {
                            defformatstring(s, "trigger_leave_%d", e.attr1);
                            execute(s);
                        }
                        break;
                    }
                    else if(e.o.dist(o)-player1->radius>=(checktriggertype(e.attr2, TRIG_COLLIDE) ? 20 : e.attr3)) break;
                    else if(checktriggertype(e.attr2, TRIG_LOCKED))
                    {
                        if(!e.attr1) break;
                        doleveltrigger(e.attr1, -1);
                        e.lasttrigger = lastmillis;
                        break;
                    }
                    e.triggerstate = TRIGGERING;
                    e.lasttrigger = lastmillis;
                    setuptriggerflags(e);
                    //if(checktriggertype(e.attr2, TRIG_RUMBLE)) playsound(-1, &e.o);
                    if(checktriggertype(e.attr2, TRIG_ENDSP)) endsp();
                    if(e.attr1) doleveltrigger(e.attr1, 1);
                    break;
                case TRIGGERED:
                    if(e.o.dist(o)-player1->radius<(checktriggertype(e.attr2, TRIG_COLLIDE) ? 20 : e.attr3))
                    {
                        if(e.lasttrigger) break;
                    }
                    else if(checktriggertype(e.attr2, TRIG_AUTO_RESET))
                    {
                        if(lastmillis-e.lasttrigger<6000) break;
                    }
                    else if(checktriggertype(e.attr2, TRIG_MANY))
                    {
                        e.lasttrigger = 0;
                        break;
                    }
                    else break;
                    if(checktriggertype(e.attr2, TRIG_COLLIDE) && overlapsdynent(e.o, 20)) break;
                    e.triggerstate = TRIGGER_RESETTING;
                    e.lasttrigger = lastmillis;
                    setuptriggerflags(e);
                    //if(checktriggertype(e.attr2, TRIG_RUMBLE)) playsound(-1, &e.o);
                    if(checktriggertype(e.attr2, TRIG_ENDSP)) endsp();
                    if(e.attr1) doleveltrigger(e.attr1, 0);
                    break;
            }
        }
    }

    void animatemapmodel(const extentity &e, int &anim, int &basetime)
    {
        const gameentity &f = (const gameentity &)e;
        if(validtrigger(f.attr2)) switch(f.triggerstate)
        {
            case TRIGGER_RESET: anim = ANIM_TRIGGER|ANIM_START; break;
            case TRIGGERING: anim = ANIM_TRIGGER; basetime = f.lasttrigger; break;
            case TRIGGERED: anim = ANIM_TRIGGER|ANIM_END; break;
            case TRIGGER_RESETTING: anim = ANIM_TRIGGER|ANIM_REVERSE; basetime = f.lasttrigger; break;
        }
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
            case RESPAWNPOINT:
                e.attr1 = (int)player1->yaw;
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
                renderentarrow(e, vec((int)(char)e.attr3*10.0f, (int)(char)e.attr2*10.0f, e.attr1*12.5f).normalize(), 16);
                break;
            case MONSTER:
            case CAMERA_POS:
            case FLAG:
            case TELEDEST:
            {
                vec dir;
                vecfromyawpitch(e.attr2, e.type==CAMERA_POS ? e.attr3 : 0, 1, 0, dir);
                renderentarrow(e, dir, e.type==CAMERA_POS ? 48 : 16);
                if(e.type==CAMERA_POS) renderentbox(e, vec(0, 0, 0), vec(92, 92, 52), e.attr2, e.attr3, e.attr4, true);
                break;
            }
            case TRIGGER_ZONE:
                if(validtrigger(e.attr2)) renderentring(e.o, e.attr3);
                break;
            default:
            {
                if(e.type==I_SUPERARME) renderentring(e.o, 20);
                else if(e.type>=I_RAIL && e.type<=I_MANA) renderentring(e.o, 16);
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
            //ent name (edit)
            "none?", "light", "mapmodel", "playerstart", "envmap", "particles", "sound", "spotlight", "decal",

            "electricrifle", "plasmarifle", "smaw", "minigun", "spockgun", "m32",
            "flamethrower", "uzi", "famas", "mossberg500", "hydra", "sv98",
            "sks", "crossbow", "ak47", "grap1", "fireworks", "glock",
            "Superweapon", "none?", "none?", "none?",

            "health", "healthboost", "roids", "shrooms", "epo", "joint",
            "woodshield", "ironshield", "goldshield", "magnetshield", "powerarmor",
            "mana",

            "teleport", "teledest", "jumppad", "flag", "base",

            "npc", "respawnpoint", "trigger", "camera",
        };
        return i>=0 && size_t(i)<sizeof(entnames)/sizeof(entnames[0]) ? entnames[i] : "";
    }

    void editent(int i, bool local)
    {
        extentity &e = *ents[i];
        if(e.type == TRIGGER_ZONE && validtrigger(e.attr2))
        {
            gameentity &f = (gameentity &)e;
            f.triggerstate = TRIGGER_RESET;
            f.lasttrigger = 0;
            setuptriggerflags(f);
        }
        else cleartriggerflags(e);
        if(local) addmsg(N_EDITENT, "rii3ii5", i, (int)(e.o.x*DMF), (int)(e.o.y*DMF), (int)(e.o.z*DMF), e.type, e.attr1, e.attr2, e.attr3, e.attr4, e.attr5);
    }

    float dropheight(entity &e)
    {
        if(e.type==FLAG) return 0.0f;
        return 4.0f;
    }
#endif
}
