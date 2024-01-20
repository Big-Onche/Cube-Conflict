//customisations.cpp: where we buy skins on the store (hopefully in NFT in the future so I can be rich)

#include "game.h"
#include "stats.h"
#include "customs.h"

enum {CUST_SMILEY = 0, CUST_CAPE, CUST_GRAVE, NUMCUSTS};

int smiley[NUMSMILEYS];
int cape[NUMCAPES];
int grave[NUMGRAVES];

bool validItem(int type, int num)
{
    switch(type)
    {
        case CUST_SMILEY: return num >= 0 && num < NUMSMILEYS;
        case CUST_CAPE: return num >= 0 && num < NUMCAPES;
        case CUST_GRAVE: return num >= 0 && num < NUMGRAVES;
    }
    return false;
}

int itemPrice(int type, int num)
{
    switch(type)
    {
        case CUST_SMILEY: return customsmileys[num].price;
        case CUST_CAPE: return customscapes[num].price;
        case CUST_GRAVE: return customstombes[num].price;
    }
    return 0;
}
ICOMMAND(getitemprice, "ii", (int *itemType, int *itemNum), intret(itemPrice(*itemType, *itemNum)));

bool hasItem(int type, int num)
{
    switch(type)
    {
        case CUST_SMILEY: return smiley[num];
        case CUST_CAPE: return cape[num];
        case CUST_GRAVE: return grave[num];
    }
    return false;
}
ICOMMAND(hasitem, "ii", (int *itemtype, int *itemnum), intret(hasItem(*itemtype, *itemnum)));

const char* tooExpensive[NUMCUSTS] = {"Console_Shop_SmileyTooExpensive", "Console_Shop_CapeTooExpensive", "Console_Shop_GraveTooExpensive" };
const char* alreadyOwned[NUMCUSTS] = {"Console_Shop_SmileyOwned", "Console_Shop_CapeOwned", "Console_Shop_GraveOwned" };
const char* itemBuyed[NUMCUSTS] = {"Console_Shop_SmileyBuyed", "Console_Shop_CapeBuyed", "Console_Shop_GraveBuyed" };

void buyItem(int type, int num)
{
    if(!validItem(type, num)) return;

    if(itemPrice(type, num) > stat[STAT_CC]) // no enough money
    {
        conoutf(CON_GAMEINFO, "\f3%s", readstr(tooExpensive[type]));
        playSound(S_ERROR);
        return;
    }
    else if(hasItem(type, num)) // item already owned
    {
        conoutf(CON_GAMEINFO, "\f3%s", readstr(alreadyOwned[type]));
        playSound(S_ERROR);
    }
    else // take the money
    {
        stat[STAT_CC] -= itemPrice(type, num);

        switch(type)
        {
            case CUST_SMILEY: smiley[num] = max(rnd(256), 1);break;
            case CUST_CAPE: cape[num] = max(rnd(256), 1); break;
            case CUST_GRAVE: grave[num] = max(rnd(256), 1);
        }

        playSound(S_CAISSEENREGISTREUSE);
        conoutf(CON_GAMEINFO, "\fe%s", readstr(itemBuyed[type]));
        unlockAchievement(ACH_TMMONEY);
    }
}

ICOMMAND(buyitem, "ii", (int *itemType, int *itemNum), buyItem(*itemType, *itemNum));

ICOMMAND(getnumsmileys, "", (), intret(NUMSMILEYS));
ICOMMAND(getnumcapes, "", (), intret(NUMCAPES));
ICOMMAND(getnumgraves, "", (), intret(NUMGRAVES));

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
