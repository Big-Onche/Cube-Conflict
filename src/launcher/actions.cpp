#include "main.h"
#include "audio.h"

#if defined(__linux__)
#include <unistd.h>
#endif

namespace action
{
#if defined(_WIN32)
    bool winStart(bool dedicatedServer, bool forceGoodOld32bits) // Windows start code
    {
        bool goodOld32bits = (forceGoodOld32bits || !is64bits());
        std::string gamePath = goodOld32bits ? "bin/cubeconflict" : "bin64/cubeconflict";
        std::string gameArgs = dedicatedServer ? "\"-u$HOME/My Games/Cube Conflict\" -gserver_log.txt -d" : "\"-u$HOME/My Games/Cube Conflict\" -ggame_log.txt -" + std::to_string(currentLanguage);

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
            std::string execPath = "bin_unix/cubeconflict";
            std::string savePath = "-u$HOME/.cubeconflict";
            std::string logsName = "-ggame_log.txt";
            std::string langArg = "-" + std::to_string(currentLanguage);

            char* clientArgs[] = {const_cast<char*>(execPath.c_str()), const_cast<char*>(savePath.c_str()), const_cast<char*>(logsName.c_str()), const_cast<char*>(langArg.c_str()), nullptr};

            std::string serverExec = execPath + " -d";
    		char* serverArgs[] = {const_cast<char*>("xterm"), const_cast<char*>("-e"), const_cast<char*>(serverExec.c_str()), nullptr};

    		if(dedicatedServer ? (execvp(serverArgs[0], serverArgs) == -1) : (execvp(clientArgs[0], clientArgs) == -1))
            {
                error::pop(getString("Error_Title").c_str(), getString("Error_Unix_Exec").c_str());
                perror("execvp");
                return false;
            }
        }
        return true;
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
