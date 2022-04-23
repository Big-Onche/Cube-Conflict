#include <windows.h>
#include <fstream>
#include <string>
#include <cstdio>
#include <stdlib.h>
#include <comutil.h>
#include <shlobj.h>
#include <wininet.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Urlmon.lib")
#pragma comment(lib, "comsuppw.lib")

extern std::string LAUNCHER_VERSION;

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
#define SOUND_MENU_SOUNDSETUP 11
#define LAUNCH_GAME 12

extern int Language, PlayMusic;
extern int wx, wh;

extern std::string InstalledCCversion, AvailableCCversion;
extern bool UnableToCheckForUpdate, UpdateAvailable;

extern void CheckCurrentCCVersion(HWND hWnd, bool NeedInfo = false);
extern void GetInstalledCCVersion();

extern void LoadConfigFile();
extern void WriteConfigFile();

WINUSERAPI
int
WINAPI
MessageBoxA(
    __in_opt HWND hWnd,
    __in_opt LPCSTR lpText,
    __in_opt LPCSTR lpCaption,
    __in UINT uType);
WINUSERAPI
int
WINAPI
MessageBoxW(
    __in_opt HWND hWnd,
    __in_opt LPCWSTR lpText,
    __in_opt LPCWSTR lpCaption,
    __in UINT uType);
