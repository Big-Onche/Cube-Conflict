# Cube Conflict: Linux Installation Guide

## Playing Cube Conflict on Linux

### Prerequisites
- The provided binaries require **GLIBC 2.22** or higher, ensuring compatibility with most up-to-date distributions.
- For older systems, compiling the binaries is necessary to run the game.

### Running the Game
- **Using the Game Launcher**: Execute `./game_launcher` in the terminal to start the game or a dedicated game server.
- **Using `play.sh` Script**: Run `./play.sh` to launch the game. This script, along with the game launcher, writes files (e.g., saves, configs, screenshots) into `~/.cubeconflict` in the user's home directory. This path is customizable.

### Important Notes
- Directly running the **`cubeconflict`** executable in the `bin_unix` directory will not work.
- **Bonus**: Compiling the game launcher (`make -C src/launcher` in the main folder) generates a `cc_launcher.desktop` file. Move it to `/usr/share/applications/` to add the game to your applications menu.

### Required Libraries
The launcher and the game require several dynamic link libraries:
- **Launcher Libraries**:
  - SDL2 (>= 2.0.0)
  - SDL2_image
  - SDL2_ttf
- **Game Libraries**:
  - libGL (OpenGL)
  - SDL2 (>= 2.0.0)
  - SDL2_image
  - libopenal
  - libsndfile
  - libpng
  - libjpeg
  - zlib

## Compiling Cube Conflict and the Launcher

If the provided binaries do not work for your platform or you have an older version of GLIBC:
1. Ensure you have the **DEVELOPMENT VERSIONS** of the required libraries installed.
2. Compile the game: Run `make -C src` in the game's main folder.
3. Compile the launcher: Execute `make -C src/launcher` in the main folder.

For detailed compilation instructions, see `src/README.md`.

## Running a Cube Conflict Server

- **Using the Game Launcher**: You can run a server directly with `game_launcher`.
- **Modifying `play.sh`**: Edit `play.sh` and replace the 12th line with `CC_OPTIONS="-u${HOME}/.cubeconflict -d"` to launch as a game server.
- **Note**: Running the game server with the game launcher requires **xterm** to be installed (`sudo apt-get install xterm`).

---

# Comment jouer sur Linux ?

## Prérequis
- Les binaires nécessitent **GLIBC 2.22** ou supérieur, garantissant la compatibilité avec la plupart des distributions récentes.
- Pour les systèmes plus anciens, il est nécessaire de compiler les binaires pour exécuter le jeu.

## Lancer le Jeu
- **Avec le Lanceur de Jeu** : Exécutez `./game_launcher` dans le terminal pour démarrer le jeu ou un serveur de jeu dédié.
- **Avec le Script `play.sh`** : Lancez `./play.sh` pour démarrer le jeu. Ce script et le lanceur de jeu enregistrent les fichiers (sauvegardes, configurations, captures d'écran) dans `~/.cubeconflict` dans le répertoire personnel de l'utilisateur. Ce chemin est personnalisable.

## Notes Importantes
- L'exécution directe de l'exécutable **`cubeconflict`** dans le répertoire `bin_unix` ne fonctionnera pas.
- **Bonus** : La compilation du lanceur de jeu (`make -C src/launcher` dans le dossier principal) génère un fichier `cc_launcher.desktop`. Déplacez-le dans `/usr/share/applications/` pour ajouter le jeu à votre menu d'applications.

### Required Libraries
Le jeu aura besoin des bibliothèques suivantes :
- **Bibliothèques pour le lanceur de jeu**:
  - SDL2 (>= 2.0.0)
  - SDL2_image
  - SDL2_ttf
- **Bibliothèques pour le jeu**:
  - libGL (OpenGL)
  - SDL2 (>= 2.0.0)
  - SDL2_image
  - libopenal
  - libsndfile
  - libpng
  - libjpeg
  - zlib

## Comment compiler le jeu et le lanceur du jeu?
Si les binaires de votre plate-forme ne sont pas inclus ou si vous disposez d'une version antérieure à GLIBC 2.22, essayez ce qui suit :
1. Assurez-vous que les **VERSIONS DE DÉVELOPPEMENT** des bibliothèques ci-dessus sont installées.
2. Dans un terminal, tapez `make -C src` dans le dossier principal des fichiers du jeu pour compiler le jeu.
3. Dans un terminal, tapez `make -C src/launcher` dans le dossier principal des fichiers du jeu pour compiler le lanceur de jeu.

Pour davantage d'informations sur la compilation, consulter "src/README.md".

## Exécution d'un serveur pour Cube Conflict
- **Utilisation du Game Launcher** : Vous pouvez exécuter un serveur directement avec `game_launcher`.
- **Modification de `play.sh`** : Modifiez `play.sh` et remplacez la 12ème ligne par `CC_OPTIONS="-u${HOME}/.cubeconflict -d"` pour lancer en tant que serveur de jeu.
- **Remarque** : L'exécution du serveur de jeu avec le lanceur de jeu nécessite l'installation de **xterm** (`sudo apt-get install xterm`).
