#ifdef WIN32
	#include <windows.h>
#endif
#include <vector>
#include <GL/freeglut.h>
#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "VideoManControl.h"

using namespace std;
using namespace cv;
using namespace VideoMan;

// Mutex for image write
boost::mutex		global_mutex;

/*
This is a simple example using OpenCV. One video input is initialized and it is processed using openCV
To use this example, VideoMan must be built with the directive VM_OGLRenderer,
also you need to build the input VMDirectShow
*/

VideoManControl		videoMan;
int					screenLeft;
int					screenUp;
int					screenWidth;
int					screenHeight;
bool				fullScreened;
double				videoLength;
int					videoInputID; 
std::vector<int>	userInputIDs; 

bool				camera_isOpened = false;
cv::Mat				inputCopy;    
bool				has_new = false;

cv::Mat				gray;
cv::Mat				edges;

void glutResize(int width, int height)
{
	screenLeft = 0;
	screenUp = 0;
	screenWidth = width;
	screenHeight = height;
	//Notify to VideoMan the change of the screen size
	videoMan.changeScreenSize( screenLeft, screenUp, screenWidth, screenHeight );
}

void clear()
{
	videoMan.deleteInputs();
	inputCopy.release();	
}

void glutKeyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		{
			glutLeaveMainLoop();
			break;
		}
	}
}


void glutSpecialKeyboard(int value, int x, int y)
{
	switch (value)
	{		
	case GLUT_KEY_F1:
		{
			if ( !fullScreened )
				glutFullScreen();
			else
			{
				glutPositionWindow( 0, 20 );
				glutReshapeWindow( 640, 480 );
			}
			fullScreened = !fullScreened;
			break;
		}
	case GLUT_KEY_F2:
		{
			if ( videoMan.isActivated( userInputIDs[0] )	)	
				videoMan.deactivateVideoInput( userInputIDs[0] );
			else
				videoMan.activateVideoInput( userInputIDs[0] );			
			break;
		}
	case GLUT_KEY_F3:
		{
			if ( videoMan.isActivated( userInputIDs[1] )	)	
				videoMan.deactivateVideoInput( userInputIDs[1] );
			else
				videoMan.activateVideoInput( userInputIDs[1] );			
			break;
		}		
	case GLUT_KEY_F4:
		{
			if ( videoMan.isActivated( userInputIDs[2] )	)	
				videoMan.deactivateVideoInput( userInputIDs[2] );
			else
				videoMan.activateVideoInput( userInputIDs[2] );			
			break;
		}
	case GLUT_KEY_F5:
		{			
			static int mode = 0;
			mode = (mode + 1 ) %9;
			videoMan.changeVisualizationMode( mode );

			break;
		}	
	case GLUT_KEY_F6:
		{			
			static int main = 0;
			main = (main + 1 ) %videoMan.getNumberOfInputs() ;
			videoMan.changeMainVisualizationInput( main );
			break;
		}	
	}
}

void glutDisplay(void)
{
	//Clear the opengl window
	glClear( GL_COLOR_BUFFER_BIT );

	char *image = videoMan.getFrame( videoInputID );
	if ( image != NULL )
	{
		{
			boost::mutex::scoped_lock lock(global_mutex);
			memcpy(inputCopy.data, (const char *)image, inputCopy.size().area()*inputCopy.channels() );	
			has_new = true;
		}

		//Update the texture of the renderer
		//videoMan.updateTexture( videoInputID, (const char*)inputCopy.data ); 

		//Release the frame
		videoMan.releaseFrame( videoInputID );
	}

	//render the image of input n in the screen
	//videoMan.renderFrame( videoInputID ); 	
	//for ( int i = 0; i < 3; ++i )
	//	videoMan.renderFrame( userInputIDs[i] );

	glutSwapBuffers();
}

bool InitializeVideoMan()
{
	VMInputFormat format;	
	VMInputIdentification device;
	bool withHighgui = true;

	//Initialize one input from a camera	
	//std::vector<VMInputIdentification> list;
	//videoMan.getAvailableDevices( list ); //list all the available devices
	VMInputIdentification *list;
	int numDevices;
	videoMan.getAvailableDevices( &list, numDevices ); //list all the available devices
	if ( numDevices == 0 )			
	{
		cout << "There is no available camera\n" << endl;
		videoMan.freeAvailableDevicesList(  &list, numDevices );
		return false;
	}
	//Intialize on one of the devices
	int d = 0;
	videoInputID = -1;

	if(numDevices == 0)
	{
		throw std::runtime_error("No video devices found");
	}
	else if(numDevices == 1)
	{
		cout << "Found exactly 1 device" << endl;
	}
	else
	{
		cout << "Found more than one device. Use (0 to " << numDevices << ") ?" << endl;
		cin >> videoInputID;
		d = videoInputID;
	}

	device = list[d];
	//Show dialog to select the format
	format.showDlg = true;
	format.SetFormat(640, 480, 30, VM_RGB24, VM_RGB24 );
	if ((videoInputID = videoMan.addVideoInput(device, &format)) == -1 )
	{
		throw std::runtime_error("videoMan.addVideoInput");
	}

	videoMan.showPropertyPage( videoInputID );
	videoMan.getFormat( videoInputID, format );
	if ( device.friendlyName )
		cout << "Initilized camera: " << device.friendlyName << endl;
	cout << "resolution: " <<  format.width << " " << format.height << endl;
	cout << "FPS: " << format.fps << endl;

	videoMan.freeAvailableDevicesList(  &list, numDevices );

	if (videoInputID != -1 )
	{		
		if(format.depth == 8 && format.nChannels == 3)
			inputCopy = cv::Mat( cv::Size(format.width, format.height), CV_8UC3 );
		else if(format.nChannels == 1)
			inputCopy = cv::Mat( cv::Size(format.width, format.height), CV_8UC1 );

		//Initialize the user inputs for showing the processed images
		device.identifier = "USER_INPUT";
		int inputID;
		for ( int i = 0; i < 3; ++i )
		{
			if ( ( inputID = videoMan.addVideoInput( device, &format ) ) != -1 )
			{
				userInputIDs.push_back( inputID );
				if ( withHighgui )
					videoMan.setVerticalFlip( inputID, true );
			}
		}
		edges = cv::Mat( cv::Size( format.width, format.height), CV_8UC1 );
		gray = cv::Mat( cv::Size( format.width, format.height), CV_8UC1 );
	}

	//We want to display all the intialized video inputs
	videoMan.activateAllVideoInputs();	

	camera_isOpened = true;
	return (videoInputID != -1);
}

void showHelp()
{
	printf("========\n");
	printf("keys:\n");	
	printf("Esc->Exit\n");
	printf("F1->Fullscreen\n");
	printf("F2-F4->Show Processed Images y/n\n");
	printf("F5->Change Visualization Mode\n");
	printf("F6->Change Main Visualization Input\n");
	printf("========\n");
}

void initAll()
{
	camera_isOpened = false;

	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize(50, 50);
	int argc = 1; 
	char* argv[] = { "a.exe" };
	glutInit( &argc, argv );
	glutCreateWindow("VideoMan-OpenCV Simple");
	glutHideWindow();   

	if (!InitializeVideoMan())
	{
		showHelp();
		cout << "Error intializing VideoMan" << endl;
		cout << "Pres Enter to exit" << endl;		 
		getchar();
	}

	showHelp();
	glutShowWindow();
	glutReshapeFunc(glutResize);
	glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutDisplay);
	glutKeyboardFunc(glutKeyboard);
	glutSpecialFunc(glutSpecialKeyboard);
	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION); 

	fullScreened = false;
}

struct VideoManSource
{
	/*boost::thread*		thread_;*/

	VideoManSource(int option = 0)		
	{
		cout << "VideoMan Video Source" << endl;
		cout << "=====================================================" << endl;
		initAll();
	}

	~VideoManSource()
	{
		glutLeaveMainLoop();
		//if(thread_)
		//{
		//	thread_->join();
		//	delete thread_;
		//}
		clear();
	}

	inline bool isOpened()
	{
		return camera_isOpened;
	}

	void operator>>(cv::Mat& frame)
	{
		while(true)
		{
			glutMainLoopEvent();
			glutDisplay();
			if(has_new)
			{
				cv::flip(inputCopy, frame, 0);
				has_new = false;
				return;
			}
			cv::waitKey(5);
		}

		//while(true)
		//{
		//	glutMainLoopEvent();
		//	boost::mutex::scoped_lock lock(global_mutex);
		//	if(has_new)
		//	{
		//		frame = inputCopy.clone();
		//		has_new = false;
		//		return;
		//	}
		//	cv::waitKey(5);
		//}
	}
};