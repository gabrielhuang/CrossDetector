CrossDetector
=============

Detector for red crosses based on Viola &amp; Jones's cascade

Configuration et Installation
-----------------------------

Ceci est la partie "analyse vidéo" du projet Dassault UAV Challenge
de l'ECP Hexa Team.

______________________________________________________
Pour configurer et compiler le projet sur votre ordinateur,
suivre ces étapes:

1/ 	Installer OpenCV, Visual C++ 2010, CMake, Boost, et VideoMan (si nécessaire)

2/ 	Ouvrir le 
	./Cpp/CMakeLists.txt
Remplacer les versions et chemins par ceux correspondant à votre configuration

4/ 	Exécuter cmake dans la racine du dépôt Git, par exemple en générant dans 
	./Cpp/build
S'il y a des erreurs, corriger le fichier CMakeLists.txt en conséquence

5/	Ouvrir la solution 
	./Cpp/build/UAV.sln

6/	Dans les fichier 
	"main.cpp" de WebcamTest
et
	"main.cpp" de "UAV"
Repérer
	string cross_cascade_name = "Z:\\Cpp\\UAV\\Cpp\\res\\cascade_5.xml";
Et remplacer par le chemin menant au classifieur souhaité

7/	Vous pouvez alors lancer la démo 
	./bin/Release/WebcamTest
avec pour premier paramètre le chemin vers le classifieur (un glisser-déposer peut suffire), compiler, puis tester sur une webcam.

8/	a.	Dans ./Cpp/src/UAV/main.cpp répérer les lignes:
	//typedef cv::VideoCapture VideoSourceType; // Use opencv VideoCapture
	typedef VideoManSource VideoSourceType; // Use VideoMan video
et (dé)commenter pour utiliser OpenCV ou VideoMan

b.	Vous pouvez enfin lancer le serveur Python ./Python/server.py, puis lancer UAV.exe (le client) qui enverra régulièrement la position de la croix détectée au serveur.

______________________________________________________
Description des dossiers

bin: 		exécutables compilés
build/prj:	fichiers Visual Studio et CMake
include: 	fichiers source (.h)
lib: 		bibliothèques compilées
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
