#ifndef PARTICLES_H
#define PARTICLES_H

#include "textures.h"

namespace particles
{
    struct particle
    {
        int textureId;      // ID of the texture
        unsigned int color; // color
        int size;
        int initTime;       // Set to currentTime
        int lifeTime;       // Life duration in milliseconds or similar unit
        vec2 origin;        // Starting position
        vec2 position;      // Current position;
        vec2 destination;   // Ending position

        particle() : textureId(0), color(0), size(0), initTime(0), lifeTime(0), origin(vec2(0,0)), position(vec2(0,0)), destination(vec2(0,0)) {}
    };

    extern void update();

    extern void explosion(int textureId, vec2 origin, unsigned int color, int size, int num, int speed, int lifeTime);
    extern void stain(int textureId, vec2 origin, unsigned int color, int size, int lifeTime);
}

#endif
