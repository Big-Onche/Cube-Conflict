#include "game.h"
#include "engine.h"
#include "cubedef.h"

int message1, message2, message3, message4, message5, message6, message7, message8, message9, message10, message11, message12;
int message_streak1;

string strclassetueur, straptitudevictime;

namespace game
{
    void drawmessages(int killstreak, string str_pseudovictime, int n_aptitudevictime, string str_pseudoacteur, int n_killstreakacteur)
    {
        int decal_message = 0;

        int wb, hb;

        pushhudmatrix();
        hudmatrix.scale(screenh/2500.0f, screenh/2500.0f, 1);
        flushhudmatrix();

        if(totalmillis-message1<=2500)
        {
            if(killstreak==3) text_bounds("Triplette ! (x3)", wb, hb);
            else if(killstreak==5) text_bounds("Pentaplette ! (x5)", wb, hb);
            else if(killstreak==10) text_bounds("Décaplette ! (x10)", wb, hb);
            else if(killstreak==15) text_bounds("Pentadécaaplette ! (x15)", wb, hb);
            else if(killstreak==20) text_bounds("Eicoplette ! (x20)", wb, hb);
            else if(killstreak==30) text_bounds("Triaconplette ! (x30)", wb, hb);
            else if(killstreak>=31) text_bounds("%dplette ! (x%d)", wb, hb, killstreak);
            int x = ((screenw/2)*2.350f)-wb/2, y = (screenh/2)*2.350f;

            if(killstreak==3) draw_textf("Triplette ! \fc(x3)",  x, y-385);
            else if(killstreak==5) draw_textf("Pentaplette ! \fc(x5)",  x, y-385);
            else if(killstreak==10) draw_textf("Décaplette ! \fc(x10)",  x, y-385);
            else if(killstreak==15) draw_textf("Pentadécaplette ! \fc(x15)",  x, y-385);
            else if(killstreak==20) draw_textf("Eicoplette ! \fc(x20)",  x, y-385);
            else if(killstreak==30) draw_textf("Triaconplette ! \fc(x30)",  x, y-385);
            else if(killstreak>=31) draw_textf("%dplette ! \fc(x%d)",  x, y-385, killstreak, killstreak);
            decal_message = decal_message - 50;
        }

        if(totalmillis-message2<=2500)
        {
            text_bounds("Superpseudo123 (Aptitude)", wb, hb);
            int x = ((screenw/2)*2.170f)-wb/2, y = (screenh/2)*2.350f;
            draw_textf("Tu as tué \fc%s \f7! (%s)",  x, y-385+decal_message, str_pseudovictime, aptitudes[n_aptitudevictime].apt_nom);
            decal_message = decal_message - 50;
        }

        if(totalmillis-message3<=2500)
        {
            text_bounds("[BOT]Abcdef est chaud (Triple", wb, hb);
            int x = ((screenw/2)*2.170f)-wb/2, y = (screenh/2)*2.350f;
            switch(n_killstreakacteur)
            {
                case 3: draw_textf("\fc%s\f7 est chaud ! (Triplette)",  x, y-385+decal_message, str_pseudoacteur); break;
                case 5: draw_textf("\fc%s\f7 domine ! (Pentaplette)",  x, y-385+decal_message, str_pseudoacteur); break;
                case 10: draw_textf("\fc%s\f7 est inarrêtable ! (Décaplette)",  x, y-385+decal_message, str_pseudoacteur); break;
            }
            decal_message = decal_message - 50;
        }
        if(gamemillismsg < 5000)
        {
            text_bounds("Armes activées", wb, hb);
            int x = ((screenw/2)*2.170f)-wb/2, y = (screenh/2)*2.350f;
            draw_textf("\f6Armes activées dans :%s %.1f",  x, y-385+decal_message, gamemillismsg<1000 ? "\fe" : gamemillismsg > 3000 ? "\fc" : "\fd", (5000-gamemillismsg)/1000.0f);
            decal_message = decal_message - 50;
        }
        if(m_battle)
        {
            text_bounds("Joueurs vi", wb, hb);
            int x = ((screenw/2)*2.170f)-wb/2, y = (screenh/2)*2.350f;
            draw_textf("\f7Joueurs vivants : %d",  x, y-1200, battlevivants);
            //decal_message = decal_message - 50;
        }
        pophudmatrix();
    }

    float dodo = 0;

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
