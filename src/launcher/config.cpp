#include "main.h"
#include "resource.h"

using namespace std;

int PlayMusic = 1;
int Language = 0; //0 = FR, 1 = EN
int ChangedLang = 1;

int wx = 1000; //Largeur de la fenêtre
int wh = 600;  //Hauteur de la fenêtre

char ConfigDir[256];

void LoadConfigFile()
{
    ConfigDir[0] = '\0';
    if(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, ConfigDir) != S_OK || !ConfigDir[0]);
    strcat(ConfigDir, "\\My Games\\Cube Conflict\\config\\launcherconfig.cfg");

    fstream versionFile;
    versionFile.open(ConfigDir, ios::in);
    if(versionFile.is_open())
    {
        string line1, line2, line3;
        versionFile >> line1 >> line2 >> line3;

        Language = std::stoi(line1);
        PlayMusic = std::stoi(line2);
        ChangedLang = std::stoi(line3);

        versionFile.close();
    }
}

void WriteConfigFile()
{
    ConfigDir[0] = '\0';
    if(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, ConfigDir) != S_OK || !ConfigDir[0]);

    for (int i = 0; i < 4; i++) //Crée les dossiers et fichiers si inexistants.
    {
        switch(i)
        {
            case 0: strcat(ConfigDir, "\\My Games"); CreateDirectoryA(ConfigDir, NULL); break;
            case 1: strcat(ConfigDir, "\\Cube Conflict"); CreateDirectoryA(ConfigDir, NULL); break;
            case 2: strcat(ConfigDir, "\\config"); CreateDirectoryA(ConfigDir, NULL); break;
            case 3: strcat(ConfigDir, "\\launcherconfig.cfg");
        }
    }

    FILE *ConfigFile = fopen(ConfigDir, "w");

    fprintf(ConfigFile, "%d\n%d\n%d", Language, PlayMusic, ChangedLang);

    fclose(ConfigFile);
}
