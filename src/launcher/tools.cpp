#include "tools.h"

std::string lineBreak = "\n";

RGBA extractRGBA(int color)
{
    RGBA colorComponents;
    colorComponents.r = (color >> 24) & 0xFF; // Extract red component
    colorComponents.g = (color >> 16) & 0xFF; // Extract green component
    colorComponents.b = (color >> 8) & 0xFF;  // Extract blue component
    colorComponents.a = color & 0xFF;         // Extract alpha component
    return colorComponents;
}
