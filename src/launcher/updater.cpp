#include "main.h"

using namespace std;

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
    else {InstalledCCversion = (NeedFrench ? "Inconnue" : "Unknown");}
}

char UpdateCheckerDlDir[256];

void CheckCurrentCCVersion(HWND hWnd, bool NeedInfo) //Récupération de la dernière version du jeu en ligne
{
    UpdateCheckerDlDir[0] = '\0';
    if(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, UpdateCheckerDlDir) != S_OK || !UpdateCheckerDlDir[0]);

    strcat(UpdateCheckerDlDir, "\\My Games\\Cube Conflict\\config\\lastversion.cfg");

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

    LPWSTR PopUpTitle =  (NeedFrench ? L"Recherche de mise à jour" : L"Check For update");

    if(UnableToCheckForUpdate) MessageBoxW(NULL, NeedFrench ? L"Impossible de vérifier les mises à jour." : L"Unable to check for updates", PopUpTitle, MB_OK);
    else if (UpdateAvailable) MessageBoxW(NULL, NeedFrench ? L"Une mise à jour est disponible !" : L"An update is available!", PopUpTitle, MB_OK);
    else MessageBoxW(NULL, NeedFrench ? L"Votre version du jeu est à jour." : L"Your game is up to date", PopUpTitle, MB_OK);
}
