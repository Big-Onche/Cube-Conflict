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

# SYSTEM_NAME should be set to the name of your operating system.
#SYSTEM_NAME=Linux
SYSTEM_NAME=`uname -s`

# MACHINE_NAME should be set to the name of your processor.
#MACHINE_NAME=i686
MACHINE_NAME=`uname -m`

case ${SYSTEM_NAME} in
Linux)
  SYSTEM_NAME=linux_
  ;;
*)
  SYSTEM_NAME=unknown_
  ;;
esac

case ${MACHINE_NAME} in
i486|i586|i686)
  MACHINE_NAME=
  ;;
x86_64|amd64)
  MACHINE_NAME=64_
  ;;
*)
  if [ ${SYSTEM_NAME} != native_ ]
  then
    SYSTEM_NAME=native_
  fi
  MACHINE_NAME=
  ;;
esac

if [ -x ${CC_BIN}/native_client ]
then
  SYSTEM_NAME=native_
  MACHINE_NAME=
fi

if [ -x ${CC_BIN}/${SYSTEM_NAME}${MACHINE_NAME}client ]
then
  cd ${CC_DATA}
  exec ${CC_BIN}/${SYSTEM_NAME}${MACHINE_NAME}client ${CC_OPTIONS} "$@"
else
  echo "Your platform does not have a pre-compiled Cube Conflict client."
  echo "Please follow the following steps to build a native client:"
  echo "1) Ensure you have the SDL2, SDL2-image, SDL2-mixer, and OpenGL libraries installed."
  echo "2) Type \"make -C src install\"."
  echo "3) If the build succeeds, run this script again."
  exit 1
fi

