#include "engine.h"
#include "customs.h"
#include "sound.h"

ICOMMAND(updateui, "", (), execfile("config/ui/ui.cfg"));

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

VARP(veryfirstlaunch, 0, 1, 1);
static int lastmainmenu = -1;
ICOMMAND(usingsteam, "", (), intret(IS_USING_STEAM));

void menuprocess()
{
    if(lastmainmenu != mainmenu)
    {
        lastmainmenu = mainmenu;
        execident("mainmenutoggled");
    }
    if(islaunching) playMusic(language == 2 ? S_MAINMENURU : S_MAINMENU);
    if(mainmenu && !isconnected(true) && !UI::hascursor())
    {
        if(veryfirstlaunch)
        {
            if(IS_USING_STEAM) {getsteamname();}
            else execute("createNickname $TRUE");
            UI::showui("firstlaunch");
        }
        else
        {
            UI::showui("main");
            UI::hideui("hud");
        }
    }
    else if (isconnected() && !UI::uivisible("main") && !UI::uivisible("settings") && !UI::uivisible("classes") && !UI::uivisible("hud")) UI::showui("hud");
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

