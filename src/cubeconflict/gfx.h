#ifndef __GFX_H__
#define __GFX_H__

#include "game.h"

namespace gfx
{
    static const struct colors{ int color; } rndcolor[] =
    {
        {0xFF0000},
        {0x00FF00},
        {0x0000FF},
        {0xFFFF00},
        {0x00FFFF},
        {0xFF00FF}
    };

    extern bool champicolor();
    extern bool hasroids(gameent *d);
    extern void resetpostfx();
    extern void renderProjectilesTrails(gameent *owner, vec &pos, vec dv, vec &from, vec &offset, int atk, bool exploded = false);
    extern void renderProjectileExplosion(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void renderExplosion(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void renderBulletImpact(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void renderInstantImpact(const vec &from, const vec &to, const vec &muzzle, int atk);
    extern void renderMuzzleEffects(const vec &from, const vec &to, gameent *d, int atk);
    extern char *getshielddir(int armourtype, int armourval, bool hud = false, bool preload = false);
    extern void addColorBlindnessFilter();

    extern int zoomfov, zoom, crosshairsize, weapposside, weapposup, forcecampos, cbfilter, nbfps;
}

#endif
