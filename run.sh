#!/bin/sh

set -u

GAME_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd) || exit 1
CC_BIN="$GAME_DIR/bins"
CC_DATA="${XDG_DATA_HOME:-${HOME:?HOME is not set}/.local/share}/cubeconflict"

MODE=client
if [ "$#" -gt 0 ]; then
    case "$1" in
        client|server)
            MODE=$1
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [client|server] [game options]"
            exit 0
            ;;
    esac
fi

case "$MODE" in
    client)
        EXECUTABLE="$CC_BIN/cc_client"
        mkdir -p "$CC_DATA/config" "$CC_DATA/screenshots" "$CC_DATA/media/map/base" || exit 1
        ;;
    server)
        EXECUTABLE="$CC_BIN/cc_server"
        mkdir -p "$CC_DATA/logs" || exit 1
        rotate_server_logs()
        {
            set -- "$CC_DATA"/logs/server_*.log
            [ -e "$1" ] || return
            while [ "$#" -ge 20 ]; do
                rm -f "$1"
                shift
            done
        }
        rotate_server_logs
        ;;
esac

if [ ! -x "$EXECUTABLE" ]; then
    echo "Cube Conflict $MODE not found or not executable:" >&2
    echo "  $EXECUTABLE" >&2
    echo "Build it from the repository root with: make -C src" >&2
    exit 1
fi

if [ -n "${LD_LIBRARY_PATH:-}" ]; then
    LD_LIBRARY_PATH="$CC_BIN:$LD_LIBRARY_PATH"
else
    LD_LIBRARY_PATH="$CC_BIN"
fi
export LD_LIBRARY_PATH

cd "$GAME_DIR" || exit 1

if [ "$MODE" = server ]; then
    LOGFILE="$CC_DATA/logs/server_$(date '+%Y-%m-%d_%H-%M-%S').log"
    echo "Server log: $LOGFILE"
    if [ -t 1 ]; then
        "$EXECUTABLE" "-u$CC_DATA" "$@" 2>&1 | tee -a "$LOGFILE"
        exit $?
    fi
    exec "$EXECUTABLE" "-u$CC_DATA" "$@" >>"$LOGFILE" 2>&1
fi

exec "$EXECUTABLE" "-u$CC_DATA" "$@"
