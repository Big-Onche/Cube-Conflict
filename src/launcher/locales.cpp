#include "main.h"
#include "buttons.h"

int currentLanguage = ENGLISH;

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
        error::pop(getString("Error_Title").c_str(), getString("Error_Locale").c_str());
        closeLauncher();
    }
    std::string targetSection = languageCodes[languageId];

    if(!file.is_open())
    {
        std::string configPath = " (config/launcher.cfg)";
        error::pop(getString("Error_Title").c_str(), getString("Error_Config").c_str() + configPath);
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
    currentLanguage = language;
    currentLocale = loadLocales("config/launcher.cfg", language);
    if(!init)
    {
        buttons::destroy();
        buttons::init();
    }
}

void detectSystemLanguage()
{
#if defined(_WIN32)

    LANGID id = GetUserDefaultUILanguage();
    WORD language = PRIMARYLANGID(id);

    switch(language)
    {
        case LANG_FRENCH: setLanguage(FRENCH, true); return;
        case LANG_RUSSIAN: setLanguage(RUSSIAN, true); return;
        case LANG_SPANISH: setLanguage(SPANISH, true); return;
        default: setLanguage(ENGLISH, true); return; // fallback to english
    }

#elif defined(__linux__)

    const char* langEnv = getenv("LANG");
    if(langEnv != nullptr)
    {
        std::string lang = langEnv;

        if(lang.find("fr") == 0) { setLanguage(FRENCH, true); return; }
        else if(lang.find("ru") == 0) { setLanguage(RUSSIAN, true); return; }
        else if(lang.find("es") == 0) { setLanguage(SPANISH, true); return; }
    }
    setLanguage(ENGLISH, true); // fallback to english
#endif
}

std::string getString(const std::string& key)
{
    auto it = currentLocale.find(key);
    if(it != currentLocale.end()) return it->second; // Return the localized string
    else return key; // Return the key itself as a fallback, or you could return a default message
}
