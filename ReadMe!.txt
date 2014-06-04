/*____________________________________________________
**
**	Title:			Live Cross Detector Demo
**	Author:			ECP Hexa Team - Gabriel Huang
**	Date:		    2014
**  License:        MIT
**____________________________________________________
*/

Ceci est la partie "analyse vid�o" du projet Dassault UAV Challenge
de l'ECP Hexa Team.

______________________________________________________
Pour configurer et compiler le projet sur votre ordinateur,
suivre ces �tapes:

1/ Installer OpenCV, Visual C++ 2010, CMake

2/ Ouvrir le ./CMakeLists.txt (� trouver dans le m�me r�pertoire
que ReadMe!.txt)

3/ Rep�rer :
"set(opencv_ver 248)
set(opencv_include_dir "E:/ExternalLibs/opencv/build/include") 
set(opencv_lib_dir "E:/ExternalLibs/opencv/build/x86/vc10/lib")"

Remplacer les versions et chemins par ceux correspondant � votre configuration

4/  Lancer ./prj/configure.bat, qui g�n�rera les fichiers projet dans ./prj
S'il y a des erreurs, corriger le fichier CMakeLists.txt en cons�quence

5/	Ouvrir la solution ./prj/UAV.sln

6/	Dans le fichier main.cpp de WebcamTest
Rep�rer string cross_cascade_name = "Z:\\Cpp\\UAV\\Cpp\\res\\cascade_5.xml";
Et remplacer par le chemin menant au classifieur souhait�

7/  Compiler en Release

8/	On peut alors lancer la d�mo ./bin/Release/WebcamTest avec pour premier param�tre le chemin vers le classifieur (un glisser-d�poser peut suffire)

______________________________________________________
Description des dossiers

bin: 		ex�cutables compil�s
include: 	fichiers source (.h)
lib: 		biblioth�ques compil�es
prj: 		fichiers projets Visual Studio
res: 		ressources (classifieur)
src: 		fichiers source (.cpp)
______________________________________________________
Description des projets

UAV: Analyse vid�o en temps r�el des images captur�es
par le drone. Renvoit des coordonn�es de la croix d�tect�e,
par sockets au serveur python associ�.

VideoFlow: Biblioth�que utilisant OpenCV pour pr�traiter
les images brutes puis d�tecter les coordonn�es d'une �ventuelle croix

WebcamTest: Test de VideoFlow avec la webcam de l'ordinateur courant