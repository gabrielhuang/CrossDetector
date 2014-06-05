// Client.cpp : définit le point d'entrée pour l'application console.

#include <winsock2.h> 
#include <iostream>
#include <exception>
#include <sstream>
#include <string>

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

int main (int argc, char* argv[])
{
	try
	{
		// Connection Data
		WSADATA wsa_data;
		unsigned short port = 12800;
		const int buffer_size = 1024;
		char buffer[buffer_size];

		// Start protocol
		assure(WSAStartup(MAKEWORD(2,2),&wsa_data) == 0, "WSAStartup", true);
	
		SOCKET main_socket = socket(AF_INET,SOCK_STREAM,0);
		assure(main_socket != INVALID_SOCKET, "socket", true);

		SOCKADDR_IN host;
		host.sin_family = AF_INET;
		host.sin_addr.s_addr = inet_addr("127.0.0.1"); 
		host.sin_port = htons(port); 
		assure(connect(main_socket,(struct sockaddr*)&host,sizeof(host)) == 0, "connect", true);

		// Start video flow
		cout << "Connection ok, initiating video flow" << endl;

		// Videoflow

		while(true)
		{
			cout << "Waiting for messages" << endl;

			int num_chars = recv(main_socket, buffer, buffer_size, 0);
			assure(num_chars != SOCKET_ERROR, "recv");

			if(num_chars == 0)
			{
				cout << "num_chars 0" << endl;
				break;
			}

			buffer[num_chars] = 0; 
			string msg(buffer);

			if(!msg.compare("update"))
			{
				double var1 = 0.345, var2 = -0.19;
				string info = to_string(var1) + "," + to_string(var2);
				num_chars = send(main_socket, info.c_str(), info.size(),0);
				assure(num_chars != SOCKET_ERROR, "send");
			}
			else if(!msg.compare("fin"))
			{

				break;
			}
			else
			{
				cout << "Ignoring message : \"" << msg << "\"" << endl;
			}
		}

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
}