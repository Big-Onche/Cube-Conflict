//hud.cpp: not responsive hud

#include "gfx.h"
#include "stats.h"

namespace game
{
    // all we need to communicate with soft-coded hud
    ICOMMAND(huddead, "", (), intret(followingplayer(player1)->state==CS_DEAD));
    ICOMMAND(hudhealth, "", (), intret(max(1, followingplayer(player1)->health/10)));
    ICOMMAND(hudarmour, "", (), intret(followingplayer(player1)->armour ? max(1, followingplayer(player1)->armour/10) : 0));
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
            else if(!followingplayer(player1)->abilityready[*id] || followingplayer(player1)->mana < classes[followingplayer(player1)->aptitude].abilities[*id].manacost) formatstring(logodir, "media/interface/hud/checkbox_off.jpg");
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
            if(intermission) formatstring(s, "%s", readstr("Hud_End"));
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
    ICOMMAND(popNotification, "siii", (char *s, int *duration, int *sound),
    {
        msgmillis[MSG_CUSTOM] = totalmillis + *duration;
        formatstring(custommsg, "%s", s);
        if(*sound >= 0) playSound(*sound == 0 ? S_NOTIFICATION : *sound == 1 ? S_Q_FAIL : S_ACHIEVEMENTUNLOCKED, vec(0, 0, 0), 0, 0, SND_FIXEDPITCH|SND_UI);
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


    void drawFullscreenQuad(int w, int h, const char *texDir, float alpha, float r = 1, float g = 1, float b = 1)
    {
        gle::colorf(alpha, alpha, alpha, alpha);

        settexture(texDir);
        hudquad(0, 0, w, h);

        gle::colorf(r, g, b, 1);
    }

    void gameplayhud(int w, int h)
    {
        gameent *hp = hudplayer();
        if(hp->state==CS_EDITING || hp->state==CS_SPECTATOR || hp->state==CS_DEAD) return;

        zoomfov = (guns[hp->gunselect].maxzoomfov);

        if(ispaused()) {zoom = 0; return;}

        if((hp->gunselect==GUN_SKS || hp->gunselect==GUN_SV98 || hp->gunselect==GUN_ARBALETE || hp->gunselect==GUN_S_CAMPOUZE || hp->gunselect==GUN_S_ROQUETTES) && zoom)
        {
            if(hp->gunselect==GUN_S_ROQUETTES) drawFullscreenQuad(w, h, "media/interface/hud/fullscreen/scope_1.png", zoomprogress);
            else if(hp->gunselect==GUN_SKS) drawFullscreenQuad(w, h, "media/interface/hud/fullscreen/scope_3.png", zoomprogress);
            else drawFullscreenQuad(w, h, "media/interface/hud/fullscreen/scope_2.png", zoomprogress);
        }

        if(hp->boostmillis[B_SHROOMS]) drawFullscreenQuad(w, h, "media/interface/hud/fullscreen/shrooms.png", min(1.0f, hp->boostmillis[B_SHROOMS] / 5000.f));
        if(hp->boostmillis[B_RAGE]) drawFullscreenQuad(w, h, "media/interface/hud/fullscreen/rage.png", min(1.0f, hp->boostmillis[B_RAGE] / 1000.f));
        if(hp->vampimillis) drawFullscreenQuad(w, h, "media/interface/hud/fullscreen/vampire.png", min(1.0f, hp->vampimillis / 500.f));

        if(((hp->abilitymillis[ABILITY_1] || hp->abilitymillis[ABILITY_3]) && hp->aptitude==APT_MAGICIEN) || (hp->abilitymillis[ABILITY_2] && hp->aptitude==APT_PHYSICIEN))
        {
            float r, g, b;
            if(hp->aptitude==APT_MAGICIEN) r = g = b = 1;
            else { r = 0.3 ; g = 0.6 ; b = 1; }
            drawFullscreenQuad(w, h, "media/interface/hud/fullscreen/vampire.png", 0.7, r, g, b);
        }

        if(hp->health<300 && hp->state==CS_ALIVE) drawFullscreenQuad(w, h, "media/interface/hud/fullscreen/damage.png", (- (hp->health) + 700) / 1000.0f);

        /*
        dynent *o = intersectclosest(d->o, worldpos, d, zoom ? 40 : 25);
        if(o && o->type==ENT_PLAYER && !isteam(player1->team, ((gameent *)o)->team) && totalmillis-lastshoot<=1000 && player1->o.dist(o->o)<guns[d->gunselect].hudrange)
        {
            float health = ((gameent *)o)->health > ((gameent *)o)->maxhealth ? ((gameent *)o)->health : ((gameent *)o)->maxhealth;
            float healthbar = (((gameent *)o)->health / health);
            float armour = ((armours[((gameent *)o)->armourtype].armoursteps)*5.f) + (((gameent *)o)->aptitude==APT_SOLDAT ? (((gameent *)o)->armourtype+1)*250.f : 0);
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
            cmode->drawhud(hp, w, h);
            pophudmatrix();
        }
        else if(m_tutorial || m_dmsp) drawrpgminimap(hp, w, h);

        pushhudmatrix();
    }
}
