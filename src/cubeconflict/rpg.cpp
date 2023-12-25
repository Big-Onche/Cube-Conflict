// rpg for cube conflict
#include "game.h"
#include "customs.h"

namespace game
{
    const int MAXQUESTS = 256;

    struct quest //all needed infos for quests
    {
        string name, desc;
        int steps, type, difficulty, reward;
    };

    quest quests[MAXQUESTS];

    ICOMMAND(quest, "issiiii", (int *id, char *name, char *desc, int *steps, int *type, int *dif, int *reward),
        formatstring(quests[*id].name, "%s", name); // Quest name
        formatstring(quests[*id].desc, "%s", desc); // Quest short description
        quests[*id].steps = *steps;                 // Amount of steps before completion
        quests[*id].type = *type;                   // Type (main or side quest)
        quests[*id].difficulty = *dif;              // Difficulty
        quests[*id].reward = *reward;               // XP reward
    );

    enum questsRewards {R_TREASURE = 0};

    ICOMMAND(getQuestReward, "i", (int *i),
        switch(*i)
        {
            case R_TREASURE:
               if(m_tutorial) grave[TOM_BASIQUE1] = cape[CAPE_PAINT1] = smiley[SMI_NOEL] = max(rnd(256), 1);
               break;
        }
    );

    ICOMMAND(isfinished, "ii", (int *id, int *s),
        intret(*s >= quests[*id].steps ? true : false);
    );

    //////////////////////////////////// Basic functions ////////////////////////////////////////////////////////////////////////
    ICOMMAND(isdead, "", (), intret(player1->state==CS_DEAD));

    ICOMMAND(giveAmmo, "ii", (int *amount, int *gun), // giving ammunitions
        if(!m_tutorial || !validgun(*gun)) return;
        player1->ammo[*gun] += *amount;
        gunselect(*gun, player1, false, true);
    );

    //////////////////////////////////// Tutorial ////////////////////////////////////////////////////////////////////////


    ICOMMAND(checkammo, "i", (int *gun), //checking if p1 has ammo for gun
        intret(player1->ammo[*gun]);
    );

    //////////////////////////////////// Drops ////////////////////////////////////////////////////////////////////////
    void utilitydrop(const vec *o, bool hightier = false) //always roll'd
    {
        loopi(hightier ? 2 : 1)
        {
            switch(rnd(6))
            {
                case 0: createdrop(o, hightier ? I_BOOSTPV : I_SANTE); break;
                case 1: createdrop(o, I_MANA); break;
                case 2: createdrop(o, hightier ? I_IRONSHIELD : I_WOODSHIELD);
            }
        }
    }

    void npcdrop(const vec *o, int type)
    {
        loopi(type==0 ? 1 : type) utilitydrop(o, type>D_RARE ? true : false);

        switch(type)
        {
            case D_COMMON:
                switch(rnd(4))
                {
                    case 0: createdrop(o, I_GLOCK); break;
                    case 1: createdrop(o, I_RAIL+rnd(17)); break;
                }
                break;

            case D_UNCOMMON:
                switch(rnd(6))
                {
                    case 0: createdrop(o, I_IRONSHIELD); break;
                    case 1: createdrop(o, I_RAIL+rnd(17)); break;
                    case 2: createdrop(o, I_BOOSTPV); break;
                }
                break;

            case D_RARE:
                switch(rnd(6))
                {
                    case 0: createdrop(o, I_MAGNETSHIELD); break;
                    case 1: createdrop(o, I_RAIL+rnd(17)); break;
                    case 2: createdrop(o, I_BOOSTPV); break;
                }
                break;

            case D_LEGENDARY:
                switch(rnd(6))
                {
                    case 0: createdrop(o, I_GOLDSHIELD); break;
                    case 1: createdrop(o, I_EPO); break;
                    case 2: createdrop(o, I_SHROOMS); break;
                    case 3: loopi(2) createdrop(o, I_RAIL+rnd(17)); break;
                }
                break;

            case D_GODLY:
                switch(rnd(6))
                {
                    case 0: createdrop(o, I_POWERARMOR); break;
                    case 1: createdrop(o, I_SUPERARME); break;
                    case 2: createdrop(o, I_ROIDS); break;
                    case 3: loopi(2) createdrop(o, I_RAIL+rnd(17)); break;
                }
                break;
        }
    }

    //////////////////////////////////// HUD minimap ////////////////////////////////////////////////////////////////////////
    void drawrpgminimap(gameent *d, int w, int h)
    {
        pushhudscale(h/1800.0f);
        pushhudscale(3);
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
        drawnpcs(d, x, y, s);
        pophudmatrix();
    }
}
