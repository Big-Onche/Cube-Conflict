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
        bool borders;
        std::string text, hoverText; // Button text
        int onTex, offTex; // textures based on on/off states
        std::function<void()> action; // Action to perform on click
        float textSize;
        bool checkBox;
        int* linkedVar = nullptr; // Pointer to a linked boolean variable

        // Constructor
        uiButton(int x, int y, int width, int height, int color, int hoverColor, bool borders, const std::string& text, const std::string& hoverText,
                 int onTex, int offTex, float textSize, bool checkBox, std::function<void()> action, int* linkedVar = nullptr)
            : x(x), y(y), width(width), height(height), color(color), hoverColor(hoverColor), borders(borders), text(text), hoverText(hoverText),
              onTex(onTex), offTex(offTex), textSize(textSize), checkBox(checkBox), action(std::move(action)), linkedVar(linkedVar) {}

        bool hovered = false; // hovered state

        bool isOn() const
        {
            if(linkedVar) return *linkedVar;
            else return false; // Default to false if no variable is linked
        }

        bool isHovered(int mouseX, int mouseY) const { return mouseX > x && mouseX < x + width && mouseY > y && mouseY < y + height; } // Check if the mouse is over the button

        void updateHoverState(int mouseX, int mouseY) { hovered = isHovered(mouseX, mouseY); }

        void click() const { action(); } // Execute the button's action
    };

    extern void render();                                           // render the buttons
    extern void update(SDL_Event &e);                               // update the events related to buttons
    extern void init();                                             // create the buttons
    extern void add();                                              // add a specific button
    extern void destroy();                                          // destroy the buttons
}

#endif
