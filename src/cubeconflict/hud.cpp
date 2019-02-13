#include "game.h"
#include "engine.h"
#include "cubedef.h"

int message1, message2, message3, message4, message5, message6, message7, message8, message9, message10, message11, message12;
int message_streak1;

string strclassetueur, straptitudevictime;

int decal_message = 0;
bool need_message1, need_message2;

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

    void drawmessages(int killstreak, string str_pseudovictime, int n_aptitudevictime, string str_pseudoacteur, int n_killstreakacteur)
    {
        decal_message = 0, need_message1 = true, need_message2 = true;

        if(totalmillis-message1<=2500)
        {
            string streakmsg;

            switch(killstreak)
            {
                case 3: formatstring(streakmsg, "Triplette ! \fc(x%d)", killstreak); break;
                case 5: formatstring(streakmsg, "Pentaplette ! \fc(x%d)", killstreak); break;
                case 10: formatstring(streakmsg, "Décaplette ! \fc(x%d)", killstreak); break;
                case 20: formatstring(streakmsg, "Eicoplette ! \fc(x%d)", killstreak); break;
                case 30: formatstring(streakmsg, "Triaconplette ! \fc(x%d)", killstreak); break;
                default : need_message1 = false;
            }

            if(need_message1) {rendermessage(streakmsg, 85, 8.8f, decal_message); decal_message -= 45;}
        }

        if(totalmillis-message2<=2500)
        {
            string killmsg;

            formatstring(killmsg, "Tu as tué \fc%s \f7! (%s)", str_pseudovictime, aptitudes[n_aptitudevictime].apt_nom);
            rendermessage(killmsg, 100, 8.8f, decal_message);
            decal_message -= 40;
        }

        if(totalmillis-message3<=2500)
        {
            string infomsg;

            switch(n_killstreakacteur)
            {
                case 3: formatstring(infomsg, "\fc%s\f7 est chaud ! (Triplette)", str_pseudoacteur); break;
                case 5: formatstring(infomsg, "\fc%s\f7 domine ! (Pentaplette)", str_pseudoacteur); break;
                case 10: formatstring(infomsg, "\fc%s\f7 est inarrêtable ! (Décaplette)", str_pseudoacteur); break;
                case 20: formatstring(infomsg, "\fc%s\f7 est invincible ! (Eicoplette)", str_pseudoacteur); break;
                case 30: formatstring(infomsg, "\fc%s\f7 est un Dieu ! (Triaconplette)", str_pseudoacteur); break;
                default: need_message2 = false;
            }

            if(need_message2) {rendermessage(infomsg, 100, 8.8f, decal_message); decal_message -= 40;}
        }
        /*
        if(m_battle)
        {
            string remainingplayersmsg;

            formatstring(remainingplayersmsg, "\f7Joueurs vivants : %d",  battlevivants);
            rendermessage(remainingplayersmsg, 75, 10.0f);
        }
        */
        if(player1->state==CS_DEAD)
        {
            string killedbymsg, withmsg, waitmsg;
            if(suicided) formatstring(killedbymsg, "Tu t'es suicidé !");
            else formatstring(killedbymsg, "Tué par %s (%s)", str_pseudotueur, aptitudes[n_aptitudetueur].apt_nom);

            rendermessage(killedbymsg, 65, 1.5f, 0);
            formatstring(withmsg, "Avec %s", str_armetueur);
            rendermessage(withmsg, 95, 1.5f, -360);

            int wait = cmode ? cmode->respawnwait(player1) : (lastmillis < player1->lastpain + 1000) ? 1 : 0 ;
            if(wait>0) formatstring(waitmsg, "Respawn possible dans %d seconde%s", wait, wait<=1?"":"s");
            else formatstring(waitmsg, "Appuie n'importe où pour revivre ! ");
            rendermessage(waitmsg, 95, 1.5f, -580);
        }


        if(player1->state==CS_SPECTATOR)
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
                formatstring(spectatormsg, "Spectateur : %s%s", color, colorname(f));
            }
            else
            {
                formatstring(spectatormsg, "Spectateur : Caméra libre");
            }
            rendermessage(spectatormsg, 75, 2.0f);
        }
    }

    void gameplayhud(int w, int h)
    {
        if(zoom && crosshairsize >= 31) {crosshairsize -= 3; if(crosshairsize<31) crosshairsize = 31;}
        else if (crosshairsize<40) crosshairsize += 3;
        zoomfov = guns[player1->gunselect].maxzoomfov;

        gameent *d = hudplayer();
        if(d->state!=CS_EDITING)
        {
            if(cmode) cmode->drawhud(d, w, h);
        }
        if(player1->state==CS_SPECTATOR && d==player1) {pophudmatrix(); return; }

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

        //////////////////////////////////////// RENDU DES IMAGES ////////////////////////////////////////

        if(d->state==CS_DEAD)
        {
            settexture("media/interface/hud/mort.png");
            bgquad(15, h-130, 115, 115);
            pophudmatrix();
            return;
        }

        if(player1->gunselect == GUN_CAC349 || player1->gunselect == GUN_CACFLEAU || player1->gunselect == GUN_CACMARTEAU || player1->gunselect == GUN_CACMASTER) settexture("media/interface/hud/epee.png");
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
            }
            bgquad(250, h-130, 115, 115);
        }

        int decal_icon = 0;
        if(player1->crouching && player1->aptitude==9)
        {
            settexture("media/interface/hud/campeur.png");
            bgquad(15, h-260, 115, 115);
            decal_icon += 130;
        }

        if(player1->ragemillis){settexture("media/interface/hud/rage.png"); bgquad(15, h-260, 115, 115); decal_icon += 130;}
        if(player1->steromillis){settexture("media/interface/hud/steros.png"); bgquad(15, h-260-decal_icon, 115, 115); decal_icon += 130;}
        if(player1->epomillis){settexture("media/interface/hud/epo.png"); bgquad(15, h-260-decal_icon, 115, 115); decal_icon += 130;}
        if(player1->champimillis){settexture("media/interface/hud/champis.png"); bgquad(15, h-260-decal_icon, 115, 115); decal_icon += 130;}
        if(player1->jointmillis){settexture("media/interface/hud/joint.png"); bgquad(15, h-260-decal_icon, 115, 115);}

        float lxbarvide = 0.5f*(w - 966), lxbarpleine = 0.5f*(w - 954);

        settexture("media/interface/hud/barrexppleine.png", 3);
        bgquad(lxbarpleine, h-19, (pourcents + 1)*954.0f, 19);

        settexture("media/interface/hud/barrexpvide.png", 3);
        bgquad(lxbarvide, h-29, 966, 40);

        //////////////////////////////////////// RENDU DES NOMBRES ////////////////////////////////////////

        int decal_number = 0;

        switch(player1->gunselect)
        {
            case GUN_S_CAMPOUZE: case GUN_S_GAU8: case GUN_S_NUKE: case GUN_S_ROQUETTES: draw_textf("%d", (d->ammo[d->gunselect] > 99 ? w-227 : d->ammo[d->gunselect] > 9 ? w-196 : w-166), h-103, d->ammo[d->gunselect]); break;
            default:
            {
                if(m_random)
                {
                    settexture("media/interface/hud/inf.png"); bgquad(w-227, h-130, 115, 115);
                }
                else draw_textf("%d", (d->ammo[d->gunselect] > 99 ? w-227 : d->ammo[d->gunselect] > 9 ? w-196 : w-166), h-103, d->ammo[d->gunselect]);
            }
        }
        draw_textf("%d", 135, h-103, d->health < 9 ? 1 : d->health/10);

        if(d->armour > 0) draw_textf("%d", 370, h-103, d->armour < 9 ? 1 : d->armour/10);

        if(player1->crouching && player1->aptitude==9) decal_number +=130;
        if(player1->ragemillis) {draw_textf("%d", 135, h-233-decal_number, d->ragemillis/1000); decal_number +=130;}
        if(player1->steromillis) {draw_textf("%d", 135, h-233-decal_number, d->steromillis/1000); decal_number +=130;}
        if(player1->epomillis) {draw_textf("%d", 135, h-233-decal_number, d->epomillis/1000); decal_number +=130;}
        if(player1->champimillis) {draw_textf("%d", 135, h-233-decal_number, d->champimillis/1000); decal_number +=130;}
        if(player1->jointmillis) {draw_textf("%d", 135, h-233-decal_number, d->jointmillis/1000); decal_number +=130;}

        defformatstring(infobarrexp, "%d/%d XP - LVL %d", neededxp-(needxp-ccxp), neededxp, cclvl);

        int tw = text_width(infobarrexp);
        float tsz = 0.4f,
              tx = 0.5f*(w - tw*tsz), ty = h - 22;
        pushhudmatrix();
        hudmatrix.translate(tx, ty, 0);
        hudmatrix.scale(tsz, tsz, 1);
        flushhudmatrix();

        draw_text(infobarrexp, 0, 0);
        pophudmatrix();


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
