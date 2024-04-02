#include "main.h"
#include "buttons.h"
#include "textures.h"
#include "audio.h"
#include "actions.h"
#include "sdl.h"

// Button representation
namespace buttons
{
    std::vector<uiButton> buttonList;

    void renderCore(const uiButton& button) // Render the button's rect
    {
        RGBA buttonColor = button.hovered ? extractRGBA(button.hoverColor) : extractRGBA(button.color); // Choose color based on hover state

        if(buttonColor.a)
        {
            if(button.borders) // Render the button's borders
            {
                SDL_Rect borderRect = {button.x - 3, button.y - 3, button.width + 6, button.height + 6};
                SDL_SetRenderDrawColor(sdl::renderer, 0x44, 0x44, 0x44, buttonColor.a);
                SDL_RenderFillRect(sdl::renderer, &borderRect);
            }

            // Render the button's core
            SDL_Rect buttonRect = {button.x, button.y, button.width, button.height};
            SDL_SetRenderDrawColor(sdl::renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
            SDL_RenderFillRect(sdl::renderer, &buttonRect); // Draws the button's background
        }
    }

    void renderText(const uiButton& button) // Render the button's text
    {
        if(!button.text.empty() && sdl::fontMain != nullptr)
        {
            SDL_Color textColor = {40, 40, 40};
            SDL_Surface* textSurface = TTF_RenderUTF8_Blended(sdl::fontMain, button.text.c_str(), textColor);
            if(textSurface != nullptr)
            {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(sdl::renderer, textSurface);
                if(textTexture != nullptr)
                {
                    // Calculate text position to center it on the button
                    int textWidth = (textSurface->w * button.textSize);
                    int textHeight = (textSurface->h * button.textSize);
                    int center = (button.checkBox ? button.height : 0);
                    SDL_Rect textRect = {button.x + (button.width + center - textWidth) / 2, button.y + (button.height - textHeight) / 2, textWidth, textHeight};
                    SDL_RenderCopy(sdl::renderer, textTexture, nullptr, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
        }
    }

    void renderTexture(const uiButton& button)
    {
        if(button.checkBox)
        {
            texture::render(button.isOn() ? TEX_ON : TEX_OFF, button.x, button.y, button.height, button.height, TEX_ALPHA, 0, 0xFFFFFFFF);
        }
        else
        {
            if(!texture::isValid(button.onTex) || !texture::isValid(button.offTex)) return;
            texture::render(button.isOn() ? button.onTex : button.offTex, button.x, button.y, button.width, button.height, TEX_ALPHA, 0, button.hovered ? 0xAAAAAAAA : 0xFFFFFFFF);
        }
    }

    void renderButton(const uiButton& button)
    {
        SDL_SetRenderDrawBlendMode(sdl::renderer, SDL_BLENDMODE_BLEND);

        renderCore(button);
        renderText(button);
        renderTexture(button);
    }

    void add(const uiButton& button) { buttonList.push_back(button); }

    void renderHoverText(const std::string& hoverText)
    {
        if(hoverText.empty() || sdl::fontTiny == nullptr) return;

        SDL_Color textColor = {25, 25, 25};
        SDL_Color backgroundColor = {230, 230, 230};
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(sdl::fontTiny, hoverText.c_str(), textColor);

        if(textSurface != nullptr)
        {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(sdl::renderer, textSurface);
            if(textTexture != nullptr)
            {
                int textWidth = textSurface->w;
                int textHeight = textSurface->h;
                int padding = 5;
                int offsetY = 20;

                bool isTooCloseToTop = mouseY - textHeight - offsetY - padding < 0;
                int boxYPos = isTooCloseToTop ? (mouseY + offsetY) : (mouseY - textHeight - offsetY - padding);

                // Calculate initial x position of the background rect
                int boxXPos = mouseX - (textWidth / 2) - padding;

                if (boxXPos + textWidth + 2*padding > SCR_W) boxXPos = SCR_W - (textWidth + 2*padding); // Adjust x position if the box extends beyond the right side of the window

                if (boxXPos < 0) boxXPos = 0; // Adjust x position if the box extends beyond the left side of the window

                SDL_Rect backgroundRect = {boxXPos, boxYPos, textWidth + 2*padding, textHeight + 2*padding};

                SDL_SetRenderDrawColor(sdl::renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, 255);
                SDL_RenderFillRect(sdl::renderer, &backgroundRect);

                // Adjust the textRect x position based on the backgroundRect's position
                SDL_Rect textRect = {boxXPos + padding, boxYPos + padding, textWidth, textHeight};

                SDL_RenderCopy(sdl::renderer, textTexture, nullptr, &textRect);

                SDL_DestroyTexture(textTexture);
                SDL_FreeSurface(textSurface);
            }
        }
    }

    void render()
    {
        for(const auto& button : buttonList) renderButton(button);

        for(const auto& button : buttonList)
        {
            if(button.isHovered(mouseX, mouseY))
            {
                renderHoverText(button.hoverText);
                break;
            }
        }
    }

    void update(SDL_Event &e)
    {
        static Uint32 lastClickTime = 0;
        Uint32 currentClickTime = SDL_GetTicks();

        for(auto& button : buttonList)
        {
            button.updateHoverState(mouseX, mouseY);

            if(e.type == SDL_MOUSEBUTTONUP && button.isHovered(mouseX, mouseY) && currentClickTime - lastClickTime > 200) // Ensure there's at least a 200ms gap between clicks to avoid double triggers
            {
                button.click();
                lastClickTime = currentClickTime;
            }
        }
    }

    void init()
    {
        uiButton playGame(630, 325, 170, 40, 0xFFFFFFBB, 0xCCCCCCBB, true, getString("Play_Button"), "", -1, -1, 0.5f, false, []() { action::launchGame(); });
        buttons::add(playGame);

        int iconSize = 40;
        int iconWidthPos = 10;
        int iconHeightPos = 10;

        uiButton launchServer(iconWidthPos, iconHeightPos, iconSize, iconSize, 0, 0, false, "", getString("Info_Server"), TEX_SERVER, TEX_SERVER, 1, false, []() { action::launchGame(true); });
        buttons::add(launchServer);
        iconWidthPos += iconSize + 10;

        uiButton openAbout(iconWidthPos, iconHeightPos, iconSize, iconSize, 0, 0, false, "", getString("About_Title"), TEX_ABOUT, TEX_ABOUT, 1, false, []() {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, getString("About_Title").c_str(), getString("About_Content").c_str(), nullptr);
        });
        buttons::add(openAbout);

        uiButton exitLauncher(SCR_W - 50, 10, iconSize, iconSize, 0, 0, false, "", "", TEX_REDCROSS, TEX_REDCROSS, 1, false, []() { closeLauncher(); });
        buttons::add(exitLauncher);

        uiButton minimizeLauncher(SCR_W - 100, 10, iconSize, iconSize, 0, 0, false, "", "", TEX_MINIMIZE, TEX_MINIMIZE, 1, false, []() { audio::stopMusic(); SDL_MinimizeWindow(sdl::window); });
        buttons::add(minimizeLauncher);

        int flagWidth = 40;
        int flagHeight = 31;
        int flagWidthPos = 10;
        int flagHeightPos = SCR_H - (10 + flagHeight);

        uiButton setFrench(flagWidthPos, flagHeightPos, flagWidth, flagHeight, 0, 0, false, "", getString("Info_Lang", 0), TEX_FRENCH, TEX_FRENCH, 1, false, []() { setLanguage(FRENCH); });
        buttons::add(setFrench);
        flagWidthPos += flagWidth + 5;

        uiButton setEnglish(flagWidthPos, flagHeightPos, flagWidth, flagHeight, 0, 0, false, "", getString("Info_Lang", 1), TEX_ENGLISH, TEX_ENGLISH, 1, false, []() { setLanguage(ENGLISH); });
        buttons::add(setEnglish);
        flagWidthPos += flagWidth + 5;

        uiButton setRussian(flagWidthPos, flagHeightPos, flagWidth, flagHeight, 0, 0, false, "", getString("Info_Lang", 2), TEX_RUSSIAN, TEX_RUSSIAN, 1, false, []() { setLanguage(RUSSIAN); });
        buttons::add(setRussian);
        flagWidthPos += flagWidth + 5;

        uiButton setSpanish(flagWidthPos, flagHeightPos, flagWidth, flagHeight, 0, 0, false, "", getString("Info_Lang", 3), TEX_SPANISH, TEX_SPANISH, 1, false, []() { setLanguage(SPANISH); });
        buttons::add(setSpanish);

        uiButton setAudio(SCR_W - (10 + flagWidth), flagHeightPos, flagWidth, flagHeight, 0, 0, false, "", getString("Info_Music"), TEX_AUDIOON, TEX_AUDIOOFF, 1, false, []() { action::setupAudio(); }, &config::configVars[CONF_MUSIC].value);
        buttons::add(setAudio);

        uiButton setQuickLaunch(SCR_W - 240, flagHeightPos, 185, flagHeight, 0xCBCBCBFF, 0xCCCCCCFF, false, getString("QuickLaunch_Button"), getString("Info_QuickLaunch"), -1, -1, 0.3f, true, []() { config::set(CONF_QUICKLAUNCH, !config::configVars[CONF_QUICKLAUNCH].value); }, &config::configVars[CONF_QUICKLAUNCH].value);
        buttons::add(setQuickLaunch);
    }

    void add()
    {
        uiButton startPong(545, 375, 340, 25, 0xFFFFFFBB, 0xCCCCCCBB, true, getString("Play_Button_Pong"), "", -1, -1, 0.3f, false, []() { audio::stopMusic(); playPong = true; });
        buttons::add(startPong);
    }

    void destroy()
    {
        buttonList.clear();
    }
}
