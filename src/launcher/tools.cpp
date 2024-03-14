#include "tools.h"

RGBA extractRGBA(int color)
{
    RGBA colorComponents;
    colorComponents.r = (color >> 24) & 0xFF; // Extract red component
    colorComponents.g = (color >> 16) & 0xFF; // Extract green component
    colorComponents.b = (color >> 8) & 0xFF;  // Extract blue component
    colorComponents.a = color & 0xFF;         // Extract alpha component
    return colorComponents;
}

bool is64bits() // checks if the current OS is 64 bits or not
{
#if defined(_WIN64)
    return true; // Windows 64-bit
#elif defined(_WIN32)
    BOOL f64 = false;
    return IsWow64Process(GetCurrentProcess(), &f64) && f64; // Windows 32-bit, checking for 64-bit emulation
#elif defined(__x86_64__) || defined(__ppc64__)
    return true; // Linux 64-bit
#else
    return false; // Linux 32-bit or other (like the costly apple thing)
#endif
}
