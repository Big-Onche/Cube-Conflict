#include "main.h"
#include "resource.h"

using namespace std;

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

HICON hWindowIcon, hIconSm;
HWND hBackground, hFavicon, hButFr, hButEn, hButSound, hButSound2;
HMENU hMenu;
HFONT hFont, hFont2;
HBITMAP hBackgroundImg, hFrFlag, hEnFlag, hSpeakerON, hSpeakerOFF;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow)
{
    WNDCLASSW wc = {0};

    hWindowIcon = LoadIcon(hInst, MAKEINTRESOURCE(CC_ICON));
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInst;
    wc.hIcon = hWindowIcon;
    wc.lpszClassName = L"LauncherClass";
    wc.lpfnWndProc = WindowProcedure;
    wc.style = CS_DROPSHADOW;
    wc.hIcon = (HICON) LoadImage(hInst, MAKEINTRESOURCE(CC_ICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

    if(!RegisterClassW(&wc))
        return -1;

    CreateWindowW(L"LauncherClass", L"Cube Conflict", WS_SYSMENU | WS_VISIBLE | WS_MINIMIZEBOX, GetSystemMetrics(SM_CXSCREEN)/2-(wx/2), GetSystemMetrics(SM_CYSCREEN)/2-(wh/2), wx, wh, NULL, NULL, NULL, NULL);

    MSG msg = {0};

    while(GetMessage(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

BOOL CALLBACK DestoryChildCallback(
  HWND   hwnd,
  LPARAM lParam
)
{
  if (hwnd != NULL) {
    DestroyWindow(hwnd);
  }

  return TRUE;
}

void Redraw(HWND hWnd)
{
    EnumChildWindows(hWnd, DestoryChildCallback, NULL);
    AddMenus(hWnd);
    AddControls(hWnd);
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
            LaunchGame(hWnd, 1);
            break;
            case LAUNCHER_MENU_LAUNCHCCX64:
            LaunchGame(hWnd, 2);
            break;
        case LAUNCHER_MENU_EXIT:
            DestroyWindow(hWnd);
            break;
            //////////////////////////////////////////////////////////////Menu Aide////////////////////////////////
        case HELP_MENU_ABOUT:
            {
                std::string str;
                switch(Language)
                {
                    case 0: str = "Launcher Cube Conflict v" + LAUNCHER_VERSION + "\n\nCode source libre et disponible via src/launcher"; break;
                    case 1: str = "Cube Conflict Launcher v" + LAUNCHER_VERSION + "\n\nSource code available at src/launcher"; break;
                }
                BSTR b = _com_util::ConvertStringToBSTR(str.c_str());
                LPWSTR LauncherInfo = b;
                SysFreeString(b);

                MessageBoxW(NULL, LauncherInfo, Language==0 ? L"À propos" : L"About", MB_OK);
            }
            break;
        case HELP_MENU_OPENWEBSITE:
            ShellExecute(NULL, "open", "http://www.cube-conflict.com", NULL, NULL, SW_SHOW);
            break;
        case HELP_MENU_OPENWIKI:
            ShellExecute(NULL, "open", "https://cube-conflict.fandom.com/fr/wiki/Wiki_Cube_Conflict", NULL, NULL, SW_SHOW);
            break;
        case LANG_MENU_SETUPFRENCH:
            Language = 0;
            Redraw(hWnd);
            break;
        case LANG_MENU_SETUPENGLISH:
            Language = 1;
            Redraw(hWnd);
            break;
            //////////////////////////////////////////////////////////////Menu Outils////////////////////////////////
        case TOOLS_MENU_CHECKFORUPDATES:
            CheckCurrentCCVersion(hWnd, true);
            break;
        case TOOLS_MENU_LAUNCHSERVER:
            ShellExecute(hWnd, "open", "bin\\cubeconflict.exe", "\"-u$HOME/My Games/Cube Conflict\" -glog_serveur.txt -d", NULL, SW_SHOW);
            break;
            //////////////////////////////////////////////////////////////Reste de la fenêtre////////////////////////////////
        case LAUNCH_GAME:
            LaunchGame(hWnd);
            break;
        case SOUND_MENU_SOUNDSETUP:
            if(PlayMusic)
            {
                PlayMusic = 0;
                mciSendString("stop mp3", NULL, 0, NULL);
            }
            else
            {
                PlayMusic = 1;
                mciSendString("open \"media/musiques/launcher.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
                mciSendString("play mp3", NULL, 0, NULL);
            }
            EnumChildWindows(hWnd, DestoryChildCallback, NULL);
            AddControls(hWnd);
            break;
        }
        break;
    case WM_CREATE:
        {
            LoadConfigFile();
            GetInstalledCCVersion();
            CheckCurrentCCVersion(hWnd);
            LoadImages();
            if(PlayMusic==1)
            {
                mciSendString("open \"media/musiques/launcher.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
                mciSendString("play mp3", NULL, 0, NULL);
            }
            AddMenus(hWnd);
            AddControls(hWnd);
        }
    break;
    case WM_DESTROY:
        WriteConfigFile();
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
    AppendMenu(hLauncherMenu, MF_STRING, LAUNCHER_MENU_LAUNCHCCX86, !Language ? "Lancer le jeu (32 bits)" : "Launch game (32 bits)");
    AppendMenu(hLauncherMenu, MF_STRING, LAUNCHER_MENU_LAUNCHCCX64, !Language ? "Lancer le jeu (64 bits)" : "Launch game (64 bits)");
    AppendMenu(hLauncherMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hLauncherMenu, MF_STRING, LAUNCHER_MENU_EXIT, !Language ? "Quitter" : "Quit");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hLauncherMenu, "Launcher");

    //////////////////////////////////////////////////////////////Menu langue////////////////////////////////
    HMENU hLangMenu = CreateMenu();
    if(PlayMusic==1) AppendMenu(hLangMenu, MF_STRING, SOUND_MENU_SOUNDSETUP, !Language ? "Couper la musique" : "Stop music");
    else AppendMenu(hLangMenu, MF_STRING, SOUND_MENU_SOUNDSETUP, !Language ? "Activer la musique" : "Play music");
    AppendMenu(hLangMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hLangMenu, MF_STRING, LANG_MENU_SETUPFRENCH, "Français");
    AppendMenu(hLangMenu, MF_STRING, LANG_MENU_SETUPENGLISH, "English");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hLangMenu, !Language ? "Options" : "Settings");

    //////////////////////////////////////////////////////////////Menu outils////////////////////////////////
    HMENU hToolsMenu = CreateMenu();
    AppendMenu(hToolsMenu, MF_STRING, TOOLS_MENU_CHECKFORUPDATES, !Language ? "Vérifier les mises à jour" : "Check for updates");
    AppendMenu(hToolsMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hToolsMenu, MF_STRING, TOOLS_MENU_LAUNCHSERVER, !Language ? "Lancer un serveur" : "Start server");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hToolsMenu, !Language ? "Outils" : "Tools");

    //////////////////////////////////////////////////////////////Menu Aide////////////////////////////////
    HMENU hHelpMenu = CreateMenu();
    AppendMenu(hHelpMenu, MF_STRING, HELP_MENU_ABOUT, !Language ? "À propos" : "About");
    AppendMenu(hHelpMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hHelpMenu, MF_STRING, HELP_MENU_OPENWEBSITE, !Language ? "Site web" : "Website");
    AppendMenu(hHelpMenu, MF_STRING, HELP_MENU_OPENWIKI, "Wiki");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, !Language ? "Aide" : "Help");

    SetMenu(hWnd, hMenu);
}

int ActiveBtnParam =  WS_VISIBLE | WS_CHILD | BS_BITMAP | WS_DLGFRAME;
int InactiveBtnParam = WS_VISIBLE | WS_CHILD | BS_BITMAP | WS_DLGFRAME | WS_DISABLED;

void AddControls(HWND hWnd)
{
    //Mise en place de l'image de fond
    hBackground = CreateWindowW(L"static", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 0, 0, 1000, 600, hWnd, NULL, NULL, NULL);
    SendMessage(hBackground, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBackgroundImg);

    //Affichage des drapeaux de langues
    HWND hButFr = CreateWindowW(L"button", NULL, !Language ? InactiveBtnParam : ActiveBtnParam, 3, 490, 60, 42, hWnd, (HMENU)LANG_MENU_SETUPFRENCH, NULL, NULL);
    SendMessageW(hButFr, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hFrFlag);
    HWND hButEn = CreateWindowW(L"button", NULL, !Language ? ActiveBtnParam : InactiveBtnParam, 68, 490, 60, 42, hWnd, (HMENU)LANG_MENU_SETUPENGLISH, NULL, NULL);
    SendMessageW(hButEn, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hEnFlag);

    //Affichage du bouton haut parleur
    HWND hButSound = CreateWindowW(L"button", NULL, ActiveBtnParam, 931, 490, 60, 42, hWnd, (HMENU)SOUND_MENU_SOUNDSETUP, NULL, NULL);
    SendMessageW(hButSound, BM_SETIMAGE, IMAGE_BITMAP, PlayMusic ? (LPARAM)hSpeakerON : (LPARAM)hSpeakerOFF);

    //Création du texte affiché en bas à gauche (Version du jeu + mise à jour dispo oopah)
    string UpdateInfo;
    if(!Language) UpdateInfo = UpdateAvailable ? " (mise à jour disponible)" : UnableToCheckForUpdate ? " (impossible de vérifier les mises à jour)" : " (à jour)";
    else UpdateInfo = UpdateAvailable ? " (update available)" : UnableToCheckForUpdate ? " (unable to check for update)" : " (up to date)";

    std::string str = (!Language ? " Version du jeu : " : " Game version : ") + InstalledCCversion + UpdateInfo;
    BSTR b = _com_util::ConvertStringToBSTR(str.c_str());
    LPWSTR BottomInfo = b;
    SysFreeString(b);

    //Affichage de la version du jeu installée
    CreateWindowW(L"static", BottomInfo, WS_VISIBLE | WS_CHILD, 0, 535, 1000, 22, hWnd, NULL, NULL, NULL);

    //Affichage du bouton "Jouer"
    hWnd = CreateWindowW(L"Button", !Language ? L"Jouer !" : L"Play !", WS_VISIBLE | WS_CHILD, 614, 320, 200, 40, hWnd, (HMENU)LAUNCH_GAME, NULL, NULL);
    hFont = CreateFont (25, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Roboto");
    SendMessage (hWnd, WM_SETFONT, WPARAM (hFont), TRUE);
}

void LoadImages()
{
    hBackgroundImg = (HBITMAP)LoadImageW(NULL, L"config/launcher/background.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hFrFlag = (HBITMAP)LoadImageW(NULL, L"config/launcher/flag_fr.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hEnFlag = (HBITMAP)LoadImageW(NULL, L"config/launcher/flag_en.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hSpeakerON = (HBITMAP)LoadImageW(NULL, L"config/launcher/speakerON.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hSpeakerOFF = (HBITMAP)LoadImageW(NULL, L"config/launcher/speakerOFF.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}
