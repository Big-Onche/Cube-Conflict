# Building and packaging on Windows

Official Cube Conflict releases include Windows binaries. These instructions are for rebuilding the game from a source checkout and for checking a Windows release package.

Windows builds are 64-bit only. The launcher expects the game client at `bins\cc_client64.exe`.

## Visual Studio 2022

Install Visual Studio 2022 with the **Desktop development with C++** workload and a Windows 10 or Windows 11 SDK. Open `src\project\Cube Conflict.sln`, select `Release` and `x64`, then choose **Build Solution**.

From a Visual Studio Developer Command Prompt, the equivalent command is:

```bat
msbuild "src\project\Cube Conflict.sln" /m /p:Configuration=Release /p:Platform=x64
```

The main configurations produce:

| Configuration | Output |
| --- | --- |
| Release | `bins\cc_client64.exe` |
| Debug | `bins\cc_dbg64.exe` |
| Profile | `bins\cc_profile64.exe` |

Release and Debug builds do not include Tracy. The Profile configuration enables Tracy and requires `dbghelp.lib`, which is part of the Windows SDK.

Steam support is disabled by default. Enable it only for a Steam package:

```bat
msbuild "src\project\Cube Conflict.sln" /m /p:Configuration=Release /p:Platform=x64 /p:UseSteam=true
```

The Steam-enabled client must be shipped with `steam_api64.dll`.

## Code::Blocks with MinGW-w64

Install Code::Blocks with a MinGW-w64 compiler that supports C++17. Open `src\project\Cube Conflict.cbp` and build one of these targets:

* `client64`: standard Release client at `bins\cc_client64.exe`
* `client64-steam`: Steam-enabled Release client at `bins\cc_client64.exe`
* `debug64`: debug client at `bins\cc_dbg64.exe`
* `profile64`: optimized Tracy client at `bins\cc_profile64.exe`

All targets use the x64 libraries in `src\lib64`. Steam is enabled only by `client64-steam`, and Tracy is enabled only by `profile64`.

The project's working directory is the repository root, which is required for the game to find `config`, `data`, `media`, and the other runtime assets.

## GNU Make with MinGW-w64

The Makefile can also be used from an MSYS2 MinGW shell. Put the matching MinGW compiler on `PATH`, then run:

```sh
mingw32-make -C src PLATFORM=MINGW64 -j4
```

The client is written to `bins/cc_client64.exe`. Optional integrations are explicit:

```sh
mingw32-make -C src PLATFORM=MINGW64 USE_STEAM=1 USE_TRACY=1 -j4
```

## Launcher

The Windows launcher is a separate x64 Code::Blocks project at `src\launcher\GameLauncher.cbp`. Its `release64` target creates `Cube Conflict.exe`, while `debug64` creates `Cube Conflict debug.exe`, both in the repository root. The required x64 static SDL libraries are listed in `src\launcher\lib\README.md`.

## Release package checklist

A runnable Windows package needs more than the executable. Keep the repository layout and include:

* `Cube Conflict.exe` and the client/server executables under `bins\`;
* the matching runtime DLLs under `bins\`, including SDL2, SDL2_image, OpenAL, libsndfile, zlib, and ENet where applicable;
* `config\`, `data\`, `media\`, `sounds\`, and the other game-data directories used by the client;
* the matching Steam API DLL only when the client was compiled with Steam support.

Run the launcher and both the client and dedicated server from a clean copy of the package before publishing it. Building into an existing development checkout can hide missing DLLs or assets.

## Troubleshooting

* **The launcher cannot find the game:** verify that `bins\cc_client64.exe` exists and that the launcher is run from the package root.
* **A DLL is missing:** copy the x64 runtime DLL into `bins\`; 32-bit DLLs and libraries are not supported.
* **Visual Studio reports an incompatible library:** confirm that the selected platform is `x64` and dependencies come from `src\lib64`.
* **Code::Blocks reports an incompatible architecture:** configure an x86_64 MinGW-w64 compiler; i686 and `-m32` builds are not supported.
