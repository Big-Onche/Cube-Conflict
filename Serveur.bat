@ECHO OFF

set TESS_BIN=bin

IF EXIST bin64\cubeconflict.exe (
    IF /I "%PROCESSOR_ARCHITECTURE%" == "amd64" (
        set TESS_BIN=bin64
    )
    IF /I "%PROCESSOR_ARCHITEW6432%" == "amd64" (
        set TESS_BIN=bin64
    )
)

start %TESS_BIN%\cubeconflict.exe "-u$HOME\My Games\Cube Conflict Respawn" -glog.txt -d %*
