//customisations.cpp: where we buy skins on the store (hopefully in NFT in the future so I can be rich)

#include "game.h"
#include "stats.h"
#include "customs.h"

int smiley[NUMSMILEYS];
int cape[NUMCAPES];
int grave[NUMGRAVES];

const char *getcapedir(int cape, bool enemy)
{
    static char dir[64];
    sprintf(dir, "capes/%s%s", customscapes[cape].ident, enemy ? "/enemy" : "");
    return dir;
}

const char *getgravedir(int grave)
{
    static char dir[64];
    sprintf(dir, "graves/%s", customstombes[grave].ident);
    return dir;
}

enum {CUST_SMILEY = 0, CUST_CAPE, CUST_GRAVE};

int itemprice(int itemtype, int itemnum)
{
    switch(itemtype)
    {
        case CUST_SMILEY: return customsmileys[itemnum].price;
        case CUST_CAPE: return customscapes[itemnum].price;
        case CUST_GRAVE: return customstombes[itemnum].price;
    }
    return 0;
}
ICOMMAND(getitemprice, "ii", (int *itemtype, int *itemnum), intret(itemprice(*itemtype, *itemnum)));

int hasitem(int itemtype, int itemnum)
{
    switch(itemtype)
    {
        case CUST_SMILEY: return smiley[SMI_HAP];
        case CUST_CAPE: return cape[CAPE_CUBE];
        case CUST_GRAVE: return grave[TOM_MERDE];
    }
    return 0;
}
ICOMMAND(hasitem, "ii", (int *itemtype, int *itemnum), intret(hasitem(*itemtype, *itemnum)));

void buyitem(int itemtype, int itemnum)
{
    switch(itemtype)
    {
        case CUST_SMILEY:
            if(customsmileys[itemnum].price > stat[STAT_CC]) {conoutf(CON_GAMEINFO, "\f3%s", readstr("Console_Shop_SmileyTooExpensive")); playSound(S_ERROR); return; }
            else if(smiley[itemnum]) {conoutf(CON_GAMEINFO, "\f3%s", readstr("Console_Shop_SmileyOwned"));  playSound(S_ERROR); return; }
            else
            {
                stat[STAT_CC] -= customsmileys[itemnum].price;
                smiley[itemnum] = max(rnd(256), 1);
                playSound(S_CAISSEENREGISTREUSE);
                conoutf(CON_GAMEINFO, "\fe%s", readstr("Console_Shop_SmileyBuyed"));
            }
            break;
        case CUST_CAPE:
            if(customscapes[itemnum].price > stat[STAT_CC]) {conoutf(CON_GAMEINFO, "\f3%s", readstr("Console_Shop_CapeTooExpensive")); playSound(S_ERROR); return; }
            else if(cape[itemnum]) {conoutf(CON_GAMEINFO, "\f3%s", readstr("Console_Shop_CapeOwned")); playSound(S_ERROR); return; }
            else
            {
                stat[STAT_CC] -= customscapes[itemnum].price;
                cape[itemnum] = max(rnd(256), 1);
                playSound(S_CAISSEENREGISTREUSE);
                conoutf(CON_GAMEINFO, "\fe%s", readstr("Console_Shop_CapeBuyed"));
            }
            break;
        case CUST_GRAVE:
            if(customstombes[itemnum].price > stat[STAT_CC]) {conoutf(CON_GAMEINFO, "\f3%s", readstr("Console_Shop_GraveTooExpensive")); playSound(S_ERROR); return; }
            else if(grave[itemnum]) {conoutf(CON_GAMEINFO, "\f3%s", readstr("Console_Shop_GraveOwned")); playSound(S_ERROR); return; }
            else
            {
                stat[STAT_CC] -= customstombes[itemnum].price;
                grave[itemnum] = max(rnd(256), 1);
                playSound(S_CAISSEENREGISTREUSE);
                conoutf(CON_GAMEINFO, "\fe%s", readstr("Console_Shop_GraveBuyed"));
            }
            break;
    }
    unlockAchievement(ACH_TMMONEY);
}
ICOMMAND(buyitem, "ii", (int *itemtype, int *itemnum), buyitem(*itemtype, *itemnum));
