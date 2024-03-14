#ifndef MAIN_H
#define MAIN_H

#define SDL_MAIN_HANDLED

#if defined(_WIN32)
#include <windows.h>
#else
#include <errno.h>
#include <cstring>
#endif

#include <iostream>
#include "tools.h"
#include "logs.h"
#include "locales.h"

extern int mouseX, mouseY;
extern int currentTime;
extern void closeLauncher();

#endif
