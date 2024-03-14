#include "main.h"
#include "audio.h"

namespace action
{
    void launchGame(bool forceGoodOld32bits)
    {
        bool success = true;
        bool goodOld32bits = (forceGoodOld32bits || !is64bits());
        std::string gamePath = goodOld32bits ? "bin/cubeconflict" : "bin64/cubeconflict";
        std::string gameArgs = "\"-u$HOME/My Games/Cube Conflict\" -glog.txt";

    #if defined(_WIN32)
        std::string command = "start " + gamePath + " " + gameArgs; // Windows start code
    #else
        std::string command = "./" + gamePath + " " + gameArgs + " &"; // Linux start code
    #endif

        if(system(command.c_str()) != 0)
        {
            std::string bits = (goodOld32bits ? "32" : "64");
            std::string message = "Error while launching the game.\nThe " + bits + " bits binary is missing";
            error::pop("Game launch error", message);
            success = false;
        }

       if(success) closeLauncher();
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
