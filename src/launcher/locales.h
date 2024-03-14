#ifndef LOCALES_H
#define LOCALES_H

#include <fstream>
#include <sstream>
#include <map>

enum {FRENCH = 0, ENGLISH, RUSSIAN, SPANISH};

extern std::map<std::string, std::string> loadLocales(const std::string& filePath, int languageId);
extern void setLanguage(int language);
extern std::string getString(const std::string& key);

#endif
