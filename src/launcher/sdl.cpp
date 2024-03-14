#include "sdl.h"
#include "textures.h"
#include "main.h"
#include "buttons.h"

namespace sdl
{
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;

    bool initSDL()
    {
        if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) < 0)
        {
            error::pop(getString("Error_Title").c_str(), getString("Error_SDL_Init").c_str(), true);
            return false;
        }
        else
        {   // Create window
            window = SDL_CreateWindow("Cube Conflict launcher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCR_W, SCR_H, SDL_WINDOW_SHOWN|SDL_WINDOW_BORDERLESS);
            if(window == nullptr)
            {
                error::pop(getString("Error_Title").c_str(), getString("Error_SDL_Window").c_str(), true);
                return false;
            }
            else
            {   // Create renderer for window
                renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
                if(renderer == nullptr)
                {
                    error::pop(getString("Error_Title").c_str(), getString("Error_SDL_Renderer").c_str(), true);
                    return false;
                }
            }
        }
        return true;
    }

    bool initIMG()
    {
        int imgFlags = IMG_INIT_PNG;
        if(!(IMG_Init(imgFlags) & imgFlags))
        {
            error::pop(getString("Error_Title").c_str(), getString("Error_IMG_Init").c_str(), true);
            return false;
        }
        return true;
    }

    bool initTTF()
    {
        if(TTF_Init() == -1)
        {
            error::pop(getString("Error_Title").c_str(), getString("Error_TTF_Init").c_str(), true);
            return false;
        }
        else
        {   // load the font
            font = TTF_OpenFont("media/interface/font/default.ttf", fontSize);
            if(!font)
            {
                error::pop(getString("Error_Title").c_str(), getString("Error_TTF_Load").c_str(), false);
                return false;
            }
        }
        return true;
    }

    bool init()
    {
        if(!initSDL() || !initIMG() || !initTTF()) return false;
        SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1"); // avoid the first click to be potentially ignored
        return true;
    }

    void destroy()
    {
        SDL_DestroyRenderer(sdl::renderer);
        SDL_DestroyWindow(sdl::window);
        sdl::window = nullptr;
        sdl::renderer = nullptr;

        loopi(NUMTEXTURES)
        {
            if(textures[i].texture != nullptr)
            {
                SDL_DestroyTexture(textures[i].texture);
                textures[i].texture = nullptr;
            }
        }

        if(sdl::font != nullptr)
        {
            TTF_CloseFont(sdl::font);
            sdl::font = nullptr;
        }

        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }
}
