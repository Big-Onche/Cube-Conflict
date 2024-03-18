#ifndef TOOLS_H
#define TOOLS_H

#if defined(_WIN32)
#include <windows.h>
#endif

#include <stdint.h>
#include <string>
#include <cmath>

#define rnd(x) (rand() % x)

#define loop(v,m) for(int v = 0; v < int(m); ++v)
#define loopi(m) loop(i,m)
#define loopj(m) loop(j,m)
#define space "\n"

extern bool is64bits();

extern std::string lineBreak;
struct RGBA { uint8_t r, g, b, a; };
extern RGBA extractRGBA(int color);

struct vec2
{
    float x, y;

    vec2(float xVal, float yVal) : x(xVal), y(yVal) {}

    vec2 operator+(const vec2& rhs) const { return vec2(x + rhs.x, y + rhs.y); }
    vec2& operator+=(const vec2& rhs) { x += rhs.x; y += rhs.y; return *this; }
    vec2 operator-(const vec2& rhs) const { return vec2(x - rhs.x, y - rhs.y); }
    vec2& operator-=(const vec2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
    vec2 operator*(float scalar) const { return vec2(x * scalar, y * scalar); }
    vec2& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
};

#endif
