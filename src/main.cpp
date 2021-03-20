#include "SocketServer.h"
#include <iostream>
#include <thread>
#include <string>

#include "ClientThread.h"

using std::cout;
using std::endl;
using std::thread;

int main()
{
	const char httpPort[] = "80";
	char IP[512] = {};

	SocketServer server;
	if (!server.GetLocalMachineIP(IP))
	{
		cout << "Could not get local IP" << endl;
		return -1;
	}

	if (!server.CreateAndBind(IP, httpPort))
	{
		cout << "Error in CreateAndBind" << endl;
		return -2;
	}

	if (!server.Listen())
	{
		cout << "Error in Listen" << endl;
		return -3;
	}

	cout << "Waiting for client on: " << IP << endl;

	while (true)
	{
		SOCKET socket = server.Accept(IP);
		if (socket != INVALID_SOCKET)
		{
			thread(ClientThread, socket, string(IP)).detach();
		}
	}
}