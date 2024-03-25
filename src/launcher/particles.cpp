#include "particles.h"

namespace particles
{
    std::vector<particle> particles;

    particle create(int textureId, int color, int size, vec2 origin, vec2 destination, int lifeTime)
    {
        particle p;
        p.textureId = textureId;
        p.color = color;
        p.size = size;
        p.initTime = currentTime;
        p.lifeTime = lifeTime;
        p.origin = origin;
        p.position = origin;
        p.destination = destination;
        return p;
    }

    void render(particle &p, int remainingTime)
    {
        if(remainingTime <= 750) // Fading effect
        {
            unsigned char newAlpha = static_cast<unsigned char>(255 * (static_cast<float>(remainingTime) / 750.0f)); // Scale alpha
            unsigned int newColor = (p.color & 0xFFFFFF00) | newAlpha; // Preserve the RGB components and apply the new alpha
            p.color = newColor;
        }

        texture::render(p.textureId, p.position.x, p.position.y, p.size, p.size, 0, 0, p.color);
    }

    bool manage(particle &p)
    {
        int elapsedTime = currentTime - p.initTime;

        if(elapsedTime > p.lifeTime) return false; // Indicate that this particle should be removed

        float progress = static_cast<float>(elapsedTime) / static_cast<float>(p.lifeTime);

        p.position.x = p.origin.x + (p.destination.x - p.origin.x) * progress; // Linear interpolation between origin and destination based on the progress
        p.position.y = p.origin.y + (p.destination.y - p.origin.y) * progress;

        render(p, p.lifeTime - elapsedTime);

        return true; // Particle is still alive
    }

    void update()
    {
        for(auto it = particles.begin(); it != particles.end(); )
        {
            if(!manage(*it)) it = particles.erase(it);
            else ++it;
        }
    }

    void explosion(int textureId, vec2 origin, unsigned int color, int size, int num, int speed, int lifeTime)
    {
        loopi(num)
        {
            speed = std::max(speed, 1);
            vec2 destination = origin + vec2(-speed + rnd((speed * 2) + 1), -speed + rnd((speed * 2) + 1));
            particles.push_back(create(textureId, color, size, origin, destination, lifeTime));
        }
    }

    void stain(int textureId, vec2 origin, unsigned int color, int size, int lifeTime)
    {
        particles.push_back(create(textureId, color, size, origin, origin, lifeTime));
    }

}

