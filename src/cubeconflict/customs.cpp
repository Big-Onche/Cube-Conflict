//customisations.cpp: where we buy skins on the store (hopefully in NFT in the future so I can be rich)

#include "game.h"
#include "stats.h"
#include "customs.h"

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

enum {CUST_SMILEY = 0, CUST_CAPE, CUST_GRAVE};

int itemprice(int itemtype, int itemnum) // R�cup�re le prix d'un objet
{
    switch(itemtype)
    {
        case CUST_SMILEY: return customsmileys[itemnum].smileyprice;
        case CUST_CAPE: return customscapes[itemnum].capeprice;
        case CUST_GRAVE: return customstombes[itemnum].tombeprice;
        //case 3: return customsdance[itemnum].danceprice;
    }
    return 0;
}
ICOMMAND(getitemprice, "ii", (int *itemtype, int *itemnum), intret(itemprice(*itemtype, *itemnum)));

int hasitem(int itemtype, int itemnum) // R�cup�re si un objet est poss�d� ou non
{
    switch(itemtype)
    {
        case CUST_SMILEY: return cust[SMI_HAP+itemnum];
        case CUST_CAPE: return cust[CAPE_CUBE+itemnum];
        case CUST_GRAVE: return cust[TOM_MERDE+itemnum];
        //case 3: return cust[VOI_CORTEX]; break;
    }
    return 0;
}
ICOMMAND(hasitem, "ii", (int *itemtype, int *itemnum), intret(hasitem(*itemtype, *itemnum)));

void buyitem(int itemtype, int itemnum) //Achète un objet
{
    switch(itemtype)
    {
        case CUST_SMILEY:
            if(customsmileys[itemnum].smileyprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, GAME_LANG ? "\f3This smiley is too expensive for you!" : "\f3Ce smiley est trop cher pour toi !"); playsound(S_ERROR); return;}
            else if(cust[SMI_HAP+itemnum]) {conoutf(CON_GAMEINFO, GAME_LANG ? "\f3You already own this smiley!" : "\f3Vous poss�dez déjà ce smiley !"); playsound(S_ERROR); return;}
            else
            {
                stat[STAT_CC] -= customsmileys[itemnum].smileyprice;
                cust[SMI_HAP+itemnum] = rnd(99)+1;
                playsound(S_CAISSEENREGISTREUSE);
            }
            break;
        case CUST_CAPE:
            if(customscapes[itemnum].capeprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, GAME_LANG ? "\f3This cape is too expensive for you!" : "\f3Cette cape est trop chère pour toi !"); playsound(S_ERROR); return;}
            else if(cust[CAPE_CUBE+itemnum]) {conoutf(CON_GAMEINFO, GAME_LANG ? "\f3You already own this cape!" : "\f3Vous poss�dez déjà cette cape !"); playsound(S_ERROR); return;}
            else
            {
                stat[STAT_CC] -= customscapes[itemnum].capeprice;
                cust[CAPE_CUBE+itemnum] = rnd(99)+1;
                playsound(S_CAISSEENREGISTREUSE);
            }
            break;
        case CUST_GRAVE:
            if(customstombes[itemnum].tombeprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, GAME_LANG ? "\f3This grave is too expensive for you!" : "\f3Cette tombe est trop ch�chère pour toi !"); playsound(S_ERROR); return;}
            else if(cust[TOM_MERDE+itemnum]) {conoutf(CON_GAMEINFO, GAME_LANG ? "\f3You already own this grave!" : "\f3Vous poss�dez déjà cette tombe !"); playsound(S_ERROR); return;}
            else
            {
                stat[STAT_CC] -= customstombes[itemnum].tombeprice;
                cust[TOM_MERDE+itemnum] = rnd(99)+1;
                playsound(S_CAISSEENREGISTREUSE);
            }
            break;
        //case 3:
        //    if(customsdance[UI_voix].danceprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, "\f3Cette voix est trop ch�re pour toi !"); playsound(S_ERROR); return;}
        //    else if(cust[VOI_CORTEX+UI_voix]>0) {conoutf(CON_GAMEINFO, "\f3Vous poss�dez d�j� cette voix !"); playsound(S_ERROR); return;}
        //    else
        //    {
        //        conoutf(CON_GAMEINFO, "\f9Voix \"%s\" achet�e !", customsdance[UI_voix].dancename);
        //        stat[STAT_CC] = stat[STAT_CC]-customsdance[UI_voix].danceprice;
        //        cust[VOI_CORTEX+UI_voix] = rnd(99)+1;
        //        playsound(S_CAISSEENREGISTREUSE);
        //       return;
        //    }
    }
    unlockachievement(ACH_TMMONEY);
}
ICOMMAND(buyitem, "ii", (int *itemtype, int *itemnum), buyitem(*itemtype, *itemnum));
