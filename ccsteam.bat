@ECHO OFF

set CC_BIN=bin

IF EXIST bin64\cubeconflict.exe (
    IF /I "%PROCESSOR_ARCHITECTURE%" == "amd64" (
        set CC_BIN=bin64
    )
    IF /I "%PROCESSOR_ARCHITEW6432%" == "amd64" (
        set CC_BIN=bin64
    )
)

start %CC_BIN%\cubeconflict.exe "-u$HOME\My Games\Cube Conflict" -glog.txt -s%*