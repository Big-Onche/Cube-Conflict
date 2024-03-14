#include "main.h"
#include "buttons.h"
#include "textures.h"
#include "audio.h"
#include "sdl.h"

bool initGameLauncher()
{
    setLanguage(ENGLISH, true);
    if(!sdl::init()) return false;
    audio::init();
    audio::playMusic();
    texture::init();
    buttons::init();
    return true;
}

void closeLauncher() // Destroy window and quit SDL subsystems
{
    buttons::destroy();
    sdl::destroy();
    audio::destroy();
    exit(EXIT_SUCCESS);
}

int currentTime;

const int FPS = 60; // no need to render it faster
const int frameDelay = 1000 / FPS;

Uint32 frameStart;
int frameTime;

bool isDragging = false;
int startX = 0, startY = 0;

void checkWindowEvent(SDL_Event &e)
{
    switch (e.window.event)
    {
        case SDL_WINDOWEVENT_MINIMIZED:
        audio::stopMusic();
        break;

    case SDL_WINDOWEVENT_RESTORED:
        audio::playMusic();
        break;
    }
}

int main()
{
    if(initGameLauncher())
    {
        SDL_Event e;

        for(;;)
        {
            frameStart = SDL_GetTicks();

            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            SDL_PumpEvents();

            while(SDL_PollEvent(&e) != 0)
            {
                buttons::update(e, mouseX, mouseY);

                switch(e.type)
                {
                    case SDL_WINDOWEVENT:
                        checkWindowEvent(e);
                        break;

                    case SDL_MOUSEBUTTONDOWN:
                        if(e.button.button == SDL_BUTTON_LEFT)
                        {
                            isDragging = true;
                            startX = e.button.x;
                            startY = e.button.y;
                        }
                        break;

                    case SDL_MOUSEBUTTONUP:
                        if(e.button.button == SDL_BUTTON_LEFT) isDragging = false;
                        break;

                    case SDL_MOUSEMOTION:
                        if(isDragging)
                        {
                            int x, y;
                            SDL_GetWindowPosition(sdl::window, &x, &y);
                            SDL_SetWindowPosition(sdl::window, x + e.motion.xrel / 1.4, y + e.motion.yrel / 1.4);
                        }
                        break;

                    case SDL_QUIT:
                        closeLauncher();
                        break;
                }
            }

            SDL_SetRenderDrawColor(sdl::renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderClear(sdl::renderer); // Clear screen

            SDL_Rect backgroundRect = {0, 0, SCR_W, SCR_H};
            SDL_RenderCopy(sdl::renderer, textures[TEX_BACKGROUND].texture, nullptr, &backgroundRect); // background texture

            // Set color modulation for the texture to create a shadow effect (dark and semi-transparent)
            SDL_SetTextureColorMod(textures[TEX_LOGO].texture, 0, 0, 0);  // Set the shadow color (black in this case)
            SDL_SetTextureAlphaMod(textures[TEX_LOGO].texture, 128);     // Semi-transparent
            SDL_Rect shadowRect = {470, 150, 512, 180}; // Calculate the shadow's position (slightly offset from the logo's position)
            SDL_RenderCopy(sdl::renderer, textures[TEX_LOGO].texture, nullptr, &shadowRect); // Render the shadow

            // Reset color modulation for the texture to render the actual logo normally
            SDL_SetTextureColorMod(textures[TEX_LOGO].texture, 255, 255, 255);  // Reset to default (no color modulation)
            SDL_SetTextureAlphaMod(textures[TEX_LOGO].texture, 255);            // Fully opaque

            SDL_Rect logoRect = {460, 140, 512, 180};
            SDL_RenderCopy(sdl::renderer, textures[TEX_LOGO].texture, nullptr, &logoRect); // Render the logo over the shadow

            buttons::render(sdl::renderer);
            audio::update(currentTime);

            SDL_RenderPresent(sdl::renderer); // Update screen

            currentTime = SDL_GetTicks();
            frameTime = currentTime - frameStart;
            if(frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
        }
    }
    else closeLauncher();
}
