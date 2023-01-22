#include <game.h>

extern bool randomevent(int probability);
extern void addsleep(int *msec, char *cmd);
extern void createdrop(const vec *o, int type);
extern void trydisconnect(bool local);

namespace game
{
    enum {D_COMMON = 0, D_UNCOMMON, D_RARE, D_LEGENDARY, D_GODLY, NUMDROPS};
    extern void npcdrop(const vec *o, int type);
}

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
    extern void resetshroomsgfx();
    extern void projgunexplosion(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void projexplosion(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void projgunhit(gameent *owner, const vec &v, const vec &vel, dynent *safe, int atk);
    extern void instantrayhit(const vec &from, const vec &to, const vec &muzzle, int atk);
    extern void shootgfx(const vec &from, const vec &to, gameent *d, int atk);
    extern char *getshielddir(int armourtype, int armourval, bool hud = false, bool preload = false);
}

namespace custom
{
    extern const char *getcapedir(int cape, bool enemy = false);
}

static const char * const enthudnames[MAXENTTYPES*2] =
{
    "invalide?", "none?", "Lumi�re", "Light", "Mod�le 3D", "3D model", "Point de r�apparition","Respawn point",
    "Placage d'environnement", "Environment map", "Effet de particules", "Particles effect", "Son", "Sound", "Spot de lumi�re", "Spotlight", "Projection", "Decal",

    "Fusil �lectrique", "Electric Rifle", "Fusil plasma", "Plasma rifle", "SMAW", "SMAW", "Minigun", "Minigun", "Pistolet spock", "Spockgun",
    "M32", "M32", "Lance-flammes", "Flamethrower", "UZI", "UZI", "FAMAS", "FAMAS", "Mossberg 500", "Mossberg 500", "Hydra", "Hydra",
    "SV-98", "SV-98", "SKS", "SKS", "Arbal�te", "Crossbow", "AK-47", "AK-47", "GAPB-1", "GAPB-1", "Feux d'artifice", "Fireworks", "Glock 45", "Glock 45",
    "Super-arme", "Superweapon", "invalide?", "none?", "invalide?", "none?", "invalide?", "none?",

    "Sant� [25]", "Health [25]", "Boost de sant� [50]", "Health boost [50]", "St�ro�des [D�g�ts]", "Stero�ds [Damages]", "Champis [Cadence]", "Shrooms [Cadency]",
    "EPO [Vitesse]", "EPO [Speed]", "Joint [R�sistance]", "Joint [Resistance]", "Bois [75]", "Wood [75]", "Fer [125]", "Iron [125]", "Or [200]", "Gold [200]",
    "Magn�tique [150]", "Magnetic [150]", "Armure assist�e [300]", "Power armor [300]", "Mana [25]", "Mana [25]",

    "T�l�poration [Entr�e]", "Teleport [In]", "T�l�portation [Destination]", "T�l�portation [Destination]", "Trampoline", "Jump pad", "Drapeau", "Flag",
    "Base", "Base", "PNJ [Solo]", "NPC [SP]", "Point de r�apparition [Solo]", "Respawn point [SP]", "Zone de d�clencheur [Solo]", "Trigger zone [SP]", "Cam�ra", "Camera",
};

//Config �l�mentaire
extern bool IS_USING_STEAM, IS_ON_OFFICIAL_SERV, UI_PLAYMUSIC;
extern int GAME_LANG; // 0 = FR, 1 = EN
extern int servlang;  // 0 = FR, 1 = EN

//Maps & ambiances
extern int randomambience;
extern float wateramplitude;

//Stats & sauvegarde
extern void getsteamname();
extern void genpseudo(int forcelang = 0);

//HUD and MENUS
extern bool suicided;
extern string str_pseudotueur, str_armetueur, tempachname;
extern int n_aptitudetueur, n_aptitudevictime, gamesecs;

enum {MSG_OWNKILLSTREAK = 0, MSG_YOUKILLED, MSG_OTHERKILLSTREAK,
        MSG_CTF_TEAMPOINT, MSG_CTF_ENNEMYPOINT, MSG_CTF_TEAMFLAGRECO, MSG_CTF_ENNEMYFLAGRECO, MSG_CTF_TEAMSTOLE, MSG_CTF_ENNEMYSTOLE,
        MSG_LEVELUP, MSG_ACHUNLOCKED, MSG_CUSTOM, MSG_HELP, MSG_PREMISSION, NUMMESSAGE};
extern int message[NUMMESSAGE];

extern int zoomfov, zoom, crosshairsize, weapposside, weapposup, forcecampos;
extern float champifov, pourcents;

extern int nbfps;
