# How to play on Linux?

Please run "native_client" from the root Cube Conflict dir (NOT from inside the "bin_unix"
directory) to launch it, or set the CC_DATA variable at the top of the "ccunix.sh" 
script to an absolute path to allow it to be run from any location. Note that the script "ccunix.sh"
is set up to write any files (saves, configs, etc.) into the user's home  directory at "~/.cubeconflict".

Clients will need the following dynamic link libraries present:
* libGL (OpenGL)
* SDL2 (>= 2.0.0)
* SDL2_image
* libopenal
* libpng
* libjpeg
* zlib

If native binaries for your platform are not included, then try the following:
1) Ensure you have the DEVELOPMENT VERSIONS of the above libraries installed.
2) Type "make -C src install".
3) Re-run the "ccunix.sh" script if it succeeded.

For more informations about compilation please read "src/README.md".

# Running a server for Cube Conflict
Open "ccunix.sh" with a text editor and go to line 12 and put this line instead: 
CC_OPTIONS="-u${HOME}/.cubeconflict -d" 
The "-d" arg will launch your client as a server. Don't forget to create a copy of "ccunix.sh"
to run the game if you want to play too!


# Comment jouer sur Linux?

Veuillez à lancer "native_client" à partir du dossier principal Cube Conflict (pas depuis le dossier
"bin_unix"), ou définissez la variable CC_DATA dans la première ligne du script "ccunix.sh" 
à un chemin absolu pour lui permettre d'être exécuté à partir de n'importe quel emplacement.
A noter que le cript "ccunix.sh" est configuré afin d'écrire les fichiers (sauvegardes configuration, etc...)
dans le dossier "home" de l'utilisateur à "~/.cubeconflict".

Le jeu aura besoin des bibliothèques suivantes :
* libGL (OpenGL)
* SDL2 (>= 2.0.0)
* SDL2_image
* libopenal
* libpng
* libjpeg
* zlib

Si l'exécutable pour votre plate forme n'est pas inclus, essayez ce qui suit :
1) Assurez-vous que les VERSIONS DE DÉVELOPPEMENT des bibliothèques ci-dessus sont installées.
2) Tapez "make -C src install".
3) Réexécutez le script "ccunix.sh" la compilation a réussi.

Pour davantage d'informations sur la compilation, consulter "src/README.md".

# Lancer un serveur Cube Conflict
Ouvrir "ccunix.sh" avec un éditeur de texte, allez à la ligne 12 et placer la ligne suivante: 
CC_OPTIONS="-u${HOME}/.cubeconflict -d" l'argument "-d" lancera le programme en tant que serveur.
Ne pas oublier de faire une copie de "ccunix.sh" afin de pouvoir lancer le jeu si vous souhaitez
jouer aussi!