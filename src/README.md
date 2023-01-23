# How to compile the game?

# Windows
Option 1 : Code::Blocks :
- Install Code::blocks with MinGW ( https://sourceforge.net/projects/codeblocks/files/Binaries/20.03/Windows/codeblocks-20.03mingw-setup.exe/download )
- Open project/Cube Conflict.cbp with Code::blocks.
- Go to Project->Build (or CTRL+F9) and that's all!

# Linux
- If not already installed: Install lastest SDL2 dev libs (libsdl2-dev, libsdl2-mixer-dev, and libsdl2-image-dev), OpenGL toolkit (freeglut3-dev) and also zlib (lib32z1)
- Go inside "/src" folder then "make"
- When compilation has ended: "make-install" to put the executable inside "/bin_unix"
- Launch game in the main folder with "ccunix.sh" (right click->run as a program) and that's all!

# Mac
Not tested.


# Comment compiler le jeu ?

# Windows
Option 1 : Code::Blocks :
- Installer Code::blocks avec MinGW ( https://sourceforge.net/projects/codeblocks/files/Binaries/20.03/Windows/codeblocks-20.03mingw-setup.exe/download )
- Ouvrir project/Cube Conflict.cbp avec Code::blocks.
- Aller dans Projet->Compiler (ou CTRL+F9) et c'est tout !

# Linux
- Si ce n'est pas fait: Installer les librairies SDL2 (libsdl2-dev, libsdl2-mixer-dev et libsdl2-image-dev), l'outil OpenGL (freeglut3-dev) ainsi que la librairie zlib (lib32z1)
- Allez dans le dossier "/src" et faites "make"
- Une fois que la compilation est terminée (avec un peu de chance) faites "make-install" afin de placer l'exécutale dans "/bin_unix"
- Lancer le jeu dans le dossier principal avec "ccunix.sh" (clic droit->Exécuter en tant que programme) et c'est tout !

# Mac
Pas essayé.