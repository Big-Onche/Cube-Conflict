#ifndef LOCALES_H
#define LOCALES_H

#include <fstream>
#include <sstream>
#include <map>

enum {FRENCH = 0, ENGLISH, RUSSIAN, SPANISH};

extern std::map<std::string, std::vector<std::string>> loadLocales(const std::string& filePath, int languageId);
extern void setLanguage(int language, bool init = false);
extern void detectSystemLanguage();
extern std::string getString(const std::string& key, int index = -1);

#endif
