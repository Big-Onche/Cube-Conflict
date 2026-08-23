# Building and running Cube Conflict on Linux

Official releases include Linux binaries. When building from a source-only checkout, run the build from the repository root; the Makefile places the client and dedicated server in `bins/`.

## Requirements

- GNU Make, a C++ compiler, and `pkg-config`
- SDL 2 and SDL2_image development files
- OpenAL Soft and libsndfile development files
- OpenGL, X11, and zlib development files

Debian and Ubuntu:

```sh
sudo apt update
sudo apt install build-essential pkg-config libsdl2-dev libsdl2-image-dev libopenal-dev libsndfile1-dev libgl-dev libx11-dev zlib1g-dev
```

Fedora:

```sh
sudo dnf install gcc-c++ make pkgconf-pkg-config SDL2-devel SDL2_image-devel openal-soft-devel libsndfile-devel libglvnd-devel libX11-devel zlib-devel
```

Arch Linux:

```sh
sudo pacman -S --needed base-devel pkgconf sdl2 sdl2_image openal libsndfile libglvnd libx11 zlib
```

Package names may differ on derivative or older distributions. The required `pkg-config` modules are `sdl2`, `SDL2_image`, `openal`, `sndfile`, `x11`, `gl`, and `zlib`.

## Build

Build the client and dedicated server:

```sh
make -C src -j"$(nproc)"
```

On systems without `nproc`, omit `-j"$(nproc)"`. Useful individual targets are `client`, `server`, and `master`. `make -C src install` remains available as a compatibility alias that builds and strips the client and server; it does not install files system-wide.

Steam support is disabled by default. A normal source build therefore does not need the Steam SDK. Maintainers with the SDK and matching library can use `USE_STEAM=1`.

## Run

The launch script is location-independent and must remain next to `config/`, `media/`, and `bins/`:

```sh
./run.sh
./run.sh client -w1920 -h1080
./run.sh server
```

Game options are forwarded unchanged, including arguments containing spaces. User-writable files are stored under `${XDG_DATA_HOME:-$HOME/.local/share}/cubeconflict`. Override `XDG_DATA_HOME` before launching if another location is required.

The program loads assets relative to the source-release root, so moving only `bins/cc_client` or starting it directly from an unrelated working directory is unsupported. Use `run.sh`.

## Dedicated server

Only a compiler, GNU Make, and zlib development files are needed when building just the standalone server:

```sh
make -C src -j"$(nproc)" server
./run.sh server
```

Server output is copied to `${XDG_DATA_HOME:-$HOME/.local/share}/cubeconflict/logs`. See [server.md](server.md) for configuration and a systemd example.

## Clean rebuild and diagnostics

```sh
make -C src clean
make -C src -j"$(nproc)"
pkg-config --cflags --libs sdl2 SDL2_image openal sndfile x11 gl zlib
```

If the dependency check fails, install the development package that provides the missing `.pc` module. Runtime graphics and audio still depend on working GPU drivers and an SDL/OpenAL-supported desktop environment.

---

# Compiler et lancer Cube Conflict sous Linux

Les versions officielles contiennent les binaires Linux. Pour une copie contenant uniquement les sources, installez les dépendances indiquées ci-dessus, puis compilez depuis la racine du dépôt avec :

```sh
make -C src -j"$(nproc)"
```

Les exécutables `bins/cc_client` et `bins/cc_server` sont créés. Lancez-les avec le script fourni afin de conserver le bon dossier de travail :

```sh
./run.sh
./run.sh server
```

Les fichiers modifiables sont enregistrés dans `${XDG_DATA_HOME:-$HOME/.local/share}/cubeconflict`. Pour compiler uniquement le serveur dédié, utilisez `make -C src -j"$(nproc)" server`. Consultez [server.md](server.md) pour la configuration du service.
