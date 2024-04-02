#include "main.h"
#include "actions.h"
#include "buttons.h"
#include "textures.h"
#include "audio.h"
#include "sdl.h"
#include "pong.h"

bool isUsingSteam = false;

bool initGameLauncher()
{
    config::load();

    if(config::get(CONF_QUICKLAUNCH))
    {
        action::launchGame();
        exit(EXIT_SUCCESS);
    }

    if(config::get(CONF_FIRSTLAUNCH)) detectSystemLanguage();
    else setLanguage(config::get(CONF_LANGUAGE), true);
    if(!sdl::init()) return false;
    audio::init();
    if(config::get(CONF_MUSIC)) audio::playMusic();
    texture::init();
    buttons::init();
    pong::init();
    return true;
}

void closeLauncher() // Destroy window and quit SDL subsystems
{
    config::set(CONF_FIRSTLAUNCH, false);
    config::write();
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
    switch(e.window.event)
    {
        case SDL_WINDOWEVENT_MINIMIZED:
            if(config::get(CONF_MUSIC)) audio::stopMusic();
            break;

        case SDL_WINDOWEVENT_RESTORED:
            if(config::get(CONF_MUSIC)) audio::playMusic();
            break;
    }
}

int mouseX, mouseY;

bool playPong = false, addedPong = false;

int main(int argc, char* argv[])
{
    loopi(argc)
    {
        std::string arg = argv[i];
        if(arg == "-steam") isUsingSteam = true;
    }

    if(initGameLauncher())
    {
        SDL_Event e;

        for(;;)
        {
            frameStart = SDL_GetTicks();

            SDL_GetMouseState(&mouseX, &mouseY);
            SDL_PumpEvents();

            if(playPong) pong::play(&e);
            else
            {
                while(SDL_PollEvent(&e) != 0)
                {
                    buttons::update(e);

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

                texture::render(TEX_BACKGROUND, 0, 0, SCR_W, SCR_H);
                texture::render(TEX_LOGO, 460, 140, 512, 180, TEX_ALPHA|TEX_SHADOW);

                buttons::render();

                SDL_RenderPresent(sdl::renderer); // Update screen
            }

            audio::updateMusic(currentTime);

            if(!addedPong && currentTime > 30000)
            {
                buttons::add();
                addedPong = true;
            }

            currentTime = SDL_GetTicks();
            frameTime = currentTime - frameStart;
            if(frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
        }
    }
    else closeLauncher();
}
