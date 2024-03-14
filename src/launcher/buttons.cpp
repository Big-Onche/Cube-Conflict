#include "main.h"
#include "buttons.h"
#include "textures.h"
#include "audio.h"
#include "actions.h"
#include "sdl.h"

// Button representation
namespace buttons
{
    std::vector<Button> buttonList;

    void renderButton(SDL_Renderer* renderer, const Button& button)
    {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        RGBA buttonColor = button.hovered ? extractRGBA(button.hoverColor) : extractRGBA(button.color); // Choose color based on hover state

        // Render the button's borders
        SDL_Rect borderRect = {button.x - 3, button.y - 3, button.width + 6, button.height + 6};
        SDL_SetRenderDrawColor(renderer, 0x44, 0x44, 0x44, buttonColor.a);
        SDL_RenderFillRect(renderer, &borderRect);

        // Render the button's core
        SDL_Rect buttonRect = {button.x, button.y, button.width, button.height};
        SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
        SDL_RenderFillRect(renderer, &buttonRect); // Draws the button's background

        // Render the button's text
        if(!button.text.empty() && sdl::font != nullptr)
        {
            SDL_Color textColor = {40, 40, 40};
            TTF_SetFontStyle(sdl::font, TTF_STYLE_BOLD);
            SDL_Surface* textSurface = TTF_RenderUTF8_Blended(sdl::font, button.text.c_str(), textColor);
            if(textSurface != nullptr)
            {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if(textTexture != nullptr)
                {
                    // Calculate text position to center it on the button
                    int textWidth = textSurface->w;
                    int textHeight = textSurface->h;
                    SDL_Rect textRect = {button.x + (button.width - textWidth) / 2, button.y + (button.height - textHeight) / 2, textWidth, textHeight};
                    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
        }

        if(texture::isValid(button.onTex) && texture::isValid(button.offTex))
        {
            SDL_Texture* buttonTexture = (button.isOn ? textures[button.onTex].texture : textures[button.offTex].texture);

            if(buttonTexture != nullptr)
            {
                int textureWidth, textureHeight;
                SDL_QueryTexture(buttonTexture, nullptr, nullptr, &textureWidth, &textureHeight); // Query the texture to get its width and height

                float scale = button.hovered ? 0.6f : 1.0f; // Increase the size for hover effect
                textureWidth = static_cast<int>(textureWidth * scale);
                textureHeight = static_cast<int>(textureHeight * scale);

                int textureX = button.x + (button.width - textureWidth) / 2; // Centers the logo in the button
                int textureY = button.y + (button.height - textureHeight) / 2;

                SDL_Rect textureRect = {textureX, textureY, textureWidth, textureHeight};

                if(textureWidth > button.width || textureHeight > button.height) // Adjust the rect if the scaled texture is larger than the button
                {
                    float aspectRatio = static_cast<float>(textureWidth) / static_cast<float>(textureHeight);
                    if(aspectRatio > 1.0f) // Width is greater than height
                    {
                        textureRect.w = button.width;
                        textureRect.h = static_cast<int>(button.width / aspectRatio);
                    }
                    else // Height is greater than width
                    {
                        textureRect.h = button.height;
                        textureRect.w = static_cast<int>(button.height * aspectRatio);
                    }

                    textureRect.x = button.x + (button.width - textureRect.w) / 2; // Recalculate to center the adjusted texture
                    textureRect.y = button.y + (button.height - textureRect.h) / 2;
                }


                if(button.hovered) SDL_SetTextureColorMod(buttonTexture, 180, 180, 180); // Hovering color modulation
                else SDL_SetTextureColorMod(buttonTexture, 255, 255, 255);

                SDL_RenderCopy(renderer, buttonTexture, nullptr, &textureRect);

                SDL_SetTextureColorMod(buttonTexture, 255, 255, 255);
            }
        }
    }

    void add(const Button& button) { buttonList.push_back(button); }

    void render(SDL_Renderer* renderer) { for(const auto& button : buttonList) renderButton(renderer, button); }

    void update(SDL_Event &e, int mouseX, int mouseY)
    {
        static Uint32 lastClickTime = 0;
        Uint32 currentClickTime = SDL_GetTicks();

        for(auto& button : buttonList)
        {
            button.updateHoverState(mouseX, mouseY);
            if(e.type == SDL_MOUSEBUTTONUP && button.isHovered(mouseX, mouseY) && currentClickTime - lastClickTime > 200) // Ensure there's at least a 200ms gap between clicks to avoid double triggers
            {
                button.click();
                button.isOn = !button.isOn;
                lastClickTime = currentClickTime;
            }
        }
    }

    void init()
    {
        Button playGame(sdl::renderer, 630, 325, 170, 40, 0xFFFFFFBB, 0xCCCCCCBB, getString("Play_Button"), -1, -1, []() { action::launchGame(); });
        buttons::add(playGame);

        Button openAbout(sdl::renderer, 10, 10, 40, 40, 0, 0, "", TEX_ABOUT, TEX_ABOUT, []() {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, getString("About_Title").c_str(), getString("About_Content").c_str(), nullptr);
        });
        buttons::add(openAbout);

        Button exitLauncher(sdl::renderer, SCR_W - 50, 10, 40, 40, 0, 0, "", TEX_REDCROSS, TEX_REDCROSS, []() { closeLauncher(); });
        buttons::add(exitLauncher);

        Button minimizeLauncher(sdl::renderer, SCR_W - 100, 10, 40, 40, 0, 0, "", TEX_MINIMIZE, TEX_MINIMIZE, []() { audio::stopMusic(); SDL_MinimizeWindow(sdl::window); });
        buttons::add(minimizeLauncher);

        int flagWidth = 40;
        int flagHeight = 31;
        int flagWidthPos = 10;
        int flagHeightPos = SCR_H - (10 + flagHeight);

        Button setFrench(sdl::renderer, flagWidthPos, flagHeightPos, flagWidth, flagHeight, 0, 0, "", TEX_FRENCH, TEX_FRENCH, []() { setLanguage(FRENCH); });
        buttons::add(setFrench);
        flagWidthPos += flagWidth + 5;

        Button setEnglish(sdl::renderer, flagWidthPos, flagHeightPos, flagWidth, flagHeight, 0, 0, "", TEX_ENGLISH, TEX_ENGLISH, []() { setLanguage(ENGLISH); });
        buttons::add(setEnglish);
        flagWidthPos += flagWidth + 5;

        Button setRussian(sdl::renderer, flagWidthPos, flagHeightPos, flagWidth, flagHeight, 0, 0, "", TEX_RUSSIAN, TEX_RUSSIAN, []() { setLanguage(RUSSIAN); });
        buttons::add(setRussian);
        flagWidthPos += flagWidth + 5;

        Button setSpanish(sdl::renderer, flagWidthPos, flagHeightPos, flagWidth, flagHeight, 0, 0, "", TEX_SPANISH, TEX_SPANISH, []() { setLanguage(SPANISH); });
        buttons::add(setSpanish);

        Button setAudio(sdl::renderer, SCR_W - (10 + flagWidth), flagHeightPos, flagWidth, flagHeight, 0, 0, "", TEX_AUDIOON, TEX_AUDIOOFF, []() { action::setupAudio(); });
        buttons::add(setAudio);
    }

    void destroy()
    {
        buttonList.clear();
    }
}
