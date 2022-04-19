#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>

#include "resource.h"
#include "curl/curl.h"

#define LAUNCHER_MENU_LAUNCHCCX86 1
#define LAUNCHER_MENU_LAUNCHCCX64 2
#define LAUNCHER_MENU_EXIT 3
#define HELP_MENU_ABOUT 4
#define HELP_MENU_OPENWEBSITE 5
#define HELP_MENU_OPENWIKI 6
#define LAUNCH_GAME 7

using namespace std;

int wx = 1000; //Largeur de la fenêtre
int wh = 600; //Hauteur de la fenêtre

BOOL Is64BitWindows() //Check si l'OS pour lancer la bonne version du jeu
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

string CCversion;

void GetCCVersion() //Récupération de la version installée du jeu (pour màj automatiques toussa toussa)
{
    fstream versionFile;
    versionFile.open("config/version", ios::in);
    if(versionFile.is_open())
    {
        string line;
        while (getline(versionFile, line)) {
            CCversion = line;
        }
        versionFile.close();
    }
    else {CCversion = "Inconnue";}
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
            MessageBoxW(NULL, L"Launcher Cube Conflict v0.7\n\nCode source libre et disponible via src/launcher", L"A propos", MB_OK);
            break;
        case HELP_MENU_OPENWEBSITE:
            system("start https://cube-conflict.fandom.com/fr/wiki/Wiki_Cube_Conflict");
            break;
        case HELP_MENU_OPENWIKI:
            system("start http://www.cube-conflict.com");
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
        GetCCVersion();
        LoadImages();
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
    AppendMenu(hLauncherMenu, MF_STRING, LAUNCHER_MENU_LAUNCHCCX86, "Lancer le jeu (32 bits)");
    AppendMenu(hLauncherMenu, MF_STRING, LAUNCHER_MENU_LAUNCHCCX64, "Lancer le jeu (64 bits)");
    AppendMenu(hLauncherMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hLauncherMenu, MF_STRING, LAUNCHER_MENU_EXIT, "Quitter");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hLauncherMenu, "Launcher");

    //////////////////////////////////////////////////////////////Menu Aide////////////////////////////////
    HMENU hHelpMenu = CreateMenu();
    AppendMenu(hHelpMenu, MF_STRING, HELP_MENU_ABOUT, "A propos");
    AppendMenu(hHelpMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hHelpMenu, MF_STRING, HELP_MENU_OPENWIKI, "Site web");
    AppendMenu(hHelpMenu, MF_STRING, HELP_MENU_OPENWIKI, "Wiki");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, "Aide");

    SetMenu(hWnd, hMenu);
}

void AddControls(HWND hWnd)
{
    //Mise en place de l'image de fond
    hBackground = CreateWindowW(L"static", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 0, 0, 1000, 600, hWnd, NULL, NULL, NULL);
    SendMessage(hBackground, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBackgroundImg);

    //Création du texte avec la version du jeu récupérée dans le fichier "version"
    string BottomInfo = "Version du jeu : " + CCversion;
	std::wstring stemp = std::wstring(BottomInfo.begin(), BottomInfo.end());
    LPCWSTR BottomInfoLPCWSTR = stemp.c_str();

    //Affichage de la version du jeu installée
    CreateWindowW(L"static", BottomInfoLPCWSTR, WS_VISIBLE | WS_CHILD, 5, 535, 150, 20, hWnd, NULL, NULL, NULL);

    //Affichage du bouton "Jouer"
    hWnd = CreateWindowW(L"Button", L"Jouer !", WS_VISIBLE | WS_CHILD, 614, 320, 200, 40, hWnd, (HMENU)LAUNCH_GAME, NULL, NULL);
    hFont = CreateFont (25, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Roboto");
    SendMessage (hWnd, WM_SETFONT, WPARAM (hFont), TRUE);
}

void LoadImages()
{
    hBackgroundImg = (HBITMAP)LoadImageW(NULL, L"config/background.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}
