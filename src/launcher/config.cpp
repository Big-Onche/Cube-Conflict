#include "config.h"

namespace config
{
    std::string configFileName = "launcher.cfg";

    configVarsConfig configVars[NUMCONFIGVARS] =
    {
        {"firstLaunch",      true},     // CONF_LANGUAGE
        {"launcherLanguage", ENGLISH},  // CONF_LANGUAGE
        {"quickLaunch",      false},    // CONF_QUICKLAUNCH
        {"enableMusic",      true}      // CONF_MUSIC
    };

    int get(int id)
    {
        if(id >= 0 && id < NUMCONFIGVARS) return configVars[id].value;
        else return 0;
    }

    void set(int id, int value)
    {
        if(id >= 0 && id < NUMCONFIGVARS) configVars[id].value = value;
    }

    std::string configPatch()
    {
    #if defined(_WIN32)
        char path[MAX_PATH];
        HRESULT result = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, path); // Get the path to the Documents folder
        if(result == S_OK) return std::string(path) + "\\My Games\\Cube Conflict\\config\\" + configFileName; // Construct the full path
        else
        {
            error::pop("Error", "Cannot use \"My Games/Cube Conflict/\" patch");
            return configFileName;
        }
    #elif defined(__linux__)
        return configFileName;
    #endif
    }

    void load()
    {
        std::unordered_map<std::string, int> configMap;

        loopi(NUMCONFIGVARS) configMap[configVars[i].name] = i; // Initialize map from config variable name to enum index

        std::ifstream configFile(configPatch());
        std::string line;

        while(std::getline(configFile, line))
        {
            std::istringstream is_line(line);
            std::string name;
            int value;
            if(is_line >> name >> value)
            {
                if(configMap.find(name) != configMap.end()) configVars[configMap[name]].value = value;
            }
        }
    }

    void write()
    {
        std::ofstream configFile(configPatch());
        loopi(NUMCONFIGVARS) configFile << configVars[i].name << " " << configVars[i].value << "\n";
    }
}
