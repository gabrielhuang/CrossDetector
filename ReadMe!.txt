/*____________________________________________________
**
**	Title:			Live Cross Detector Demo
**	Author:			ECP Hexa Team - Gabriel Huang
**	Date:		    2014
**  License:        MIT
**____________________________________________________
*/

Ceci est la partie "analyse vidéo" du projet Dassault UAV Challenge
de l'ECP Hexa Team.

______________________________________________________
Pour configurer et compiler le projet sur votre ordinateur,
suivre ces étapes:

1/ Installer OpenCV, Visual C++ 2010, CMake

2/ Ouvrir le ./CMakeLists.txt (à trouver dans le même répertoire
que ReadMe!.txt)

3/ Repérer :
"set(opencv_ver 248)
set(opencv_include_dir "E:/ExternalLibs/opencv/build/include") 
set(opencv_lib_dir "E:/ExternalLibs/opencv/build/x86/vc10/lib")"

Remplacer les versions et chemins par ceux correspondant à votre configuration

4/  Lancer ./prj/configure.bat, qui générera les fichiers projet dans ./prj
S'il y a des erreurs, corriger le fichier CMakeLists.txt en conséquence

5/	Ouvrir la solution ./prj/UAV.sln

6/	Dans le fichier main.cpp de WebcamTest
Repérer string cross_cascade_name = "Z:\\Cpp\\UAV\\Cpp\\res\\cascade_5.xml";
Et remplacer par le chemin menant au classifieur souhaité

7/  Compiler en Release

8/	On peut alors lancer la démo ./bin/Release/WebcamTest avec pour premier paramètre le chemin vers le classifieur (un glisser-déposer peut suffire)

______________________________________________________
Description des dossiers

bin: 		exécutables compilés
include: 	fichiers source (.h)
lib: 		bibliothèques compilées
prj: 		fichiers projets Visual Studio
res: 		ressources (classifieur)
src: 		fichiers source (.cpp)
______________________________________________________
Description des projets

UAV: Analyse vidéo en temps réel des images capturées
par le drone. Renvoit des coordonnées de la croix détectée,
par sockets au serveur python associé.

VideoFlow: Bibliothèque utilisant OpenCV pour prétraiter
les images brutes puis détecter les coordonnées d'une éventuelle croix

WebcamTest: Test de VideoFlow avec la webcam de l'ordinateur courant