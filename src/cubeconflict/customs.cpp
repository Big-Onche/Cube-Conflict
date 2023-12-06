//customisations.cpp: where we buy skins on the store (hopefully in NFT in the future so I can be rich)

#include "game.h"
#include "stats.h"
#include "customs.h"

int cust[NUMCUST];

namespace custom
{
    const char *getcapedir(int cape, bool enemy)
    {
        static char dir[64];
        sprintf(dir, "capes/%s%s", customscapes[cape].capedir, enemy ? "/enemy" : "");
        return dir;
    }
}

enum {CUST_SMILEY = 0, CUST_CAPE, CUST_GRAVE};

int itemprice(int itemtype, int itemnum)
{
    switch(itemtype)
    {
        case CUST_SMILEY: return customsmileys[itemnum].smileyprice;
        case CUST_CAPE: return customscapes[itemnum].capeprice;
        case CUST_GRAVE: return customstombes[itemnum].tombeprice;
    }
    return 0;
}
ICOMMAND(getitemprice, "ii", (int *itemtype, int *itemnum), intret(itemprice(*itemtype, *itemnum)));

int hasitem(int itemtype, int itemnum)
{
    switch(itemtype)
    {
        case CUST_SMILEY: return cust[SMI_HAP+itemnum];
        case CUST_CAPE: return cust[CAPE_CUBE+itemnum];
        case CUST_GRAVE: return cust[TOM_MERDE+itemnum];
    }
    return 0;
}
ICOMMAND(hasitem, "ii", (int *itemtype, int *itemnum), intret(hasitem(*itemtype, *itemnum)));

void buyitem(int itemtype, int itemnum)
{
    switch(itemtype)
    {
        case CUST_SMILEY:
            if(customsmileys[itemnum].smileyprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, "\f3%s", readstr("Console_Shop_SmileyTooExpensive")); playSound(S_ERROR); return; }
            else if(cust[SMI_HAP+itemnum]) {conoutf(CON_GAMEINFO, "\f3%s", readstr("Console_Shop_SmileyOwned"));  playSound(S_ERROR); return; }
            else
            {
                stat[STAT_CC] -= customsmileys[itemnum].smileyprice;
                cust[SMI_HAP+itemnum] = rnd(99)+1;
                playSound(S_CAISSEENREGISTREUSE);
                conoutf(CON_GAMEINFO, "\fe%s", readstr("Console_Shop_SmileyBuyed"));
            }
            break;
        case CUST_CAPE:
            if(customscapes[itemnum].capeprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, "\f3%s", readstr("Console_Shop_CapeTooExpensive")); playSound(S_ERROR); return; }
            else if(cust[CAPE_CUBE+itemnum]) {conoutf(CON_GAMEINFO, "\f3%s", readstr("Console_Shop_CapeOwned")); playSound(S_ERROR); return; }
            else
            {
                stat[STAT_CC] -= customscapes[itemnum].capeprice;
                cust[CAPE_CUBE+itemnum] = rnd(99)+1;
                playSound(S_CAISSEENREGISTREUSE);
                conoutf(CON_GAMEINFO, "\fe%s", readstr("Console_Shop_CapeBuyed"));
            }
            break;
        case CUST_GRAVE:
            if(customstombes[itemnum].tombeprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, "\f3%s", readstr("Console_Shop_GraveTooExpensive")); playSound(S_ERROR); return; }
            else if(cust[TOM_MERDE+itemnum]) {conoutf(CON_GAMEINFO, "\f3%s", readstr("Console_Shop_GraveOwned")); playSound(S_ERROR); return; }
            else
            {
                stat[STAT_CC] -= customstombes[itemnum].tombeprice;
                cust[TOM_MERDE+itemnum] = rnd(99)+1;
                playSound(S_CAISSEENREGISTREUSE);
                conoutf(CON_GAMEINFO, "\fe%s", readstr("Console_Shop_GraveBuyed"));
            }
            break;
    }
    unlockAchievement(ACH_TMMONEY);
}
ICOMMAND(buyitem, "ii", (int *itemtype, int *itemnum), buyitem(*itemtype, *itemnum));
