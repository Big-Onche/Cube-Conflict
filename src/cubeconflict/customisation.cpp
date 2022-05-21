//customisations.cpp: where we buy skins on the store (hopefully in NFT in the future so I can be rich)

#include "ccheader.h"
#include "stats.h"
#include "customisation.h"

using namespace std;

int cust[NUMCUST];

const int itemprice(int itemtype, int itemID) // Récupère le prix d'un objet
{
    int price = 0;
    switch(itemtype)
    {
        case 0: price = customssmileys[itemID].smileyprice; break;
        case 1: price = customscapes[itemID].capeprice; break;
        case 2: price = customstombes[itemID].tombeprice; break;
        case 3: price = customsdance[itemID].danceprice; break;
    }
    return price;
}
ICOMMAND(getitemprice, "ii", (int *itemtype, int *itemID), intret(itemprice(*itemtype, *itemID)));

const int hasitem() // Récupère si un objet est possédé ou non
{
    int itemstat = 0;
    switch(UI_custtab)
    {
        case 0: itemstat = cust[SMI_HAP+UI_smiley]; break;
        case 1: itemstat = cust[CAPE_CUBE+UI_cape]; break;
        case 2: itemstat = cust[TOM_MERDE+UI_tombe]; break;
        case 3: itemstat = cust[VOI_CORTEX+UI_voix]; break;
    }
    return itemstat;
}
ICOMMAND(hasitem, "", (), intret(hasitem()));

void buyitem(int itemtype) //Achète un objet
{
    switch(itemtype)
    {
        case 0:
            if(customssmileys[UI_smiley].smileyprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, langage ? "\f3This smiley is too expensive for you!" : "\f3Ce smiley est trop cher pour toi !"); playsound(S_ERROR); return;}
            else if(cust[SMI_HAP+UI_smiley]>0) {conoutf(CON_GAMEINFO, langage ? "\f3You already own this smiley!" : "\f3Vous possédez déjà ce smiley !"); playsound(S_ERROR); return;}
            else
            {
                conoutf(CON_GAMEINFO, langage ? "\f9You bought an \"%s\" smiley!" : "\f9Smiley \"%s\" acheté !", customssmileys[UI_smiley].smileyname);
                stat[STAT_CC] = stat[STAT_CC]-customssmileys[UI_smiley].smileyprice;
                cust[SMI_HAP+UI_smiley] = rnd(99)+1;
                playsound(S_CAISSEENREGISTREUSE);
                return;
            }
        case 1:
            if(customscapes[UI_cape].capeprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, langage ? "\f3This cape is too expensive for you!" : "\f3Cette cape est trop chère pour toi !"); playsound(S_ERROR); return;}
            else if(cust[CAPE_CUBE+UI_cape]>0) {conoutf(CON_GAMEINFO, langage ? "\f3You already own this cape!" : "\f3Vous possédez déjà cette cape !"); playsound(S_ERROR); return;}
            else
            {
                conoutf(CON_GAMEINFO, langage ? "\f9You bought an \"%s\" cape!" : "\f9Cape \"%s\" achetée !", customscapes[UI_cape].capename);
                stat[STAT_CC] = stat[STAT_CC]-customscapes[UI_cape].capeprice;
                cust[CAPE_CUBE+UI_cape] = rnd(99)+1;
                playsound(S_CAISSEENREGISTREUSE);
                return;
            }
        case 2:
            if(customstombes[UI_tombe].tombeprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, langage ? "\f3This grave is too expensive for you!" : "\f3Cette tombe est trop chère pour toi !"); playsound(S_ERROR); return;}
            else if(cust[TOM_MERDE+UI_tombe]>0) {conoutf(CON_GAMEINFO, langage ? "\f3You already own this grave!" : "\f3Vous possédez déjà cette tombe !"); playsound(S_ERROR); return;}
            else
            {
                conoutf(CON_GAMEINFO, langage ? "\f9You bought an \"%s\" grave!" : "\f9Tombe \"%s\" achetée !", customstombes[UI_tombe].tombename);
                stat[STAT_CC] = stat[STAT_CC]-customstombes[UI_tombe].tombeprice;
                cust[TOM_MERDE+UI_tombe] = rnd(99)+1;
                playsound(S_CAISSEENREGISTREUSE);
                return;
            }
        case 3:
            if(customsdance[UI_voix].danceprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, "\f3Cette voix est trop chère pour toi !"); playsound(S_ERROR); return;}
            else if(cust[VOI_CORTEX+UI_voix]>0) {conoutf(CON_GAMEINFO, "\f3Vous possédez déjà cette voix !"); playsound(S_ERROR); return;}
            else
            {
                conoutf(CON_GAMEINFO, "\f9Voix \"%s\" achetée !", customsdance[UI_voix].dancename);
                stat[STAT_CC] = stat[STAT_CC]-customsdance[UI_voix].danceprice;
                cust[VOI_CORTEX+UI_voix] = rnd(99)+1;
                playsound(S_CAISSEENREGISTREUSE);
                return;
            }
    }
}
ICOMMAND(buyitem, "i", (int *itemtype), buyitem(*itemtype));

