#if defined(_WIN32)
#include <windows.h>
#else
#include <errno.h>
#include <cstring>
#endif

#include <string>
#include <SDL.h>

namespace error
{
    void pop(std::string windowTitle, std::string windowMessage, bool sdlError)
    {
        if(sdlError) windowMessage += "\nSDL_Error: " + std::string(SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, windowTitle.c_str(), windowMessage.c_str(), nullptr);
    }

    std::string get()
    {
        #if defined(_WIN32)
        // Windows-specific code
        DWORD errorMessageID = ::GetLastError();
        if(errorMessageID == 0) return "No error message has been recorded";

        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);
        return message;
        #else
        // Linux-specific code
        if(errno == 0) return "No error message has been recorded";
        return std::strerror(errno);
        #endif
    }
}
