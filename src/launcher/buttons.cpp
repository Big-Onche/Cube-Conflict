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

    void renderButton(SDL_Renderer* renderer, const uiButton& button)
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
        if(!button.text.empty() && sdl::fontMain != nullptr)
        {
            SDL_Color textColor = {40, 40, 40};
            SDL_Surface* textSurface = TTF_RenderUTF8_Blended(sdl::fontMain, button.text.c_str(), textColor);
            if(textSurface != nullptr)
            {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if(textTexture != nullptr)
                {
                    // Calculate text position to center it on the button
                    int textWidth = (textSurface->w * button.textSize);
                    int textHeight = (textSurface->h * button.textSize);
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

    void add(const uiButton& button) { buttonList.push_back(button); }

    void renderHoverText(const std::string& hoverText, SDL_Renderer* renderer)
    {
        if(hoverText.empty() || sdl::fontTiny == nullptr) return;

        SDL_Color textColor = {25, 25, 25};
        SDL_Color backgroundColor = {230, 230, 230};
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(sdl::fontTiny, hoverText.c_str(), textColor);

        if(textSurface != nullptr)
        {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
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

                SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, 255);
                SDL_RenderFillRect(renderer, &backgroundRect);

                // Adjust the textRect x position based on the backgroundRect's position
                SDL_Rect textRect = {boxXPos + padding, boxYPos + padding, textWidth, textHeight};

                SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

                SDL_DestroyTexture(textTexture);
                SDL_FreeSurface(textSurface);
            }
        }
    }

    void render(SDL_Renderer* renderer)
    {
        for(const auto& button : buttonList) renderButton(renderer, button);

        for(const auto& button : buttonList)
        {
            if(button.isHovered(mouseX, mouseY))
            {
                renderHoverText(button.hoverText, renderer);
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
                button.isOn = !button.isOn;
                lastClickTime = currentClickTime;
            }
        }
    }

    void init()
    {
        uiButton playGame(sdl::renderer, 630, 325, 170, 40, 0xFFFFFFBB, 0xCCCCCCBB, getString("Play_Button"), "", -1, -1, 0.5f, []() { action::launchGame(); });
        buttons::add(playGame);

        int iconSize = 40;
        int iconWidthPos = 10;
        int iconHeightPos = 10;

        uiButton launchServer(sdl::renderer, iconWidthPos, iconHeightPos, iconSize, iconSize, 0, 0, "", getString("Info_Server"), TEX_SERVER, TEX_SERVER, 1, []() { action::launchGame(true); });
        buttons::add(launchServer);
        iconWidthPos += iconSize + 10;

        uiButton openAbout(sdl::renderer, iconWidthPos, iconHeightPos, iconSize, iconSize, 0, 0, "", getString("About_Title"), TEX_ABOUT, TEX_ABOUT, 1, []() {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, getString("About_Title").c_str(), getString("About_Content").c_str(), nullptr);
        });
        buttons::add(openAbout);

        uiButton exitLauncher(sdl::renderer, SCR_W - 50, 10, iconSize, iconSize, 0, 0, "", "", TEX_REDCROSS, TEX_REDCROSS, 1, []() { closeLauncher(); });
        buttons::add(exitLauncher);

        uiButton minimizeLauncher(sdl::renderer, SCR_W - 100, 10, iconSize, iconSize, 0, 0, "", "", TEX_MINIMIZE, TEX_MINIMIZE, 1, []() { audio::stopMusic(); SDL_MinimizeWindow(sdl::window); });
        buttons::add(minimizeLauncher);

        int flagWidth = 40;
        int flagHeight = 31;
        int flagWidthPos = 10;
        int flagHeightPos = SCR_H - (10 + flagHeight);

        uiButton setFrench(sdl::renderer, flagWidthPos, flagHeightPos, flagWidth, flagHeight, 0, 0, "", getString("Info_Lang", 0), TEX_FRENCH, TEX_FRENCH, 1, []() { setLanguage(FRENCH); });
        buttons::add(setFrench);
        flagWidthPos += flagWidth + 5;

        uiButton setEnglish(sdl::renderer, flagWidthPos, flagHeightPos, flagWidth, flagHeight, 0, 0, "", getString("Info_Lang", 1), TEX_ENGLISH, TEX_ENGLISH, 1, []() { setLanguage(ENGLISH); });
        buttons::add(setEnglish);
        flagWidthPos += flagWidth + 5;

        uiButton setRussian(sdl::renderer, flagWidthPos, flagHeightPos, flagWidth, flagHeight, 0, 0, "", getString("Info_Lang", 2), TEX_RUSSIAN, TEX_RUSSIAN, 1, []() { setLanguage(RUSSIAN); });
        buttons::add(setRussian);
        flagWidthPos += flagWidth + 5;

        uiButton setSpanish(sdl::renderer, flagWidthPos, flagHeightPos, flagWidth, flagHeight, 0, 0, "", getString("Info_Lang", 3), TEX_SPANISH, TEX_SPANISH, 1, []() { setLanguage(SPANISH); });
        buttons::add(setSpanish);

        uiButton setAudio(sdl::renderer, SCR_W - (10 + flagWidth), flagHeightPos, flagWidth, flagHeight, 0, 0, "", getString("Info_Music"), TEX_AUDIOON, TEX_AUDIOOFF, 1, []() { action::setupAudio(); });
        buttons::add(setAudio);
    }

    void add()
    {
        uiButton startPong(sdl::renderer, 545, 375, 340, 25, 0xFFFFFFBB, 0xCCCCCCBB, getString("Play_Button_Pong"), "", -1, -1, 0.3f, []() { audio::stopMusic(); playPong = true; });
        buttons::add(startPong);
    }

    void destroy()
    {
        buttonList.clear();
    }
}
