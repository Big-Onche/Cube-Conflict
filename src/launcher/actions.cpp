#include "main.h"
#include "audio.h"

namespace action
{
    void launchGame(bool dedicatedServer, bool forceGoodOld32bits)
    {
        bool success = true;
        bool goodOld32bits = (forceGoodOld32bits || !is64bits());
        std::string gamePath = goodOld32bits ? "bin/cubeconflict" : "bin64/cubeconflict";
        std::string gameArgs = dedicatedServer ? "\"-u$HOME/My Games/Cube Conflict\" -gserver_log.txt -d" :
                                                 "\"-u$HOME/My Games/Cube Conflict\" -glog.txt -" + std::to_string(currentLanguage);

    #if defined(_WIN32)
        std::string command = "start " + gamePath + " " + gameArgs; // Windows start code
    #else
        std::string command = "./" + gamePath + " " + gameArgs + " &"; // Linux start code
    #endif

        if(system(command.c_str()) != 0)
        {
            std::string bits = (goodOld32bits ? "" : "64");
            std::string message = getString("Error_Game") + lineBreak + "bin/" + bits + "/cubeconflict.exe " + getString("Error_Missing");
            error::pop(getString("Error_Title").c_str(), message);
            success = false;
        }

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
