#define MINIAUDIO_IMPLEMENTATION
#include "audio.h"

namespace audio
{
    soundConfig sounds[NUMSOUNDS] =
    {
        {nullptr, "media/songs/launcher.mp3"},              // S_MUSIC
        {nullptr, "media/sounds/physics/grenade_1.wav"},    // S_GRENADE
        {nullptr, "media/sounds/physics/pixel_1.wav"},      // S_PLAYER
        {nullptr, "media/sounds/explosions/grenade_1.wav"}  // S_EXPLOSION
    };

    ma_engine audioEngine;

    void init()
    {
        if(ma_engine_init(NULL, &audioEngine) != MA_SUCCESS) error::pop(getString("Error_Title"), getString("Error_MA_Init"));

        loopi(NUMSOUNDS)
        {
            sounds[i].sound = new ma_sound;
            if(ma_sound_init_from_file(&audioEngine, sounds[i].path, 0, NULL, NULL, sounds[i].sound) != MA_SUCCESS)
            {
                std::string message = getString("Error_MA_Load") + " " + sounds[i].path;
                error::pop(getString("Error_Title"), message);
            }
        }
    }

    bool isFadingIn = false;
    bool isFadingOut = false;
    uint32_t fadeStartTime = 0;
    float fadeDuration = 750; // Duration in milliseconds

    void updateMusic(int currentTime)
    {
        uint32_t elapsedTime = currentTime - fadeStartTime;

        if(isFadingIn)
        {
            float volume = std::min(1.0f, elapsedTime / fadeDuration);
            ma_sound_set_volume(sounds[S_MUSIC].sound, volume);

            if(volume >= 1.0f) isFadingIn = false; // Fade-in complete
        }
        else if(isFadingOut)
        {
            float volume = std::max(0.0f, 1.0f - (elapsedTime / fadeDuration));
            ma_sound_set_volume(sounds[S_MUSIC].sound, volume);

            if(volume <= 0.0f)
            {
                ma_sound_stop(sounds[S_MUSIC].sound);
                isFadingOut = false; // Fade-out complete
            }
        }
    }

    vec2 cameraPos(SCR_W / 2, SCR_H / 2);

    void spatializeSound(ma_sound* sound, vec2 soundPos)
    {
        vec2 dir = soundPos - cameraPos;
        float distance = sqrt(dir.x * dir.x + dir.y * dir.y);

        float maxDistance = std::max(SCR_W, SCR_H); // Maximum distance for volume scaling
        float volume = 1.0f - (distance / maxDistance); // Simple linear attenuation
        ma_sound_set_volume(sound, volume);

        float pan = dir.x / (SCR_W / 2); // Normalize pan based on screen width, shift to center
        ma_sound_set_pan(sound, pan);
    }

    void playSound(int soundId, vec2 pos)
    {
        if(pos.x != 0 && pos.y != 0) spatializeSound(sounds[soundId].sound, pos);
        ma_sound_start(sounds[soundId].sound);
    }

    void playMusic()
    {
        isFadingIn = true;
        isFadingOut = false;
        fadeStartTime = currentTime;
        ma_sound_set_volume(sounds[S_MUSIC].sound, 0); // Start with volume 0
        ma_sound_start(sounds[S_MUSIC].sound);
    }

    void stopMusic()
    {
        isFadingIn = false;
        isFadingOut = true;
        fadeStartTime = currentTime;
    }

    void destroy()
    {
        loopi(NUMSOUNDS)
        {
            ma_sound_uninit(sounds[i].sound);
            delete sounds[i].sound;
        }

        ma_engine_uninit(&audioEngine);
    }
}
