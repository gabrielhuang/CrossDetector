#ifdef WIN32
	#include <windows.h>
#endif
#include <vector>
#include <GL/freeglut.h>
#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <VideoManControl.h>

using namespace std;
using namespace cv;
using namespace VideoMan;

/*
This is a simple example using OpenCV. One video input is initialized and it is processed using openCV
To use this example, VideoMan must be built with the directive VM_OGLRenderer,
also you need to build the input VMDirectShow
*/

VideoManControl videoMan;
int screenLeft, screenUp, screenWidth, screenHeight;
bool fullScreened;
double videoLength;
int videoInputID; //Index of the video input
std::vector< int > userInputIDs; //Indexes of the userinputs for showing the processed images

cv::Mat inputCopy;                //Copy of the input image
cv::Mat processedImages[3];        //The processed images
cv::Mat gray;
cv::Mat edges;

char *dirPath = 0;

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
    for ( int i = 0; i< 3; ++i )
        processedImages[i].release();   	
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
		memcpy( inputCopy.data, (const char *)image, inputCopy.size().area()*inputCopy.channels() );	
	
		//Update the texture of the renderer
		videoMan.updateTexture( videoInputID, (const char*)inputCopy.data ); 
	       
		//cvSetImageData( inputHeader, image, inputHeader->widthStep );
		// Process the images...
		
		//Convert to grayscale
		if ( inputCopy.channels() != 1 )
			cv::cvtColor( inputCopy, gray, CV_RGB2GRAY);       
    	else
			inputCopy.copyTo(gray);
		
		//process 0
		if ( videoMan.isActivated( userInputIDs[0] ) )
		{
			cv::bitwise_not( inputCopy, processedImages[0] );
			videoMan.updateTexture( userInputIDs[0] );
		}

		//process 1
		if ( videoMan.isActivated( userInputIDs[1] ) )
		{
			cv::blur( inputCopy, processedImages[1], cv::Size(7,7));
			videoMan.updateTexture( userInputIDs[1] );
		}

		//process 2
		if ( videoMan.isActivated( userInputIDs[2]) )
		{
			processedImages[2].setTo(0);
        	cv::Canny( gray, edges, 50, 60 );
        	inputCopy.copyTo( processedImages[2], edges );
			videoMan.updateTexture( userInputIDs[2] );	
		}

		//Release the frame
		videoMan.releaseFrame( videoInputID );
	}

	//render the image of input n in the screen
	videoMan.renderFrame( videoInputID ); 	
	for ( int i = 0; i < 3; ++i )
		videoMan.renderFrame( userInputIDs[i] );

	//Check if the video file (input number 0) has reached the end	
	if ( videoMan.getPositionSeconds(videoInputID) == videoLength )
		videoMan.goToFrame( videoInputID, 0 ); //restart from the begining

    glutSwapBuffers();
}

bool InitializeVideoMan()
{
	VMInputFormat format;	
	VMInputIdentification device;
	bool withHighgui = false;
	if ( dirPath )
	{
		//Initialize one input from a video file
		//device.fileName = dirPath.c_str();
		device.fileName = dirPath;
		if ( videoMan.supportedIdentifier( "DSHOW_VIDEO_FILE" ) )
			device.identifier = "DSHOW_VIDEO_FILE"; //using directshow	
		else if ( videoMan.supportedIdentifier( "HIGHGUI_VIDEO_FILE" ) )
		{
			device.identifier = "HIGHGUI_VIDEO_FILE"; //using highugui	
			withHighgui = true;
		}
		//play in real-time
		format.clock = true;
		format.renderAudio = true;
		//Initialize the video file is the path 
		if ( ( videoInputID = videoMan.addVideoInput( device, &format ) ) != -1 )
		{
			if ( device.fileName )
				cout << "Loaded video file: " << device.fileName << endl;
			cout << "resolution: " << format.width << " " << format.height << endl;

			//get the length of the video
			videoLength = videoMan.getLengthSeconds( videoInputID );
			cout << "duration: " << videoLength << " seconds" << endl;
			if ( withHighgui )
				videoMan.setVerticalFlip( videoInputID, true );
			videoMan.playVideo( videoInputID );
		}
	}
	else
	{
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
		//Intialize on of the devices
		int d = 0;
		videoInputID = -1;
		while ( d < numDevices && videoInputID == -1 )
		{			
			device = list[d];
			//Show dialog to select the format
			format.showDlg = true;
			format.SetFormat( 640, 480, 30, VM_RGB24, VM_RGB24 );
			if ( ( videoInputID = videoMan.addVideoInput( device, &format ) ) != -1 )
			{
				videoMan.showPropertyPage( videoInputID );
				videoMan.getFormat( videoInputID, format );
				if ( device.friendlyName )
					cout << "Initilized camera: " << device.friendlyName << endl;
				cout << "resolution: " <<  format.width << " " << format.height << endl;
				cout << "FPS: " << format.fps << endl;
			}
			++d;
		}
		videoMan.freeAvailableDevicesList(  &list, numDevices );
	}

	if ( videoInputID != -1 )
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
				 if(format.depth == 8 && format.nChannels == 3)
                    processedImages[i] = cv::Mat( cv::Size(format.width, format.height), CV_8UC3 );
                else if(format.nChannels == 1)
                    processedImages[i] = cv::Mat( cv::Size(format.width, format.height), CV_8UC1 );
				videoMan.setUserInputImage( inputID, (char*)processedImages[i].data);
				if ( withHighgui )
					videoMan.setVerticalFlip( inputID, true );
			}
		}
		edges = cv::Mat( cv::Size( format.width, format.height), CV_8UC1 );
        gray = cv::Mat( cv::Size( format.width, format.height), CV_8UC1 );
	}
	
	//We want to display all the intialized video inputs
	videoMan.activateAllVideoInputs();	

	return ( videoInputID != -1 );
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

int main(int argc, char** argv)
{
	cout << "This is a simple example using OpenCV. One video input is initialized and it is processed using openCV" << endl;	
	cout << "Usage: opencvSimple.exe filePath(string)" << endl;
	cout << "Example: opencvSimple.exe c:\\video.avi" << endl;
	cout << "If you don't specify a filepath, a camera will be initialized" << endl;
	cout << "=====================================================" << endl;
	if ( argc > 1 )
		dirPath = argv[1];
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( 640, 480 );
    glutInit( &argc, argv );
    glutCreateWindow("VideoMan-OpenCV Simple");
	glutHideWindow();   
	
	if ( !InitializeVideoMan() )
	{
		showHelp();
		cout << "Error intializing VideoMan" << endl;
		cout << "Pres Enter to exit" << endl;		 
		getchar();
		return -1;
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
    glutMainLoop();
	clear();
	return 0;
}
