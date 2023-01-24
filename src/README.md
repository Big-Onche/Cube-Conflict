# How to compile the game?

# Windows
- Install Code::blocks with MinGW-W64 ( https://sourceforge.net/projects/codeblocks/files/Binaries/20.03/Windows/codeblocks-20.03mingw-setup.exe/download )
- Open "src/project/Cube Conflict.cbp" with Code::blocks.
- Go to Project->Build (or CTRL+F9) and that's all!

# Linux
- If not already installed: Install lastest SDL2 dev libs (libsdl2-dev, libsdl2-mixer-dev, and libsdl2-image-dev), and GCC compiler (build-essential).
- Please note that on some Linux distributions, zlib (lib32z1-dev) and OpenGL (freeglut3-dev) needs to be manually installed too.
- In the main folder of the game do "make -C src install"
- In the main folder of the game do "chmod +x ccunix.sh" to enable ccunix.sh to run as a program.
- Launch game in the main folder with ccunix.sh (right click->run as a program) and that's all!

# Mac
Not tested.


# Comment compiler le jeu ?

# Windows
- Installer Code::blocks avec MinGW-W64 ( https://sourceforge.net/projects/codeblocks/files/Binaries/20.03/Windows/codeblocks-20.03mingw-setup.exe/download )
- Ouvrir "src/project/Cube Conflict.cbp" avec Code::blocks.
- Aller dans Projet->Compiler (ou CTRL+F9) et c'est tout !

# Linux
- Si ce n'est pas fait: Installer les librairies SDL2 (libsdl2-dev, libsdl2-mixer-dev et libsdl2-image-dev) et le compilateur GCC (build-essential).
- A noter que sur certaines distribuations Linux, les libraires zlib (lib32z1-dev) et OpenGL (freeglut3-dev) devront être aussi installées manuellement.
- Dans le dossier principal du jeu faites "make -C src install"
- Dans le dossier principal du jeu, faites "chmod +x ccunix.sh" pour permettre à ccunix.sh de s'exécuter en tant que programme.
- Lancer le jeu dans le dossier principal avec ccunix.sh (clic droit->Exécuter en tant que programme) et c'est tout !

# Mac
Pas essayé.