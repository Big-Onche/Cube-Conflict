// rpg for cube conflict

#include "game.h"
#include "customisation.h"

namespace game
{
    //////////////////////////////////// Tutorial ////////////////////////////////////////////////////////////////////////

    VAR(examresult, 0, 0, 4); //calc exam result from tutorial's map
    ICOMMAND(calcexamresult, "iiii", (int *a, int *b, int *c, int *d),
        int max_value = max(max(*a, *b), max(*c, *d));
        max_value == *b ? examresult = 2 : max_value == *d ? examresult = 4 : max_value == *a ? examresult = 1 : examresult = 3;
    );

    ICOMMAND(getfreecust, "", (), //free customs from treasure
        if(m_tutorial) cust[TOM_BASIQUE1] = cust[CAPE_JVC] = cust[SMI_NOEL] = rnd(99)+1;
    );

    void drawrpgminimap(gameent *d, int w, int h)
    {
        pushhudscale(h/1800.0f);
        pushhudscale(2);
        pophudmatrix();
        resethudshader();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        int s = 1800/4, x = 1800*w/h - s - s/10, y = s/10;

        gle::colorf(1, 1, 1, minimapalpha);
        if(minimapalpha >= 1) glDisable(GL_BLEND);
        bindminimap();
        drawminimap(d, x, y, s);
        if(minimapalpha >= 1) glEnable(GL_BLEND);
        gle::colorf(1, 1, 1);
        float margin = 0.04f, roffset = s*margin, rsize = s + 2*roffset;
        setradartex();
        drawradar(x - roffset, y - roffset, rsize);
        settexture("media/interface/hud/boussole.png", 3);
        pushhudmatrix();
        hudmatrix.translate(x - roffset + 0.5f*rsize, y - roffset + 0.5f*rsize, 0);
        hudmatrix.rotate_around_z((camera1->yaw + 180)*-RAD);
        flushhudmatrix();
        drawradar(-0.5f*rsize, -0.5f*rsize, rsize);
        pophudmatrix();
        drawplayerblip(d, x, y, s, 1.5f);
        drawfriends(d, x, y, s);
        pophudmatrix();
    }
}
