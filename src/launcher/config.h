#ifndef CONFIG_H
#define CONFIG_H

#include "tools.h"
#include "logs.h"
#include "locales.h"
#include <unordered_map>
#include <fstream>

enum {CONF_FIRSTLAUNCH = 0, CONF_LANGUAGE, CONF_QUICKLAUNCH, CONF_MUSIC, NUMCONFIGVARS};

namespace config
{
    struct configVarsConfig
    {
        std::string name;
        int value;
    };

    int get(int id);
    void set(int id, int value);

    extern configVarsConfig configVars[NUMCONFIGVARS];

    extern void load();
    extern void write();
}

#endif
