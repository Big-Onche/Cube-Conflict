#include <climits>
#include <cstdlib>
#include "cubedef.h"

string bouclier;
const char *getshielddir(gameent *d, bool hud) //récupère l'id d'un bouclier
{
    switch(d->armourtype)
    {
        case A_BLUE:    {int shieldvalue = d->armour<=150 ? 20 : d->armour<=300  ? 40 : d->armour<=450  ? 60 : d->armour<=600  ? 80 : 100; formatstring(bouclier, "%s%s%d", hud ? "hudshield/" : "worldshield/", "bois/", shieldvalue);} break;
        case A_GREEN:   {int shieldvalue = d->armour<=250 ? 20 : d->armour<=500  ? 40 : d->armour<=750  ? 60 : d->armour<=1000 ? 80 : 100; formatstring(bouclier, "%s%s%d", hud ? "hudshield/" : "worldshield/", "fer/", shieldvalue);} break;
        case A_MAGNET:  {int shieldvalue = d->armour<=300 ? 20 : d->armour<=600  ? 40 : d->armour<=900  ? 60 : d->armour<=1200 ? 80 : 100; formatstring(bouclier, "%s%s%d", hud ? "hudshield/" : "worldshield/", "magnetique/", shieldvalue);} break;
        case A_YELLOW:  {int shieldvalue = d->armour<=400 ? 20 : d->armour<=800  ? 40 : d->armour<=1200 ? 60 : d->armour<=1600 ? 80 : 100; formatstring(bouclier, "%s%s%d", hud ? "hudshield/" : "worldshield/", "or/", shieldvalue);} break;
        case A_ASSIST:  {int shieldvalue = d->armour<=600 ? 20 : d->armour<=1200 ? 40 : d->armour<=1800 ? 60 : d->armour<=2400 ? 80 : 100; formatstring(bouclier, "%s%d", "hudshield/armureassistee/", shieldvalue);} break;
    }
    return bouclier;
}
