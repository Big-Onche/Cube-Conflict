#include "main.h"
#include "audio.h"

#if defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#endif

namespace action
{
    std::string gamePath()
    {
    #if defined(_WIN32)
        return "bins/cc_client64";
    #elif defined(__linux__)
        return "bins/cc_client";
    #endif
    }

    std::string logsArg() { return "-ggame_log.txt"; }
    std::string langArg() { return "-" + std::to_string(config::get(CONF_LANGUAGE)); }
    std::string steamArg() { return (isUsingSteam ? "-s" : ""); }

#if defined(_WIN32)
    bool winStart(bool dedicatedServer) // Windows start code
    {
        std::string gameArgs = dedicatedServer ? "\"-u$HOME/My Games/Cube Conflict\" -gserver_log.txt -d" :
                                                 "\"-u$HOME/My Games/Cube Conflict\" " + logsArg() + " " + langArg() + " " + steamArg();

        std::string execCommand = "start " + gamePath() + " " + gameArgs;

        if(system(execCommand.c_str()) != 0)
        {
            std::string message = getString("Error_Game") + lineBreak + "bins/cc_client64.exe " + getString("Error_Missing");
            error::pop(getString("Error_Title"), message);
            return false;
        }

        return true;
    }
#elif defined(__linux__)
    bool linuxStart(bool dedicatedServer) // Linux start code
    {
        const char *script = "./run.sh";
        if(access(script, X_OK) == -1)
        {
            error::pop(getString("Error_Title"), getString("Error_Unix_Setperms"));
            perror("access");
            return false;
        }

        pid_t pid = fork();
        if(pid == -1)
        {
            error::pop(getString("Error_Title"), getString("Error_Unix_Fork"));
            return false;
        }
        else if(pid == 0) // Child process, execute the game binary
        {
            std::vector<std::string> args = { script, dedicatedServer ? "server" : "client" };
            if(!dedicatedServer)
            {
                args.push_back(langArg());
                if(isUsingSteam) args.push_back("-s");
            }
            std::vector<char *> execArgs;
            for(auto &arg : args) execArgs.push_back(const_cast<char *>(arg.c_str()));
            execArgs.push_back(nullptr);

            execv(script, execArgs.data());
            error::pop(getString("Error_Title"), dedicatedServer ? getString("Error_Unix_Exec_Serv") : getString("Error_Unix_Exec"));
            perror("execv");
            _exit(EXIT_FAILURE);
        }
        return true;
    }
#endif

    void launchGame(bool dedicatedServer)
    {
    #if defined(_WIN32)
        bool success = winStart(dedicatedServer);
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
