#ifndef TEXTURES_H
#define TEXTURES_H

#include <SDL.h>
#include <string>
#include "main.h"

enum { TEX_BACKGROUND = 0, TEX_LOGO, TEX_ON, TEX_OFF, TEX_FRENCH, TEX_ENGLISH, TEX_RUSSIAN, TEX_SPANISH, TEX_AUDIOON, TEX_AUDIOOFF, TEX_SERVER, TEX_ABOUT, TEX_MINIMIZE, TEX_REDCROSS,
       TEX_PONG, TEX_HAP, TEX_NOEL, TEX_BALL, TEX_BALL_GLOW, PART_SPARK, PART_SMOKE, NUMTEXTURES };

struct textureConfig
{
    SDL_Texture* texture;
    std::string path;
};

extern textureConfig textures[NUMTEXTURES];

enum TextureFlags
{
    TEX_ALPHA = 1 << 0,    // render with alpha
    TEX_SHADOW = 2 << 0    // render with a shadow
};

namespace texture
{
    extern bool isValid(int id);    // checks if the texture is valid or not
    extern void init();             // load all textures
    extern void render(int id, int x, int y, int width, int height, int flags = 0, float rotation = 0.f, unsigned int color = 0); // render a texture
}

#endif
