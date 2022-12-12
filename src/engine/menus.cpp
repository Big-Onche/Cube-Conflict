#include "engine.h"
#include "steam_api.h"
#include "ccheader.h"
#include "customisation.h"

ICOMMAND(getgravedir, "i", (int *graveID), result(customstombes[*graveID].tombemenudir));

void notifywelcome()
{
    UI::hideui("servers");
}

struct change
{
    int type;
    const char *desc;

    change() {}
    change(int type, const char *desc) : type(type), desc(desc) {}
};
static vector<change> needsapply;

VARP(applydialog, 0, 1, 1);

void addchange(const char *desc, int type)
{
    if(!applydialog) return;
    loopv(needsapply) if(!strcmp(needsapply[i].desc, desc)) return;
    needsapply.add(change(type, desc));
}

void clearchanges(int type)
{
    loopvrev(needsapply)
    {
        change &c = needsapply[i];
        if(c.type&type)
        {
            c.type &= ~type;
            if(!c.type) needsapply.remove(i);
        }
    }
    if(needsapply.empty()) UI::hideui("changes");
}

ICOMMAND(clearchanges, "", (), clearchanges(CHANGE_GFX|CHANGE_SHADERS|CHANGE_SOUND));

void applychanges()
{
    int changetypes = 0;
    loopv(needsapply) changetypes |= needsapply[i].type;
    if(changetypes&CHANGE_GFX) execident("resetgl");
    else if(changetypes&CHANGE_SHADERS) execident("resetshaders");
    if(changetypes&CHANGE_SOUND) execident("resetsound");
}

COMMAND(applychanges, "");
ICOMMAND(pendingchanges, "b", (int *idx), { if(needsapply.inrange(*idx)) result(needsapply[*idx].desc); else if(*idx < 0) intret(needsapply.length()); });

bool UI_PLAYMUSIC = true;
VARP(veryfirstlaunch, 0, 1, 1);
static int lastmainmenu = -1;

void menuprocess()
{
    if(IS_USING_STEAM && strcasecmp(SteamFriends()->GetPersonaName(), game::player1->name)==0) UI_showsteamnamebtn = 0;
    if(lastmainmenu != mainmenu)
    {
        lastmainmenu = mainmenu;
        execident("mainmenutoggled");
    }
    if(UI_PLAYMUSIC) {musicmanager(0); UI_PLAYMUSIC = false;}
    if(mainmenu && !isconnected(true) && !UI::hascursor())
    {
        if(veryfirstlaunch)
        {
            if(IS_USING_STEAM) {getsteamname(); UI_showsteamnamebtn = 0;}
            UI::showui("firstlaunch");
            veryfirstlaunch = 0;
        }
        else UI::showui("main");
    }
}

VAR(mainmenu, 1, 1, 0);

void clearmainmenu()
{
    if(mainmenu && isconnected())
    {
        mainmenu = 0;
        UI::hideui(NULL);
    }
}

