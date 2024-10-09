#ifndef __GFX_H__
#define __GFX_H__

#include "game.h"

namespace game
{
    extern void resetpostfx();
    extern void renderProjectilesTrails(gameent *owner, vec &pos, vec dv, vec &from, vec &offset, int atk, bool exploded = false);
    extern void renderProjectileExplosion(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void renderExplosion(gameent *owner, const vec &v, const vec &vel, int atk);
    extern void renderBulletImpact(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void renderInstantImpact(const vec &from, const vec &to, const vec &muzzle, int atk, bool hasRoids);
    extern void renderMuzzleEffects(const vec &from, const vec &to, gameent *d, int atk);
    extern void addColorBlindnessFilter();

    extern int zoom, crosshairsize, forcecampos, cbfilter;
}

#endif
