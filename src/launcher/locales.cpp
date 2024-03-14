#include "main.h"
#include "buttons.h"

std::map<int, std::string> languageCodes =
{
    {0, "FRENCH"},
    {1, "ENGLISH"},
    {2, "RUSSIAN"},
    {3, "SPANISH"}
};

std::map<std::string, std::string> loadLocales(const std::string& filePath, int languageId)
{
    std::map<std::string, std::string> locales;
    std::ifstream file(filePath);
    std::string line, currentSection;
    bool sectionFound = false;

    if(languageCodes.find(languageId) == languageCodes.end())
    {
        error::pop("Initialization error", "Invalid language ID");
        closeLauncher();
    }
    std::string targetSection = languageCodes[languageId];

    if(!file.is_open())
    {
        error::pop("Initialization error", "Failed to open launcher's configuration file (config/launcher.cfg)");
        closeLauncher();
    }

    while(std::getline(file, line))
    {
        if(line.empty() || line[0] == '#') continue; // Skip comments and empty lines

        if(line[0] == '[') // Check for section headers
        {
            size_t endPos = line.find(']');
            if (endPos != std::string::npos)
            {
                currentSection = line.substr(1, endPos - 1);
                sectionFound = (currentSection == targetSection);
            }
            continue;
        }

        if(sectionFound)
        {
            std::istringstream lineStream(line);
            std::string key;
            if (std::getline(lineStream, key, '='))
            {
                std::string value;
                if (std::getline(lineStream, value))
                {
                    key.erase(0, key.find_first_not_of(" \t"));
                    key.erase(key.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t\""));
                    value.erase(value.find_last_not_of(" \t\"") + 1);

                    locales[key] = value;
                }
            }
        }
    }

    return locales;
}

std::map<std::string, std::string> currentLocale;

void setLanguage(int language, bool init)
{
    currentLocale = loadLocales("config/launcher.cfg", language);
    if(!init)
    {
        buttons::destroy();
        buttons::init();
    }

}

std::string getString(const std::string& key)
{
    auto it = currentLocale.find(key);
    if(it != currentLocale.end()) return it->second; // Return the localized string
    else return key; // Return the key itself as a fallback, or you could return a default message
}
