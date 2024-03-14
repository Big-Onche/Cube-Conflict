#ifndef TOOLS_H
#define TOOLS_H

#if defined(_WIN32)
#include <windows.h>
#endif

#include <stdint.h>
#include <string>

#define rnd(x) (rand() % x)

#define loop(v,m) for(int v = 0; v < int(m); ++v)
#define loopi(m) loop(i,m)
#define loopj(m) loop(j,m)
#define space "\n"

extern bool is64bits();

struct RGBA { uint8_t r, g, b, a; };
extern RGBA extractRGBA(int color);
extern std::string lineBreak;

#endif
