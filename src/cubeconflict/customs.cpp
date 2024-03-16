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
        case CUST_CAPE: return capes[num].price;
        case CUST_GRAVE: return graves[num].price;
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

int rollItemRarity()
{
    int roll = rnd(100); // determine the rarity based on rolloff probabilities

    if(roll < 50) return D_COMMON;
    else if(roll < 75) return D_UNCOMMON;
    else if(roll < 90) return D_RARE;
    else if(roll < 98) return D_LEGENDARY;
    else return D_GODLY;
}

void unlockRandomItem()
{
    int rarity = rollItemRarity();
    bool unlockItem = rnd(2);

    if(unlockItem)
    {
        int itemType = rnd(NUMCUSTS);
        int itemNum;

        switch(itemType)
        {
            case CUST_SMILEY: itemNum = NUMSMILEYS; break;
            case CUST_CAPE: itemNum = NUMCAPES; break;
            case CUST_GRAVE: itemNum = NUMGRAVES; break;
            default: return;
        }

        bool itemUnlocked = false;
        while(!itemUnlocked)
        {
            int count = 0;
            loopi(itemNum)
            {
                switch(itemType)
                {
                    case CUST_SMILEY: if(customsmileys[i].value == rarity && !hasItem(itemType, i)) count++; break;
                    case CUST_CAPE: if(capes[i].value == rarity && !hasItem(itemType, i)) count++; break;
                    case CUST_GRAVE: if(graves[i].value == rarity && !hasItem(itemType, i)) count++; break;
                }
            }

            if(count > 0) // ensure there is at least one item of the selected rarity
            {
                int randomSelection = rnd(count);
                for(int i = 0, found = 0; i < itemNum && found <= randomSelection; i++)
                {
                    switch(itemType)
                    {
                        case CUST_SMILEY:
                            if(customsmileys[i].value == rarity && !hasItem(itemType, i))
                            {
                                if(found == randomSelection)
                                {
                                    conoutf("You've unlocked a %s smiley!", customsmileys[i].ident);
                                    itemUnlocked = true; // exit the while loop
                                    break; // exit the for loop
                                }
                                found++;
                            }
                            break;

                        case CUST_CAPE:
                            if(capes[i].value == rarity && !hasItem(itemType, i))
                            {
                                if(found == randomSelection)
                                {
                                    conoutf("You've unlocked a %s cape!", capes[i].name);
                                    itemUnlocked = true; // exit the while loop
                                    break; // exit the for loop
                                }
                                found++;
                            }
                            break;

                        case CUST_GRAVE:
                            if(graves[i].value == rarity && !hasItem(itemType, i))
                            {
                                if(found == randomSelection)
                                {
                                    conoutf("You've unlocked a %s grave!", graves[i].name);
                                    itemUnlocked = true; // exit the while loop
                                    break; // exit the for loop
                                }
                                found++;
                            }
                            break;
                    }
                }
            }
            else
            {
                conoutf("No more items (%d) of this rarity (%d) to unlock.", itemType, rarity);
                unlockItem = false;
                break; // exit the while loop if no items are available to unlock
            }
        }
    }

    if(!unlockItem)
    {
        int ccoAmount;
        switch(rarity)
        {
            case D_COMMON: ccoAmount = 3; break;
            case D_UNCOMMON: ccoAmount = 5; break;
            case D_RARE: ccoAmount = 10; break;
            case D_LEGENDARY: ccoAmount = 15; break;
            case D_GODLY: ccoAmount = 30;
        }
        conoutf("You've been awarded %d Golden Cisla Coins!", ccoAmount);
    }
}

const char* tooExpensive[NUMCUSTS] = {"Console_Shop_SmileyTooExpensive", "Console_Shop_CapeTooExpensive", "Console_Shop_GraveTooExpensive" };
const char* alreadyOwned[NUMCUSTS] = {"Console_Shop_SmileyOwned", "Console_Shop_CapeOwned", "Console_Shop_GraveOwned" };
const char* itemBuyed[NUMCUSTS] = {"Console_Shop_SmileyBuyed", "Console_Shop_CapeBuyed", "Console_Shop_GraveBuyed" };

void buyItem(int type, int num)
{
    if(!validItem(type, num)) return;

    if(itemPrice(type, num) > stat[STAT_CC]) // no enough money
    {
        conoutf(CON_GAMEINFO, "\f3%s", readstr(tooExpensive[type]));
        playSound(S_ERROR, vec(0, 0, 0), 0, 0, SND_UI);
        return;
    }
    else if(hasItem(type, num)) // item already owned
    {
        conoutf(CON_GAMEINFO, "\f3%s", readstr(alreadyOwned[type]));
        playSound(S_ERROR, vec(0, 0, 0), 0, 0, SND_UI);
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
