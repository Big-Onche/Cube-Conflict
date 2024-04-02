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
#include "config.h"

const int SCR_W = 1000;
const int SCR_H = 540;

extern bool isUsingSteam;
extern bool playPong;
extern int mouseX, mouseY;
extern int currentTime;
extern void closeLauncher();

#endif
