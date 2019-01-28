#include "game.h"
#include "engine.h"
#include "cubedef.h"

int message1, message2, message3, message4, message5, message6, message7, message8, message9, message10, message11, message12;
int message_streak1;

string strclassetueur, straptitudevictime;

int decal_message = 0;
bool need_message1, need_message3;

namespace game
{
    void drawmessages(int killstreak, string str_pseudovictime, int n_aptitudevictime, string str_pseudoacteur, int n_killstreakacteur)
    {
        decal_message = 0;

        hudmatrix.ortho(0, screenw, screenh, 0, -1, 1);
        resethudmatrix();
        resethudshader();

        gle::defvertex(2);
        gle::deftexcoord0();

        glEnable(GL_BLEND);

        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        if(totalmillis-message1<=2500)
        {
            string streakmsg;

            switch(killstreak)
            {
                case 3: formatstring(streakmsg, "Triplette ! \fc(x%d)", killstreak); need_message1 = true; break;
                case 5: formatstring(streakmsg, "Pentaplette ! \fc(x%d)", killstreak); need_message1 = true; break;
                case 10: formatstring(streakmsg, "Décaplette ! \fc(x%d)", killstreak); need_message1 = true; break;
                case 20: formatstring(streakmsg, "Eicoplette ! \fc(x%d)", killstreak); need_message1 = true; break;
                case 30: formatstring(streakmsg, "Triaconplette ! \fc(x%d)", killstreak); need_message1 = true; break;
                default: need_message1 = false;
            }

            if(need_message1)
            {
                int tw = text_width(streakmsg);
                float tsz = 0.04f*min(screenw, screenh)/85,
                      tx = 0.5f*(screenw - tw*tsz), ty = screenh - 0.075f*8.8f*min(screenw, screenh) - 85*tsz;
                pushhudmatrix();
                hudmatrix.translate(tx, ty, 0);
                hudmatrix.scale(tsz, tsz, 1);
                flushhudmatrix();
                draw_text(streakmsg, 0, 0);
                pophudmatrix();

                decal_message -= 45;
            }
        }

        if(totalmillis-message2<=2500)
        {
            string killmsg;

            formatstring(killmsg, "Tu as tué \fc%s \f7! (%s)", str_pseudovictime, aptitudes[n_aptitudevictime].apt_nom);

            int tw = text_width(killmsg);
            float tsz = 0.04f*min(screenw, screenh)/100,
                  tx = 0.5f*(screenw - tw*tsz), ty = screenh - 0.075f*8.8f*min(screenw, screenh+decal_message) - 100*tsz;
            pushhudmatrix();
            hudmatrix.translate(tx, ty, 0);
            hudmatrix.scale(tsz, tsz, 1);
            flushhudmatrix();
            draw_text(killmsg, 0, 0);
            pophudmatrix();

            decal_message -= 40;
        }

        if(totalmillis-message3<=2500)
        {
            string infomsg;

            switch(n_killstreakacteur)
            {
                case 3: formatstring(infomsg, "\fc%s\f7 est chaud ! (Triplette)", str_pseudoacteur); need_message3 = true; break;
                case 5: formatstring(infomsg, "\fc%s\f7 domine ! (Pentaplette)", str_pseudoacteur); need_message3 = true; break;
                case 10: formatstring(infomsg, "\fc%s\f7 est inarrêtable ! (Décaplette)", str_pseudoacteur); need_message3 = true; break;
                case 20: formatstring(infomsg, "\fc%s\f7 est invincible ! (Eicoplette)", str_pseudoacteur); need_message3 = true; break;
                case 30: formatstring(infomsg, "\fc%s\f7 est un Dieu ! (Triaconplette)", str_pseudoacteur);  need_message3 = true; break;
                default: need_message3 = false;
            }

            if(need_message3)
            {
                int tw = text_width(infomsg);
                float tsz = 0.04f*min(screenw, screenh)/100,
                      tx = 0.5f*(screenw - tw*tsz), ty = screenh - 0.075f*8.8f*min(screenw, screenh+decal_message) - 100*tsz;
                pushhudmatrix();
                hudmatrix.translate(tx, ty, 0);
                hudmatrix.scale(tsz, tsz, 1);
                flushhudmatrix();
                draw_text(infomsg, 0, 0);
                pophudmatrix();

                decal_message -= 40;
            }
        }

        if(gamemillismsg < 5000)
        {
            string countdownmsg;

            formatstring(countdownmsg, "\f6Armes activées dans :%s %.1f",  gamemillismsg<1000 ? "\fe" : gamemillismsg > 3000 ? "\fc" : "\fd", (5000-gamemillismsg)/1000.0f);

            int tw = text_width(countdownmsg);
            float tsz = 0.04f*min(screenw, screenh)/85,
                  tx = 0.5f*(screenw - tw*tsz), ty = screenh - 0.075f*8.8f*min(screenw, screenh) - 85*tsz;
            pushhudmatrix();
            hudmatrix.translate(tx, ty, 0);
            hudmatrix.scale(tsz, tsz, 1);
            flushhudmatrix();
            draw_text(countdownmsg, 0, 0);
            pophudmatrix();
        }

        if(m_battle)
        {
            string remainingplayersmsg;

            formatstring(remainingplayersmsg, "\f7Joueurs vivants : %d",  battlevivants);

            int tw = text_width(remainingplayersmsg);
            float tsz = 0.04f*min(screenw, screenh)/75,
                  tx = 0.5f*(screenw - tw*tsz), ty = screenh - 0.075f*10.0f*min(screenw, screenh) - 75*tsz;
            pushhudmatrix();
            hudmatrix.translate(tx, ty, 0);
            hudmatrix.scale(tsz, tsz, 1);
            flushhudmatrix();
            draw_text(remainingplayersmsg, 0, 0);
            pophudmatrix();
        }
    }

    void gameplayhud(int w, int h)
    {
        if((player1->gunselect == GUN_SKS  || player1->gunselect == GUN_SV98 || player1->gunselect==GUN_ARBALETE || player1->gunselect==GUN_S_CAMPOUZE) && zoom == 1)
        {
            gle::colorf(crosshairalpha, crosshairalpha, crosshairalpha, crosshairalpha);

            if(player1->gunselect==GUN_S_ROQUETTES) settexture("media/interface/hud/viseurA.png");
            if(player1->gunselect==GUN_SKS) settexture("media/interface/hud/viseurC.png");
            else settexture("media/interface/hud/viseurB.png");
            bgquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(player1->ragemillis>0 && player1->state==CS_ALIVE)
        {
            if(player1->ragemillis>1000) gle::colorf(1, 1, 1, 1);
            else gle::colorf(player1->ragemillis/1000.0f, player1->ragemillis/1000.0f, player1->ragemillis/1000.0f, player1->ragemillis/1000.0f);

            settexture("media/interface/hud/ragescreen.png");
            bgquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(player1->health<500)
        {
            gle::colorf((-(player1->health)+700)/1000.0f, (-(player1->health)+700)/1000.0f, (-(player1->health)+700)/1000.0f, (-(player1->health)+700)/1000.0f);
            settexture("media/interface/hud/damage.png");
            bgquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        pushhudmatrix();
        hudmatrix.scale(h/1800.0f, h/1800.0f, 1);
        flushhudmatrix();

        if(zoom && crosshairsize >= 31) {crosshairsize -= 3; if(crosshairsize<31) crosshairsize = 31;}
        else if (crosshairsize<40) crosshairsize += 3;
        zoomfov = guns[player1->gunselect].maxzoomfov;

        if(player1->state==CS_SPECTATOR)
        {
            int pw, ph, tw, th, fw, fh;
            text_bounds("  ", pw, ph);
            text_bounds("SPECTATEUR", tw, th);
            th = max(th, ph);
            gameent *f = followingplayer();
            text_bounds(f ? colorname(f) : " ", fw, fh);
            fh = max(fh, ph);
            draw_text("SPECTATEUR", w*1800/h - tw - pw, 1550 - th - fh);
            if(f)
            {
                int color = f->state!=CS_DEAD ? 0xFFFFFF : 0x606060;
                if(f->privilege)
                {
                    color = f->privilege>=PRIV_ADMIN ? 0xFF8000 : 0x40FF80;
                    if(f->state==CS_DEAD) color = (color>>1)&0x7F7F7F;
                }
                draw_text(colorname(f), w*1800/h - fw - pw, 1550 - fh, (color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);
            }
            resethudshader();
        }

        if(player1->state==CS_DEAD)
        {
            int pw, ph, tw, th, fh;
            text_bounds("  ", pw, ph);
            text_bounds("Tué par Superpseudo12345 (Americain)", tw, th);
            th = max(th, ph);
            fh = max(fh, ph);

            draw_textf("Tué par %s (%s)", w*1800/h - tw - pw, 1650 - th - fh + 50, str_pseudotueur, aptitudes[n_aptitudetueur].apt_nom); //, strclassetueur
            draw_textf("Avec %s", w*1800/h - tw - pw, 1650 - th - fh + 100, str_armetueur);

            int wait = cmode ? cmode->respawnwait(player1) : (lastmillis < player1->lastpain + 1000) ? 1 : 0 ;
            if(wait>0) draw_textf("Respawn possible dans %d seconde%s", w*1800/h - tw - pw, 1650 - th - fh + 150, wait, wait<=1?"":"s");
            else draw_textf("Appuie n'importe où pour revivre ! ", w*1800/h - tw - pw, 1650 - th - fh + 150);
            resethudshader();
        }

        gameent *d = hudplayer();
        if(d->state!=CS_EDITING)
        {
            if(cmode) cmode->drawhud(d, w, h);
        }

        //////////////////////////////////////// RENDU DES IMAGES ////////////////////////////////////////
        int pw, ph, tw, th, fh;
        th = max(th, ph);
        fh = max(fh, ph);

        if(player1->state==CS_SPECTATOR && d==player1) {pophudmatrix(); return; }

        if(d->state==CS_DEAD)
        {
            settexture("media/interface/hud/mort.png");
            bgquad(30, 1580,  200 - tw - pw,  200 - th - ph);
            pophudmatrix();
            return;
        }

        settexture("media/interface/hud/coeur.png");
        bgquad(30, 1580,  200 - tw - pw,  200 - th - ph);

        if(d->armour)
        {
            switch(d->armourtype)
            {
                case A_BLUE: settexture("media/interface/hud/bouclier_bois.png"); break;
                case A_GREEN: settexture("media/interface/hud/bouclier_fer.png"); break;
                case A_YELLOW: settexture("media/interface/hud/bouclier_or.png"); break;
                case A_MAGNET: settexture("media/interface/hud/bouclier_magnetique.png"); break;
            }
            bgquad(350, 1580,  200 - tw - pw,  200 - th - ph);
        }

        settexture("media/interface/hud/balle.png");
        bgquad(2980, 1580,  200 - tw - pw,  200 - th - ph);

        if(m_random)
        {
            if(player1->gunselect==GUN_S_CAMPOUZE  || player1->gunselect==GUN_S_GAU8 || player1->gunselect==GUN_S_NUKE || player1->gunselect==GUN_S_ROQUETTES);
            else {settexture("media/interface/hud/inf.png"); bgquad(2820, 1580,  200 - tw - pw,  200 - th - ph);}
        }

        int dope_img = 0;
        if(player1->crouching && player1->aptitude==9)
        {
            settexture("media/interface/hud/campeur.png");
            bgquad(30, 1380,  200 - tw - pw,  200 - th - ph);
            dope_img = dope_img+2;
        }
        if(player1->steromillis)
        {
            settexture("media/interface/hud/steros.png");
            bgquad(30, 1380,  200 - tw - pw,  200 - th - ph);
            dope_img = dope_img+2;
        }
        if(player1->ragemillis)
        {
            settexture("media/interface/hud/rage.png");
            bgquad(30, 1380-dope_img*100,  200 - tw - pw,  200 - th - ph);
            dope_img = dope_img+2;
        }
        if(player1->epomillis)
        {
            settexture("media/interface/hud/epo.png");
            bgquad(30, 1380-dope_img*100,  200 - tw - pw,  200 - th - ph);
            dope_img = dope_img+2;
        }
        if(player1->champimillis)
        {
            settexture("media/interface/hud/champis.png");
            bgquad(30, 1380-dope_img*100,  200 - tw - pw,  200 - th - ph);
            dope_img = dope_img+2;
        }
        if(player1->jointmillis)
        {
            settexture("media/interface/hud/joint.png");
            bgquad(30, 1380-dope_img*100,  200 - tw - pw,  200 - th - ph);
        }

        //////////////////////////////////////// RENDU DES TEXTES ////////////////////////////////////////
        hudmatrix.scale(1*1.4f, 1*1.4f, 1);
        flushhudmatrix();


        text_bounds("100", tw, th);
        draw_textf("%d", 250 - tw - pw, 1235 - th - fh, d->health < 9 ? 1 : d->health/10);

        if(d->armour > 0)
        {
            text_bounds("100", tw, th);
            draw_textf("%d", 480 - tw - pw, 1235 - th - fh, d->armour < 9 ? 1 : d->armour/10);
        }

        text_bounds("100", tw, th);
        if(m_random)
        {
            if(player1->gunselect==GUN_S_CAMPOUZE  || player1->gunselect==GUN_S_GAU8 || player1->gunselect==GUN_S_NUKE || player1->gunselect==GUN_S_ROQUETTES) draw_textf("%d", (d->ammo[d->gunselect] > 99 ? 2138 : d->ammo[d->gunselect] > 9 ? 2168 : 2200) - tw - pw, 1235 - th - fh, d->ammo[d->gunselect]);
        }
        else draw_textf("%d", (d->ammo[d->gunselect] > 99 ? 2138 : d->ammo[d->gunselect] > 9 ? 2168 : 2200) - tw - pw, 1235 - th - fh, d->ammo[d->gunselect]);



        int dope_txt = 0;
        if(player1->steromillis)
        {
            text_bounds("00", tw, th);
            draw_textf("%d", 220 - tw - pw, 1085 - th - fh-dope_txt*70, d->steromillis/1000);
            dope_txt = dope_txt+2;
        }
        if(player1->epomillis)
        {
            text_bounds("00", tw, th);
            draw_textf("%d", 220 - tw - pw, 1085 - th - fh-dope_txt*70, d->epomillis/1000);
            dope_txt = dope_txt+2;
        }
        if(player1->champimillis)
        {
            text_bounds("00", tw, th);
            draw_textf("%d", 220 - tw - pw, 1085 - th - fh-dope_txt*70, d->champimillis/1000);
            dope_txt = dope_txt+2;
        }
        if(player1->jointmillis)
        {
            text_bounds("00", tw, th);
            draw_textf("%d", 220 - tw - pw, 1085 - th - fh-dope_txt*70, d->jointmillis/1000);
            dope_txt = dope_txt+2;
        }
        if(player1->ragemillis)
        {
            text_bounds("00", tw, th);
            draw_textf("%d", 220 - tw - pw, 1085 - th - fh-dope_txt*70, d->ragemillis/100);
            dope_txt = dope_txt+2;
        }

        if(player1->epomillis)
        {
            text_bounds("  ", tw, th);
            dope_txt = dope_txt+2;
        }

        if(player1->champimillis)
        {
            text_bounds("100", tw, th);
            dope_txt = dope_txt+2;
        }

        if(player1->jointmillis)
        {
            text_bounds("100", tw, th);

        }

        pophudmatrix();
    }
}




// Code des autres modes de jeux à remettre

    /*
        if(totalmillis-message1<=3000)
        {
            text_bounds("\f9Notre équipe a marqué un point !", wb, hb);
            int x = ((w/2)*2.35f)-wb/2, y = (h/2)*2.35f;
            draw_textf("\f9Notre équipe a marqué un point !",  x, y-385+decal_message);
            decal_message = decal_message - 50;
        }
        if(totalmillis-message2<=3000)
        {
            text_bounds("\f3L'équipe ennemie a marqué un point.", wb, hb);
            int x = ((w/2)*2.35f)-wb/2, y = (h/2)*2.35f;
            draw_textf("\f3L'équipe ennemie a marqué un point.",  x, y-385+decal_message);
            decal_message = decal_message - 50;
        }
        if(totalmillis-message3<=3000)
        {
            m_hold ? text_bounds("\f9Un allié s'est transformé en ananas !", wb, hb) : text_bounds("\f2Un allié a volé le drapeau ennemi !", wb, hb);
            int x = ((w/2)*2.35f)-wb/2, y = (h/2)*2.35f;
            m_hold ? draw_textf("\f9Un allié s'est transformé en ananas !", x, y-385+decal_message) : draw_textf("\f9Un allié a volé le drapeau ennemi !",  x, y-385+decal_message);
            decal_message = decal_message - 50;
        }
        if(totalmillis-message4<=3000)
        {
            m_hold ? text_bounds("\f3Un ennemi s'est transformé en ananas.", wb, hb) : text_bounds("\f3Un ennemi a volé notre drapeau.", wb, hb);
            int x = ((w/2)*2.35f)-wb/2, y = (h/2)*2.35f;
            m_hold ? draw_textf("\f3Un ennemi s'est transformé en ananas.", x, y-385+decal_message) : draw_textf("\f3Un ennemi a volé notre drapeau.",  x, y-385+decal_message);
            decal_message = decal_message - 50;
        }
        if(totalmillis-message6<=3000)
        {
            text_bounds("\f9Notre équipe a récupéré son drapeau !", wb, hb);
            int x = ((w/2)*2.35f)-wb/2, y = (h/2)*2.35f;
            draw_textf("\f9Notre équipe a récupéré son drapeau !", x, y-385+decal_message);
            decal_message = decal_message - 50;
        }
        if(totalmillis-message7<=3000)
        {
            text_bounds("\f3L'équipe ennemie a récupéré son drapeau.", wb, hb);
            int x = ((w/2)*2.35f)-wb/2, y = (h/2)*2.35f;
            draw_textf("\f3L'équipe ennemie a récupéré son drapeau.", x, y-385+decal_message);
            decal_message = decal_message - 50;
        }
        if(totalmillis-message8<=3000)
        {
            m_hold ? text_bounds("\f9Un allié est mort en ananas.", wb, hb) : text_bounds("\f2Un allié a perdu le drapeau ennemi.", wb, hb);
            int x = ((w/2)*2.35f)-wb/2, y = (h/2)*2.35f;
            m_hold ? draw_textf("\f9Un allié est mort en ananas.", x, y-385+decal_message) : draw_textf("\f9Un allié a perdu le drapeau ennemi.",  x, y-385+decal_message);
            decal_message = decal_message - 50;
        }
        if(totalmillis-message9<=3000)
        {
            m_hold ? text_bounds("\f3Un ennemi est mort en ananas !", wb, hb) : text_bounds("\f3Un ennemi a perdu notre drapeau !", wb, hb);
            int x = ((w/2)*2.35f)-wb/2, y = (h/2)*2.35f;
            m_hold ? draw_textf("\f3Un ennemi est mort en ananas !", x, y-385+decal_message) : draw_textf("\f3Un ennemi a perdu notre drapeau !",  x, y-385+decal_message);
            decal_message = decal_message - 50;
        }
        if(totalmillis-message10<=3000)
        {
            text_bounds("\f6ATTENTION, ALERTE NUCLÉAIRE !", wb, hb);
            int x = ((w/2)*2.35f)-wb/2, y = (h/2)*2.35f;
            draw_textf("\f6ATTENTION, ALERTE NUCLÉAIRE !", x, y-385+decal_message);
            decal_message = decal_message - 50;
        }
    }
    */
