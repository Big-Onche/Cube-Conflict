#ifndef BUTTONS_H
#define BUTTONS_H

#include <functional>
#include <SDL_image.h>

namespace buttons
{
    struct uiButton
    {
        int x, y; // Position on the window
        int width, height; // Size
        int color, hoverColor; // hexadecimal RGBA color
        std::string text, hoverText; // Button text
        int onTex, offTex; // textures based on on/off states
        std::function<void()> action; // Action to perform on click
        float textSize;

        // Constructor
        uiButton(SDL_Renderer* renderer, int x, int y, int width, int height, int color, int hoverColor, const std::string& text, const std::string& hoverText, int onTex, int offTex, float textSize, std::function<void()> action)
                : x(x), y(y), width(width), height(height), color(color), hoverColor(hoverColor), text(text), hoverText(hoverText), onTex(onTex), offTex(offTex), textSize(textSize), action(std::move(action)) {
        }

        bool hovered = false; // hovered state
        bool isOn = true;

        bool isHovered(int mouseX, int mouseY) const { return mouseX > x && mouseX < x + width && mouseY > y && mouseY < y + height; } // Check if the mouse is over the button

        void updateHoverState(int mouseX, int mouseY) { hovered = isHovered(mouseX, mouseY); }

        void click() const { action(); } // Execute the button's action
    };

    extern void render(SDL_Renderer* renderer);                     // render the buttons
    extern void update(SDL_Event &e);                               // update the events related to buttons
    extern void init();                                             // create the buttons
    extern void add();                                              // add a specific button
    extern void destroy();                                          // destroy the buttons
}

#endif
