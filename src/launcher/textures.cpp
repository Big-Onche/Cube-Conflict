#include "main.h"
#include "textures.h"
#include "sdl.h"

TextureConfig textures[NUMTEXTURES] =
{
    {nullptr, "media/interface/launcher.png"},          // TEX_BACKGROUND
    {nullptr, "media/interface/logo.png"},              // TEX_LOGO
    {nullptr, "media/interface/flags/french.png"},      // TEX_FRENCH
    {nullptr, "media/interface/flags/english.png"},     // TEX_ENGLISH
    {nullptr, "media/interface/flags/russian.png"},     // TEX_RUSSIAN
    {nullptr, "media/interface/flags/spanish.png"},     // TEX_SPANISH
    {nullptr, "media/interface/hud/speaker_on.png"},    // TEX_AUDIOON
    {nullptr, "media/interface/hud/speaker_off.png"},   // TEX_AUDIOOFF
    {nullptr, "media/interface/hud/info.jpg"},          // TEX_ABOUT
    {nullptr, "media/interface/hud/minimize.png"},      // TEX_MINIMIZE
    {nullptr, "media/interface/hud/checkbox_off.jpg"}   // TEX_REDCROSS
};

namespace texture
{
    bool isValid(int id)
    {
        return id >= 0 && id < NUMTEXTURES;
    }

    SDL_Texture* loadTexture(std::string path, SDL_Renderer* renderer)
    {
        SDL_Texture* newTexture = nullptr;
        SDL_Surface* loadedSurface = IMG_Load(path.c_str());
        if(loadedSurface == nullptr) { error::pop(getString("Error_Title").c_str(), getString("Error_IMG_Load").c_str(), true); }
        else
        {
            newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
            if(newTexture == nullptr)
            {
                error::pop(getString("Error_Title").c_str(), getString("Error_IMG_Load").c_str() + lineBreak + path, true);
            }
            SDL_FreeSurface(loadedSurface);
        }
        return newTexture;
    }

    void init()
    {
        loopi(NUMTEXTURES) { textures[i].texture = loadTexture(textures[i].path, sdl::renderer); }
    }
}
