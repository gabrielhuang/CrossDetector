//#pragma once
//
//#include <boost/thread.hpp>
//#include <opencv2/highgui/highgui.hpp>
//
//#ifdef WIN32
//	#include <windows.h>
//#endif
//#include <GL/freeglut.h>
//#include <iostream>
//#include <sstream>
//#include <stdlib.h>
//#include <vector>
//
//#include "VideoManControl.h"
//
//using namespace std;
//using namespace VideoMan;
//
///*
//This is an example using VideoMan with cameras and OpenGL.
//All the available cameras are initilized
//To use this example, VideoMan must be built with the directive VM_OGLRenderer, 
//*/
//
//VideoManControl videoMan;
//int screenLeft, screenUp, screenWidth, screenHeight;
//bool fullScreened = false;
//int visualMode = 0;
//int mainInput = 0;
//std::vector< int > videoInputIDs; //List of indexes of the initialized inputs
//bool newFrame = false;
//
//size_t maxDevices = 10;
//
//void glutResize(int width, int height)
//{
//	screenLeft = 0;
//	screenUp = 0;
//	screenWidth = width;
//	screenHeight = height;
//	//Notify to VideoMan the change of the screen size
//	videoMan.changeScreenSize( screenLeft, screenUp, screenWidth, screenHeight );
//}
//
//void glutKeyboard(unsigned char key, int x, int y)
//{
//	switch (key)
//	{
//		case 27:
//		{
//			exit(0);
//			break;
//		}
//	}
//}
//
//
//void glutSpecialKeyboard(int value, int x, int y)
//{
//	switch (value)
//    {
//		case GLUT_KEY_F1:
//		{
//			if ( !fullScreened )
//				glutFullScreen();
//			else
//			{
//				glutPositionWindow( 0, 20 );
//				glutReshapeWindow( 640, 480 );
//			}
//			fullScreened = !fullScreened;
//			break;
//		}
//		case GLUT_KEY_F2:
//		{
//			visualMode = (visualMode + 1 ) %9;
//			videoMan.changeVisualizationMode( visualMode);
//			break;
//		}		
//		case GLUT_KEY_F3:
//		{
//			mainInput = ( mainInput + 1 ) %videoInputIDs.size();
//			videoMan.changeMainVisualizationInput( mainInput );
//			break;
//		}
//    }
//}
//
//bool InitializeVideoMan()
//{
//	VMInputFormat format;	
//	VMInputIdentification device;
//	VMInputIdentification *list;
//	int numDevices;
//	videoMan.getAvailableDevices( &list, numDevices ); //list all the available devices
//	for ( int v = 0; v < numDevices && videoInputIDs.size() < maxDevices; ++v )
//	{
//		//Initialize one input from a camera
//		device = list[v];
//		format.showDlg = true;
//		int inputID;
//		cout << endl;
//		cout << "Initializing " << device.friendlyName << " from " << device.identifier << endl;
//		if ( ( inputID = videoMan.addVideoInput( device, &format ) ) != -1 )
//		{
//			if ( device.friendlyName )
//				cout << "-Friendly name: " << device.friendlyName << endl;
//			if ( device.uniqueName )
//				cout << "-uniqueName: " << device.uniqueName << endl;
//			cout << "-resolution: " << format.width <<" " << format.height << endl;
//			cout << "-FPS: " << format.fps << endl;
//			videoInputIDs.push_back( inputID );
//			videoMan.showPropertyPage( inputID );
//		}	
//		cout << "===========" << endl;
//	}
//	videoMan.freeAvailableDevicesList(  &list, numDevices );
//
//	//We want to display all the intialized video inputs
//	videoMan.activateAllVideoInputs();	
//
//	return ( videoInputIDs.size() > 0);
//}
//
//
//void glutDisplay(cv::Mat& frame, bool new_frame)
//{
//	//Clear the opengl window
//	glClear( GL_COLOR_BUFFER_BIT );
//	//For each initialized inputs
//	for ( size_t n=0; n < videoInputIDs.size(); n++ )
//	{
//		//Get a new frame from input n
//		char *image = videoMan.getFrame( videoInputIDs[n] );
//		if ( image != NULL )
//		{
//			//Update the texture of the renderer
//			videoMan.updateTexture( videoInputIDs[n] ); 
//            
//			// To opencv
//			frame = cv::Mat(cv::Size(format.width, format.height), CV_8UC3);
//			memcpy(frame.data, (const char *)image, frame.size().area()*inputCopy.channels() );	
//
//			/*
//				Process the image...
//			*/
//
//			//Release the frame
//			videoMan.releaseFrame( videoInputIDs[n] );
//
//			new_frame = true;
//		}
//		//render the image of input n in the screen
//		videoMan.renderFrame( videoInputIDs[n] ); 
//	}
//	glFlush();
//    glutSwapBuffers();
//}
//
//
//void showHelp()
//{
//	cout << "========" << endl;
//	cout << "keys:" << endl;
//	cout << "Esc->Exit" << endl;
//	cout << "F1->Fullscreen" << endl;
//	cout << "F2->Switch Visualization Mode" << endl;
//	cout << "F3->Switch Main Input" << endl;
//	cout << "========" << endl;
//}
//
//struct VideoManCam
//{
//	cv::Mat frame_;
//	bool has_frame_;
//	boost::thread thread_;
//
//	VideoManCam() :
//		has_frame_(false)
//	{
//		glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
//		glutInitWindowPosition( 0, 0 );
//		glutInitWindowSize( 640, 480 );
//		int argc = 1; 
//		char* argv[] = { "a.exe" };
//		glutInit( &argc, argv );
//		glutCreateWindow("VideoMan MultiCamera Example");
//		glutHideWindow();
//
//		glutShowWindow();
//		glutReshapeFunc(glutResize);
//		glutDisplayFunc(glutDisplay, frame_, has_frame_);
//		glutIdleFunc(glutDisplay, frame_, has_frame_);
//		glutKeyboardFunc(glutKeyboard);
//		glutSpecialFunc(glutSpecialKeyboard);
//		glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION); 
//
//		fullScreened = false;
//
//		thread_ = boost::thread(glutMainLoop);
//	}
//
//	inline bool has_frame()
//	{
//		return newFrame;
//	}
//
//	cv::Mat& frame()
//	{
//		return frame_;
//	}
//
//};
//int main(int argc, char** argv)
//{
//	cout << "The specified number of available cameras are initilized" << endl;		
//	cout << "Usage: multiCamera.exe numberOfCameras(int)" << endl;
//	cout << "Example: multiCamera.exe 3" << endl;
//	cout << "=====================================================" << endl;
//	if ( argc > 1 )
//	{
//		std::istringstream st( argv[1] );
//		st >> maxDevices;
//	}
//
//	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
//    glutInitWindowPosition( 0, 0 );
//    glutInitWindowSize( 640, 480 );
//    glutInit( &argc, argv );
//    glutCreateWindow("VideoMan MultiCamera Example");
//	glutHideWindow();
//
//	if ( !InitializeVideoMan() )	
//	{
//		showHelp();
//		cout << "Error intializing VideoMan" << endl;
//		cout << "Pres Enter to exit" << endl;		 
//		getchar();
//		return -1;
//	}
//	
//	showHelp();
//	glutShowWindow();
//    glutReshapeFunc(glutResize);
//    glutDisplayFunc(glutDisplay);
//    glutIdleFunc(glutDisplay);
//    glutKeyboardFunc(glutKeyboard);
//	glutSpecialFunc(glutSpecialKeyboard);
//	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION); 
//
//	fullScreened = false;
//    glutMainLoop();
//	return 0;
//}