#ifndef MAIN_H
#define MAIN_H

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

//config
extern std::string LAUNCHER_VERSION;
extern int Language, PlayMusic;
extern int wx, wh;
extern void LoadConfigFile();
extern void WriteConfigFile();

//launcher
extern void LaunchGame(HWND hWnd, int ForceBits = 0);

//main
extern void AddMenus(HWND);
extern void AddControls(HWND);
extern void LoadImages();

//updater
extern std::string InstalledCCversion, AvailableCCversion;
extern bool UnableToCheckForUpdate, UpdateAvailable;
extern void CheckCurrentCCVersion(HWND hWnd, bool NeedInfo = false);
extern void GetInstalledCCVersion();

#endif //MAIN_H
