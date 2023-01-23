# How to compile the game?

*nix versions of Cube Conflict client.
The clients function identical to the win32 client, see config.html for more
information about command-line options if necessary.

Please run "native_client" from the root Cube Conflict dir (NOT from inside the "bin_unix"
directory) to launch these, or set the CC_DATA variable at the top of the "ccunix.sh" 
script to an absolute path to allow it to be run from any location. Note that the "ccunix.sh" 
script is set up to write any files (saved maps, configs, etc.) into the user's home 
directory at "~/.cubeconflict".

Clients will need the following dynamic link libraries present:
* libGL (OpenGL)
* SDL2 (>= 2.0.0)
* SDL2_image
* SDL2_mixer
* libpng
* libjpeg
* zlib

If native binaries for your platform are not included, then try the following:
1) Ensure you have the DEVELOPMENT VERSIONS of the above libraries installed.
2) Type "make -C src install".
3) Re-run the "ccunix.sh" script if it succeeded.

The servers (bin_unix/linux_server or bin_unix/native_server) should need no libs 
other than libstdc++ and zlib. Note that for the server to see the "config/server-init.cfg", 
it must be run from the root Tesseract directory. If you run a server with the 
"tesseract_unix -d" command, this happens automatically. However, if you wish to 
run the standalone servers instead, then you may need to write an appropriate wrapper 
script to change to the appropriate data directory before running the standalone
server binary, as described below in the packaging guide.

# Running a server for Cube Conflict
Open "ccunix.sh" with a text editor and go to line 12 and put this line instead: 
CC_OPTIONS="-u${HOME}/.cubeconflict -d" the "-d" arg will launch your client as a server.
Don't forget to create a copy of "ccunix.sh" to run the game if you want to play too!