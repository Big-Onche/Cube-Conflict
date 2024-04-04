#include "main.h"
#include "textures.h"
#include "sdl.h"

textureConfig textures[NUMTEXTURES] =
{
    {nullptr, "media/launcher/background.png"},         // TEX_BACKGROUND
    {nullptr, "media/interface/logo.png"},              // TEX_LOGO
    {nullptr, "media/interface/hud/checkbox_on.jpg"},   // TEX_ON
    {nullptr, "media/interface/hud/checkbox_off.jpg"},  // TEX_OFF
    {nullptr, "media/interface/flags/french.png"},      // TEX_FRENCH
    {nullptr, "media/interface/flags/english.png"},     // TEX_ENGLISH
    {nullptr, "media/interface/flags/russian.png"},     // TEX_RUSSIAN
    {nullptr, "media/interface/flags/spanish.png"},     // TEX_SPANISH
    {nullptr, "media/interface/hud/speaker_on.png"},    // TEX_AUDIOON
    {nullptr, "media/interface/hud/speaker_off.png"},   // TEX_AUDIOOFF
    {nullptr, "media/interface/hud/ping.png"},          // TEX_SERVER
    {nullptr, "media/interface/hud/info.jpg"},          // TEX_ABOUT
    {nullptr, "media/interface/hud/minimize.png"},      // TEX_MINIMIZE
    {nullptr, "media/interface/hud/checkbox_off.jpg"},  // TEX_REDCROSS
    {nullptr, "media/launcher/pong.png"},               // TEX_PONG
    {nullptr, "media/launcher/hap.png"},                // TEX_HAP
    {nullptr, "media/launcher/noel.png"},               // TEX_NOEL
    {nullptr, "media/launcher/ball.png"},               // TEX_BALL
    {nullptr, "media/launcher/ball_glow.png"},          // TEX_BALL_GLOW
    {nullptr, "media/launcher/spark.png"},              // PART_SPARK
    {nullptr, "media/launcher/smoke.png"}               // PART_SMOKE
};

namespace texture
{
    bool isValid(int id) { return id >= 0 && id < NUMTEXTURES; }

    SDL_Texture* loadTexture(std::string path, SDL_Renderer* renderer)
    {
        SDL_Texture* newTexture = nullptr;
        SDL_Surface* loadedSurface = IMG_Load(path.c_str());
        if(loadedSurface == nullptr) { error::pop(getString("Error_Title"), getString("Error_IMG_Load"), true); }
        else
        {
            newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
            if(newTexture == nullptr)
            {
                error::pop(getString("Error_Title"), getString("Error_IMG_Load") + lineBreak + path, true);
            }
            SDL_FreeSurface(loadedSurface);
        }
        return newTexture;
    }

    void init()
    {
        loopi(NUMTEXTURES) { textures[i].texture = loadTexture(textures[i].path, sdl::renderer); }
    }

    void render(int id, int x, int y, int width, int height, int flags, float rotation, unsigned int color)
    {
        if(!isValid(id)) return;

        SDL_Texture* texture = textures[id].texture;
        if(texture != nullptr)
        {
            bool rotate = (rotation != 0.f);
            if(rotate) std::fmod(rotation, 360.0f);

            if(flags & TEX_SHADOW)
            {
                SDL_BlendMode oldBlendMode;
                SDL_GetTextureBlendMode(texture, &oldBlendMode);

                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

                SDL_SetTextureColorMod(texture, 0, 0, 0); // Dark shadow
                SDL_SetTextureAlphaMod(texture, 128); // Semi-transparent

                SDL_Rect shadowQuad = {x + 5, y + 5, width, height};

                if(rotate)
                {
                    SDL_Point axis = {width / 2, height / 2};
                    SDL_RenderCopyEx(sdl::renderer, texture, NULL, &shadowQuad, rotation, &axis, SDL_FLIP_NONE);
                }
                else SDL_RenderCopy(sdl::renderer, texture, NULL, &shadowQuad);

                SDL_SetTextureColorMod(texture, 255, 255, 255); // Reset color mod
                SDL_SetTextureAlphaMod(texture, 255); // Reset alpha mod
                SDL_SetTextureBlendMode(texture, oldBlendMode); // Reset blend mode
            }

            SDL_BlendMode oldBlendMode;
            SDL_GetTextureBlendMode(texture, &oldBlendMode);

            if(flags & TEX_ALPHA) SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            SDL_Rect renderQuad = {x, y, width, height};

            bool resetColors = false;

            if(color)
            {
                RGBA textureColor = extractRGBA(color);
                SDL_SetTextureColorMod(texture, textureColor.r, textureColor.g, textureColor.b); // Set RGB color modulation
                SDL_SetTextureAlphaMod(texture, textureColor.a); // Set alpha modulation
                resetColors = true;
            }

            if(rotate)
            {
                SDL_Point axis = {width / 2, height / 2};
                SDL_RenderCopyEx(sdl::renderer, texture, NULL, &renderQuad, rotation, &axis, SDL_FLIP_NONE);
            }
            else SDL_RenderCopy(sdl::renderer, texture, NULL, &renderQuad);

            if(flags & TEX_ALPHA) SDL_SetTextureBlendMode(texture, oldBlendMode);

            if(resetColors)
            {
                SDL_SetTextureColorMod(texture, 255, 255, 255); // Reset color mod
                SDL_SetTextureAlphaMod(texture, 255); // Reset alpha mod
            }
        }
    }
}

