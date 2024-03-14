#ifndef AUDIO_H
#define AUDIO_H

namespace audio
{
    extern void init();
    extern void updateFading(int currentTime);
    extern void playMusic();
    extern void stopMusic();
    extern void destroy();
}

#endif
