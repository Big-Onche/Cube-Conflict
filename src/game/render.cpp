#include "game.h"
#include "engine.h"
#include "../cubeconflict/cubedef.h"

float weapposside, weapposup, maxweapposside, maxweapposup, shieldside;
float maxshieldside = -15;
float crosshairalpha = 1;

namespace game
{
    vector<gameent *> bestplayers;
    vector<int> bestteams;

    VARP(ragdoll, 0, 1, 1);
    VARP(ragdollmillis, 0, 10000, 300000);
    VARP(ragdollfade, 0, 100, 5000);
    VARP(forceplayermodels, 0, 0, 1);
    VARP(hidedead, 0, 0, 1);

    extern int playermodel;

    vector<gameent *> ragdolls;

    void saveragdoll(gameent *d)
    {
        if(!d->ragdoll || !ragdollmillis || (!ragdollfade && lastmillis > d->lastpain + ragdollmillis)) return;
        //if(!ragdollmillis || (!ragdollfade && lastmillis > d->lastpain + ragdollmillis)) return; RAGRAG
        gameent *r = new gameent(*d);
        r->lastupdate = ragdollfade && lastmillis > d->lastpain + max(ragdollmillis - ragdollfade, 0) ? lastmillis - max(ragdollmillis - ragdollfade, 0) : d->lastpain;
        r->edit = NULL;
        r->ai = NULL;
        if(d==player1) r->playermodel = playermodel;
        r->attackchan = -1;
        ragdolls.add(r);
        d->ragdoll = NULL;
    }

    void clearragdolls()
    {
        ragdolls.deletecontents();
    }

    void moveragdolls()
    {
        loopv(ragdolls)
        {
            gameent *d = ragdolls[i];
            if(lastmillis > d->lastupdate + ragdollmillis)
            {
                delete ragdolls.remove(i--);
                continue;
            }
            moveragdoll(d);
        }
    }

    static const int playercolors[] =
    {
        0xA12020,
        0xA15B28,
        0xB39D52,
        0x3E752F,
        0x3F748C,
        0x214C85,
        0xB3668C,
        0x523678,
        0xB3ADA3
    };

    static const int playercolorsazul[] =
    {
        0x27508A,
        0x3F748C,
        0x3B3B80,
        0x5364B5
    };

    static const int playercolorsrojo[] =
    {
        0xAC2C2A,
        0x992417,
        0x802438,
        0xA3435B
    };

    extern void changedplayercolor();
    VARFP(playercolor, 0, 4, sizeof(playercolors)/sizeof(playercolors[0])-1, changedplayercolor());
    VARFP(playercolorazul, 0, 0, sizeof(playercolorsazul)/sizeof(playercolorsazul[0])-1, changedplayercolor());
    VARFP(playercolorrojo, 0, 0, sizeof(playercolorsrojo)/sizeof(playercolorsrojo[0])-1, changedplayercolor());

    static const playermodelinfo playermodels[9] =
    {
        { { "smileys/hap/red", "smileys/hap", "smileys/hap/red" },                  { "hudgun", "hudgun", "hudgun" },   { "hap_red", "hap", "hap_red" }, true },
        { { "smileys/noel/red", "smileys/noel", "smileys/noel/red" },               { "hudgun", "hudgun", "hudgun" },   { "noel_red", "noel", "noel_red" }, true },
        { { "smileys/malade/red", "smileys/malade", "smileys/malade/red" },         { "hudgun", "hudgun", "hudgun" },   { "malade_red", "malade", "malade_red" }, true },
        { { "smileys/content/red", "smileys/content", "smileys/content/red" },      { "hudgun", "hudgun", "hudgun" },   { "content_red", "content", "content_red" }, true },
        { { "smileys/colere/red", "smileys/colere", "smileys/colere/red" },         { "hudgun", "hudgun", "hudgun" },   { "colere_red", "colere", "colere_red" }, true },
        { { "smileys/sournois/red", "smileys/sournois", "smileys/sournois/red" },   { "hudgun", "hudgun", "hudgun" },   { "sournois_red", "sournois", "sournois_red" }, true },
        { { "smileys/fou/red", "smileys/fou", "smileys/fou/red" },                  { "hudgun", "hudgun", "hudgun" },   { "fou_red", "fou", "fou_red" }, true },
        { { "smileys/cool/red", "smileys/cool", "smileys/cool/red" },               { "hudgun", "hudgun", "hudgun" },   { "cool_red", "cool", "cool_red" }, true },
        { { "smileys/bug/red", "smileys/bug", "smileys/bug/red" },                  { "hudgun", "hudgun", "hudgun" },   { "bug_red", "bug", "bug_red" }, true },
    };

    extern void changedplayermodel();
    VARFP(playermodel, 0, 0, sizeof(playermodels)/sizeof(playermodels[0])-1, changedplayermodel());

    int chooserandomplayermodel(int seed)
    {
        return (seed&0xFFFF)%(sizeof(playermodels)/sizeof(playermodels[0]));
    }

    const playermodelinfo *getplayermodelinfo(int n)
    {
        if(size_t(n) >= sizeof(playermodels)/sizeof(playermodels[0])) return NULL;
        return &playermodels[n];
    }

    const playermodelinfo &getplayermodelinfo(gameent *d)
    {
        const playermodelinfo *mdl = getplayermodelinfo(d==player1 || forceplayermodels ? playermodel : d->playermodel);
        if(!mdl) mdl = getplayermodelinfo(playermodel);
        return *mdl;
    }

    int getplayercolor(int team, int color)
    {
        #define GETPLAYERCOLOR(playercolors) \
            return playercolors[color%(sizeof(playercolors)/sizeof(playercolors[0]))];
        switch(team)
        {
            case 1: GETPLAYERCOLOR(playercolorsazul)
            case 2: GETPLAYERCOLOR(playercolorsrojo)
            default: GETPLAYERCOLOR(playercolors)
        }
    }

    ICOMMAND(getplayercolor, "ii", (int *color, int *team), intret(getplayercolor(*team, *color)));

    int getplayercolor(gameent *d, int team)
    {
        if(d==player1) switch(team)
        {
            case 1: return getplayercolor(1, playercolorazul);
            case 2: return getplayercolor(2, playercolorrojo);
            default: return getplayercolor(0, playercolor);
        }
        else return getplayercolor(team, (d->playercolor>>(5*team))&0x1F);
    }

    void changedplayermodel()
    {
        if(player1->clientnum < 0) player1->playermodel = playermodel;
        if(player1->ragdoll) cleanragdoll(player1);
        loopv(ragdolls)
        {
            gameent *d = ragdolls[i];
            if(!d->ragdoll) continue;
            if(!forceplayermodels)
            {
                const playermodelinfo *mdl = getplayermodelinfo(d->playermodel);
                if(mdl) continue;
            }
            cleanragdoll(d);
        }
        loopv(players)
        {
            gameent *d = players[i];
            if(d == player1 || !d->ragdoll) continue;
            if(!forceplayermodels)
            {
                const playermodelinfo *mdl = getplayermodelinfo(d->playermodel);
                if(mdl) continue;
            }
            cleanragdoll(d);
        }
    }

    void changedplayercolor()
    {
        if(player1->clientnum < 0) player1->playercolor = playercolor | (playercolorazul<<5) | (playercolorrojo<<10);
    }

    void syncplayer()
    {
        if(player1->playermodel != playermodel)
        {
            player1->playermodel = playermodel;
            addmsg(N_SWITCHMODEL, "ri", player1->playermodel);
        }

        int col = playercolor | (playercolorazul<<5) | (playercolorrojo<<10);
        if(player1->playercolor != col)
        {
            player1->playercolor = col;
            addmsg(N_SWITCHCOLOR, "ri", player1->playercolor);
        }
    }

    void preloadplayermodel()
    {
        loopi(sizeof(playermodels)/sizeof(playermodels[0]))
        {
            const playermodelinfo *mdl = getplayermodelinfo(i);
            if(!mdl) break;
            if(i != playermodel && (!multiplayer(false) || forceplayermodels)) continue;
            if(m_teammode)
            {
                loopj(MAXTEAMS) preloadmodel(mdl->model[1+j]);
            }
            else preloadmodel(mdl->model[0]);
        }
    }

    int numanims() { return NUMANIMS; }

    void findanims(const char *pattern, vector<int> &anims)
    {
        loopi(sizeof(animnames)/sizeof(animnames[0])) if(matchanim(animnames[i], pattern)) anims.add(i);
    }

    VAR(animoverride, -1, 0, NUMANIMS-1);
    VAR(testanims, 0, 0, 1);
    VAR(testpitch, -90, 0, 90);

    VARFP(player1_chapeau, 0, 0, 14, player1->customhat = player1_chapeau);
    VARFP(player1_cape, 0, 0, 14, player1->customcape = player1_cape);

    string bouclier;

    void renderplayer(gameent *d, const playermodelinfo &mdl, int color, int team, float fade, int flags = 0, bool mainpass = true)
    {
        int lastaction = d->lastaction, anim = ANIM_IDLE|ANIM_LOOP, attack = 0, delay = 0;
        //if(d->lastattack >= 0)
        //{
        //    attack = attacks[d->lastattack].anim;
        //    delay = attacks[d->lastattack].attackdelay+50;
        //}
        if(intermission && d->state!=CS_DEAD)
        {
            anim = attack = ANIM_LOSE|ANIM_LOOP;
            if(validteam(team) ? bestteams.htfind(team)>=0 : bestplayers.find(d)>=0) anim = attack = ANIM_WIN|ANIM_LOOP;
        }
        else if(d->state==CS_ALIVE && d->lasttaunt && lastmillis-d->lasttaunt<1000 && lastmillis-d->lastaction>delay)
        {
            lastaction = d->lasttaunt;
            anim = attack = ANIM_TAUNT;
            delay = 1000;
        }
        modelattach a[7];
        int ai = 0;
        if(guns[d->gunselect].vwep)
        {
            int vanim = ANIM_VWEP_IDLE|ANIM_LOOP, vtime = 0;
            if(lastaction && d->lastattack >= 0 && attacks[d->lastattack].gun==d->gunselect && lastmillis < lastaction + delay)
            {
                vanim = attacks[d->lastattack].vwepanim;
                vtime = lastaction;
            }
            a[ai++] = modelattach("tag_weapon", guns[d->gunselect].vwep, vanim, vtime);
        }

        if(mainpass && !(flags&MDL_ONLYSHADOW))
        {
            d->muzzle = vec(-1, -1, -1);
            if(guns[d->gunselect].vwep) a[ai++] = modelattach("tag_muzzle", &d->muzzle);
        }
        const char *mdlname = mdl.model[validteam(team) ? team : 0];
        float yaw = testanims && d==player1 ? 0 : d->yaw,
              pitch = testpitch && d==player1 ? testpitch : d->pitch;
        vec o = d->feetpos();
        int basetime = 0;
        if(animoverride) anim = (animoverride<0 ? ANIM_ALL : animoverride)|ANIM_LOOP;
        else if(d->state==CS_DEAD)
        {
            anim = ANIM_DYING|ANIM_NOPITCH;
            basetime = d->lastpain;
            if(ragdoll && mdl.ragdoll) anim |= ANIM_RAGDOLL;
            else if(lastmillis-basetime>1000) anim = ANIM_DEAD|ANIM_LOOP|ANIM_NOPITCH;
        }
        else if(d->state==CS_EDITING || d->state==CS_SPECTATOR) anim = ANIM_EDIT|ANIM_LOOP;
        else if(d->state==CS_LAGGED)                            anim = ANIM_LAG|ANIM_LOOP;
        else if(!intermission)
        {
            if(lastmillis-d->lastpain < 300)
            {
                anim = ANIM_PAIN;
                basetime = d->lastpain;
            }
            else if(d->lastpain < lastaction && lastmillis-lastaction < delay)
            {
                anim = attack;
                basetime = lastaction;
            }

            if(d->inwater && d->physstate<=PHYS_FALL) anim |= (((game::allowmove(d) && (d->move || d->strafe)) || d->vel.z+d->falling.z>0 ? ANIM_SWIM : ANIM_SINK)|ANIM_LOOP)<<ANIM_SECONDARY;
            else
            {
                static const int dirs[9] =
                {
                    ANIM_RUN_SE, ANIM_RUN_S, ANIM_RUN_SW,
                    ANIM_RUN_E,  0,          ANIM_RUN_W,
                    ANIM_RUN_NE, ANIM_RUN_N, ANIM_RUN_NW
                };
                int dir = dirs[(d->move+1)*3 + (d->strafe+1)];
                if(d->timeinair>100) anim |= ((dir ? dir+ANIM_JUMP_N-ANIM_RUN_N : ANIM_JUMP) | ANIM_END) << ANIM_SECONDARY;
                else if(dir && game::allowmove(d)) anim |= (dir | ANIM_LOOP) << ANIM_SECONDARY;
            }

            if(d->crouching) switch((anim>>ANIM_SECONDARY)&ANIM_INDEX)
            {
                case ANIM_IDLE: anim &= ~(ANIM_INDEX<<ANIM_SECONDARY); anim |= ANIM_CROUCH<<ANIM_SECONDARY; break;
                case ANIM_JUMP: anim &= ~(ANIM_INDEX<<ANIM_SECONDARY); anim |= ANIM_CROUCH_JUMP<<ANIM_SECONDARY; break;
                case ANIM_SWIM: anim &= ~(ANIM_INDEX<<ANIM_SECONDARY); anim |= ANIM_CROUCH_SWIM<<ANIM_SECONDARY; break;
                case ANIM_SINK: anim &= ~(ANIM_INDEX<<ANIM_SECONDARY); anim |= ANIM_CROUCH_SINK<<ANIM_SECONDARY; break;
                case 0: anim |= (ANIM_CROUCH|ANIM_LOOP)<<ANIM_SECONDARY; break;
                case ANIM_RUN_N: case ANIM_RUN_NE: case ANIM_RUN_E: case ANIM_RUN_SE: case ANIM_RUN_S: case ANIM_RUN_SW: case ANIM_RUN_W: case ANIM_RUN_NW:
                    anim += (ANIM_CROUCH_N - ANIM_RUN_N) << ANIM_SECONDARY;
                    break;
                case ANIM_JUMP_N: case ANIM_JUMP_NE: case ANIM_JUMP_E: case ANIM_JUMP_SE: case ANIM_JUMP_S: case ANIM_JUMP_SW: case ANIM_JUMP_W: case ANIM_JUMP_NW:
                    anim += (ANIM_CROUCH_JUMP_N - ANIM_JUMP_N) << ANIM_SECONDARY;
                    break;
            }

            if((anim&ANIM_INDEX)==ANIM_IDLE && (anim>>ANIM_SECONDARY)&ANIM_INDEX) anim >>= ANIM_SECONDARY;
        }
        if(!((anim>>ANIM_SECONDARY)&ANIM_INDEX)) anim |= (ANIM_IDLE|ANIM_LOOP)<<ANIM_SECONDARY;
        if(d!=player1) flags |= MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY;
        if(d->type==ENT_PLAYER) flags |= MDL_FULLBRIGHT;
        else flags |= MDL_CULL_DIST;
        if(!mainpass) flags &= ~(MDL_FULLBRIGHT | MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY | MDL_CULL_DIST);
        float trans = d->state == CS_LAGGED ? 0.5f : 1.0f;

        if(d->armour && d->state == CS_ALIVE && camera1->o.dist(d->o) <= maxmodelradiusdistance*10)
        {
            switch(d->armourtype)
            {
                case A_YELLOW:
                if(d->armour<=2000) copystring(bouclier, "worldshield/or/100");
                if(d->armour<=1600) copystring(bouclier, "worldshield/or/80");
                if(d->armour<=1200) copystring(bouclier, "worldshield/or/60");
                if(d->armour<=800)  copystring(bouclier, "worldshield/or/40");
                if(d->armour<=400)  copystring(bouclier, "worldshield/or/20");
                break;

                case A_GREEN:
                if(d->armour<=1250)  copystring(bouclier, "worldshield/fer/100");
                if(d->armour<=1000)  copystring(bouclier, "worldshield/fer/80");
                if(d->armour<=750)   copystring(bouclier, "worldshield/fer/60");
                if(d->armour<=500)   copystring(bouclier, "worldshield/fer/40");
                if(d->armour<=250)   copystring(bouclier, "worldshield/fer/20");
                break;

                case A_BLUE:
                if(d->armour<=750) copystring(bouclier, "worldshield/bois/100");
                if(d->armour<=600) copystring(bouclier, "worldshield/bois/80");
                if(d->armour<=450) copystring(bouclier, "worldshield/bois/60");
                if(d->armour<=300) copystring(bouclier, "worldshield/bois/40");
                if(d->armour<=150) copystring(bouclier, "worldshield/bois/20");
                break;

                case A_MAGNET:
                if(d->armour<=1500)  copystring(bouclier,  "worldshield/magnetique/100");
                if(d->armour<=1200)  copystring(bouclier,  "worldshield/magnetique/80");
                if(d->armour<=900)   copystring(bouclier,  "worldshield/magnetique/60");
                if(d->armour<=600)   copystring(bouclier,  "worldshield/magnetique/40");
                if(d->armour<=300)   copystring(bouclier, "worldshield/magnetique/20");
                break;

            }
            a[ai++] = modelattach("tag_shield", bouclier, ANIM_VWEP_IDLE|ANIM_LOOP, 0);
        }

        //if(d->jointmillis) a[ai++] = modelattach("tag_boost", "boosts/joint", ANIM_VWEP_IDLE|ANIM_LOOP, 0);
        //if(d->steromillis) a[ai++] = modelattach("tag_boost", "boosts/steros", ANIM_VWEP_IDLE|ANIM_LOOP, 0);
        //if(d->epomillis) a[ai++] = modelattach("tag_boost", "boosts/epo", ANIM_VWEP_IDLE|ANIM_LOOP, 0);

                    if(d->customhat>=1 && d->customhat<=14)
                    {
                        switch(d->customhat) {
                            case 1: a[ai++] = modelattach("tag_hat", "chapeaux/sombrero", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                            case 2: a[ai++] = modelattach("tag_hat", "chapeaux/lapin", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                            case 3: a[ai++] = modelattach("tag_hat", "chapeaux/aureole", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                            case 4: a[ai++] = modelattach("tag_hat", "chapeaux/cornes", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                            case 5: a[ai++] = modelattach("tag_hat", "chapeaux/joker", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                            case 6: a[ai++] = modelattach("tag_hat", "chapeaux/champignon", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                            case 7: a[ai++] = modelattach("tag_hat", "chapeaux/couronne", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                            case 8: a[ai++] = modelattach("tag_hat", "chapeaux/heaume", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                            case 9: a[ai++] = modelattach("tag_hat", "chapeaux/bandana", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                            case 10: a[ai++] = modelattach("tag_hat", "chapeaux/melon", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                            case 11: a[ai++] = modelattach("tag_hat", "chapeaux/casque", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                            case 12: a[ai++] = modelattach("tag_hat", "chapeaux/helices", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                            case 13: a[ai++] = modelattach("tag_hat", "chapeaux/aventurier", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                            case 14: a[ai++] = modelattach("tag_hat", "chapeaux/bug", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                            default : a[ai++] = modelattach("tag_hat", "chapeaux/lapin", ANIM_VWEP_IDLE|ANIM_LOOP, 0);
                        }
                    }

                    string capedir;

                    if(d->customcape>=1)
                    {
                        if(team==1 || team==0) {
                            switch(d->customcape) {
                                case 1: copystring(capedir, "capes/Cape_JVC"); break;
                                case 2: copystring(capedir, "capes/Cape_Cisla"); break;
                                case 3: copystring(capedir, "capes/Cape_Tabasco"); break;
                                case 4: copystring(capedir, "capes/Cape_CubeEngine"); break;
                                case 5: copystring(capedir, "capes/Cape_Cislattack"); break;
                                case 6: copystring(capedir, "capes/Cape_Ruinee"); break;
                                case 7: copystring(capedir, "capes/Cape_Weed"); break;
                                case 8: copystring(capedir, "capes/Cape_Diable"); break;
                                case 9: copystring(capedir, "capes/Cape_High"); break;
                                case 10: copystring(capedir, "capes/Cape_Quenelle"); break;
                                case 11: copystring(capedir, "capes/Cape_Poulet"); break;
                            }
                        }
                        if(team==2) {
                            switch(d->customcape) {
                                case 1: copystring(capedir, "capes/Cape_JVC/orange"); break;
                                case 2: copystring(capedir, "capes/Cape_Cisla/orange"); break;
                                case 3: copystring(capedir, "capes/Cape_Tabasco/orange"); break;
                                case 4: copystring(capedir, "capes/Cape_CubeEngine/orange"); break;
                                case 5: copystring(capedir, "capes/Cape_Cislattack/orange"); break;
                                case 6: copystring(capedir, "capes/Cape_Ruinee/orange"); break;
                                case 7: copystring(capedir, "capes/Cape_Weed/orange"); break;
                                case 8: copystring(capedir, "capes/Cape_Diable/orange"); break;
                                case 9: copystring(capedir, "capes/Cape_High/orange"); break;
                                case 10: copystring(capedir, "capes/Cape_Quenelle/orange"); break;
                                case 11: copystring(capedir, "capes/Cape_Poulet/orange"); break;
                            }
                        }
                    a[ai++] = modelattach("tag_hat", capedir, ANIM_VWEP_IDLE|ANIM_LOOP, 0);
                    }


        if(d->pitch>30) pitch=30;
        if(d->pitch<-30) pitch=-30;
        rendermodel(mdlname, anim, o, yaw, pitch, 0, flags, d, a[0].tag ? a : NULL, basetime, 0, fade, vec4(vec::hexcolor(color), trans));
    }

void renderplayerui(gameent *d, const playermodelinfo &mdl, int color, int team, float fade, int flags = 0, bool mainpass = true)
    {
        int lastaction = d->lastaction, anim = ANIM_IDLE|ANIM_LOOP, attack = 0, delay = 0;

        if(intermission && d->state!=CS_DEAD)
        {
            anim = attack = ANIM_LOSE|ANIM_LOOP;
            if(validteam(team) ? bestteams.htfind(team)>=0 : bestplayers.find(d)>=0) anim = attack = ANIM_WIN|ANIM_LOOP;
        }
        else if(d->state==CS_ALIVE && d->lasttaunt && lastmillis-d->lasttaunt<1000 && lastmillis-d->lastaction>delay)
        {
            lastaction = d->lasttaunt;
            anim = attack = ANIM_TAUNT;
            delay = 1000;
        }
        modelattach a[6];
        int ai = 0;
        if(guns[d->gunselect].vwep)
        {
            int vanim = ANIM_VWEP_IDLE|ANIM_LOOP, vtime = 0;
            if(lastaction && d->lastattack >= 0 && attacks[d->lastattack].gun==d->gunselect && lastmillis < lastaction + delay)
            {
                vanim = attacks[d->lastattack].vwepanim;
                vtime = lastaction;
            }
            a[ai++] = modelattach("tag_weapon", guns[d->gunselect].vwep, vanim, vtime);
        }

        if(mainpass && !(flags&MDL_ONLYSHADOW))
        {
            d->muzzle = vec(-1, -1, -1);
            if(guns[d->gunselect].vwep) a[ai++] = modelattach("tag_muzzle", &d->muzzle);
        }
        const char *mdlname = mdl.model[validteam(team) ? team : 0];
        float yaw = testanims && d==player1 ? 0 : d->yaw,
              pitch = testpitch && d==player1 ? testpitch : d->pitch;
        vec o = d->feetpos();
        int basetime = 0;

        if(!((anim>>ANIM_SECONDARY)&ANIM_INDEX)) anim |= (ANIM_IDLE|ANIM_LOOP)<<ANIM_SECONDARY;
        if(d!=player1) flags |= MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY;
        if(d->type==ENT_PLAYER) flags |= MDL_FULLBRIGHT;
        else flags |= MDL_CULL_DIST;
        if(!mainpass) flags &= ~(MDL_FULLBRIGHT | MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY | MDL_CULL_DIST);

        if(player1->customhat>=1 && player1->customhat<=14)
        {
            switch(player1->customhat) {
                case 1: a[ai++] = modelattach("tag_hat", "chapeaux/sombrero", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 2: a[ai++] = modelattach("tag_hat", "chapeaux/lapin", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 3: a[ai++] = modelattach("tag_hat", "chapeaux/aureole", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 4: a[ai++] = modelattach("tag_hat", "chapeaux/cornes", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 5: a[ai++] = modelattach("tag_hat", "chapeaux/joker", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 6: a[ai++] = modelattach("tag_hat", "chapeaux/champignon", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 7: a[ai++] = modelattach("tag_hat", "chapeaux/couronne", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 8: a[ai++] = modelattach("tag_hat", "chapeaux/heaume", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 9: a[ai++] = modelattach("tag_hat", "chapeaux/bandana", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 10: a[ai++] = modelattach("tag_hat", "chapeaux/melon", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 11: a[ai++] = modelattach("tag_hat", "chapeaux/casque", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 12: a[ai++] = modelattach("tag_hat", "chapeaux/helices", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 13: a[ai++] = modelattach("tag_hat", "chapeaux/aventurier", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 14: a[ai++] = modelattach("tag_hat", "chapeaux/bug", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                default : a[ai++] = modelattach("tag_hat", "chapeaux/lapin", ANIM_VWEP_IDLE|ANIM_LOOP, 0);
            }
        }

        if(player1->customcape>=1)
        {
            switch(player1->customcape) {
                case 1: a[ai++] = modelattach("tag_hat", "capes/Cape_JVC", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 2: a[ai++] = modelattach("tag_hat", "capes/Cape_Cisla", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 3: a[ai++] = modelattach("tag_hat", "capes/Cape_Tabasco", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 4: a[ai++] = modelattach("tag_hat", "capes/Cape_CubeEngine", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 5: a[ai++] = modelattach("tag_hat", "capes/Cape_Cislattack", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 6: a[ai++] = modelattach("tag_hat", "capes/Cape_Ruinee", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 7: a[ai++] = modelattach("tag_hat", "capes/Cape_Weed", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 8: a[ai++] = modelattach("tag_hat", "capes/Cape_Diable", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 9: a[ai++] = modelattach("tag_hat", "capes/Cape_High", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 10: a[ai++] = modelattach("tag_hat", "capes/Cape_Quenelle", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
                case 11: a[ai++] = modelattach("tag_hat", "capes/Cape_Poulet", ANIM_VWEP_IDLE|ANIM_LOOP, 0); break;
            }
        }
        switch(player1->playermodel)
        {
            case 0: mdlname = "smileys/hap"; break;
            case 1: mdlname = "smileys/noel"; break;
            case 2: mdlname = "smileys/malade"; break;
            case 3: mdlname = "smileys/content"; break;
            case 4: mdlname = "smileys/colere"; break;
            case 5: mdlname = "smileys/sournois"; break;
            case 6: mdlname = "smileys/fou"; break;
            case 7: mdlname = "smileys/cool"; break;
            case 8: mdlname = "smileys/bug"; break;
        }
        rendermodel(mdlname, anim, o.add(vec(0, 10, -5)), yaw, pitch, 0, flags, d, a[0].tag ? a : NULL, basetime, 0, fade, vec4(vec::hexcolor(color), 5));
    }


    static inline void renderplayer(gameent *d, float fade = 1, int flags = 0)
    {
        int team = m_teammode && validteam(d->team) ? d->team : 0;
        renderplayer(d, getplayermodelinfo(d), getplayercolor(d, team), team, fade, flags);
    }

    void rendergame()
    {
        ai::render();

        if(intermission)
        {
            bestteams.shrink(0);
            bestplayers.shrink(0);
            if(m_teammode) getbestteams(bestteams);
            else getbestplayers(bestplayers);
        }

        bool third = isthirdperson();
        gameent *f = followingplayer(), *exclude = third ? NULL : f;
        loopv(players)
        {
            gameent *d = players[i];
            if(d == player1 || d->state==CS_SPECTATOR || d->state==CS_SPAWNING || d->lifesequence < 0 || d == exclude || (d->state==CS_DEAD && hidedead)) continue;
            renderplayer(d);
            copystring(d->info, colorname(d));
            if(d->state!=CS_DEAD)
            {
                int team = m_teammode && validteam(d->team) ? d->team : 0;
                if(player1->state!=CS_SPECTATOR)particle_text(d->abovehead(), d->info, PART_TEXT, 1, teamtextcolor[team], 2.0f);
                //if(player1->state!=CS_SPECTATOR)particle_text(d->abovehead(), d->killstreak, PART_TEXT, 1, teamtextcolor[team], 5.0f);
                if(d->health<100) switch(rnd(d->health)) { case 1: regular_particle_splash(PART_BLOOD, 1, 9999, d->o, 0x60FFFF  , 1.0f, 50);}
                if(player1->aptitude==1 && team==1)
                {
                    if (d->health > 1000)
                    {
                        unsigned int lifeColor = (d->health > 1800) ? 800 : d->health-1000;
                        particle_meter(d->abovehead().add(vec(0,0,4)), d->health/1000.0f, PART_METER, 0.5f, (static_cast<unsigned char>((800-lifeColor)*3.18) << 8) | (static_cast<unsigned char>(lifeColor*3.18) << 0), 2.5f);
                    }
                    else if (d->health > 0)
                    {
                        particle_meter(d->abovehead().add(vec(0,0,4)), d->health/1000.0f, PART_METER, 0.5f, (static_cast<unsigned char>((1000-d->health)*2.55) << 16) | (static_cast<unsigned char>(d->health*2.55) << 8), 2.5f);
                    }
                }
                //clientinfo *ci = clients[d];
                if(d->ragemillis>0) switch(rnd(12)){case 0: particle_splash(PART_SMOKE,  1,  120, d->o, 0xFF3300, 10+rnd(5),  300, 100);}
            }
        }
        loopv(ragdolls)
        {
            gameent *d = ragdolls[i];
            float fade = 1.0f;
            if(ragdollmillis && ragdollfade)
                fade -= clamp(float(lastmillis - (d->lastupdate + max(ragdollmillis - ragdollfade, 0)))/min(ragdollmillis, ragdollfade), 0.0f, 1.0f);
            renderplayer(d, fade);
        }
        if(exclude)
            renderplayer(exclude, 1, MDL_ONLYSHADOW);
        else if(!f && (player1->state==CS_ALIVE || (player1->state==CS_EDITING && third) || (player1->state==CS_DEAD && !hidedead)))
            renderplayer(player1, 1, third ? 0 : MDL_ONLYSHADOW);
        entities::renderentities();
        renderbouncers();
        renderprojectiles();
        if(cmode) cmode->rendergame();
    }

    VARP(hudgun, 0, 1, 1);
    VARP(hudgunsway, 0, 1, 1);

    FVAR(swaystep, 1, 35.0f, 100);
    FVAR(swayside, 0, 3, 3);
    FVAR(swayup, -1, 2, 2);

    float swayfade = 0, swayspeed = 0, swaydist = 0;
    vec swaydir(0, 0, 0);

    void swayhudgun(int curtime)
    {
        gameent *d = hudplayer();
        if(d->state != CS_SPECTATOR)
        {
            if(d->physstate >= PHYS_SLOPE)
            {
                swayspeed = min(sqrtf(d->vel.x*d->vel.x + d->vel.y*d->vel.y), d->maxspeed);
                swaydist += swayspeed*curtime/1000.0f;
                swaydist = fmod(swaydist, 2*swaystep);
                swayfade = 1;
            }
            else if(swayfade > 0)
            {
                swaydist += swayspeed*swayfade*curtime/1000.0f;
                swaydist = fmod(swaydist, 2*swaystep);
                swayfade -= 0.5f*(curtime*d->maxspeed)/(swaystep*1000.0f);
            }

            float k = pow(0.7f, curtime/10.0f);
            swaydir.mul(k);
            vec vel(d->vel);
            vel.add(d->falling);
            swaydir.add(vec(vel).mul((1-k)/(15*max(vel.magnitude(), d->maxspeed))));
        }
    }

    struct hudent : dynent
    {
        hudent() { type = ENT_CAMERA; }
    } guninterp;

    void drawhudmodel(gameent *d, int anim, int basetime)
    {
        const char *file = guns[d->gunselect].file;
        if(!file) return;

        vec sway;
        vecfromyawpitch(d->yaw, 0, 0, 1, sway);
        float steps = swaydist/swaystep*M_PI;
        sway.mul((swayside)*cosf(steps));
        vec weapzoom;

        if(zoom==1) sway.z = -weapposup-2; //CubeConflict, permet de faire bouger l'arme en fonction du zoom ou non
        else
        {
            sway.z = -weapposup;
            vecfromyawpitch(d->yaw, 0, -10, weapposside, weapzoom);
        }

        sway.z += swayup*(fabs(sinf(steps)) - 1);
        sway.add(swaydir).add(d->o);
        if(!hudgunsway) sway = d->o;

        vecfromyawpitch(d->yaw, 0, 0, weapposside, weapzoom);

        const playermodelinfo &mdl = getplayermodelinfo(d);
        int team = m_teammode && validteam(d->team) ? d->team : 0,
            color = getplayercolor(d, team);
        defformatstring(gunname, "%s/%s", mdl.hudguns[team], file);
        modelattach a[2];
        d->muzzle = vec(-1, -1, -1);
        a[0] = modelattach("tag_muzzle", &d->muzzle);
        if((d->gunselect==GUN_MINIGUN || d->gunselect==GUN_LANCEFLAMMES || d->gunselect==GUN_PULSE || d->gunselect==GUN_UZI) && anim!=ANIM_GUN_MELEE)
        {
            anim |= ANIM_LOOP;
            basetime = 0;
        }

        rendermodel(gunname, anim, weapzoom.add(sway), d->yaw, d->pitch, 0, MDL_NOBATCH, NULL, a, basetime, 0, 1, vec4(vec::hexcolor(color), 1));

        if(d->muzzle.x >= 0) d->muzzle = calcavatarpos(d->muzzle, 12);

        if(player1->armour==0) return;
        string bouclier;

        vec sway2;
        vecfromyawpitch(d->yaw, 0, 0, shieldside, sway2);

        float steps2 = swaydist/swaystep*M_PI;
        sway2.mul((swayside/3.0f)*cosf(steps2));
        sway2.z += (swayup/3.0f)*(fabs(sinf(steps2)) - 1);
        if(!zoom) sway2.add(swaydir).add(d->o);

        switch(player1->armourtype)
        {
            case A_YELLOW:
            if(player1->armour<=2000)  copystring(bouclier, "hudshield/or/100");
            if(player1->armour<=1600)  copystring(bouclier, "hudshield/or/80");
            if(player1->armour<=1200)  copystring(bouclier, "hudshield/or/60");
            if(player1->armour<=800)   copystring(bouclier, "hudshield/or/40");
            if(player1->armour<=400)   copystring(bouclier, "hudshield/or/20");
            break;

            case A_GREEN:
            if(player1->armour<=1250)  copystring(bouclier, "hudshield/fer/100");
            if(player1->armour<=1000)  copystring(bouclier, "hudshield/fer/80");
            if(player1->armour<=750)   copystring(bouclier, "hudshield/fer/60");
            if(player1->armour<=500)   copystring(bouclier, "hudshield/fer/40");
            if(player1->armour<=250)   copystring(bouclier, "hudshield/fer/20");
            break;

            case A_BLUE:
            if(player1->armour<=750) copystring(bouclier, "hudshield/bois/100");
            if(player1->armour<=600) copystring(bouclier, "hudshield/bois/80");
            if(player1->armour<=450) copystring(bouclier, "hudshield/bois/60");
            if(player1->armour<=300) copystring(bouclier, "hudshield/bois/40");
            if(player1->armour<=150) copystring(bouclier, "hudshield/bois/20");
            break;

            case A_MAGNET:;
            if(player1->armour<=1500)  copystring(bouclier, "hudshield/magnetique/100");
            if(player1->armour<=1200)  copystring(bouclier, "hudshield/magnetique/80");
            if(player1->armour<=900)   copystring(bouclier, "hudshield/magnetique/60");
            if(player1->armour<=600)   copystring(bouclier, "hudshield/magnetique/40");
            if(player1->armour<=300)   copystring(bouclier, "hudshield/magnetique/20");
            break;
        }
        rendermodel(bouclier, anim, sway2, d->yaw, d->pitch, 0, MDL_NOBATCH, NULL, a, basetime, 0, 1, vec4(vec::hexcolor(color), 1));
    }

    void drawhudgun()
    {
        gameent *d = hudplayer();
        if(d->state==CS_SPECTATOR || d->state==CS_EDITING || !hudgun || editmode)
        {
            d->muzzle = player1->muzzle = vec(-1, -1, -1);
            return;
        }

        int anim = ANIM_GUN_IDLE|ANIM_LOOP, basetime = 0;
        if(d->lastaction && d->lastattack >= 0 && attacks[d->lastattack].gun==d->gunselect && lastmillis-d->lastaction<attacks[d->lastattack].attackdelay)
        {
            //if(zoom) anim = ANIM_GUN_ZOOM_SHOOT;
            anim = attacks[d->lastattack].hudanim;
            basetime = d->lastaction;
        }
        drawhudmodel(d, anim, basetime);
    }

    void renderavatar()
    {
        drawhudgun();
    }

    void renderplayerpreview(int model, int color, int team, int weap)
    {
        static gameent *previewent = NULL;
        if(!previewent)
        {
            previewent = new gameent;
            loopi(NUMGUNS) previewent->ammo[i] = 1;
        }
        float height = previewent->eyeheight + previewent->aboveeye,
              zrad = height/2;
        vec2 xyrad = vec2(previewent->xradius, previewent->yradius).max(height/4);
        previewent->o = calcmodelpreviewpos(vec(xyrad, zrad), previewent->yaw).addz(previewent->eyeheight - zrad);
        previewent->gunselect = validgun(weap) ? weap : GUN_RAIL;
        const playermodelinfo *mdlinfo = getplayermodelinfo(model);
        if(!mdlinfo) return;
        renderplayerui(previewent, *mdlinfo, getplayercolor(team, color), team, 1, 0, false);
    }

    vec hudgunorigin(int gun, const vec &from, const vec &to, gameent *d)
    {
        if(d->muzzle.x >= 0) return d->muzzle;
        vec offset(from);
        if(d!=hudplayer() || isthirdperson())
        {
            vec front, right;
            vecfromyawpitch(d->yaw, d->pitch, 1, 0, front);
            offset.add(front.mul(d->radius));
            offset.z += (d->aboveeye + d->eyeheight)*0.75f - d->eyeheight;
            vecfromyawpitch(d->yaw, 0, 0, -1, right);
            offset.add(right.mul(0.5f*d->radius));
            offset.add(front);
            return offset;
        }
        offset.add(vec(to).sub(from).normalize().mul(2));
        if(hudgun)
        {
            offset.sub(vec(camup).mul(1.0f));
            offset.add(vec(camright).mul(0.8f));
        }
        else offset.sub(vec(camup).mul(0.8f));
        return offset;
    }

    void preloadweapons()
    {
        const playermodelinfo &mdl = getplayermodelinfo(player1);
        loopi(NUMGUNS)
        {
            const char *file = guns[i].file;
            if(!file) continue;
            string fname;
            if(m_teammode)
            {
                loopj(MAXTEAMS)
                {
                    formatstring(fname, "%s/%s", mdl.hudguns[1+j], file);
                    preloadmodel(fname);
                }
            }
            else
            {
                formatstring(fname, "%s/%s", mdl.hudguns[0], file);
                preloadmodel(fname);
            }
            formatstring(fname, "worldgun/%s", file);
            preloadmodel(fname);
        }
    }

    void preloadsounds()
    {
        for(int i = S_JUMP; i <= S_DIE2; i++) preloadsound(i);
    }

    void preload()
    {
        if(hudgun) preloadweapons();
        preloadbouncers();
        preloadplayermodel();
        preloadsounds();
        entities::preloadentities();
    }

}

