#ifndef AUDIO_H
#define AUDIO_H

namespace audio
{
    extern bool playSong;

    extern void init();
    extern void update(int currentTime);
    extern void playMusic();
    extern void stopMusic();
    extern void destroy();
}

#endif
