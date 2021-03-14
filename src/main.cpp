#include <MyHeaders/SocketServer.h>
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
	const TCHAR httpPort[] = L"80";
	TCHAR IP[256] = {};

	SocketServer server;
	if (!server.GetLocalMachineIP(IP))
	{
		wcout << "Could not get local IP" << endl;
		return -1;
	}

	if (!server.CreateAndBind(IP, httpPort))
	{
		wcout << "Error in CreateAndBind. Get_Last_Error returned " << GetLastError() << endl;
		return -2;
	}

	if (!server.Listen())
	{
		wcout << "Error in Listen. Get_Last_Error returned " << GetLastError() << endl;
		return -3;
	}

	wcout << "Waiting for client on: " << IP << endl;
	
	while (true)
	{
		SOCKET socket = server.Accept(IP);
		if (socket != INVALID_SOCKET)
		{
			thread(ClientThread, socket, wstring(IP)).detach();
		}
	}
}