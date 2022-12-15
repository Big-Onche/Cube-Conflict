//customisations.cpp: where we buy skins on the store (hopefully in NFT in the future so I can be rich)

#include "ccheader.h"
#include "stats.h"
#include "customisation.h"

using namespace std;

int cust[NUMCUST];

namespace custom
{
    string capedir;
    const char *getcapedir(int cape, bool enemy)
    {
        formatstring(capedir, "capes/%s%s", customscapes[cape].capedir, enemy ? "/enemy" : "");
        return capedir;
    }
}

int itemprice(int itemtype, int itemnum) // Récupère le prix d'un objet
{
    switch(itemtype)
    {
        case 0: return customsmileys[itemnum].smileyprice;
        case 1: return customscapes[itemnum].capeprice;
        case 2: return customstombes[itemnum].tombeprice;
        //case 3: return customsdance[itemnum].danceprice;
    }
    return 0;
}
ICOMMAND(getitemprice, "ii", (int *itemtype, int *itemnum), intret(itemprice(*itemtype, *itemnum)));

int hasitem(int itemtype, int itemnum) // Récupère si un objet est possédé ou non
{
    switch(itemtype)
    {
        case 0: return cust[SMI_HAP+itemnum]; break;
        case 1: return cust[CAPE_CUBE+itemnum]; break;
        case 2: return cust[TOM_MERDE+itemnum]; break;
        //case 3: return cust[VOI_CORTEX]; break;
    }
    return 0;
}
ICOMMAND(hasitem, "ii", (int *itemtype, int *itemnum), intret(hasitem(*itemtype, *itemnum)));

void buyitem(int itemtype, int itemnum) //Achète un objet
{
    switch(itemtype)
    {
        case 0:
            if(customsmileys[itemnum].smileyprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, GAME_LANG ? "\f3This smiley is too expensive for you!" : "\f3Ce smiley est trop cher pour toi !"); playsound(S_ERROR); return;}
            else if(cust[SMI_HAP+itemnum]>0) {conoutf(CON_GAMEINFO, GAME_LANG ? "\f3You already own this smiley!" : "\f3Vous possédez déjà ce smiley !"); playsound(S_ERROR); return;}
            else
            {
                stat[STAT_CC] = stat[STAT_CC] - customsmileys[itemnum].smileyprice;
                cust[SMI_HAP+itemnum] = rnd(99)+1;
                playsound(S_CAISSEENREGISTREUSE);
                return;
            }
        case 1:
            if(customscapes[itemnum].capeprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, GAME_LANG ? "\f3This cape is too expensive for you!" : "\f3Cette cape est trop chère pour toi !"); playsound(S_ERROR); return;}
            else if(cust[CAPE_CUBE+itemnum]>0) {conoutf(CON_GAMEINFO, GAME_LANG ? "\f3You already own this cape!" : "\f3Vous possédez déjà cette cape !"); playsound(S_ERROR); return;}
            else
            {
                //conoutf(CON_GAMEINFO, GAME_LANG ? "\f9You bought an \"%s\" cape!" : "\f9Cape \"%s\" achetée !", customscapes[UI_cape].capename);
                stat[STAT_CC] = stat[STAT_CC]-customscapes[itemnum].capeprice;
                cust[CAPE_CUBE+itemnum] = rnd(99)+1;
                playsound(S_CAISSEENREGISTREUSE);
                return;
            }
        case 2:
            if(customstombes[itemnum].tombeprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, GAME_LANG ? "\f3This grave is too expensive for you!" : "\f3Cette tombe est trop chère pour toi !"); playsound(S_ERROR); return;}
            else if(cust[TOM_MERDE+itemnum]>0) {conoutf(CON_GAMEINFO, GAME_LANG ? "\f3You already own this grave!" : "\f3Vous possédez déjà cette tombe !"); playsound(S_ERROR); return;}
            else
            {
                //conoutf(CON_GAMEINFO, GAME_LANG ? "\f9You bought an \"%s\" grave!" : "\f9Tombe \"%s\" achetée !", customstombes[UI_tombe].tombename);
                stat[STAT_CC] = stat[STAT_CC]-customstombes[itemnum].tombeprice;
                cust[TOM_MERDE+itemnum] = rnd(99)+1;
                playsound(S_CAISSEENREGISTREUSE);
                return;
            }
        //case 3:
        //    if(customsdance[UI_voix].danceprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, "\f3Cette voix est trop chère pour toi !"); playsound(S_ERROR); return;}
        //    else if(cust[VOI_CORTEX+UI_voix]>0) {conoutf(CON_GAMEINFO, "\f3Vous possédez déjà cette voix !"); playsound(S_ERROR); return;}
        //    else
        //    {
        //        conoutf(CON_GAMEINFO, "\f9Voix \"%s\" achetée !", customsdance[UI_voix].dancename);
        //        stat[STAT_CC] = stat[STAT_CC]-customsdance[UI_voix].danceprice;
        //        cust[VOI_CORTEX+UI_voix] = rnd(99)+1;
        //        playsound(S_CAISSEENREGISTREUSE);
        //       return;
        //    }
    }
}
ICOMMAND(buyitem, "ii", (int *itemtype, int *itemnum), buyitem(*itemtype, *itemnum));

