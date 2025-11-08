#!/bin/sh
# Save dir ($HOME/.local/share by default)
CC_DATA=${XDG_DATA_HOME:-$HOME/.local/share}/cubeconflict
# Executables dir
CC_BIN=${CC_DATA}/bin_unix

# Check for mode argument
MODE=${1:-client}

case "$MODE" in
    client)
        EXECUTABLE="${CC_BIN}/cc_client"
        # Engine options: point user directory (-u) to CC_DATA 
        CC_OPTIONS="-u${CC_DATA}"
        
        # Append all arguments passed to this script (except the first)
        shift
        for arg in "$@"; do
            CC_OPTIONS="${CC_OPTIONS} ${arg}"
        done
        
        # Create user directories
        mkdir -p "${CC_DATA}/config" "${CC_DATA}/screenshots" \
                 "${CC_DATA}/packages/base" "${CC_BIN}"
        
        cd "${CC_DATA}"
        ;;
        
    server)
        EXECUTABLE="${CC_BIN}/cubeconflict"
        # Server arg
        CC_OPTIONS="-d./"
        
        # Append all arguments passed to this script (except the first)
        shift
        for arg in "$@"; do
            CC_OPTIONS="${CC_OPTIONS} ${arg}"
        done
        
        # Create logs directory
        mkdir -p "${CC_DATA}/logs"
        
        # Keep only the 20 most recent logs
        find "${CC_DATA}/logs" -maxdepth 1 -type f -name 'server_*.log' -printf '%T@ %p\n' \
          | sort -nr | tail -n +21 | cut -d' ' -f2- | xargs -r rm --
        
        LOGFILE="${CC_DATA}/logs/server_$(date '+%Y-%m-%d_%H-%M-%S').log"
        
        cd "${CC_DATA}"
        ;;
        
    *)
        echo "Usage: $0 {client|server} [options]"
        echo "  client - Run Cube Conflict client"
        echo "  server - Run Cube Conflict dedicated server"
        exit 1
        ;;
esac

export LD_LIBRARY_PATH="${CC_BIN}:$LD_LIBRARY_PATH"

# Ensure binary exists and is executable
if [ ! -x "${EXECUTABLE}" ]; then
    echo "Cube Conflict ${MODE} not found or not executable:"
    echo "  ${EXECUTABLE}"
    echo "Please build with: make -C src install"
    exit 1
fi

# Execute the appropriate binary
if [ "$MODE" = "server" ]; then
    exec "${EXECUTABLE}" ${CC_OPTIONS} >>"$LOGFILE" 2>&1
else
    exec "${EXECUTABLE}" ${CC_OPTIONS}
fi