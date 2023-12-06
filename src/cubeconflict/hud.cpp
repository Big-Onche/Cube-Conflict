//hud.cpp: not responsive hud

#include "gfx.h"
#include "stats.h"

namespace game
{
    // all we need to communicate with soft-coded hud
    ICOMMAND(huddead, "", (), intret(followingplayer(player1)->state==CS_DEAD));
    ICOMMAND(hudhealth, "", (), intret(followingplayer(player1)->health/10));
    ICOMMAND(hudarmour, "", (), intret(followingplayer(player1)->armour/10));
    ICOMMAND(hudarmourtype, "", (), intret(followingplayer(player1)->armourtype));
    ICOMMAND(hudinfammo, "", (),
        bool b = false;
        if((m_identique || m_random) && followingplayer(player1)->gunselect<GUN_S_NUKE) b = true;
        if((followingplayer(player1)->gunselect>=GUN_CAC349 && followingplayer(player1)->gunselect<=GUN_CACFLEAU) || followingplayer(player1)->gunselect==GUN_CACNINJA) b = true;
        intret(b);
    );
    ICOMMAND(hudcapture, "", (), intret(m_capture));
    ICOMMAND(hudctf, "", (), intret(m_ctf));
    ICOMMAND(hudammo, "", (), intret(followingplayer(player1)->ammo[followingplayer(player1)->gunselect]));
    ICOMMAND(hudmelee, "", (), intret((player1->gunselect>=GUN_CAC349 && player1->gunselect<=GUN_CACFLEAU) || player1->gunselect==GUN_CACNINJA));
    ICOMMAND(hudboost, "i", (int *id), if(*id>=0 && *id<=3) intret(followingplayer(player1)->boostmillis[*id]/1000););
    ICOMMAND(hudclass, "", (), intret(followingplayer(player1)->aptitude));

    ICOMMAND(hudability, "", (),
        switch(followingplayer(player1)->aptitude)
        {
            case APT_MAGICIEN: case APT_PHYSICIEN: case APT_PRETRE: case APT_SHOSHONE: case APT_ESPION:
                intret(followingplayer(player1)->mana);
                break;
            case APT_KAMIKAZE:
                if(followingplayer(player1)->abilitymillis[ABILITY_2]) intret((followingplayer(player1)->abilitymillis[ABILITY_2]-1500)/1000);
                break;
            case APT_VIKING:
                intret(followingplayer(player1)->boostmillis[B_RAGE]/1000);
                break;
            default:
                intret(false);
        }
    );

    ICOMMAND(hudabilitylogo, "i", (int *id),
        defformatstring(logodir, "media/interface/hud/abilities/%d_%d.png", followingplayer(player1)->aptitude, *id);
        result(logodir);
    );

    ICOMMAND(hudabilitystatus, "i", (int *id),
        if(*id>=0 && *id<=2)
        {
            string logodir;
            if(followingplayer(player1)->abilitymillis[*id]) formatstring(logodir, "media/interface/hud/checkbox_on.jpg");
            else if(!followingplayer(player1)->abilityready[*id] || followingplayer(player1)->mana < aptitudes[followingplayer(player1)->aptitude].abilities[*id].manacost) formatstring(logodir, "media/interface/hud/checkbox_off.jpg");
            else formatstring(logodir, "media/interface/hud/abilities/%d_%d.png", followingplayer(player1)->aptitude, *id);
            result(logodir);
        }
    );

    ICOMMAND(hudshowabilities, "", (),
        switch(followingplayer(player1)->aptitude)
        {
           case APT_MAGICIEN: case APT_PHYSICIEN: case APT_PRETRE: case APT_SHOSHONE: case APT_ESPION: case APT_KAMIKAZE: intret(true); break;
           default: intret(false);
        }
    );

    ICOMMAND(hudxpcount, "", (),
        defformatstring(s, "%d / %d XP (%s %d)", totalXpNeeded - (xpForNextLevel - stat[STAT_XP]), totalXpNeeded, readstr("Stat_Level"), stat[STAT_LEVEL]);
        result(s);
    );

    ICOMMAND(hudscores, "i", (int *uicoltxt),
        string s;
        if(!m_teammode) formatstring(s, "%s%d %sfrag%s", *uicoltxt ? "" : "\fd", followingplayer(player1)->frags, *uicoltxt ? "" : "\f7", followingplayer(player1)->frags>1 ? "s" : ""); // solo dm
        else if(m_ctf || m_capture) formatstring(s, "%s%d %s- %s%d", *uicoltxt ? "" : "\fd", cmode->getteamscore(followingplayer(player1)->team), *uicoltxt ? "" : "\f7", *uicoltxt ? "" : "\fc", cmode->getteamscore(followingplayer(player1)->team == 1 ? 2 : 1)); // ctf, domination mode
        else formatstring(s, "%s%d %s- %s%d", *uicoltxt ? "" : "\fd", getteamfrags(followingplayer(player1)->team), *uicoltxt ? "" : "\f7", *uicoltxt ? "" : "\fc", getteamfrags(followingplayer(player1)->team == 1 ? 2 : 1)); //  team dm
        result(s);
    );

    int colortimer;
    ICOMMAND(hudtimer, "", (),
        string s = " ";
        if(((m_timed && getclientmap()) && (maplimit >= 0 || intermission)) || m_dmsp)
        {
            int secs = m_dmsp ? gamesecs : max(maplimit-lastmillis + 999, 0)/1000;
            defformatstring(col, "\f7");
            if(secs/60<1 && secs%60<30 && !m_dmsp)
            {
                colortimer += curtime;
                if(colortimer>1000) colortimer = 0;
                if(colortimer > 500) formatstring(col, "\fc");
            }
            if(intermission) formatstring(s, readstr("Hud_End"));
            else formatstring(s, "%s%d:%02d", col, secs/60, secs%60);
        }
        result(s);
    );

    ICOMMAND(hudspecname, "", (),
        if(player1->state==CS_SPECTATOR)
        {
            string s;
            if(!followingplayer()) formatstring(s, "%s", readstr("Hud_FreeCamera"));
            else
            {
                formatstring(s, "%s", followingplayer(player1)->name);
            }
            result(s);
        }
    );

    ICOMMAND(hudfreecamera, "", (), intret(!followingplayer() && player1->state==CS_SPECTATOR); );

    int respawnwait(gameent *d, int delay = 0)
    {
        return d->respawnwait(3, delay);
    }

    ICOMMAND(hudrespawnwait, "", (),
        intret(cmode ? cmode->respawnwait(followingplayer(player1)) : respawnwait(followingplayer(player1)));
    );

    enum {MSG_INTERRACT = 0, MSG_CUSTOM, NUMMSGS}; int msgmillis[NUMMSGS];

    string custommsg;
    ICOMMAND(popNotification, "sii", (char *s, int *duration, int *sound),
    {
        msgmillis[MSG_CUSTOM] = totalmillis + *duration;
        formatstring(custommsg, "%s", s);
        if(*sound >= 0 && *sound<=NUMSNDS) playSound(*sound, NULL, 0, 0, SND_FIXEDPITCH|SND_NOTIFICATION);
    });

    string interractmsg;
    ICOMMAND(helpmsg, "s", (char *msg),
    {
        msgmillis[MSG_INTERRACT] = totalmillis;
        formatstring(interractmsg, "%s", msg);
    });

    void rendersoftmessages(int y)
    {
        if(!isconnected())
        {
            loopi(NUMMSGS) msgmillis[i] = 0;
            return;
        }

        if(totalmillis - msgmillis[MSG_INTERRACT] <= 1)
        {
            defformatstring(s, "%s", interractmsg);

            float tsz = 0.35f + hudscale/300.f,
                  tx = 0.5f*(screenw - text_width(s)*tsz);

            pushhudscale(3);
            hudmatrix.translate(tx, screenh/1.5f, 0);
            hudmatrix.scale(tsz, tsz, 1);
            flushhudmatrix();
            draw_text(s, 0, 0);
            pophudmatrix();
        }

        if(totalmillis < msgmillis[MSG_CUSTOM])
        {
            defformatstring(s, "%s", custommsg);

            float tsz = 0.35f + hudscale/300.f,
                  tx = 0.5f*(screenw - text_width(s)*tsz);

            pushhudscale(3);
            hudmatrix.translate(tx, screenh/4.f, 0);
            hudmatrix.scale(tsz, tsz, 1);
            flushhudmatrix();
            draw_text(s, 0, 0);
            pophudmatrix();
        }
    }

    void rendermessages(char *line, int y)
    {
        float tsz = 0.35f + hudscale/300.f,
              tx = 0.5f*(screenw - text_width(line)*tsz);

        pushhudscale(3);
        hudmatrix.translate(tx, screenh/4.f, 0);
        hudmatrix.scale(tsz, tsz, 1);
        flushhudmatrix();
        draw_text(line, 0, y);
        pophudmatrix();
    }

    bool enlargefov = true;

    void gameplayhud(int w, int h)
    {
        gameent *d = hudplayer();
        if(d->state==CS_EDITING || d->state==CS_SPECTATOR || d->state==CS_DEAD) return;

        gfx::zoomfov = (guns[player1->gunselect].maxzoomfov);

        if(ispaused()) {gfx::zoom = 0; return;}

        if((player1->gunselect==GUN_SKS || player1->gunselect==GUN_SV98 || player1->gunselect==GUN_ARBALETE || player1->gunselect==GUN_S_CAMPOUZE || player1->gunselect==GUN_S_ROQUETTES) && gfx::zoom)
        {
            if(player1->gunselect==GUN_S_ROQUETTES) settexture("media/interface/hud/fullscreen/scope_1.png");
            if(player1->gunselect==GUN_SKS) settexture("media/interface/hud/fullscreen/scope_3.png");
            else settexture("media/interface/hud/fullscreen/scope_2.png");
            hudquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(hudplayer()->boostmillis[B_SHROOMS])
        {
            float col = hudplayer()->boostmillis[B_SHROOMS]>5000 ? 1 : hudplayer()->boostmillis[B_SHROOMS]/5000.f;
            gle::colorf(col, col, col, col);

            settexture("media/interface/hud/fullscreen/shrooms.png");
            hudquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(hudplayer()->boostmillis[B_RAGE])
        {
            float col = hudplayer()->boostmillis[B_RAGE]>1000 ? 1 :  hudplayer()->boostmillis[B_RAGE]/1000.f;
            gle::colorf(col, col, col, col);

            settexture("media/interface/hud/fullscreen/rage.png");
            hudquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(hudplayer()->vampimillis)
        {
            float col = hudplayer()->vampimillis>1000 ? 1 : hudplayer()->vampimillis/1000.f;
            gle::colorf(col, col, col, col);

            settexture("media/interface/hud/fullscreen/vampire.png");
            hudquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(((d->abilitymillis[ABILITY_1] || d->abilitymillis[ABILITY_3]) && d->aptitude==APT_MAGICIEN) || (d->abilitymillis[ABILITY_2] && d->aptitude==APT_PHYSICIEN))
        {
            d->aptitude==APT_MAGICIEN ? gle::colorf(1, 1, 1, 0.7f) : gle::colorf(0.3, 0.6, 1, 0.7f);

            settexture("media/interface/hud/fullscreen/ability.png");
            hudquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(player1->health<500 && player1->state==CS_ALIVE)
        {
            gle::colorf((-(player1->health)+700)/1000.0f, (-(player1->health)+700)/1000.0f, (-(player1->health)+700)/1000.0f, (-(player1->health)+700)/1000.0f);
            settexture("media/interface/hud/fullscreen/damage.png");
            hudquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }
        /*
        dynent *o = intersectclosest(d->o, worldpos, d, gfx::zoom ? 40 : 25);
        if(o && o->type==ENT_PLAYER && !isteam(player1->team, ((gameent *)o)->team) && totalmillis-lastshoot<=1000 && player1->o.dist(o->o)<guns[d->gunselect].hudrange)
        {
            float health = ((gameent *)o)->health > ((gameent *)o)->maxhealth ? ((gameent *)o)->health : ((gameent *)o)->maxhealth;
            float healthbar = (((gameent *)o)->health / health);
            float armour = ((gfx::armours[((gameent *)o)->armourtype].armoursteps)*5.f) + (((gameent *)o)->aptitude==APT_SOLDAT ? (((gameent *)o)->armourtype+1)*250.f : 0);
            float armourbar = (((gameent *)o)->armour / armour);

            float lxhbarvide = 0.5f*(w - 483), lxhbarpleine = 0.5f*(w - 477);

            settexture("media/interface/hud/fondbarrestats.png", 3);
            hudquad(lxhbarpleine, h-screenh/1.57f, 477, 19);
            settexture("media/interface/hud/barresantepleine.png", 3);
            hudquad(lxhbarpleine, h-screenh/1.57f, healthbar*477.f, 19);
            settexture("media/interface/hud/barrebouclierpleine.png", 3);
            hudquad(lxhbarpleine, h-screenh/1.57f, armourbar*477.f, 19);
            settexture("media/interface/hud/barrestatsvide.png", 3);
            hudquad(lxhbarvide, h-screenh/1.57f-10, 483, 40);
        }
        */
        if(cmode)
        {
            cmode->drawhud(d, w, h);
            pophudmatrix();
        }
        else if(m_tutorial || m_dmsp) drawrpgminimap(d, w, h);

        pushhudmatrix();
    }
}
