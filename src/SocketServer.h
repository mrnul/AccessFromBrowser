#pragma once
#define MyHeaders_SocketServer

#ifndef MyHeaders_SocketClient

#if defined(_INC_WINDOWS) || defined(_WINSOCK2API_)
#error SocketServer.h  or SocketClient must be included first!
#endif

#endif //MyHeaders_SocketClient

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

class SocketServer
{
private:
	SOCKET Socket;
	bool Initialized;
public:
	SocketServer();
	bool Init();
	bool CreateAndBind(const char* host, const char* port);
	bool GetLocalMachineIP(char* ip) const; //it just returns the last address found
	bool GetClientAddressName(SOCKET client, char* address = 0, char* name = 0, char* port = 0) const;
	bool Listen();
	SOCKET Accept(char* address = 0, char* name = 0, char* port = 0) const;
	bool IsInitialized() const;
	bool SetNonBlocking(const bool nonBlocking) const;
	bool CheckReadability(const long int secs = 0, const long int microsecs = 0) const;
	bool CheckWritability(const long int secs = 0, const long int microsecs = 0) const;
	operator SOCKET();
	bool Close();
	bool Clean();
	~SocketServer();
};
