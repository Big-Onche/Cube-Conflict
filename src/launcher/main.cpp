#include <windows.h>
#include <fstream>
#include <string>
#include <cstdio>
#include <comutil.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Urlmon.lib")
#pragma comment(lib, "comsuppw.lib")

#include "resource.h"

#define LAUNCHER_MENU_LAUNCHCCX86 1
#define LAUNCHER_MENU_LAUNCHCCX64 2
#define LAUNCHER_MENU_EXIT 3
#define HELP_MENU_ABOUT 4
#define HELP_MENU_OPENWEBSITE 5
#define HELP_MENU_OPENWIKI 6
#define TOOLS_MENU_CHECKFORUPDATES 7
#define TOOLS_MENU_LAUNCHSERVER 8
#define LANG_MENU_SETUPFRENCH 9
#define LANG_MENU_SETUPENGLISH 10
#define LAUNCH_GAME 11

using namespace std;

int wx = 1000; //Largeur de la fenêtre
int wh = 600; //Hauteur de la fenêtre

BOOL Is64BitWindows() //Check l'OS pour lancer la bonne version du jeu
{
#if defined(_WIN64)
 return TRUE;
#elif defined(_WIN32)
 BOOL f64 = FALSE;
 return IsWow64Process(GetCurrentProcess(), &f64) && f64;
#else
 return FALSE;
#endif
}

bool NeedFrench = true;

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

void AddMenus(HWND);
void AddControls(HWND);
void LoadImages();
void GetNews();

HICON hWindowIcon, hIconSm;
HWND hBackground, hFavicon;
HMENU hMenu;
HFONT hFont, hFont2;
HBITMAP hPlayButton, hBackgroundImg;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow)
{
    WNDCLASSW wc = {0};

    hWindowIcon = LoadIcon(hInst, MAKEINTRESOURCE(CC_ICON));
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInst;
    wc.hIcon = hWindowIcon;
    wc.lpszClassName = L"myWindowClass";
    wc.lpfnWndProc = WindowProcedure;
    wc.style = CS_DROPSHADOW;
    wc.hIcon = (HICON) LoadImage(hInst, MAKEINTRESOURCE(CC_ICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

    if(!RegisterClassW(&wc))
        return -1;

    CreateWindowW(L"myWindowClass", L"Cube Conflict", WS_SYSMENU | WS_VISIBLE | WS_OVERLAPPED, GetSystemMetrics(SM_CXSCREEN)/2-(wx/2), GetSystemMetrics(SM_CYSCREEN)/2-(wh/2), wx, wh, NULL, NULL, NULL, NULL);

    MSG msg = {0};

    while(GetMessage(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

string InstalledCCversion, AvailableCCversion;
bool UnableToCheckForUpdate, UpdateAvailable;

void GetInstalledCCVersion() //Récupération de la version installée du jeu (pour màj automatiques toussa toussa)
{
    fstream versionFile;
    versionFile.open("config/version", ios::in);
    if(versionFile.is_open())
    {
        string line;
        while (getline(versionFile, line)) {
            InstalledCCversion = line;
        }
        versionFile.close();
    }
    else {InstalledCCversion = (NeedFrench ? "Inconnue" : "Unknown");}
}

void CheckCurrentCCVersion(HWND hWnd, bool NeedInfo = false) //Récupération de la dernière version du jeu en ligne
{
    if (S_OK == URLDownloadToFile(NULL, "http://cube-conflict.com/lastversion", "config/lastversion", 0, NULL))
    {
        fstream lastversionFile;
        lastversionFile.open("config/lastversion", ios::in);
        if(lastversionFile.is_open())
        {
            string line;
            while (getline(lastversionFile, line)) {
                AvailableCCversion = line;
            }
            lastversionFile.close();

            if(InstalledCCversion != AvailableCCversion) UpdateAvailable = true;
        }
        else {UnableToCheckForUpdate = true;}
    }
    else {UnableToCheckForUpdate = true;}

    if(!NeedInfo) return;

    if(UnableToCheckForUpdate) MessageBoxW(NULL, L"Impossible de vérifier les mises à jour.", L"Recherche de mise à jour", MB_OK);
    else if (UpdateAvailable) MessageBoxW(NULL, L"Une mise à jour est disponible !", L"Recherche de mise à jour", MB_OK);
    else MessageBoxW(NULL, L"Votre version du jeu est à jour.", L"Recherche de mise à jour", MB_OK);
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch(msg)
    {
    case WM_COMMAND:
        switch(wp)
        {
            //////////////////////////////////////////////////////////////Menu Launcher////////////////////////////////
        case LAUNCHER_MENU_LAUNCHCCX86:
            WinExec("bin/cubeconflict.exe \"-u$HOME/My Games/Cube Conflict\" -glog.txt", SW_SHOW);
            break;
            case LAUNCHER_MENU_LAUNCHCCX64:
            WinExec("bin64/cubeconflict.exe \"-u$HOME/My Games/Cube Conflict\" -glog.txt", SW_SHOW);
            break;
        case LAUNCHER_MENU_EXIT:
            DestroyWindow(hWnd);
            break;
            //////////////////////////////////////////////////////////////Menu Aide////////////////////////////////
        case HELP_MENU_ABOUT:
            MessageBoxW(NULL, L"Launcher Cube Conflict v0.8\n\nCode source libre et disponible via src/launcher", L"A propos", MB_OK);
            break;
        case HELP_MENU_OPENWEBSITE:
            system("start http://www.cube-conflict.com");
            break;
        case HELP_MENU_OPENWIKI:
            system("start https://cube-conflict.fandom.com/fr/wiki/Wiki_Cube_Conflict");
            break;
        case LANG_MENU_SETUPFRENCH: NeedFrench = true; AddMenus(hWnd); AddControls(hWnd); break;
        case LANG_MENU_SETUPENGLISH: NeedFrench = false; AddMenus(hWnd); AddControls(hWnd); break;
            //////////////////////////////////////////////////////////////Menu Outils////////////////////////////////
        case TOOLS_MENU_CHECKFORUPDATES:
            CheckCurrentCCVersion(hWnd, true);
            break;
        case TOOLS_MENU_LAUNCHSERVER:
            WinExec("bin/cubeconflict.exe \"-u$HOME/My Games/Cube Conflict\" -glog_serveur.txt -d", SW_SHOW);
            break;
            //////////////////////////////////////////////////////////////Reste de la fenêtre////////////////////////////////
        case LAUNCH_GAME:
            if(Is64BitWindows)WinExec("bin64/cubeconflict.exe \"-u$HOME/My Games/Cube Conflict\" -glog.txt", SW_SHOW);
            else WinExec("bin/cubeconflict.exe \"-u$HOME/My Games/Cube Conflict\" -glog.txt", SW_SHOW);
            DestroyWindow(hWnd);
            break;
        }
        break;
    case WM_CREATE:
        GetInstalledCCVersion();
        CheckCurrentCCVersion(hWnd);
        LoadImages();
        mciSendString("open \"media/musiques/launcher.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
        mciSendString("play mp3", NULL, 0, NULL);
        //PlaySound("test.wav", NULL, SND_ASYNC | SND_LOOP);
        AddMenus(hWnd);
        AddControls(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hWnd, msg, wp, lp);
    }
}

void AddMenus(HWND hWnd)
{
    hMenu = CreateMenu();

    //////////////////////////////////////////////////////////////Menu Launcher////////////////////////////////
    HMENU hLauncherMenu = CreateMenu();
    AppendMenu(hLauncherMenu, MF_STRING, LAUNCHER_MENU_LAUNCHCCX86, NeedFrench ? "Lancer le jeu (32 bits)" : "Launch game (32 bits)");
    AppendMenu(hLauncherMenu, MF_STRING, LAUNCHER_MENU_LAUNCHCCX64, NeedFrench ? "Lancer le jeu (64 bits)" : "Launch game (64 bits)");
    AppendMenu(hLauncherMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hLauncherMenu, MF_STRING, LAUNCHER_MENU_EXIT, NeedFrench ? "Quitter" : "Quit");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hLauncherMenu, "Launcher");

    //////////////////////////////////////////////////////////////Menu langue////////////////////////////////
    HMENU hLangMenu = CreateMenu();
    AppendMenu(hLangMenu, MF_STRING, LANG_MENU_SETUPFRENCH, "Français");
    AppendMenu(hLangMenu, MF_STRING, LANG_MENU_SETUPENGLISH, "English");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hLangMenu, NeedFrench ? "Langue" : "Language");

    //////////////////////////////////////////////////////////////Menu outils////////////////////////////////
    HMENU hToolsMenu = CreateMenu();
    AppendMenu(hToolsMenu, MF_STRING, TOOLS_MENU_CHECKFORUPDATES, NeedFrench ? "Vérifier les mises à jour" : "Check for updates");
    AppendMenu(hToolsMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hToolsMenu, MF_STRING, TOOLS_MENU_LAUNCHSERVER, NeedFrench ? "Lancer un serveur" : "Start server");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hToolsMenu, NeedFrench ? "Outils" : "Tools");

    //////////////////////////////////////////////////////////////Menu Aide////////////////////////////////
    HMENU hHelpMenu = CreateMenu();
    AppendMenu(hHelpMenu, MF_STRING, HELP_MENU_ABOUT, NeedFrench ? "À propos" : "About");
    AppendMenu(hHelpMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hHelpMenu, MF_STRING, HELP_MENU_OPENWEBSITE, NeedFrench ? "Site web" : "Website");
    AppendMenu(hHelpMenu, MF_STRING, HELP_MENU_OPENWIKI, "Wiki");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, NeedFrench ? "Aide" : "Help");

    SetMenu(hWnd, hMenu);
}

void AddControls(HWND hWnd)
{
    //Mise en place de l'image de fond
    hBackground = CreateWindowW(L"static", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 0, 0, 1000, 600, hWnd, NULL, NULL, NULL);
    SendMessage(hBackground, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBackgroundImg);

    //Création du texte affiché en bas à gauche (Version du jeu + mise à jour dispo oopah)
    string UpdateInfo;
    if(NeedFrench) UpdateInfo = UpdateAvailable ? " (mise à jour disponible)" : UnableToCheckForUpdate ? " (impossible de vérifier les mises à jour)" : " (à jour)";
    else UpdateInfo = UpdateAvailable ? " (update available)" : UnableToCheckForUpdate ? " (unable to check for update)" : " (up to date)";

    std::string str = (NeedFrench ? " Version du jeu : " : "Game version : ") + InstalledCCversion + UpdateInfo;
    BSTR b = _com_util::ConvertStringToBSTR(str.c_str());
    LPWSTR BottomInfo = b;
    SysFreeString(b);

    //Affichage de la version du jeu installée
    CreateWindowW(L"static", BottomInfo, WS_VISIBLE | WS_CHILD, 0, 535, 1000, 22, hWnd, NULL, NULL, NULL);

    //Affichage du bouton "Jouer"
    hWnd = CreateWindowW(L"Button", NeedFrench ? L"Jouer !" : L"Play !", WS_VISIBLE | WS_CHILD, 614, 320, 200, 40, hWnd, (HMENU)LAUNCH_GAME, NULL, NULL);
    hFont = CreateFont (25, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Roboto");
    SendMessage (hWnd, WM_SETFONT, WPARAM (hFont), TRUE);
}

void LoadImages()
{
    hBackgroundImg = (HBITMAP)LoadImageW(NULL, L"config/background.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}
