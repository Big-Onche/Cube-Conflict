#include "main.h"
#include "resource.h"

using namespace std;

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

void LaunchGame(HWND hWnd, int ForceBits)
{
    string GameBin = (ForceBits==1 || !Is64BitWindows) ? "bin\\" : (ForceBits==2 || Is64BitWindows) ? "bin64\\" : "bin\\";
    string GamePath = GameBin + "cubeconflict.exe";

    string Arg1 = "\"-u$HOME/My Games/Cube Conflict\" -glog.txt "; string Arg2 = Language == 0 ? "-a" : "-b"; string GameArgs = Arg1 + Arg2;

    if(ShellExecute(hWnd, "open", (GamePath).c_str(), (GameArgs).c_str(), NULL, SW_SHOW)>(HINSTANCE)31) DestroyWindow(hWnd);
    else
    {
        MessageBeep(MB_ICONWARNING);
        MessageBoxW(NULL, Language==0 ? L"Une erreur est survenue, essayez de réinstaller votre jeu." : L"An error as occured, try reinstalling your game.", Language==0 ? L"Erreur" : L"Error", MB_OK);
    }
}
