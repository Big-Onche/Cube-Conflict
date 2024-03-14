#ifndef LOGS_H
#define LOGS_H

#include <iostream>

namespace error
{
    extern void pop(std::string windowTitle, std::string windowMessage, bool sdlError = false); // error pop-up
    extern std::string get(); // get the last system error
}

#endif
