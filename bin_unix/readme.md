# How to run the game on Linux?
Please run "native_client" from the root Cube Conflict dir (NOT from inside the "bin_unix"
directory) to launch it, or set the CC_DATA variable at the top of the "ccunix.sh" 
script to an absolute path to allow it to be run from any location. Note that the "ccunix.sh" 
script is set up to write any files (saves, configs, etc.) into the user's home 
directory at "~/.cubeconflict".

Clients will need the following dynamic link libraries present:
* libGL (OpenGL)
* SDL2 (>= 2.0.0)
* SDL2_image
* SDL2_mixer
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
The "-d" arg will launch your client as a server. Don't forget to create a version
of "ccunix.sh" to run the game if you want to play too!



# Comment lancer le jeu sous Linux ?

Veuillez exécuter "native_client" à partir du répertoire racine Cube Conflict (PAS depuis le dossier "bin_unix"),
ou définissez la variable CC_DATA en haut du script "ccunix.sh" à un chemin absolu pour lui permettre d'être exécuté
à partir de n'importe quel emplacement. Notez que le script "ccunix.sh" est configuré pour écrire tous les fichiers
(sauvegardes, configurations, etc.) dans le répertoire "home" dans le dossier "~/.cubeconflict".

Les bibliothèques suivantes devront être installées si ce n'est pas le cas :
* libGL (OpenGL)
* SDL2 (>= 2.0.0)
* SDL2_image
* SDL2_mixeur
* libpng
* libjpeg
* zlib

Si l'exécutable pour Linux n'est pas inclus, essayez ce qui suit :
1) Assurez-vous que les VERSIONS DE DÉVELOPPEMENT des bibliothèques ci-dessus sont installées.
2) Tapez "make -C src install".
3) Réexécutez le script "ccunix.sh" si la compilation a réussi.

Pour plus d'informations sur la compilation, veuillez lire "src/README.md".

# Exécuter un serveur pour Cube Conflict
Ouvrez "ccunix.sh" avec un éditeur de texte et allez à la ligne 12 et mettez cette ligne à la place :
CC_OPTIONS="-u${HOME}/.cubeconflict -d"
L'argument "-d" lancera votre client en tant que serveur. N'oubliez pas de créer une version
de "ccunix.sh" pour lancer le jeu si vous voulez jouer aussi !