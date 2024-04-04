#include "main.h"
#include "audio.h"

#if defined(__linux__)
#include <unistd.h>
#endif

namespace action
{
    std::string gamePath(bool win32 = false)
    {
    #if defined(_WIN32)
        if(win32) return "bin/cubeconflict";
        else return "bin64/cubeconflict";
    #elif defined(__linux__)
        return "bin_unix/cubeconflict";
    #endif
    }

    std::string logsArg() { return "-ggame_log.txt"; }
    std::string langArg() { return "-" + std::to_string(config::get(CONF_LANGUAGE)); }
    std::string steamArg() { return (isUsingSteam ? "-s" : ""); }

#if defined(_WIN32)
    bool winStart(bool dedicatedServer, bool forceGoodOld32bits) // Windows start code
    {
        bool goodOld32bits = (forceGoodOld32bits || !is64bits());
        std::string gameArgs = dedicatedServer ? "\"-u$HOME/My Games/Cube Conflict\" -gserver_log.txt -d" :
                                                 "\"-u$HOME/My Games/Cube Conflict\" " + logsArg() + " " + langArg() + " " + steamArg();

        std::string execCommand = "start " + gamePath(goodOld32bits) + " " + gameArgs;

        if(system(execCommand.c_str()) != 0)
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
            std::vector<std::string> args = {gamePath(), "-u$HOME/.cubeconflict", langArg()};
            std::vector<char*> clientArgs;
            for (const auto& arg : args) clientArgs.push_back(const_cast<char*>(arg.c_str()));
            if(isUsingSteam) clientArgs.push_back(const_cast<char*>("-s"));
            clientArgs.push_back(nullptr); // Add a null terminator

            std::string serverExec = gamePath() + " -d";
    		char* serverArgs[] = {const_cast<char*>("xterm"), const_cast<char*>("-e"), const_cast<char*>(serverExec.c_str()), nullptr};

            if(dedicatedServer ? (execvp(serverArgs[0], serverArgs) == -1) : (execvp(clientArgs[0], clientArgs.data()) == -1))
            {
                if(dedicatedServer) error::pop(getString("Error_Title").c_str(), getString("Error_Unix_Exec_Serv").c_str());
                else error::pop(getString("Error_Title").c_str(), getString("Error_Unix_Exec").c_str());
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
        if(config::get(CONF_MUSIC))
        {
            config::set(CONF_MUSIC, false);
            audio::stopMusic();
        }
        else
        {
            config::set(CONF_MUSIC, true);
            audio::playMusic();
        }
    }
}
