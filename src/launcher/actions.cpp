#include "main.h"
#include "audio.h"

#if defined(__linux__)
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace action
{
#if defined(_WIN32)
    bool winStart(bool dedicatedServer, bool forceGoodOld32bits) // Windows start code
    {
        bool goodOld32bits = (forceGoodOld32bits || !is64bits());
        std::string gamePath = goodOld32bits ? "bin/cubeconflict" : "bin64/cubeconflict";
        std::string gameArgs = dedicatedServer ? "\"-u$HOME/My Games/Cube Conflict\" -gserver_log.txt -d" : "\"-u$HOME/My Games/Cube Conflict\" -glog.txt -" + std::to_string(currentLanguage);

        std::string command = "start " + gamePath + " " + gameArgs;

        if(system(command.c_str()) != 0)
        {
            std::string bits = (goodOld32bits ? "" : "64");
            std::string message = getString("Error_Game") + lineBreak + "bin/" + bits + "/cubeconflict.exe " + getString("Error_Missing");
            error::pop(getString("Error_Title").c_str(), message);
            return false;
        }

        return true;
    }
#elif defined(__linux__)
    bool linuxStart(bool dedicatedServer) // Linux start code
    {
        std::string gamePath = "bin_unix/cubeconflict"; // Construct game executable path
        std::string gameArgs = dedicatedServer ? "-u$HOME/.cubeconflict -d " : "-u$HOME/.cubeconflict " + std::to_string(currentLanguage);

        if(setenv("LD_LIBRARY_PATH", "./bin_unix/:$LD_LIBRARY_PATH", 1) == -1) // Set LD_LIBRARY_PATH to include bin_unix directory relative to the launcher's directory
        {
            error::pop(getString("Error_Title").c_str(), getString("Error_Unix_Setenv").c_str());
            perror("setenv");
            return false;
        }

        pid_t pid = fork();
        if(pid == -1)
        {
            error::pop(getString("Error_Title").c_str(), getString("Error_Unix_Fork").c_str());
            return false;
        }
        else if(pid == 0) // Child process, execute the game binary
        {
            char* args[] = { const_cast<char*>(gamePath.c_str()), const_cast<char*>(gameArgs.c_str()), nullptr };
            if(execvp(args[0], args) == -1)
            {
                error::pop(getString("Error_Title").c_str(), getString("Error_Unix_Exec").c_str());
                perror("execvp");
                return false;
            }
        }
        else // Parent process
        {
            wait(nullptr); // Wait for the child process to finish
            return true;
        }
    }
#endif

    void launchGame(bool dedicatedServer, bool forceGoodOld32bits)
    {
    #if defined(_WIN32)
        bool success = winStart(dedicatedServer, forceGoodOld32bits);
    #elif defined(__linux__)
        bool success = linuxStart(dedicatedServer);
    #endif
        if(success && !dedicatedServer) closeLauncher();
    }

    void setupAudio()
    {
        if(audio::playSong)
        {
            audio::playSong = false;
            audio::stopMusic();
        }
        else
        {
            audio::playSong = true;
            audio::playMusic();
        }
    }
}
