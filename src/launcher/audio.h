#ifndef AUDIO_H
#define AUDIO_H

#include "miniaudio.h"
#include "main.h"
#include <string>
#include <algorithm>

enum {S_MUSIC = 0, S_GRENADE, S_PLAYER, S_EXPLOSION, NUMSOUNDS};

namespace audio
{
    struct soundConfig
    {
        ma_sound *sound;
        const char *path;
    };

    extern void init();
    extern void updateMusic(int currentTime);
    extern void playSound(int soundId, vec2 pos = vec2(0, 0));
    extern void playMusic();
    extern void stopMusic();
    extern void destroy();
}

#endif
