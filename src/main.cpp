#include "SocketServer.h"
#include <iostream>
#include <thread>
#include <string>

#include "ClientThread.h"

using std::wcout;
using std::endl;
using std::thread;
using std::wstring;

int main()
{
	const char httpPort[] = "80";
	char IP[512] = {};

	SocketServer server;
	if (!server.GetLocalMachineIP(IP))
	{
		wcout << "Could not get local IP" << endl;
		return -1;
	}

	if (!server.CreateAndBind(IP, httpPort))
	{
		wcout << "Error in CreateAndBind" << endl;
		return -2;
	}

	if (!server.Listen())
	{
		wcout << "Error in Listen" << endl;
		return -3;
	}

	wcout << "Waiting for client on: " << IP << endl;

	while (true)
	{
		SOCKET socket = server.Accept(IP);
		if (socket != INVALID_SOCKET)
		{
			thread(ClientThread, socket, string(IP)).detach();
		}
	}
}