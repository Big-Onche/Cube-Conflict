#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "main.h"

namespace audio
{
    bool playSong = true;

    ma_engine audioEngine;
    ma_sound launcherSong;

    void init()
    {
        if(ma_engine_init(NULL, &audioEngine) != MA_SUCCESS) error::pop("Initialization error", "Could not initialize miniaudio");
        if(ma_sound_init_from_file(&audioEngine, "media/songs/launcher.mp3", 0, NULL, NULL, &launcherSong) != MA_SUCCESS) error::pop("Error", "Error while playing song");
    }

    bool isFadingIn = false;
    bool isFadingOut = false;
    uint32_t fadeStartTime = 0;
    float fadeDuration = 750; // Duration in milliseconds

    void update(int currentTime)
    {
        uint32_t elapsedTime = currentTime - fadeStartTime;

        if(isFadingIn)
        {
            float volume = std::min(1.0f, elapsedTime / fadeDuration);
            ma_sound_set_volume(&launcherSong, volume);

            if(volume >= 1.0f) isFadingIn = false; // Fade-in complete
        }
        else if(isFadingOut)
        {
            float volume = std::max(0.0f, 1.0f - (elapsedTime / fadeDuration));
            ma_sound_set_volume(&launcherSong, volume);

            if(volume <= 0.0f)
            {
                ma_sound_stop(&launcherSong);
                isFadingOut = false; // Fade-out complete
            }
        }
    }

    void playMusic()
    {
        isFadingIn = true;
        isFadingOut = false;
        fadeStartTime = currentTime;
        ma_sound_set_volume(&launcherSong, 0); // Start with volume 0
        ma_sound_start(&launcherSong);
    }

    void stopMusic()
    {
        isFadingIn = false;
        isFadingOut = true;
        fadeStartTime = currentTime;
    }

    void destroy()
    {
        ma_sound_uninit(&launcherSong);
        ma_engine_uninit(&audioEngine);
    }
}
