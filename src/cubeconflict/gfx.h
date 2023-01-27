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

    static const struct armourinfo { int armoursteps; const char *armournames;} armours[] = { { 150, "wood/"}, { 250, "iron/"}, { 400, "gold/"}, { 300, "magnet/"}, { 600, "power/"} };

    extern bool champicolor;
    extern void resetshroomsgfx();
    extern void projgunexplosion(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void projexplosion(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void projgunhit(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void instantrayhit(const vec &from, const vec &to, const vec &muzzle, int atk);
    extern void shootgfx(const vec &from, const vec &to, gameent *d, int atk);
    extern char *getshielddir(int armourtype, int armourval, bool hud = false, bool preload = false);
    extern char *getdisguisement(int seed);

    extern int zoomfov, zoom, crosshairsize, weapposside, weapposup, forcecampos;
    extern float champifov, pourcents;

    extern int nbfps;
}

#endif
