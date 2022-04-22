#include "main.h"
#include "resource.h"

using namespace std;

string LAUNCHER_VERSION = "0.8.4";

int wx = 1000; //Largeur de la fenêtre
int wh = 600; //Hauteur de la fenêtre

bool NeedFrench = true;

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
            {
                std::string str;
                switch(NeedFrench)
                {
                    case TRUE: str = "Launcher Cube Conflict v" + LAUNCHER_VERSION + "\n\nCode source libre et disponible via src/launcher"; break;
                    case FALSE: str = "Cube Conflict Launcher v" + LAUNCHER_VERSION + "\n\nSource code available at src/launcher"; break;
                }
                BSTR b = _com_util::ConvertStringToBSTR(str.c_str());
                LPWSTR LauncherInfo = b;
                SysFreeString(b);

                MessageBoxW(NULL, LauncherInfo, NeedFrench ? L"À propos" : L"About", MB_OK);
            }
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
