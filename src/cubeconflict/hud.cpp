//hud.cpp: not responsive hud

#include "gfx.h"
#include "stats.h"

int hudmsg[NUMMSGS];
int n_aptitudetueur, n_aptitudevictime, n_killstreakacteur;
string str_pseudovictime, str_pseudotueur, str_armetueur, str_pseudoacteur;

namespace game
{
    // all we need to communicate with soft-coded hud
    ICOMMAND(huddead, "", (), intret(hudplayer()->state==CS_DEAD));
    ICOMMAND(hudhealth, "", (), intret(hudplayer()->health/10));
    ICOMMAND(hudarmour, "", (), intret(hudplayer()->armour/10));
    ICOMMAND(hudarmourtype, "", (), intret(hudplayer()->armourtype));
    ICOMMAND(hudinfammo, "", (),
        bool b = false;
        if((m_identique || m_random) && hudplayer()->gunselect<GUN_S_NUKE) b = true;
        if((hudplayer()->gunselect>=GUN_CAC349 && hudplayer()->gunselect<=GUN_CACFLEAU) || hudplayer()->gunselect==GUN_CACNINJA) b = true;
        intret(b);
    );
    ICOMMAND(hudcapture, "", (), intret(m_capture));
    ICOMMAND(hudctf, "", (), intret(m_ctf));
    ICOMMAND(hudammo, "", (), intret(hudplayer()->ammo[hudplayer()->gunselect]));
    ICOMMAND(hudmelee, "", (), intret((player1->gunselect>=GUN_CAC349 && player1->gunselect<=GUN_CACFLEAU) || player1->gunselect==GUN_CACNINJA));
    ICOMMAND(hudboost, "i", (int *id), if(*id>=0 && *id<=3) intret(hudplayer()->boostmillis[*id]/1000););
    ICOMMAND(hudclass, "", (), intret(hudplayer()->aptitude));

    ICOMMAND(hudability, "", (),
        switch(hudplayer()->aptitude)
        {
            case APT_MAGICIEN: case APT_PHYSICIEN: case APT_PRETRE: case APT_SHOSHONE: case APT_ESPION:
                intret(hudplayer()->mana);
                break;
            case APT_KAMIKAZE:
                if(hudplayer()->abilitymillis[ABILITY_2]) intret((hudplayer()->abilitymillis[ABILITY_2]-1500)/1000);
                break;
            case APT_VIKING:
                intret(hudplayer()->boostmillis[B_RAGE]/1000);
                break;
            default:
                intret(false);
        }
    );

    ICOMMAND(hudabilitylogo, "i", (int *id),
        defformatstring(logodir, "media/interface/hud/abilities/%d_%d.png", hudplayer()->aptitude, *id);
        result(logodir);
    );

    ICOMMAND(hudabilitystatus, "i", (int *id),
        if(*id>=0 && *id<=2)
        {
            string logodir;
            if(hudplayer()->abilitymillis[*id]) formatstring(logodir, "media/interface/hud/checkbox_on.jpg");
            else if(!hudplayer()->abilityready[*id] || hudplayer()->mana < aptitudes[hudplayer()->aptitude].abilities[*id].manacost) formatstring(logodir, "media/interface/hud/checkbox_off.jpg");
            else formatstring(logodir, "media/interface/hud/abilities/%d_%d.png", hudplayer()->aptitude, *id);
            result(logodir);
        }
    );

    ICOMMAND(hudshowabilities, "", (),
        switch(hudplayer()->aptitude)
        {
           case APT_MAGICIEN: case APT_PHYSICIEN: case APT_PRETRE: case APT_SHOSHONE: case APT_ESPION: case APT_KAMIKAZE: intret(true); break;
           default: intret(false);
        }
    );

    ICOMMAND(hudxpcount, "", (),
        defformatstring(s, "%d / %d XP (%s %d)", totalneededxp - (xpneededfornextlvl - stat[STAT_XP]), totalneededxp, GAME_LANG ? "Lvl" : "Niv", stat[STAT_LEVEL]);
        result(s);
    );

    ICOMMAND(hudscores, "i", (int *uicoltxt),
        string s;
        if(!m_teammode) formatstring(s, "%s%d %sfrag%s", *uicoltxt ? "" : "\fd", hudplayer()->frags, *uicoltxt ? "" : "\f7", hudplayer()->frags>1 ? "s" : ""); // solo dm
        else if(m_ctf || m_capture) formatstring(s, "%s%d %s- %s%d", *uicoltxt ? "" : "\fd", cmode->getteamscore(hudplayer()->team), *uicoltxt ? "" : "\f7", *uicoltxt ? "" : "\fc", cmode->getteamscore(hudplayer()->team == 1 ? 2 : 1)); // ctf, domination mode
        else formatstring(s, "%s%d %s- %s%d", *uicoltxt ? "" : "\fd", getteamfrags(hudplayer()->team), *uicoltxt ? "" : "\f7", *uicoltxt ? "" : "\fc", getteamfrags(hudplayer()->team == 1 ? 2 : 1)); //  team dm
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
            if(intermission) formatstring(s, GAME_LANG ? "\fcEND" : "\fcFINI");
            else formatstring(s, "%s%d:%02d", col, secs/60, secs%60);
        }
        result(s);
    );

    ICOMMAND(hudspecname, "", (),
        if(player1->state==CS_SPECTATOR)
        {
            string s;
            if(!followingplayer()) formatstring(s, "%s", GAME_LANG ? "Free camera" : "Caméra libre");
            else
            {
                gameent *f = followingplayer();
                formatstring(s, "%s", f->name);
            }
            result(s);
        }
    );

    string custommsg, helpmsg;
    ICOMMAND(popupmsg, "ssii", (char *msg_fr, char *msg_en, int *duration, int *sound),
    {
        formatstring(custommsg, "%s", GAME_LANG ? msg_en : msg_fr);
        hudmsg[MSG_CUSTOM] = totalmillis + *duration;
        if(sound>=0) playsound(*sound);
    });

    ICOMMAND(helpmsg, "s", (char *msg),
    {
        formatstring(helpmsg, "%s", msg);
        hudmsg[MSG_HELP]=totalmillis;
    });

    void rendermessage(string message, int textsize = 100, float pos = 8.8f, int decal = 0)
    {
        int tw = text_width(message);
        float tsz = 0.04f*min(screenw, screenh)/textsize,
              tx = 0.5f*(screenw - tw*tsz), ty = screenh - 0.075f*pos*min(screenw, screenh+decal) - textsize*tsz;
        pushhudmatrix();
        hudmatrix.translate(tx, ty, 0);
        hudmatrix.scale(tsz, tsz, 1);
        flushhudmatrix();
        draw_text(message, 0, 0);
        pophudmatrix();
    }

    void drawmessages(int killstreak, string str_pseudovictime, int n_aptitudevictime, string str_pseudoacteur, int n_killstreakacteur, float killdistance)
    {
        if(ispaused()) return;

        int decal_message = 0;
        bool need_message1 = true, need_message2 = true;

        if(totalmillis - hudmsg[MSG_PREMISSION] <= (m_dmsp ? -10000 : 10000))
        {
            string msg;
            if(m_dmsp)
            {
                formatstring(msg, GAME_LANG ? "\fcThe invasion has begun!" : "\fcL'invasion commence !");
                rendermessage(msg, 85, 8.8f, decal_message); decal_message -= screenh/23;
            }
            else if(totalmillis - hudmsg[MSG_PREMISSION] <= 6900 && premission)
            {
                formatstring(msg, GAME_LANG ? "\fcThe game is about to begin" : "\fcLa partie va commencer !");
                rendermessage(msg, 85, 8.8f, decal_message); decal_message -= screenh/23;
                formatstring(msg, GAME_LANG ? "\fdThe game mode is: %s" : "\fdLe mode de jeu est : %s", server::modename(gamemode));
                rendermessage(msg, 85, 8.8f, decal_message); decal_message -= screenh/23;
            }
            else if(premission)
            {
                formatstring(msg, GAME_LANG ? "\fd%.1f" : "\fd%.1f", (10000 - (totalmillis - hudmsg[MSG_PREMISSION]))/1000.f);
                rendermessage(msg, 60, 8.8f, decal_message); decal_message -= screenh/23;
            }
        }

        if(totalmillis - hudmsg[MSG_LEVELUP] <=2500) //////////////////////////////////////////////////////////////// LVL UP MESSAGE
        {
            string msg;
            formatstring(msg, GAME_LANG ? "\f1LEVEL UP! \fi(Lvl %d)" : "\f1NIVEAU SUPÉRIEUR ! \fi(Niveau %d)", stat[STAT_LEVEL]);
            rendermessage(msg, 85, 8.8f, decal_message); decal_message -= screenh/24;
        }

        if(totalmillis - hudmsg[MSG_ACHUNLOCKED] <=3000)//////////////////////////////////////////////////////////////// ACHIEVEMENT UNLOCKED MESSAGE
        {
            string msg;
            formatstring(msg, GAME_LANG ? "\f1ACHIEVEMENT UNLOCKED! \fi(%s)" : "\f1SUCCES DÉBLOQUÉ ! \fi(%s)", tempachname);
            rendermessage(msg, 85, 8.8f, decal_message); decal_message -= screenh/24;
        }

        if(totalmillis - hudmsg[MSG_OWNKILLSTREAK] <=2500) //////////////////////////////////////////////////////////////// PLAYER1 KILLSTREAK MESSAGE
        {
            string msg;
            switch(killstreak)
            {
                case 3: formatstring(msg, "%s \fc(x%d)", GAME_LANG ? "Good job!" : "Triplette !", killstreak); break;
                case 5: formatstring(msg, "%s \fc(x%d)", GAME_LANG ? "You're killing it!" : "Pentaplette !", killstreak); break;
                case 10: formatstring(msg, "%s \fc(x%d)", GAME_LANG ? "Unstoppable!" : "Décaplette !", killstreak); break;
                case 20: formatstring(msg, "%s \fc(x%d)", GAME_LANG ? "You're a god!" : "Eicoplette !", killstreak); break;
                case 30: formatstring(msg, "%s \fc(x%d)", GAME_LANG ? "Are you cheating?" : "Triaconplette !", killstreak); break;
                default : need_message1 = false;
            }

            if(need_message1) {rendermessage(msg, 85, 8.8f, decal_message); decal_message -= screenh/24;}
        }

        if(totalmillis < hudmsg[MSG_CUSTOM]) //////////////////////////////////////////////////////////////// CUSTOM MSG
        {
            string msg;
            formatstring(msg, custommsg);
            rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/24;
        }

        if(totalmillis - hudmsg[MSG_HELP] <= 1) //////////////////////////////////////////////////////////////// CUSTOM MSG
        {
            string msg;
            formatstring(msg, helpmsg);
            rendermessage(msg, 80, 2.75f, 0);
        }

        if(totalmillis - hudmsg[MSG_YOUKILLED] <=2500)//////////////////////////////////////////////////////////////// PLAYER 1 KILL MESSAGE
        {
            string msg;
            formatstring(msg, "%s \fc%s \f7! (%s à %.1fm)", GAME_LANG ? "You killed" : "Tu as tué", str_pseudovictime, GAME_LANG ? aptitudes[n_aptitudevictime].apt_nomEN : aptitudes[n_aptitudevictime].apt_nomFR, killdistance);
            rendermessage(msg, 100, 8.8f, decal_message);
            decal_message -= screenh/27;
        }

        if(totalmillis - hudmsg[MSG_OTHERKILLSTREAK] <=2500) //////////////////////////////////////////////////////////////// OTHER PLAYER KILLSTREAK MESSAGE
        {
            string msg;
            switch(n_killstreakacteur)
            {
                case 3: formatstring(msg, "\fc%s\f7 %s %s", str_pseudoacteur, GAME_LANG ? "is hot !" : "est chaud !", GAME_LANG ? "(Triple kill)" : "(Triplette)"); break;
                case 5: formatstring(msg, "\fc%s\f7 %s %s", str_pseudoacteur, GAME_LANG ? "dominate !" : "est chaud !", GAME_LANG ? "(Pentakill)" : "(Pentaplette)"); break;
                case 10: formatstring(msg, "\fc%s\f7 %s %s", str_pseudoacteur, GAME_LANG ? "is instoppable !" : "est chaud !", GAME_LANG ? "(x7 !)" : "(Heptaplette)"); break;
                case 20: formatstring(msg, "\fc%s\f7 %s %s", str_pseudoacteur, GAME_LANG ? "is invincible !" : "est chaud !", GAME_LANG ? "(x10 !)" : "(Décaplette)"); break;
                case 30: formatstring(msg, "\fc%s\f7 %s %s", str_pseudoacteur, GAME_LANG ? "is as god !" : "est chaud !", GAME_LANG ? "(x15 !)" : "(Pentakaidecaplette)"); break;
                default: need_message2 = false;
            }
            if(need_message2) {rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/27;}
        }

        if(m_identique) ////////////////////////////////////////////////////////////////////////////////////////////////// IDENTICAL WEAPON MESSAGE
        {
            string msg;
            if(totalmillis - hudmsg[MSG_IDENTICAL] <= 3000) {formatstring(msg, GAME_LANG ? "\fdNext weapon: \fc%s" : "\fdArme suivante : \fc%s", GAME_LANG ? itemstats[nextcnweapon].name_en : itemstats[nextcnweapon].name_fr); rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/27;}
        }

        if(m_ctf) ////////////////////////////////////////////////////////////////////////////////////////////////// CAPTURE THE FLAG MESSAGES
        {
            string msg;
            if(totalmillis - hudmsg[MSG_CTF_TEAMPOINT]      <=3000) {formatstring(msg, GAME_LANG ? "\f9We scored a point!" : "\f9Notre équipe a marqué un point !"); rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/27;}
            if(totalmillis - hudmsg[MSG_CTF_ENNEMYPOINT]    <=3000) {formatstring(msg, GAME_LANG ? "\f3The enemy team has scored a point." : "\f3L'équipe ennemie a marqué un point."); rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/27;}
            if(totalmillis - hudmsg[MSG_CTF_TEAMFLAGRECO]   <=3000) {formatstring(msg, GAME_LANG ? "\f9We recovered our flag!" : "\f9Notre équipe a récupéré son drapeau !"); rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/27;}
            if(totalmillis - hudmsg[MSG_CTF_ENNEMYFLAGRECO] <=3000) {formatstring(msg, GAME_LANG ? "\f3The enemy team has recovered their flag" : "\f3L'équipe ennemie a récupéré son drapeau."); rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/27;}
            if(totalmillis - hudmsg[MSG_CTF_TEAMSTOLE]      <=3000) {formatstring(msg, GAME_LANG ? "\f9We stole the enemy flag !" : "\f9Notre équipe a volé le drapeau ennemi !"); rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/27;}
            if(totalmillis - hudmsg[MSG_CTF_ENNEMYSTOLE]    <=3000) {formatstring(msg, GAME_LANG ? "\f3The enemy team stole our flag." : "\f3L'équipe ennemie a volé notre drapeau !"); rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/27;}
        }

        if(player1->state==CS_DEAD)///////////////////////////////////////////////////////////////////////////////// DEATH SCREEN TEXT
        {
            string killedbymsg, withmsg, waitmsg;
            if(hassuicided) formatstring(killedbymsg, GAME_LANG ? "You committed suicide !" : "Tu t'es suicidé !");
            else formatstring(killedbymsg, "%s %s (%s)", GAME_LANG ? "Killed by" : "Tué par", str_pseudotueur, GAME_LANG ? aptitudes[n_aptitudetueur].apt_nomEN : aptitudes[n_aptitudetueur].apt_nomFR);

            rendermessage(killedbymsg, 65, 1.5f, 0);
            formatstring(withmsg, "%s %s", GAME_LANG ? "With" : "Avec", str_armetueur);
            rendermessage(withmsg, 95, 1.5f, -screenh/3);

            int wait = cmode ? cmode->respawnwait(player1) : (lastmillis < player1->lastpain + 1000) ? 1 : 0 ;
            if(wait>0) formatstring(waitmsg, "%s %d second%s%s", GAME_LANG ? "Respawn available in" : "Respawn possible dans", wait, GAME_LANG ? "" : "e", wait<=1?"":"s");
            else formatstring(waitmsg, GAME_LANG ? "Press any key to respawn !" : "Appuie n'importe où pour revivre !");
            rendermessage(waitmsg, 95, 1.5f, -screenh/1.862f);
            return;
        }
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
            if(enlargefov) {gfx::champifov+=22.f/gfx::nbfps; if(gfx::champifov>hudplayer()->boostmillis[B_SHROOMS]/1500) enlargefov = false;}
            else {gfx::champifov-=22.f/gfx::nbfps; if(gfx::champifov<-hudplayer()->boostmillis[B_SHROOMS]/1500) enlargefov = true;}

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

        if(cmode)
        {
            cmode->drawhud(d, w, h);
            pophudmatrix();
        }
        else if(m_tutorial || m_dmsp) drawrpgminimap(d, w, h);

        pushhudmatrix();
    }
}
