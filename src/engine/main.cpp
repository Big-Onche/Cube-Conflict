// main.cpp: initialisation & main loop

#include "engine.h"
#include "cubedef.h"
#include "steam_api.h"

#ifdef SDL_VIDEO_DRIVER_X11
#include "SDL_syswm.h"
#endif

extern void cleargamma();

void cleanup()
{
    recorder::stop();
    cleanupserver();
    SDL_ShowCursor(SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    if(screen) SDL_SetWindowGrab(screen, SDL_FALSE);
    cleargamma();
    freeocta(worldroot);
    UI::cleanup();
    extern void clear_command(); clear_command();
    extern void clear_console(); clear_console();
    extern void clear_models();  clear_models();
    extern void clear_sound();   clear_sound();
    closelogfile();
    #ifdef __APPLE__
        if(screen) SDL_SetWindowFullscreen(screen, 0);
    #endif
    SDL_Quit();
}

extern void writeinitcfg();

void quit(bool savecfgs = true)                      // normal exit
{
    if(savecfgs)
    {
        writesave();
        writeinitcfg();
        writeservercfg();
        writecfg();
    }

    abortconnect();
    disconnect();
    localdisconnect();
    if(usesteam) SteamAPI_Shutdown();
    cleanup();
    exit(EXIT_SUCCESS);
}

void fatal(const char *s, ...)    // failure exit
{
    static int errors = 0;
    errors++;

    if(errors <= 2) // print up to one extra recursive error
    {
        defvformatstring(msg,s,s);
        logoutf("%s", msg);

        if(errors <= 1) // avoid recursion
        {
            if(SDL_WasInit(SDL_INIT_VIDEO))
            {
                SDL_ShowCursor(SDL_TRUE);
                SDL_SetRelativeMouseMode(SDL_FALSE);
                if(screen) SDL_SetWindowGrab(screen, SDL_FALSE);
                cleargamma();
                #ifdef __APPLE__
                    if(screen) SDL_SetWindowFullscreen(screen, 0);
                #endif
            }
            SDL_Quit();
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Cube Conflict fatal error", msg, NULL);
        }
    }

    exit(EXIT_FAILURE);
}

VAR(desktopw, 1, 0, 0);
VAR(desktoph, 1, 0, 0);
int screenw = 0, screenh = 0;
SDL_Window *screen = NULL;
SDL_GLContext glcontext = NULL;

int curtime = 0, lastmillis = 20000, elapsedtime = 0, totalmillis = 1;

dynent *player = NULL;

int initing = NOT_INITING;

bool initwarning(const char *desc, int level, int type)
{
    if(initing < level)
    {
        addchange(desc, type);
        return true;
    }
    return false;
}

#define SCR_MINW 320
#define SCR_MINH 200
#define SCR_MAXW 10000
#define SCR_MAXH 10000
#define SCR_DEFAULTW 1280
#define SCR_DEFAULTH 720
VARFN(screenw, scr_w, SCR_MINW, -1, SCR_MAXW, initwarning(langage ? "Screen resolution" : "Résolution d'écran"));
VARFN(screenh, scr_h, SCR_MINH, -1, SCR_MAXH, initwarning(langage ? "Screen resolution" : "Résolution d'écran"));

void writeinitcfg()
{
    stream *f = openutf8file("config/init.cfg", "w");
    if(!f) return;
    f->printf("// automatically written on exit, DO NOT MODIFY\n// modify settings in game\n");
    extern int fullscreen;
    f->printf("fullscreen %d\n", fullscreen);
    f->printf("screenw %d\n", scr_w);
    f->printf("screenh %d\n", scr_h);
    extern int sound, soundchans, soundfreq, soundbufferlen;
    extern char *audiodriver;
    f->printf("sound %d\n", sound);
    f->printf("soundchans %d\n", soundchans);
    f->printf("soundfreq %d\n", soundfreq);
    f->printf("soundbufferlen %d\n", soundbufferlen);
    if(audiodriver[0]) f->printf("audiodriver %s\n", escapestring(audiodriver));
    delete f;
}

COMMAND(quit, "");

static void getbackgroundres(int &w, int &h)
{
    float wk = 1, hk = 1;
    if(w < 1024) wk = 1024.0f/w;
    if(h < 768) hk = 768.0f/h;
    wk = hk = max(wk, hk);
    w = int(ceil(w*wk));
    h = int(ceil(h*hk));
}

string backgroundcaption = "";
Texture *backgroundmapshot = NULL;
string backgroundmapname = "";
char *backgroundmapinfo = NULL;
char *backgroundastuce = NULL;

void bgquad(float x, float y, float w, float h, float tx, float ty, float tw, float th)
{
    gle::begin(GL_TRIANGLE_STRIP);
    gle::attribf(x,   y);   gle::attribf(tx,      ty);
    gle::attribf(x+w, y);   gle::attribf(tx + tw, ty);
    gle::attribf(x,   y+h); gle::attribf(tx,      ty + th);
    gle::attribf(x+w, y+h); gle::attribf(tx + tw, ty + th);
    gle::end();
}

string backgroundimg = "media/interface/background.png", backgroundname;

void renderbackgroundview(int w, int h, const char *caption, Texture *mapshot, const char *mapname, const char *mapinfo, const char *astuce, bool force = false)
{
    static int lastupdate = -1, lastw = -1, lasth = -1;
    bool needlogo = true;

    if((renderedframe && !mainmenu && lastupdate != lastmillis) || lastw != w || lasth != h)
    {
        lastupdate = lastmillis;
        lastw = w;
        lasth = h;
    }
    else if(lastupdate != lastmillis) lastupdate = lastmillis;

    hudmatrix.ortho(0, w, h, 0, -1, 1);
    resethudmatrix();
    resethudshader();

    gle::defvertex(2);
    gle::deftexcoord0();

    settexture(backgroundimg, 0);
    bgquad(0, 0, w, h);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if(mapshot || mapname)
    {
        needlogo = false;

        float lh = 0.5f*min(w, h), lw = lh,
              lx = 0.5f*(w - lw), ly = 0.5f*(h - lh);

        defformatstring(mshot,"media/map/%s.png", mapname);
        copystring(backgroundimg, mshot);

        settexture("media/interface/chargement.png", 3);
        bgquad(lx, ly, lw, lh);

        settexture("media/interface/shadow.png", 3);
        bgquad(0, 0, w, h);

        float infowidth = 17.5f*FONTH;

        if(mapname)
        {
            int tw = text_width(mapname);
            float tsz = 0.04f*min(screenw, screenh)/70,
                  tx = 0.5f*(screenw - tw*tsz), ty = screenh - 0.075f*8.0f*min(screenw, screenh) - 70*tsz;
            pushhudmatrix();
            hudmatrix.translate(tx, ty, 0);
            hudmatrix.scale(tsz, tsz, 1);
            flushhudmatrix();
            draw_text(mapname, 0, 0, 0x01, 0x01, 0x01, 0xFF, -1);
            pophudmatrix();
        }
        if(astuce)
        {
            int tw = infowidth+2;
            float tsz = 0.04f*min(screenw, screenh)/100,
                  tx = 0.5f*(screenw - tw*tsz), ty = screenh - 0.075f*7.3f*min(screenw, screenh) - 100*tsz;
            pushhudmatrix();
            hudmatrix.translate(tx, ty, 0);
            hudmatrix.scale(tsz, tsz, 1);
            flushhudmatrix();
            draw_text(astuce, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, -1, infowidth+2);
            pophudmatrix();
        }
    }
    else
    {
        if(force) //CubeConflict
        {
            needlogo = false;

            float ilh = 1.1f*min(w, h), ilw = ilh*1.8f,
                  ilx = 0.5f*(w - ilw), ily = 0.5f*(h - ilh);

            switch(n_map)
            {
                case 0: formatstring(backgroundimg, "media/map/village.png"); break;
                case 1: formatstring(backgroundimg, "media/map/usine.png"); break;
                case 2: formatstring(backgroundimg, "media/map/chateaux.png"); break;
                case 3: formatstring(backgroundimg, "media/map/lune.png"); break;
                case 4: formatstring(backgroundimg, "media/map/volcan.png"); break;
                default: formatstring(backgroundimg, "media/interface/background.png"); break;
            }

            settexture(backgroundimg);
            bgquad(ilx-parallaxX/-40, ily-parallaxY/-40, ilw, ilh);

            float lh = 0.43f*min(w, h), lw = lh*2,
                  lx = 0.5f*(w - lw), ly = 0.5f*(h*0.5f - lh);

            settexture((maxtexsize ? min(maxtexsize, hwtexsize) : hwtexsize) >= 1024 && (hudw > 1280 || hudh > 800) ? "<premul>media/interface/logo_1024.png" : "<premul>media/interface/logo.png", 3);
            bgquad(lx, ly, lw, lh);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }

    if(needlogo)
    {
        float jlh = 0.5f*min(w, h), jlw = jlh,
              jlx = 0.5f*(w - jlw), jly = 0.5f*(h - jlh);

        settexture("media/interface/intrologo.png", 3);
        bgquad(jlx, jly, jlw, jlh);
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(!needlogo) gle::colorf(1, 1, 1, 1);
    settexture("media/interface/shadow.png", 3);
    bgquad(0, 0, w, h);



    glDisable(GL_BLEND);
}

VAR(menumute, 0, 0, 1);

void renderbackground(const char *caption, Texture *mapshot, const char *mapname, const char *mapinfo, const char *astuce, bool force)
{
    if(!inbetweenframes && !force) return;

    if(menumute) stopsounds(); // stop sounds while loading

    int w = hudw, h = hudh;
    if(forceaspect) w = int(ceil(h*forceaspect));
    getbackgroundres(w, h);
    gettextres(w, h);

    if(force)
    {
        renderbackgroundview(w, h, caption, mapshot, mapname, mapinfo, astuce, true);
        return;
    }

    loopi(3)
    {
        renderbackgroundview(w, h, caption, mapshot, mapname, mapinfo, astuce);
        swapbuffers(false);
    }

    renderedframe = false;
    copystring(backgroundcaption, caption ? caption : "");
    backgroundmapshot = mapshot;
    copystring(backgroundmapname, mapname ? mapname : "");
    if(mapinfo != backgroundmapinfo)
    {
        DELETEA(backgroundmapinfo);
        if(mapinfo) backgroundmapinfo = newstring(mapinfo);
    }
    if(astuce != backgroundastuce)
    {
        DELETEA(backgroundastuce);
        if(astuce) backgroundastuce = newstring(astuce);
    }
}

void restorebackground(int w, int h, bool force)
{
    if(renderedframe && !force) return;
    renderbackgroundview(w, h, backgroundcaption[0] ? backgroundcaption : NULL, backgroundmapshot, backgroundmapname[0] ? backgroundmapname : NULL, backgroundmapinfo, backgroundastuce);
}

float loadprogress = 0;
int nbtexte = rnd(11);
int textetimer, pointstimer = 0;

static const struct loadingtextinfo { const char *loadingtext_FR, *loadingtext_EN; } loadingtext[] =
{
    {"élaboration de théories complotistes",                     "sharing conspiracy theories"},
    {"remise des lunettes rondes aux golems",                    "putting five FFP2 masks on soyjak"},
    {"invocation du Gigachad",                                   "summoning Gigachad"},
    {"mise en place du crédit social",                           "setting social credit"},
    {"swatting des streamers",                                   "swatting streamers"},
    {"minage de bitcoins",                                       "mining some bitcoins"},
    {"mise en place des bons pixels aux bons endroits",          "putting the right pixels in the right places"},
    {"chargement des textures (non je déconne il n'y en a pas)", "loading textures (just kidding, there are no textures)"},
    {"truquage des élections présidentielles",                   "rigging presidential elections"},
    {"augmentation des dégâts des armes cheatées",               "buffing the most powerful weapons"},
    {"réduction des dégâts des armes les plus nulles",           "nerfing the most pointless weapons"},
    {"ERREUR 410 :D",                                            "Connection lost | Please wait - attempting to reestablish"},
    {"injection de la 5ème dose de rappel",                      "injection of the fifth booster dose"},
};

void renderprogressview(int w, int h, float bar, const char *text)   // also used during loading
{
    hudmatrix.ortho(0, w, h, 0, -1, 1);
    resethudmatrix();
    resethudshader();

    gle::defvertex(2);
    gle::deftexcoord0();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float fh = 0.060f*min(w, h), fw = fh*15,
          fx = renderedframe ? w - fw - fh/4 : 0,
          fy = renderedframe ? fh/4 : h - fh;
    settexture("media/interface/loading_frame.png", 3);
    bgquad(fx, fy, fw*2, fh);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float bw = fw*(512 - 2*8)/512.0f, bh = fh*20/32.0f,
          bx = fx + fw*8/512.0f, by = fy + fh*6/32.0f,
          su1 = 0/32.0f, su2 = 8/32.0f, sw = fw*8/512.0f,
          eu1 = 24/32.0f, eu2 = 32/32.0f, ew = fw*8/512.0f,
          mw = bw - sw - ew,
          ex = bx+sw + max(mw*bar, fw*8/512.0f);
    if(bar > 0)
    {
        settexture("media/interface/loading_bar.png", 3);
        bgquad(bx-50, by*1.04f, sw*2.1f, bh, su1, 0, su2-su1, 1);
        bgquad(bx+sw-50, by*1.04f, (ex-(bx+sw))*2.1f, bh, su2, 0, eu1-su2, 1);
        bgquad(ex-50, by*1.04f, ew*2.1f, bh, eu1, 0, eu2-eu1, 1);
    }

    if(text)
    {
        int tw = text_width(text);
        float tsz = bh*0.6f/FONTH;
        if(tw*tsz > mw) tsz = mw/tw;

        pushhudtranslate(bx+sw, by + (bh - FONTH*tsz)/2, tsz);

        textetimer += curtime;
        pointstimer += curtime;

        if(textetimer>3000) {nbtexte = rnd(13); textetimer = 0;}
        if(pointstimer>1000) pointstimer = 0;

        defformatstring(petitpoints, "%s", pointstimer < 250 ? "..." : pointstimer < 500 ? "" : pointstimer < 750 ? "." : "..");
        defformatstring(text, "%s%s", langage ? loadingtext[nbtexte].loadingtext_EN : loadingtext[nbtexte].loadingtext_FR, petitpoints);

        draw_text(text, 0, 0);
        pophudmatrix();
    }

    glDisable(GL_BLEND);
}

VAR(progressbackground, 0, 0, 1);

void renderprogress(float bar, const char *text, bool background)   // also used during loading
{
    if(!inbetweenframes || drawtex) return;

    extern int menufps, maxfps;
    int fps = menufps ? (maxfps ? min(maxfps, menufps) : menufps) : maxfps;
    if(fps)
    {
        static int lastprogress = 0;
        int ticks = SDL_GetTicks(), diff = ticks - lastprogress;
        if(bar > 0 && diff >= 0 && diff < (1000 + fps-1)/fps) return;
        lastprogress = ticks;
    }

    clientkeepalive();      // make sure our connection doesn't time out while loading maps etc.

    SDL_PumpEvents(); // keep the event queue awake to avoid 'beachball' cursor

    int w = hudw, h = hudh;
    if(forceaspect) w = int(ceil(h*forceaspect));
    getbackgroundres(w, h);
    gettextres(w, h);

    extern int mesa_swap_bug, curvsync;
    bool forcebackground = progressbackground || (mesa_swap_bug && (curvsync || totalmillis==1));
    if(background || forcebackground) restorebackground(w, h, forcebackground);

    renderprogressview(w, h, bar, text);
    swapbuffers(false);
}

#ifdef WIN32
// SDL_WarpMouseInWindow behaves erratically on Windows, so force relative mouse instead.
VARN(relativemouse, userelativemouse, 1, 1, 0);
#else
VARNP(relativemouse, userelativemouse, 0, 1, 1);
#endif

bool shouldgrab = false, grabinput = false, minimized = false, canrelativemouse = true, relativemouse = false;
int keyrepeatmask = 0, textinputmask = 0;
Uint32 textinputtime = 0;
VAR(textinputfilter, 0, 5, 1000);

void keyrepeat(bool on, int mask)
{
    if(on) keyrepeatmask |= mask;
    else keyrepeatmask &= ~mask;
}

void textinput(bool on, int mask)
{
    if(on)
    {
        if(!textinputmask)
        {
            SDL_StartTextInput();
            textinputtime = SDL_GetTicks();
        }
        textinputmask |= mask;
    }
    else if(textinputmask)
    {
        textinputmask &= ~mask;
        if(!textinputmask) SDL_StopTextInput();
    }
}

#ifdef SDL_VIDEO_DRIVER_X11
VAR(sdl_xgrab_bug, 0, 0, 1);
#endif

void inputgrab(bool on, bool delay = false)
{
#ifdef SDL_VIDEO_DRIVER_X11
    bool wasrelativemouse = relativemouse;
#endif
    if(on)
    {
        SDL_ShowCursor(SDL_FALSE);
        if(canrelativemouse && userelativemouse)
        {
            if(SDL_SetRelativeMouseMode(SDL_TRUE) >= 0)
            {
                SDL_SetWindowGrab(screen, SDL_TRUE);
                relativemouse = true;
            }
            else
            {
                SDL_SetWindowGrab(screen, SDL_FALSE);
                canrelativemouse = false;
                relativemouse = false;
            }
        }
    }
    else
    {
        SDL_ShowCursor(SDL_TRUE);
        if(relativemouse)
        {
            SDL_SetWindowGrab(screen, SDL_FALSE);
            SDL_SetRelativeMouseMode(SDL_FALSE);
            relativemouse = false;
        }
    }
    shouldgrab = delay;

#ifdef SDL_VIDEO_DRIVER_X11
    if((relativemouse || wasrelativemouse) && sdl_xgrab_bug)
    {
        // Workaround for buggy SDL X11 pointer grabbing
        union { SDL_SysWMinfo info; uchar buf[sizeof(SDL_SysWMinfo) + 128]; };
        SDL_GetVersion(&info.version);
        if(SDL_GetWindowWMInfo(screen, &info) && info.subsystem == SDL_SYSWM_X11)
        {
            if(relativemouse)
            {
                uint mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask;
                XGrabPointer(info.info.x11.display, info.info.x11.window, True, mask, GrabModeAsync, GrabModeAsync, info.info.x11.window, None, CurrentTime);
            }
            else XUngrabPointer(info.info.x11.display, CurrentTime);
        }
    }
#endif
}

bool initwindowpos = false;

void setfullscreen(bool enable)
{
    if(!screen) return;
    //initwarning(enable ? "fullscreen" : "windowed");
    SDL_SetWindowFullscreen(screen, enable ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    if(!enable)
    {
        SDL_SetWindowSize(screen, scr_w, scr_h);
        if(initwindowpos)
        {
            int winx = SDL_WINDOWPOS_CENTERED, winy = SDL_WINDOWPOS_CENTERED;
            SDL_SetWindowPosition(screen, winx, winy);
            initwindowpos = false;
        }
    }
}

#ifdef _DEBUG
VARF(fullscreen, 0, 0, 1, setfullscreen(fullscreen!=0));
#else
VARF(fullscreen, 0, 1, 1, setfullscreen(fullscreen!=0));
#endif

void screenres(int w, int h)
{
    scr_w = clamp(w, SCR_MINW, SCR_MAXW);
    scr_h = clamp(h, SCR_MINH, SCR_MAXH);
    if(screen)
    {
        scr_w = min(scr_w, desktopw);
        scr_h = min(scr_h, desktoph);
        if(SDL_GetWindowFlags(screen) & SDL_WINDOW_FULLSCREEN) gl_resize();
        else SDL_SetWindowSize(screen, scr_w, scr_h);
    }
    else
    {
        initwarning(langage ? "Screen resolution" : "Résolution d'écran");
    }
}

ICOMMAND(screenres, "ii", (int *w, int *h), screenres(*w, *h));

static void setgamma(int val)
{
    if(screen && SDL_SetWindowBrightness(screen, val/100.0f) < 0) conoutf(CON_ERROR, "Could not set gamma: %s", SDL_GetError());
}

static int curgamma = 100;
VARFNP(gamma, reqgamma, 30, 100, 300,
{
    if(initing || reqgamma == curgamma) return;
    curgamma = reqgamma;
    setgamma(curgamma);
});

void restoregamma()
{
    if(initing || reqgamma == 100) return;
    curgamma = reqgamma;
    setgamma(curgamma);
}

void cleargamma()
{
    if(curgamma != 100 && screen) SDL_SetWindowBrightness(screen, 1.0f);
}

int curvsync = -1;
void restorevsync()
{
    if(initing || !glcontext) return;
    extern int vsync, vsynctear;
    if(!SDL_GL_SetSwapInterval(vsync ? (vsynctear ? -1 : 1) : 0))
        curvsync = vsync;
}

VARFP(vsync, 0, 0, 1, restorevsync());
VARFP(vsynctear, 0, 0, 1, { if(vsync) restorevsync(); });

VAR(dbgmodes, 0, 0, 1);

void setupscreen()
{
    if(glcontext)
    {
        SDL_GL_DeleteContext(glcontext);
        glcontext = NULL;
    }
    if(screen)
    {
        SDL_DestroyWindow(screen);
        screen = NULL;
    }
    curvsync = -1;

    SDL_Rect desktop;
    if(SDL_GetDisplayBounds(0, &desktop) < 0) fatal("failed querying desktop bounds: %s", SDL_GetError());
    desktopw = desktop.w;
    desktoph = desktop.h;

    if(scr_h < 0) scr_h = SCR_DEFAULTH;
    if(scr_w < 0) scr_w = (scr_h*desktopw)/desktoph;
    scr_w = min(scr_w, desktopw);
    scr_h = min(scr_h, desktoph);

    int winx = SDL_WINDOWPOS_UNDEFINED, winy = SDL_WINDOWPOS_UNDEFINED, winw = scr_w, winh = scr_h, flags = SDL_WINDOW_RESIZABLE;
    if(fullscreen)
    {
        winw = desktopw;
        winh = desktoph;
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        initwindowpos = true;
    }

    SDL_GL_ResetAttributes();
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    screen = SDL_CreateWindow("Cube Conflict", winx, winy, winw, winh, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | flags);
    if(!screen) fatal("failed to create OpenGL window: %s", SDL_GetError());

    SDL_SetWindowMinimumSize(screen, SCR_MINW, SCR_MINH);
    SDL_SetWindowMaximumSize(screen, SCR_MAXW, SCR_MAXH);

#ifdef __APPLE__
    static const int glversions[] = { 32, 20 };
#else
    static const int glversions[] = { 40, 33, 32, 31, 30, 20 };
#endif
    loopi(sizeof(glversions)/sizeof(glversions[0]))
    {
        glcompat = glversions[i] <= 30 ? 1 : 0;
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glversions[i] / 10);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glversions[i] % 10);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, glversions[i] >= 32 ? SDL_GL_CONTEXT_PROFILE_CORE : 0);
        glcontext = SDL_GL_CreateContext(screen);
        if(glcontext) break;
    }
    if(!glcontext) fatal("failed to create OpenGL context: %s", SDL_GetError());

    SDL_GetWindowSize(screen, &screenw, &screenh);
    renderw = min(scr_w, screenw);
    renderh = min(scr_h, screenh);
    hudw = screenw;
    hudh = screenh;
}

void resetgl()
{
    clearchanges(CHANGE_GFX|CHANGE_SHADERS);

    renderbackground("resetting OpenGL");

    recorder::cleanup();
    cleanupva();
    cleanupparticles();
    cleanupstains();
    cleanupsky();
    cleanupmodels();
    cleanupprefabs();
    cleanuptextures();
    cleanupblendmap();
    cleanuplights();
    cleanupshaders();
    cleanupgl();

    setupscreen();

    inputgrab(grabinput);

    gl_init();

    inbetweenframes = false;
    if(!reloadtexture(*notexture) ||
       !reloadtexture("<premul>media/interface/logo.png") ||
       !reloadtexture("<premul>media/interface/logo_1024.png") ||
       !reloadtexture("media/interface/shadow.png") ||
       !reloadtexture("media/interface/mapshot_frame.png") ||
       !reloadtexture("media/interface/loading_frame.png") ||
       !reloadtexture("media/interface/loading_bar.png") ||
       !reloadtexture("media/interface/backgroundimg.png") ||
       !reloadtexture("media/interface/chargement.png"))
        fatal("failed to reload core texture");
    reloadfonts();
    inbetweenframes = true;
    renderbackground(langage ? "Loading..." : "Chargement...");
    restoregamma();
    restorevsync();
    initgbuffer();
    reloadshaders();
    reloadtextures();
    allchanged(true);
}

COMMAND(resetgl, "");

static queue<SDL_Event, 32> events;

static inline bool filterevent(const SDL_Event &event)
{
    switch(event.type)
    {
        case SDL_MOUSEMOTION:
            if(grabinput && !relativemouse && !(SDL_GetWindowFlags(screen) & SDL_WINDOW_FULLSCREEN))
            {
                if(event.motion.x == screenw / 2 && event.motion.y == screenh / 2)
                    return false;  // ignore any motion events generated by SDL_WarpMouse
                #ifdef __APPLE__
                if(event.motion.y == 0)
                    return false;  // let mac users drag windows via the title bar
                #endif
            }
            break;
    }
    return true;
}

template <int SIZE> static inline bool pumpevents(queue<SDL_Event, SIZE> &events)
{
    while(events.empty())
    {
        SDL_PumpEvents();
        databuf<SDL_Event> buf = events.reserve(events.capacity());
        int n = SDL_PeepEvents(buf.getbuf(), buf.remaining(), SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
        if(n <= 0) return false;
        loopi(n) if(filterevent(buf.buf[i])) buf.put(buf.buf[i]);
        events.addbuf(buf);
    }
    return true;
}

static int interceptkeysym = 0;

static int interceptevents(void *data, SDL_Event *event)
{
    switch(event->type)
    {
        case SDL_MOUSEMOTION: return 0;
        case SDL_KEYDOWN:
            if(event->key.keysym.sym == interceptkeysym)
            {
                interceptkeysym = -interceptkeysym;
                return 0;
            }
            break;
    }
    return 1;
}

static void clearinterceptkey()
{
    SDL_DelEventWatch(interceptevents, NULL);
    interceptkeysym = 0;
}

bool interceptkey(int sym)
{
    if(!interceptkeysym)
    {
        interceptkeysym = sym;
        SDL_FilterEvents(interceptevents, NULL);
        if(interceptkeysym < 0)
        {
            interceptkeysym = 0;
            return true;
        }
        SDL_AddEventWatch(interceptevents, NULL);
    }
    else if(abs(interceptkeysym) != sym) interceptkeysym = sym;
    SDL_PumpEvents();
    if(interceptkeysym < 0)
    {
        clearinterceptkey();
        interceptkeysym = sym;
        SDL_FilterEvents(interceptevents, NULL);
        interceptkeysym = 0;
        return true;
    }
    return false;
}

static void ignoremousemotion()
{
    SDL_PumpEvents();
    SDL_FlushEvent(SDL_MOUSEMOTION);
}

static void resetmousemotion()
{
    if(grabinput && !relativemouse && !(SDL_GetWindowFlags(screen) & SDL_WINDOW_FULLSCREEN))
    {
        SDL_WarpMouseInWindow(screen, screenw / 2, screenh / 2);
    }
}

static void checkmousemotion(int &dx, int &dy)
{
    while(pumpevents(events))
    {
        SDL_Event &event = events.removing();
        if(event.type != SDL_MOUSEMOTION) return;
        dx += event.motion.xrel;
        dy += event.motion.yrel;
        events.remove();
    }
}

int parallaxX, parallaxY;

void checkinput()
{
    if(interceptkeysym) clearinterceptkey();
    //int lasttype = 0, lastbut = 0;
    bool mousemoved = false;
    int focused = 0;
    while(pumpevents(events))
    {
        SDL_Event &event = events.remove();

        if(focused && event.type!=SDL_WINDOWEVENT) { if(grabinput != (focused>0)) inputgrab(grabinput = focused>0, shouldgrab); focused = 0; }

        switch(event.type)
        {
            case SDL_QUIT:
                quit();
                return;

            case SDL_TEXTINPUT:
                if(textinputmask && int(event.text.timestamp-textinputtime) >= textinputfilter)
                {
                    uchar buf[SDL_TEXTINPUTEVENT_TEXT_SIZE+1];
                    size_t len = decodeutf8(buf, sizeof(buf)-1, (const uchar *)event.text.text, strlen(event.text.text));
                    if(len > 0) { buf[len] = '\0'; processtextinput((const char *)buf, len); }
                }
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if(keyrepeatmask || !event.key.repeat)
                    processkey(event.key.keysym.sym, event.key.state==SDL_PRESSED, event.key.keysym.mod | SDL_GetModState());
                break;

            case SDL_WINDOWEVENT:
                switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_CLOSE:
                        quit();
                        break;

                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        shouldgrab = true;
                        break;
                    case SDL_WINDOWEVENT_ENTER:
                        shouldgrab = false;
                        focused = 1;
                        break;

                    case SDL_WINDOWEVENT_LEAVE:
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        shouldgrab = false;
                        focused = -1;
                        break;

                    case SDL_WINDOWEVENT_MINIMIZED:
                        minimized = true;
                        break;

                    case SDL_WINDOWEVENT_MAXIMIZED:
                    case SDL_WINDOWEVENT_RESTORED:
                        minimized = false;
                        break;

                    case SDL_WINDOWEVENT_RESIZED:
                        break;

                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        SDL_GetWindowSize(screen, &screenw, &screenh);
                        if(!(SDL_GetWindowFlags(screen) & SDL_WINDOW_FULLSCREEN))
                        {
                            scr_w = clamp(screenw, SCR_MINW, SCR_MAXW);
                            scr_h = clamp(screenh, SCR_MINH, SCR_MAXH);
                        }
                        gl_resize();
                        break;
                }
                break;

            case SDL_MOUSEMOTION:
                if(grabinput)
                {
                    int dx = event.motion.xrel, dy = event.motion.yrel;
                    checkmousemotion(dx, dy);
                    if(!UI::movecursor(dx, dy)) mousemove(dx, dy);

                    if(parallaxX>=-700 && parallaxX<=700) parallaxX = parallaxX+dx; //CubeConflict
                    else parallaxX < 0 ? parallaxX = -700 : parallaxX = 700;

                    if(parallaxY>=-430 && parallaxY<=430) parallaxY = parallaxY+dy;
                    else parallaxY < 0 ? parallaxY = -430 : parallaxY = 430;

                    mousemoved = true;
                }
                else if(shouldgrab) inputgrab(grabinput = true);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                //if(lasttype==event.type && lastbut==event.button.button) break; // why?? get event twice without it
                switch(event.button.button)
                {
                    case SDL_BUTTON_LEFT: processkey(-1, event.button.state==SDL_PRESSED); break;
                    case SDL_BUTTON_MIDDLE: processkey(-2, event.button.state==SDL_PRESSED); break;
                    case SDL_BUTTON_RIGHT: processkey(-3, event.button.state==SDL_PRESSED); break;
                    case SDL_BUTTON_X1: processkey(-6, event.button.state==SDL_PRESSED); break;
                    case SDL_BUTTON_X2: processkey(-7, event.button.state==SDL_PRESSED); break;
                }
                //lasttype = event.type;
                //lastbut = event.button.button;
                break;

            case SDL_MOUSEWHEEL:
                if(event.wheel.y > 0) { processkey(-4, true); processkey(-4, false); }
                else if(event.wheel.y < 0) { processkey(-5, true); processkey(-5, false); }
                break;
        }
    }
    if(focused) { if(grabinput != (focused>0)) inputgrab(grabinput = focused>0, shouldgrab); focused = 0; }
    if(mousemoved) resetmousemotion();
}

void swapbuffers(bool overlay)
{
    recorder::capture(overlay);
    gle::disable();
    SDL_GL_SwapWindow(screen);
}

VAR(menufps, 0, 60, 1000);
VARP(maxfps, 0, 125, 1000);

void limitfps(int &millis, int curmillis)
{
    int limit = (mainmenu || minimized) && menufps ? (maxfps ? min(maxfps, menufps) : menufps) : maxfps;
    if(!limit) return;
    static int fpserror = 0;
    int delay = 1000/limit - (millis-curmillis);
    if(delay < 0) fpserror = 0;
    else
    {
        fpserror += 1000%limit;
        if(fpserror >= limit)
        {
            ++delay;
            fpserror -= limit;
        }
        if(delay > 0)
        {
            SDL_Delay(delay);
            millis += delay;
        }
    }
}

#ifdef WIN32
// Force Optimus setups to use the NVIDIA GPU
extern "C"
{
#ifdef __GNUC__
__attribute__((dllexport))
#else
__declspec(dllexport)
#endif
    DWORD NvOptimusEnablement = 1;

#ifdef __GNUC__
__attribute__((dllexport))
#else
__declspec(dllexport)
#endif
    DWORD AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#if defined(WIN32) && !defined(_DEBUG) && !defined(__GNUC__)
void stackdumper(unsigned int type, EXCEPTION_POINTERS *ep)
{
    if(!ep) fatal("unknown type");
    EXCEPTION_RECORD *er = ep->ExceptionRecord;
    CONTEXT *context = ep->ContextRecord;
    char out[512];
    formatstring(out, "Cube Conflict Win32 Exception: 0x%x [0x%x]\n\n", er->ExceptionCode, er->ExceptionCode==EXCEPTION_ACCESS_VIOLATION ? er->ExceptionInformation[1] : -1);
    SymInitialize(GetCurrentProcess(), NULL, TRUE);
#ifdef _AMD64_
    STACKFRAME64 sf = {{context->Rip, 0, AddrModeFlat}, {}, {context->Rbp, 0, AddrModeFlat}, {context->Rsp, 0, AddrModeFlat}, 0};
    while(::StackWalk64(IMAGE_FILE_MACHINE_AMD64, GetCurrentProcess(), GetCurrentThread(), &sf, context, NULL, ::SymFunctionTableAccess, ::SymGetModuleBase, NULL))
    {
        union { IMAGEHLP_SYMBOL64 sym; char symext[sizeof(IMAGEHLP_SYMBOL64) + sizeof(string)]; };
        sym.SizeOfStruct = sizeof(sym);
        sym.MaxNameLength = sizeof(symext) - sizeof(sym);
        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(line);
        DWORD64 symoff;
        DWORD lineoff;
        if(SymGetSymFromAddr64(GetCurrentProcess(), sf.AddrPC.Offset, &symoff, &sym) && SymGetLineFromAddr64(GetCurrentProcess(), sf.AddrPC.Offset, &lineoff, &line))
#else
    STACKFRAME sf = {{context->Eip, 0, AddrModeFlat}, {}, {context->Ebp, 0, AddrModeFlat}, {context->Esp, 0, AddrModeFlat}, 0};
    while(::StackWalk(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), &sf, context, NULL, ::SymFunctionTableAccess, ::SymGetModuleBase, NULL))
    {
        union { IMAGEHLP_SYMBOL sym; char symext[sizeof(IMAGEHLP_SYMBOL) + sizeof(string)]; };
        sym.SizeOfStruct = sizeof(sym);
        sym.MaxNameLength = sizeof(symext) - sizeof(sym);
        IMAGEHLP_LINE line;
        line.SizeOfStruct = sizeof(line);
        DWORD symoff, lineoff;
        if(SymGetSymFromAddr(GetCurrentProcess(), sf.AddrPC.Offset, &symoff, &sym) && SymGetLineFromAddr(GetCurrentProcess(), sf.AddrPC.Offset, &lineoff, &line))
#endif
        {
            char *del = strrchr(line.FileName, '\\');
            concformatstring(out, "%s - %s [%d]\n", sym.Name, del ? del + 1 : line.FileName, line.LineNumber);
        }
    }
    fatal(out);
}
#endif

#define MAXFPSHISTORY 60

int fpspos = 0, fpshistory[MAXFPSHISTORY];

void resetfpshistory()
{
    loopi(MAXFPSHISTORY) fpshistory[i] = 1;
    fpspos = 0;
}

void updatefpshistory(int millis)
{
    fpshistory[fpspos++] = max(1, min(1000, millis));
    if(fpspos>=MAXFPSHISTORY) fpspos = 0;
}

void getframemillis(float &avg, float &bestdiff, float &worstdiff)
{
    int total = fpshistory[MAXFPSHISTORY-1], best = total, worst = total;
    loopi(MAXFPSHISTORY-1)
    {
        int millis = fpshistory[i];
        total += millis;
        if(millis < best) best = millis;
        if(millis > worst) worst = millis;
    }

    avg = total/float(MAXFPSHISTORY);
    best = best - avg;
    worstdiff = avg - worst;
}

void getfps(int &fps, int &bestdiff, int &worstdiff)
{
    int total = fpshistory[MAXFPSHISTORY-1], best = total, worst = total;
    loopi(MAXFPSHISTORY-1)
    {
        int millis = fpshistory[i];
        total += millis;
        if(millis < best) best = millis;
        if(millis > worst) worst = millis;
    }

    fps = (1000*MAXFPSHISTORY)/total;
    bestdiff = 1000/best-fps;
    worstdiff = fps-1000/worst;
}

void getfps_(int *raw)
{
    if(*raw) floatret(1000.0f/fpshistory[(fpspos+MAXFPSHISTORY-1)%MAXFPSHISTORY]);
    else
    {
        int fps, bestdiff, worstdiff;
        getfps(fps, bestdiff, worstdiff);
        intret(fps);
    }
}

COMMANDN(getfps, getfps_, "i");

bool inbetweenframes = false, renderedframe = true;

static bool findarg(int argc, char **argv, const char *str)
{
    for(int i = 1; i<argc; i++) if(strstr(argv[i], str)==argv[i]) return true;
    return false;
}

static int clockrealbase = 0, clockvirtbase = 0;
static void clockreset() { clockrealbase = SDL_GetTicks(); clockvirtbase = totalmillis; }
VARFP(clockerror, 990000, 1000000, 1010000, clockreset());
VARFP(clockfix, 0, 0, 1, clockreset());

int getclockmillis()
{
    int millis = SDL_GetTicks() - clockrealbase;
    if(clockfix) millis = int(millis*(double(clockerror)/1000000));
    millis += clockvirtbase;
    return max(millis, totalmillis);
}

VAR(numcpus, 1, 1, 16);

extern void changerlangue();

bool init = true;
VARFP(langage, 0, 0, 1, changerlangue());
VAR(UI_menutabs, 0, 0, 9);

void changerlangue()
{
    if(langage) {execfile("config/keymap_EN.cfg"); execfile("config/ui_EN.cfg"); execfile("config/default_EN.cfg"); execfile("config/astuces_EN.cfg");}
    else {execfile("config/keymap_FR.cfg"); execfile("config/ui_FR.cfg"); execfile("config/default_FR.cfg"); execfile("config/astuces_FR.cfg");}

    if(!init)
    {
        game::ispaused() ? UI_menutabs = 3 : UI_menutabs = 8;
        switch(langage)
        {
            case 0: UI::showui(game::ispaused() ? "pause_fr" : "main_fr"); break;
            case 1: UI::showui(game::ispaused() ? "pause_en" : "main_en");
        }
    }
    else {UI_menutabs = 0; init = false;}
}

bool initsteam()
{
    if(!SteamAPI_Init())
    {
        conoutf(CON_WARN, langage ? "Steam API failed to start." : "API Steam non démarrée");
        return false;
    }
    else return true;
}

bool rewritelangage = false;
int newlangage;

bool usesteam = false;

int main(int argc, char **argv)
{
    #ifdef WIN32
    //atexit((void (__cdecl *)(void))_CrtDumpMemoryLeaks);
    #ifndef _DEBUG
    #ifndef __GNUC__
    __try {
    #endif
    #endif
    #endif

    setlogfile(NULL);

    int dedicated = 0;
    char *load = NULL, *initscript = NULL;

    initing = INIT_RESET;
    // set home dir first
    for(int i = 1; i<argc; i++) if(argv[i][0]=='-' && argv[i][1] == 'u') { sethomedir(&argv[i][2]); break; }
    // set log after home dir, but before anything else
    for(int i = 1; i<argc; i++) if(argv[i][0]=='-' && argv[i][1] == 'g')
    {
        const char *file = argv[i][2] ? &argv[i][2] : "log.txt";
        setlogfile(file);
        logoutf("Setting log file: %s", file);
        break;
    }
    execfile("config/init.cfg", false);
    for(int i = 1; i<argc; i++)
    {
        if(argv[i][0]=='-') switch(argv[i][1])
        {
            case 'u': if(homedir[0]) logoutf("Using home directory: %s", homedir); break;
            case 'k':
            {
                const char *dir = addpackagedir(&argv[i][2]);
                if(dir) logoutf("Adding package directory: %s", dir);
                break;
            }
            case 'g': break;
            case 'd': dedicated = atoi(&argv[i][2]); if(dedicated<=0) dedicated = 2; break;
            case 'w': scr_w = clamp(atoi(&argv[i][2]), SCR_MINW, SCR_MAXW); if(!findarg(argc, argv, "-h")) scr_h = -1; break;
            case 'h': scr_h = clamp(atoi(&argv[i][2]), SCR_MINH, SCR_MAXH); if(!findarg(argc, argv, "-w")) scr_w = -1; break;
            case 'f': fullscreen = atoi(&argv[i][2]); break;
            case 'a': langage = 0; rewritelangage = true; newlangage = 0; break;
            case 'b': langage = 1; rewritelangage = true; newlangage = 1; break;
            case 'l':
            {
                char pkgdir[] = "media/";
                load = strstr(path(&argv[i][2]), path(pkgdir));
                if(load) load += sizeof(pkgdir)-1;
                else load = &argv[i][2];
                break;
            }
            case 'x': initscript = &argv[i][2]; break;
            case 's': usesteam = true; break;
            default: if(!serveroption(argv[i])) gameargs.add(argv[i]); break;
        }
        else gameargs.add(argv[i]);
    }

    numcpus = clamp(SDL_GetCPUCount(), 1, 16);

    if(dedicated <= 1)
    {
        if(usesteam && SteamAPI_RestartAppIfNecessary(1454700)) quit(false);

        logoutf("init: sdl");

        if(SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0) fatal("Unable to initialize SDL: %s", SDL_GetError());

#ifdef SDL_VIDEO_DRIVER_X11
        SDL_version version;
        SDL_GetVersion(&version);
        if (SDL_VERSIONNUM(version.major, version.minor, version.patch) <= SDL_VERSIONNUM(2, 0, 12))
            sdl_xgrab_bug = 1;
#endif
    }

    logoutf("init: net");
    if(enet_initialize()<0) fatal("Unable to initialise network module");
    atexit(enet_deinitialize);
    enet_time_set(0);

    if(usesteam)
    {
        initsteam();
        getsteamachievements();
    }

    logoutf("init: game");
    game::parseoptions(gameargs);
    initserver(dedicated>0, dedicated>1);  // never returns if dedicated
    ASSERT(dedicated <= 1);
    game::initclient();

    logoutf("init: video");
    SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "0");
    #if !defined(WIN32) && !defined(__APPLE__)
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
    #endif
    setupscreen();
    SDL_ShowCursor(SDL_FALSE);
    SDL_StopTextInput(); // workaround for spurious text-input events getting sent on first text input toggle?

    logoutf("init: gl");
    gl_checkextensions();
    gl_init();
    notexture = textureload("media/texture/game/notexture.png");
    if(!notexture) fatal("could not find core textures");
    textureload("media/interface/hud/viseurA.png");
    textureload("media/interface/hud/viseurB.png");
    textureload("media/interface/hud/viseurC.png");
    textureload("media/map/village.png");
    textureload("media/map/usine.png");
    textureload("media/map/chateaux.png");
    textureload("media/map/lune.png");
    textureload("media/map/volcan.png");

    logoutf("init: console");
    if(!execfile("config/stdlib.cfg", false)) fatal("cannot find data files (you are running from the wrong folder, try .bat file in the main folder)");   // this is the first file we load.
    if(!execfile("config/font.cfg", false)) fatal("cannot find font definitions");
    if(!setfont("default")) fatal("no default font specified");

    UI::setup();

    inbetweenframes = true;
    renderbackground("");

    logoutf("init: world");
    camera1 = player = game::iterdynents(0);
    emptymap(0, true, NULL, false);

    logoutf("init: sound");
    initsound();

    logoutf("init: cfg");
    initing = INIT_LOAD;

    switch(newlangage)
    {
        case 0: execfile("config/keymap_FR.cfg"); break;
        case 1: execfile("config/keymap_EN.cfg");
    }

    execfile("config/stdedit.cfg");
    execfile(game::gameconfig());
    execfile("config/sound.cfg");
    execfile("config/heightmap.cfg");
    execfile("config/blendbrush.cfg");
    if(game::savedservers()) execfile(game::savedservers(), false);
    loadsave();

    identflags |= IDF_PERSIST;

    game::player1->playermodel = 0;

    if(!execfile(game::savedconfig(), false))
    {
        execfile(newlangage ? "config/default_EN.cfg" : "config/default_FR.cfg");
        writecfg(game::restoreconfig());
    }
    execfile(game::autoexec(), false);

    if(rewritelangage) langage = newlangage;

    switch(langage)
    {
        case 0: if(rewritelangage)execfile("config/keymap_FR.cfg"); execfile("config/ui_FR.cfg"); execfile("config/default_FR.cfg"); execfile("config/astuces_FR.cfg"); break;
        case 1: if(rewritelangage)execfile("config/keymap_EN.cfg"); execfile("config/ui_EN.cfg"); execfile("config/default_EN.cfg"); execfile("config/astuces_EN.cfg");
    }

    renderbackground(langage ? "Loading..." : "Chargement...");

    identflags &= ~IDF_PERSIST;

    initing = INIT_GAME;
    game::loadconfigs();

    initing = NOT_INITING;

    logoutf("init: render");
    restoregamma();
    restorevsync();
    initgbuffer();
    loadshaders();
    initparticles();
    initstains();

    identflags |= IDF_PERSIST;

    addpostfx("pause", 1, 1, 1, 1, vec4(1, 1, 1, 1));
    addpostfx("sobel", 1, 1, 1, 1, vec4(1, 1, 1, 1));
    clearpostfx();

    logoutf("init: mainloop");

    if(execfile("once.cfg", false)) remove(findfile("once.cfg", "rb"));

    if(load)
    {
        logoutf("init: localconnect");
        //localconnect();
        game::changemap(load);
    }

    if(initscript) execute(initscript);

    initmumble();
    resetfpshistory();

    inputgrab(grabinput = true);
    ignoremousemotion();

    nbtexte = rnd(8);

    for(;;)
    {
        static int frames = 0;
        int millis = getclockmillis();
        limitfps(millis, totalmillis);
        elapsedtime = millis - totalmillis;
        static int timeerr = 0;
        int scaledtime = game::scaletime(elapsedtime) + timeerr;
        curtime = scaledtime/100;
        timeerr = scaledtime%100;
        if(!multiplayer(false) && curtime>200) curtime = 200;
        if(game::ispaused()) curtime = 0;
        lastmillis += curtime;
        totalmillis = millis;
        updatetime();
        if(!mainmenu) game::dotime();

        checkinput();
        UI::update();
        menuprocess();
        tryedit();

        if(lastmillis) game::updateworld();

        checksleep(lastmillis);

        serverslice(false, 0);

        if(frames) updatefpshistory(elapsedtime);
        frames++;

        // miscellaneous general game effects
        recomputecamera();
        updateparticles();
        updatesounds();

        if(minimized) continue;

        gl_setupframe(!mainmenu);

        inbetweenframes = false;
        gl_drawframe();
        swapbuffers();
        renderedframe = inbetweenframes = true;

        if(usesteam)SteamAPI_RunCallbacks();
    }

    ASSERT(0);
    return EXIT_FAILURE;

    #if defined(WIN32) && !defined(_DEBUG) && !defined(__GNUC__)
    } __except(stackdumper(0, GetExceptionInformation()), EXCEPTION_CONTINUE_SEARCH) { return 0; }
    #endif
}
