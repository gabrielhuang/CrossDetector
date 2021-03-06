// Client.cpp�: d�finit le point d'entr�e pour l'application console.

#include <winsock2.h> 
#include <iostream>
#include <exception>
#include <sstream>
#include <string>

#include <utilities.h>
#include "background_video_flow.h"
#include "videoman_dev.h"


using namespace std;

template <typename T>
string to_string(const T& e)
{
	ostringstream stream;
	stream << e;
	return stream.str();
}

void assure(bool condition, const string& msg = "Assure", bool confirm = false)
{
	if(!condition)
	{
		throw std::runtime_error(msg);
	}
	if(confirm)
	{
		cout << msg << " : " << " OK" << endl;
	}
}

template<typename VideoSourceType>
class Communicator : public Observer<BackgroundVideoFlow<VideoSourceType> >
{
	typedef BackgroundVideoFlow<VideoSourceType> SpecificBackgroundVideoFlow;
	int main_socket_;

public:
	Communicator(SpecificBackgroundVideoFlow* subject, int main_socket) :
		Observer<SpecificBackgroundVideoFlow>(subject),
		main_socket_(main_socket)
	{
	}

	void update()
	{
		string info;
		if(subject_->has_visual())
		{
			double x = subject_->x();
			double y = subject_->y();
			info = to_string(x) + "," + to_string(y);
		}
		else
		{
			info = "no target";
		}
		cout << "[Communicator] Send : \"" << info << "\"" << endl;
		int num_chars = send(main_socket_, info.c_str(), info.size(), 0);
		if(num_chars == SOCKET_ERROR, "send")
		{
			cout << "[Communicator] Server closed connection" << endl;
			subject_->set_loop(false);
		}
		else
		{
			cout << "[Communicator] [Update] -> \"" << info << "\"" << endl;
		}
	}
};

int main(int argc, char* argv[])
{
	// Parameters
	string cross_cascade_name = "Z:\\Cpp\\UAV\\Cpp\\res\\cascade_5.xml";
	//typedef cv::VideoCapture VideoSourceType; // Use opencv VideoCapture
	typedef VideoManSource VideoSourceType; // Use VideoMan video

	try
	{
		// Connection Data
		WSADATA wsa_data;
		unsigned short port = 12800;
		const int buffer_size = 1024;
	
		// Start protocol
		assure(WSAStartup(MAKEWORD(2,2),&wsa_data) == 0, "WSAStartup", true);
	
		SOCKET main_socket = socket(AF_INET,SOCK_STREAM,0);
		assure(main_socket != INVALID_SOCKET, "socket", true);

		SOCKADDR_IN host;
		host.sin_family = AF_INET;
		host.sin_addr.s_addr = inet_addr("127.0.0.1"); 
		host.sin_port = htons(port); 
		assure(connect(main_socket,(struct sockaddr*)&host,sizeof(host)) == 0, "connect", true);

		// VideoFlow Parameter
		cout << "Connection ok, initiating video flow" << endl;
		BackgroundVideoFlow<VideoSourceType> background_video_flow;
		background_video_flow.cross_cascade_name = cross_cascade_name;
		
		// Set Observers
		Communicator<VideoSourceType> communicator(&background_video_flow, main_socket);
		Viewer<VideoSourceType> viewer(&background_video_flow);
		Parametrizer<VideoSourceType> parametrizer(&background_video_flow);

		// Start Video Flow - blocking
		background_video_flow.init();

		// End 
		cout << "Connection closed by host" << endl;

		assure(shutdown(main_socket,2) == 0, "shutdown", true);
		assure(closesocket(main_socket) == 0, "closesocket", true);
		assure(WSACleanup() == 0, "WSACleanup", true);
	}
	catch(const runtime_error& e)
	{
		cout << "Exception : " << e.what() << endl;
	}
	cout << "Application will now exit" << endl;
	cin.get();
	return 0;
}