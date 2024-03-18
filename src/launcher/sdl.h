#ifndef SDL_H
#define SDL_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

namespace sdl
{
    const int fontSize = 50;

    extern SDL_Window* window;
    extern SDL_Renderer* renderer;
    extern TTF_Font* fontMain;
    extern TTF_Font* fontTiny;

    extern bool init();     // init sdl
    extern void destroy();  // clean up
}

#endif
