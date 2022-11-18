#include <game.h>

extern bool randomevent(int probability);

namespace gfx
{
    static const struct colors{ int color; } rndcolor[] =
    {
        {0xFF0000},
        {0x00FF00},
        {0x0000FF},
        {0xFFFF00},
        {0x00FFFF},
        {0xFF00FF}
    };

    extern bool champicolor;
    extern void projgunexplosion(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void projexplosion(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void projgunhit(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void instantrayhit(const vec &from, const vec &to, const vec &muzzle, int atk);
    extern const char *getshielddir(int armourtype, int armourval, bool hud = false, bool preload = false);
}

//Config élémentaire
extern bool IS_USING_STEAM, IS_ON_OFFICIAL_SERV, UI_PLAYMUSIC;
extern int GAME_LANG; // 0 = FR, 1 = EN

//Config
extern int dynlightdist;

//Maps & ambiances
extern int randomambience;
extern float wateramplitude;

//Stats & sauvegarde
extern void getsteamname();
extern void genpseudo(int forcelang = 0);

//HUD and MENUS
extern bool suicided;
extern string str_pseudotueur, str_armetueur, tempachname;
extern int n_aptitudetueur, n_aptitudevictime;

enum {MSG_OWNKILLSTREAK = 0, MSG_YOUKILLED, MSG_OTHERKILLSTREAK,
        MSG_CTF_TEAMPOINT, MSG_CTF_ENNEMYPOINT, MSG_CTF_TEAMFLAGRECO, MSG_CTF_ENNEMYFLAGRECO, MSG_CTF_TEAMSTOLE, MSG_CTF_ENNEMYSTOLE,
        MSG_LEVELUP, MSG_ACHUNLOCKED, MSG_CUSTOM, MSG_HELP, NUMMESSAGE};
extern int message[NUMMESSAGE];

extern int zoomfov, zoom, crosshairsize;
extern float champifov, crosshairalpha, pourcents;

extern float weapposside, weapposup, maxweapposside, maxweapposup, shieldside, maxshieldside;
extern int nbfps;

extern int parallaxX, parallaxY, UI_menutabs, UI_smiley, UI_cape, UI_tombe, UI_voix, UI_custtab, UI_showsteamnamebtn; //Menu
