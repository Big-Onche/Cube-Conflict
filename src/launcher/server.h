#ifndef SERVERUI_H
#define SERVERUI_H

#include <SDL.h>

namespace serverui
{
    extern bool isOpen();
    extern bool blocksLauncherInput();
    extern void open();
    extern void close();
    extern void render();
    extern void update(SDL_Event &e);
}

#endif
