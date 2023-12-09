#include "main.h"

using namespace std;

string LAUNCHER_VERSION = "0.9.5";

string InstalledCCversion, AvailableCCversion;
bool UnableToCheckForUpdate, UpdateAvailable;

void GetInstalledCCVersion() //Récupération de la version installée du jeu (pour màj automatiques toussa toussa)
{
    fstream versionFile;
    versionFile.open("config/gameversion.cfg", ios::in);
    if(versionFile.is_open())
    {
        string line;
        while (getline(versionFile, line)) {
            InstalledCCversion = line;
        }
        versionFile.close();
    }
    else {InstalledCCversion = (Language==0 ? "Inconnue" : "Unknown");}
}

char UpdateCheckerDlDir[256];

void CheckCurrentCCVersion(HWND hWnd, bool NeedInfo) //Récupération de la dernière version du jeu en ligne
{
    UpdateCheckerDlDir[0] = '\0';
    if(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, UpdateCheckerDlDir) != S_OK || !UpdateCheckerDlDir[0]);

    for (int i = 0; i < 3; i++)
    {
        switch(i)
        {
            case 0: strcat(UpdateCheckerDlDir, "\\My Games"); CreateDirectoryA(UpdateCheckerDlDir, NULL); break;
            case 1: strcat(UpdateCheckerDlDir, "\\Cube Conflict"); CreateDirectoryA(UpdateCheckerDlDir, NULL); break;
            case 2: strcat(UpdateCheckerDlDir, "\\config"); CreateDirectoryA(UpdateCheckerDlDir, NULL); break;
        }
    }

    strcat(UpdateCheckerDlDir, "\\lastversion.cfg");

    DeleteUrlCacheEntry("http://cube-conflict.com/lastversion.cfg") ;

    if (S_OK == URLDownloadToFile(NULL, "http://cube-conflict.com/lastversion.cfg", UpdateCheckerDlDir, 0, NULL))
    {
        fstream lastversionFile;
        lastversionFile.open(UpdateCheckerDlDir, ios::in);
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

    LPWSTR PopUpTitle =  (Language==0 ? L"Recherche de mise à jour" : L"Check For update");

    if(UnableToCheckForUpdate)
    {
        MessageBeep(MB_ICONWARNING);
        MessageBoxW(NULL, Language==0 ? L"Impossible de vérifier les mises à jour." : L"Unable to check for updates", PopUpTitle, MB_OK);
    }
    else if (UpdateAvailable)
    {
        MessageBeep(MB_ICONWARNING);
        MessageBoxW(NULL, Language==0 ? L"Une mise à jour est disponible !" : L"An update is available!", PopUpTitle, MB_OK);
    }
    else MessageBoxW(NULL, Language==0 ? L"Votre version du jeu est à jour." : L"Your game is up to date", PopUpTitle, MB_OK);
}
