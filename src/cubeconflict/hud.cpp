//hud.cpp: not responsive hud

#include "game.h"
#include "ccheader.h"
#include "stats.h"

int message[NUMMESSAGE];

string custommsg, helpmsg;
ICOMMAND(popupmsg, "si", (char *msg, int *duration),
{
    formatstring(custommsg, "%s", msg);
    message[MSG_CUSTOM] = totalmillis + *duration;
    playsound(S_NOTIFICATION);
});

ICOMMAND(helpmsg, "s", (char *msg),
{
    formatstring(helpmsg, "%s", msg);
    message[MSG_HELP]=totalmillis;
});

namespace game
{
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

        if(totalmillis - message[MSG_PREMISSION] <= 10000)
        {
            string msg;
            if(totalmillis - message[MSG_PREMISSION] <= 6900)
            {
                formatstring(msg, GAME_LANG ? "\fcThe game is about to begin" : "\fcLa partie va commencer !");
                rendermessage(msg, 85, 8.8f, decal_message); decal_message -= screenh/23;
                formatstring(msg, GAME_LANG ? "\fdThe game mode is: %s" : "\fdLe mode de jeu est : %s", server::modeprettyname(gamemode));
                rendermessage(msg, 85, 8.8f, decal_message); decal_message -= screenh/23;
            }
            else
            {
                formatstring(msg, GAME_LANG ? "\fd%.1f" : "\fd%.1f", (10000 - (totalmillis - message[MSG_PREMISSION]))/1000.f);
                rendermessage(msg, 60, 8.8f, decal_message); decal_message -= screenh/23;
            }

        }

        if(totalmillis - message[MSG_LEVELUP] <=2500) //////////////////////////////////////////////////////////////// LVL UP MESSAGE
        {
            string msg;
            formatstring(msg, GAME_LANG ? "\f1LEVEL UP! \fi(Lvl %d)" : "\f1NIVEAU SUPÉRIEUR ! \fi(Niveau %d)", stat[STAT_LEVEL]);
            rendermessage(msg, 85, 8.8f, decal_message); decal_message -= screenh/24;
        }

        if(totalmillis - message[MSG_ACHUNLOCKED] <=3000)//////////////////////////////////////////////////////////////// ACHIEVEMENT UNLOCKED MESSAGE
        {
            string msg;
            formatstring(msg, GAME_LANG ? "\f1ACHIEVEMENT UNLOCKED! \fi(%s)" : "\f1SUCCES DÉBLOQUÉ ! \fi(%s)", tempachname);
            rendermessage(msg, 85, 8.8f, decal_message); decal_message -= screenh/24;
        }

        if(totalmillis - message[MSG_OWNKILLSTREAK] <=2500) //////////////////////////////////////////////////////////////// PLAYER1 KILLSTREAK MESSAGE
        {
            string msg;
            switch(killstreak)
            {
                case 3: formatstring(msg, "%s \fc(x%d)", GAME_LANG ? "You are hot !" : "Triplette !", killstreak); break;
                case 5: formatstring(msg, "%s \fc(x%d)", GAME_LANG ? "You are on fire !" : "Pentaplette !", killstreak); break;
                case 10: formatstring(msg, "%s \fc(x%d)", GAME_LANG ? "You are unstoppable !" : "Décaplette !", killstreak); break;
                case 20: formatstring(msg, "%s \fc(x%d)", GAME_LANG ? "You are a god !" : "Eicoplette !", killstreak); break;
                case 30: formatstring(msg, "%s \fc(x%d)", GAME_LANG ? "Are you cheating ?" : "Triaconplette !", killstreak); break;
                default : need_message1 = false;
            }

            if(need_message1) {rendermessage(msg, 85, 8.8f, decal_message); decal_message -= screenh/24;}
        }

        if(totalmillis < message[MSG_CUSTOM]) //////////////////////////////////////////////////////////////// CUSTOM MSG
        {
            string msg;
            formatstring(msg, custommsg);
            rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/24;
        }

        if(totalmillis - message[MSG_HELP] <= 1) //////////////////////////////////////////////////////////////// CUSTOM MSG
        {
            string msg;
            formatstring(msg, helpmsg);
            rendermessage(msg, 80, 2.75f, 0);
        }

        if(totalmillis - message[MSG_YOUKILLED] <=2500)//////////////////////////////////////////////////////////////// PLAYER 1 KILL MESSAGE
        {
            string msg;
            formatstring(msg, "%s \fc%s \f7! (%s à %.1fm)", GAME_LANG ? "You killed" : "Tu as tué",str_pseudovictime, GAME_LANG ? aptitudes[n_aptitudevictime].apt_nomEN : aptitudes[n_aptitudevictime].apt_nomFR, killdistance);
            rendermessage(msg, 100, 8.8f, decal_message);
            decal_message -= screenh/27;
        }

        if(totalmillis - message[MSG_OTHERKILLSTREAK] <=2500) //////////////////////////////////////////////////////////////// OTHER PLAYER KILLSTREAK MESSAGE
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

        if(m_ctf) ////////////////////////////////////////////////////////////////////////////////////////////////// CAPTURE THE FLAG MESSAGES
        {
            string msg;
            if(totalmillis - message[MSG_CTF_TEAMPOINT]      <=3000) {formatstring(msg, GAME_LANG ? "\f9We scored a point!" : "\f9Notre équipe a marqué un point !"); rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/27;}
            if(totalmillis - message[MSG_CTF_ENNEMYPOINT]    <=3000) {formatstring(msg, GAME_LANG ? "\f3The enemy team has scored a point." : "\f3L'équipe ennemie a marqué un point."); rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/27;}
            if(totalmillis - message[MSG_CTF_TEAMFLAGRECO]   <=3000) {formatstring(msg, GAME_LANG ? "\f9We recovered our flag!" : "\f9Notre équipe a récupéré son drapeau !"); rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/27;}
            if(totalmillis - message[MSG_CTF_ENNEMYFLAGRECO] <=3000) {formatstring(msg, GAME_LANG ? "\f3The enemy team has recovered their flag" : "\f3L'équipe ennemie a récupéré son drapeau."); rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/27;}
            if(totalmillis - message[MSG_CTF_TEAMSTOLE]      <=3000) {formatstring(msg, GAME_LANG ? "\f9We stole the enemy flag !" : "\f9Notre équipe a volé le drapeau ennemi !"); rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/27;}
            if(totalmillis - message[MSG_CTF_ENNEMYSTOLE]    <=3000) {formatstring(msg, GAME_LANG ? "\f3The enemy team stole our flag." : "\f3L'équipe ennemie a volé notre drapeau !"); rendermessage(msg, 100, 8.8f, decal_message); decal_message -= screenh/27;}
        }

        if(player1->state==CS_DEAD)///////////////////////////////////////////////////////////////////////////////// DEATH SCREEN TEXT
        {
            string killedbymsg, withmsg, waitmsg;
            if(suicided) formatstring(killedbymsg, GAME_LANG ? "You committed suicide !" : "Tu t'es suicidé !");
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

        if(player1->state==CS_SPECTATOR) //////////////////////////////////////////////////////////////// SPECTATOR SCREEN TEXT
        {
            string spectatormsg, color;

            gameent *f = followingplayer();

            if(f)
            {
                f->state!=CS_DEAD ? formatstring(color, "\f7") : formatstring(color, "\fg") ;

                if(f->privilege)
                {
                    f->privilege>=PRIV_ADMIN ? formatstring(color, "\fc") : formatstring(color, "\f7");
                    if(f->state==CS_DEAD) formatstring(color, "\fg") ;
                }
                formatstring(spectatormsg, GAME_LANG ? "Spectator : %s%s (%s)" : "Spectateur : %s%s (%s)", color, colorname(f), GAME_LANG ? aptitudes[f->aptitude].apt_nomEN : aptitudes[f->aptitude].apt_nomFR);
            }
            else
            {
                formatstring(spectatormsg, GAME_LANG ? "Spectator: Free Camera" : "Spectateur : Caméra libre");
            }
            rendermessage(spectatormsg, 75, 1.0f);
        }
    }

    int colortimer = 0;
    bool enlargefov = true;

    void gameplayhud(int w, int h)
    {
        gameent *d = hudplayer();
        if(d->state==CS_EDITING || d->state==CS_SPECTATOR) return;
        else if(cmode)
        {
            cmode->drawhud(d, w, h);
            pophudmatrix();
        }
        else if(m_tutorial) drawcampaignmap(d, w, h);

        zoomfov = (guns[player1->gunselect].maxzoomfov);

        if((player1->gunselect==GUN_SKS || player1->gunselect==GUN_SV98 || player1->gunselect==GUN_ARBALETE || player1->gunselect==GUN_S_CAMPOUZE || player1->gunselect==GUN_S_ROQUETTES) && zoom == 1)
        {
            if(player1->gunselect==GUN_S_ROQUETTES) settexture("media/interface/hud/fullscreen/scope_1.png");
            if(player1->gunselect==GUN_SKS) settexture("media/interface/hud/fullscreen/scope_3.png");
            else settexture("media/interface/hud/fullscreen/scope_2.png");
            bgquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(player1->champimillis>0)
        {
            if(player1->champimillis>5000) gle::colorf(1, 1, 1, 1);
            else gle::colorf(player1->champimillis/5000.0f, player1->champimillis/5000.0f, player1->champimillis/5000.0f, player1->champimillis/5000.0f);

            if(enlargefov && !ispaused()){champifov+=22.f/nbfps; if(champifov>player1->champimillis/1500) enlargefov = false;}
            else if (!ispaused()) {champifov-=22.f/nbfps; if(champifov<-player1->champimillis/1500) enlargefov = true;}

            settexture("media/interface/hud/fullscreen/shrooms.png");
            bgquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(ispaused()) return;

        if(player1->ragemillis>0)
        {
            if(player1->ragemillis>1000) gle::colorf(1, 1, 1, 1);
            else gle::colorf(player1->ragemillis/1000.0f, player1->ragemillis/1000.0f, player1->ragemillis/1000.0f, player1->ragemillis/1000.0f);

            settexture("media/interface/hud/fullscreen/rage.png");
            bgquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(player1->vampimillis>0)
        {
            if(player1->vampimillis>1000) gle::colorf(1, 1, 1, 1);
            else gle::colorf(player1->vampimillis/1000.0f, player1->vampimillis/1000.0f, player1->vampimillis/1000.0f, player1->vampimillis/1000.0f);

            settexture("media/interface/hud/fullscreen/vampire.png");
            bgquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(((d->aptisort3 || d->aptisort1) && d->aptitude==APT_MAGICIEN) || (d->aptisort2 && d->aptitude==APT_PHYSICIEN))
        {
            d->aptitude==APT_MAGICIEN ? gle::colorf(1, 1, 1, 0.7f) : gle::colorf(0.3, 0.6, 1, 0.7f);

            settexture("media/interface/hud/fullscreen/ability.png");
            bgquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(player1->health<500 && player1->state==CS_ALIVE)
        {
            gle::colorf((-(player1->health)+700)/1000.0f, (-(player1->health)+700)/1000.0f, (-(player1->health)+700)/1000.0f, (-(player1->health)+700)/1000.0f);
            settexture("media/interface/hud/fullscreen/damage.png");
            bgquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        pushhudmatrix();

        //////////////////////////////////////////////////////////////// RENDU DES IMAGES ////////////////////////////////////////////////////////////////

        if(d->state==CS_DEAD || player1->state==CS_SPECTATOR)
        {
            if(d->state==CS_DEAD)
            {
                settexture("media/interface/hud/mort.png");
                bgquad(15, h-130, 115, 115);
            }
            pophudmatrix();
            return;
        }

        if((player1->gunselect>=GUN_CAC349 && player1->gunselect<=GUN_CACFLEAU) || player1->gunselect==GUN_CACNINJA) settexture("media/interface/hud/epee.png");
        else settexture("media/interface/hud/balle.png");
        bgquad(w-130, h-130, 115, 115);

        settexture("media/interface/hud/coeur.png");
        bgquad(15, h-130, 115, 115);

        if(d->armour)
        {
            switch(d->armourtype)
            {
                case A_BLUE: settexture("media/interface/hud/bouclier_bois.png"); break;
                case A_GREEN: settexture("media/interface/hud/bouclier_fer.png"); break;
                case A_YELLOW: settexture("media/interface/hud/bouclier_or.png"); break;
                case A_MAGNET: settexture("media/interface/hud/bouclier_magnetique.png"); break;
                case A_ASSIST: settexture("media/interface/hud/robot.png"); break;
            }
            bgquad(250, h-130, 115, 115);
        }

        int decal_icon = 0;

        if(player1->crouching && player1->aptitude==APT_CAMPEUR)
        {
            settexture("media/interface/hud/campeur.png");
            bgquad(15, h-260, 115, 115);
            decal_icon += 130;
        }

        if(player1->aptitude==APT_MAGICIEN || player1->aptitude==APT_PHYSICIEN || player1->aptitude==APT_PRETRE || player1->aptitude==APT_SHOSHONE || player1->aptitude==APT_ESPION || player1->aptitude==APT_KAMIKAZE)
        {
            if(player1->aptisort2 && player1->aptitude==APT_KAMIKAZE && player1->ammo[GUN_KAMIKAZE]>0)
            {
                settexture("media/interface/hud/chrono.png");
                bgquad(15, h-260, 115, 115);
                decal_icon += 130;
            }
            else if(player1->aptitude!=APT_KAMIKAZE)
            {
                settexture("media/interface/hud/mana.png");
                bgquad(15, h-260, 115, 115);
                decal_icon += 130;
            }

            float positionsorts = 0.5f*(w - 100);

            defformatstring(logodir, "media/interface/hud/abilities/%s_1.png", aptitudes[player1->aptitude].apt_nomEN);

            if(player1->aptitude!=APT_KAMIKAZE)
            {
                if(d->aptisort1) gle::colorf(2, 2, 2, 1);
                else if(d->mana<sorts[abilitydata(d->aptitude)].mana1 || !d->canability1) gle::colorf(0.2, 0.2, 0.2, 1);
                else gle::colorf(1, 1, 1, 1);
                settexture(logodir, 3);
                bgquad(positionsorts-85, h-114, 100, 100);
                gle::colorf(1, 1, 1, 1);

                if(d->aptisort3) gle::colorf(2, 2, 2, 1);
                else if(d->mana<sorts[abilitydata(d->aptitude)].mana3 || !d->canability3) gle::colorf(0.2, 0.2, 0.2, 1);
                else gle::colorf(1, 1, 1, 1);
                formatstring(logodir, "media/interface/hud/abilities/%s_3.png", aptitudes[player1->aptitude].apt_nomEN);
                settexture(logodir, 3);
                bgquad(positionsorts+85, h-114, 100, 100);
                gle::colorf(1, 1, 1, 1);
            }

            if(d->aptisort2) gle::colorf(2, 2, 2, 1);
            else if(d->mana<sorts[abilitydata(d->aptitude)].mana2 || !d->canability2) gle::colorf(0.2, 0.2, 0.2, 1);
            else gle::colorf(1, 1, 1, 1);
            formatstring(logodir, "media/interface/hud/abilities/%s_2.png", aptitudes[player1->aptitude].apt_nomEN);
            settexture(logodir, 3);
            bgquad(positionsorts, h-114, 100, 100);
            gle::colorf(1, 1, 1, 1);
        }

        if(player1->ragemillis){settexture("media/interface/hud/rage.png"); bgquad(15, h-260, 115, 115); decal_icon += 130;}
        if(player1->steromillis){settexture("media/interface/hud/steros.png"); bgquad(15, h-260-decal_icon, 115, 115); decal_icon += 130;}
        if(player1->epomillis){settexture("media/interface/hud/epo.png"); bgquad(15, h-260-decal_icon, 115, 115); decal_icon += 130;}
        if(player1->champimillis){settexture("media/interface/hud/champis.png"); bgquad(15, h-260-decal_icon, 115, 115); decal_icon += 130;}
        if(player1->jointmillis){settexture("media/interface/hud/joint.png"); bgquad(15, h-260-decal_icon, 115, 115);}

        float lxbarvide = 0.5f*(w - 966), lxbarpleine = 0.5f*(w - 954);

        settexture("media/interface/hud/fondbarrexp.png", 3);
        bgquad(lxbarpleine, h-19, 954, 19);

        settexture("media/interface/hud/barrexppleine.png", 3);
        bgquad(lxbarpleine, h-19, (pourcents + 1)*954.0f, 19);

        settexture("media/interface/hud/barrexpvide.png", 3);
        bgquad(lxbarvide, h-29, 966, 40);

        dynent *o = intersectclosest(d->o, worldpos, d, zoom ? 40 : 25);
        if(o && o->type==ENT_PLAYER && !isteam(player1->team, ((gameent *)o)->team) && totalmillis-lastshoot<=1000 && player1->o.dist(o->o)<guns[d->gunselect].hudrange)
        {
            float pour1 = ((gameent *)o)->health, pour2 = ((gameent *)o)->health > ((gameent *)o)->maxhealth ? ((gameent *)o)->health : ((gameent *)o)->maxhealth;
            float pourcents2 = (pour1/pour2);
            float pour3 = ((gameent *)o)->armour, pour4 = ((gameent *)o)->armourtype == A_BLUE ? 750.f : ((gameent *)o)->armourtype == A_GREEN ? 1250.f : ((gameent *)o)->armourtype == A_MAGNET ? 1500.f : ((gameent *)o)->armourtype == A_YELLOW ? 2000.f : 3000.f;
            float pourcents3 = (pour3/pour4);

            float lxhbarvide = 0.5f*(w - 483), lxhbarpleine = 0.5f*(w - 477);

            settexture("media/interface/hud/fondbarrestats.png", 3);
            bgquad(lxhbarpleine, h-screenh/1.57f, 477, 19);
            settexture("media/interface/hud/barresantepleine.png", 3);
            bgquad(lxhbarpleine, h-screenh/1.57f, pourcents2*477.0f, 19);
            settexture("media/interface/hud/barrebouclierpleine.png", 3);
            bgquad(lxhbarpleine, h-screenh/1.57f, pourcents3*477.0f, 19);
            settexture("media/interface/hud/barrestatsvide.png", 3);
            bgquad(lxhbarvide, h-screenh/1.57f-10, 483, 40);
        }

        float hudscores = 0.5f*(w - 320);
        settexture("media/interface/hud/scores.png", 3);
        bgquad(hudscores, h-screenh, 320, 100);

        if(m_ctf)
        {
            settexture("media/interface/hud/scores_ctf.png", 3);
            bgquad(hudscores, h-screenh, 320, 100);
        }

        //////////////////////////////////////////////////////////////// RENDU DES NOMBRES ////////////////////////////////////////////////////////////////

        int decal_number = 0;

        switch(player1->gunselect)
        {
            case GUN_S_CAMPOUZE: case GUN_S_GAU8: case GUN_S_NUKE: case GUN_S_ROQUETTES: draw_textf("%d", (d->ammo[d->gunselect] > 99 ? w-227 : d->ammo[d->gunselect] > 9 ? w-196 : w-166), h-103, d->ammo[d->gunselect]); break;
            default:
            {
                if(m_muninfinie)
                {
                    settexture("media/interface/hud/inf.png"); bgquad(w-227, h-130, 115, 115);
                }
                else draw_textf("%d", (d->ammo[d->gunselect] > 99 ? w-227 : d->ammo[d->gunselect] > 9 ? w-196 : w-166), h-103, d->ammo[d->gunselect]);
            }
        }

        draw_textf("%d", 135, h-103, d->health < 9 ? 1 : d->health/10);
        if(d->armour > 0) draw_textf("%d", 370, h-103, d->armour < 9 ? 1 : d->armour/10);


        if(player1->aptitude==APT_MAGICIEN || player1->aptitude==APT_PHYSICIEN || player1->aptitude==APT_PRETRE || player1->aptitude==APT_SHOSHONE || player1->aptitude==APT_ESPION || (player1->aptitude==APT_KAMIKAZE && player1->aptisort2>0 && player1->ammo[GUN_KAMIKAZE]>0))
           {draw_textf("%d", 135, h-233-decal_number, player1->aptitude==APT_KAMIKAZE ? (player1->aptisort2-1500)/1000 : player1->mana); decal_number +=130;}

        if(player1->crouching && player1->aptitude==9) decal_number +=130;
        if(player1->ragemillis) {draw_textf("%d", 135, h-233-decal_number, d->ragemillis/1000); decal_number +=130;}
        if(player1->steromillis) {draw_textf("%d", 135, h-233-decal_number, d->steromillis/1000); decal_number +=130;}
        if(player1->epomillis) {draw_textf("%d", 135, h-233-decal_number, d->epomillis/1000); decal_number +=130;}
        if(player1->champimillis) {draw_textf("%d", 135, h-233-decal_number, d->champimillis/1000); decal_number +=130;}
        if(player1->jointmillis) {draw_textf("%d", 135, h-233-decal_number, d->jointmillis/1000); decal_number +=130;}

        defformatstring(infobarrexp, "%d/%d XP - LVL %d", totalneededxp - (xpneededfornextlvl - stat[STAT_XP]), totalneededxp, stat[STAT_LEVEL]);
        int tw = text_width(infobarrexp);
        float tsz = 0.4f,
              tx = 0.5f*(w - tw*tsz), ty = h - 22;
        pushhudmatrix();
        hudmatrix.translate(tx, ty, 0);
        hudmatrix.scale(tsz, tsz, 1);
        flushhudmatrix();
        draw_text(infobarrexp, 0, 0);
        pophudmatrix();

        string infoscores;
        if(m_ctf || m_capture) {formatstring(infoscores, "\fd%d \f7- \fc%d", cmode->getteamscore(hudplayer()->team), cmode->getteamscore(hudplayer()->team == 1 ? 2 : 1));}
        else if (!m_teammode) {formatstring(infoscores, GAME_LANG ? "\fd%d \f7frags" : "\fd%d \f7éliminations", hudplayer()->frags);}
        else {formatstring(infoscores, "\fd%d \f7- \fc%d", getteamfrags(hudplayer()->team), getteamfrags(hudplayer()->team == 1 ? 2 : 1));}

        int tw2 = text_width(infoscores);
        float tsz2 = 0.6f,
              tx2 = 0.5f*(w - tw2*tsz2);
        pushhudmatrix();
        hudmatrix.translate(tx2, 0, 0);
        hudmatrix.scale(tsz2, tsz2, 1);
        flushhudmatrix();
        draw_text(infoscores, 0, +30);
        pophudmatrix();

        if(m_timed && getclientmap() && (maplimit >= 0 || intermission))
        {
            string infotimer, color;
            if(intermission) formatstring(infotimer, GAME_LANG ? "\fcEND" : "\fcFINI");
            else
            {
                int secs = max(maplimit-lastmillis + 999, 0)/1000;

                if(secs/60<1 && secs%60<30)
                {
                    colortimer += curtime;
                    if(colortimer>1000) colortimer = 0;
                    formatstring(color, colortimer > 500 ? "\fc" : "\f7");
                }
                else formatstring(color, "\f7");
                formatstring(infotimer, "%s%d:%02d", color, secs/60, secs%60);
            }

            int tw3 = text_width(infotimer);
            float tsz3 = 0.4f,
                  tx3 = 0.5f*(w - tw3*tsz3);
            pushhudmatrix();
            hudmatrix.translate(tx3, 0, 0);
            hudmatrix.scale(tsz3, tsz3, 1);
            flushhudmatrix();
            draw_text(infotimer, 0, +164);

            pophudmatrix();
        }
    }
}
