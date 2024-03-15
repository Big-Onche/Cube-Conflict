# How to compile the game launcher?

# Windows
- Install Code::blocks with MinGW-W64 ( https://sourceforge.net/projects/codeblocks/files/Binaries/20.03/Windows/codeblocks-20.03mingw-setup.exe/download )
- Open "src/launcher/GameLauncher.cbp" with Code::blocks.
- Go to Project->Build (or CTRL+F9) and that's all!

# Linux
- If not already installed: Install the following SDL2 dev libs (libsdl2-dev, libsdl2-image-dev, libsdl2-ttf-dev), and GCC compiler (build-essential).
- Do "make" (or "make -C src/launcher" if you are in the main folder of the game)
- After compilation, the "game_launcher" binary will be in the game's main folder. Run it by doing "./game_launcher"

Optional:
- A "cc_launcher.desktop" file will be generated alongside the game launcher binaries in the main folder.
- If you want to run the game directly from the applications, move this file to /usr/share/applications/ by doing "mv cc_launcher.desktop /usr/share/applications/"


# Comment compiler le lanceur du jeu ?

# Windows
- Installer Code::blocks avec MinGW-W64 ( https://sourceforge.net/projects/codeblocks/files/Binaries/20.03/Windows/codeblocks-20.03mingw-setup.exe/download )
- Ouvrir "src/launcher/GameLauncher.cbp" avec Code::blocks.
- Aller dans Projet->Compiler (ou CTRL+F9) et c'est tout !

# Linux
- Si ce n'est pas fait: Installer les librairies SDL2 suivantes (libsdl2-dev, libsdl2-image-dev, libsdl2-ttf-dev) et le compilateur GCC (build-essential).
- Faites "make" (ou "make -C src/launcher" si vous êtes dans le dossier principal du jeu)
- Le fichier "game_launcher" sera présent dans le dossier principal du jeu. Lancez-le en faisant "./game_launcher"

Facultatif:
- Un fichier "cc_launcher.desktop" sera généré à côté du lanceur de jeu dans le dossier principal.
- Si vous souhaitez lancer le jeu directement depuis les applications, déplacez ce fichier vers /usr/share/applications/ en faisant "mv cc_launcher.desktop /usr/share/applications/"

