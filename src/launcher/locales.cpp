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

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip> // For std::quoted

std::map<std::string, std::vector<std::string>> loadLocales(const std::string& filePath, int languageId)
{
    std::map<std::string, std::vector<std::string>> locales;
    std::ifstream file(filePath);
    std::string line, currentSection;
    bool sectionFound = false;

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

        if(line[0] == '[') { // Section header
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
            std::string key, value;
            if(std::getline(lineStream, key, '=') && std::getline(lineStream, value))
            {
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);

                // Remove potential starting and ending spaces or quotes
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                std::vector<std::string> values;

                if(value.front() == '[' && value.back() == ']') // Check if the value starts with a bracket indicating an array
                {
                    value = value.substr(1, value.size() - 2); // Strip the brackets
                    std::istringstream valueStream(value);
                    std::string element;

                    while(valueStream >> std::quoted(element)) values.push_back(element); // Extract each quoted element

                }
                else // It's a single quoted string, not an array
                {
                    std::istringstream valueStream(value);
                    std::string element;
                    if(valueStream >> std::quoted(element)) values.push_back(element);
                }
                locales[key] = values;
            }
        }
    }
    return locales;
}


std::map<std::string, std::vector<std::string>> currentLocale;

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

std::string getString(const std::string& key, int index)
{
    auto it = currentLocale.find(key);
    if(it != currentLocale.end())
    {
        if(index == -1 && !it->second.empty()) return it->second.front(); // Return the first element as a fallback
        else if(index >= 0 && index < it->second.size()) return it->second[index];
    }
    return key; // Return the key itself as a fallback
}
