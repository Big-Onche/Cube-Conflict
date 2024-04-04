#!/bin/sh
# CC_DATA should refer to the directory in which Cube Conflict data files are placed.
#CC_DATA=~/cubeconflict
#CC_DATA=/usr/local/cubeconflict
CC_DATA=.

# CC_BIN should refer to the directory in which Cube Conflict executable files are placed.
CC_BIN=${CC_DATA}/bin_unix

# CC_OPTIONS contains any command line options you would like to start Cube Conflict with.
#CC_OPTIONS=""
CC_OPTIONS="-u${HOME}/.cubeconflict"

for arg in "$@"; do
    CC_OPTIONS="${CC_OPTIONS} ${arg}"
done

export LD_LIBRARY_PATH="${CC_BIN}:$LD_LIBRARY_PATH"

# Ensure the cubeconflict binary is executable
chmod +x ${CC_BIN}/cubeconflict

if [ -x ${CC_BIN}/cubeconflict ]
then
  cd ${CC_DATA}
  exec ${CC_BIN}/cubeconflict ${CC_OPTIONS} "$@"
else
  echo "Your platform does not have a pre-compiled Cube Conflict client."
  echo "Please follow the following steps to build a native client:"
  echo "1) Ensure you have the SDL2, SDL2-image, OpenAL, Libsndfile and OpenGL libraries installed."
  echo "2) Type \"make -C src install\"."
  echo "3) If the build succeeds, run this script again."
  exit 1
fi

