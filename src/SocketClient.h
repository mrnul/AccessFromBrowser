#pragma once

#define MyHeaders_SocketClient

#ifndef MyHeaders_SocketServer

#if defined(_INC_WINDOWS) || defined(_WINSOCK2API_)
#error SocketServer.h  or SocketClient must be included first!
#endif

#endif //MyHeaders_SocketServer

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

struct SCSFlag
{
	unsigned int val;
	explicit SCSFlag(unsigned int v) : val(v) { }
	operator unsigned int() const { return val; }
	SCSFlag operator|(SCSFlag v) const { return SCSFlag(val | v); }
	SCSFlag operator|(unsigned int v) const { return SCSFlag(val | v); }
};

struct SCRFlag
{
	unsigned int val;
	explicit SCRFlag(unsigned int v) : val(v) { }
	operator unsigned int() const { return val; }
	SCRFlag operator|(SCRFlag v) const { return SCRFlag(val | v); }
	SCRFlag operator|(unsigned int v) const { return SCRFlag(val | v); }
};

namespace socketclient
{
	namespace Sflag
	{
		const SCSFlag DontRoute = SCSFlag(MSG_DONTROUTE);
		const SCSFlag MsgOOB = SCSFlag(MSG_OOB);
		const SCSFlag Default = SCSFlag(0);
	}
	namespace Rflag
	{
		const SCRFlag Peek = SCRFlag(MSG_PEEK);
		const SCRFlag MsgOOB = SCRFlag(MSG_OOB);
		const SCRFlag MsgWaitAll = SCRFlag(MSG_WAITALL);
		const SCRFlag Default = SCRFlag(0);
	}
}

class SocketClient
{
private:
	SOCKET Socket;
	bool Initialized;
public:
	SocketClient();
	SocketClient(SOCKET con);
	bool Init();
	bool Init(SOCKET con);
	bool Connect(const wchar_t* host, const wchar_t* port, const unsigned int secs = 5, const unsigned int microsecs = 0);
	int Send(const void* buffer, const int len, const SCSFlag flags = socketclient::Sflag::Default) const;
	int Recieve(void* buffer, const int len, const SCRFlag flags = socketclient::Rflag::Default) const;
	int RecieveNBytes(void* buffer, const int n, const unsigned int waitSeconds = 5) const;
	int DiscardNBytes(const int n, const unsigned int waitSeconds = 5) const;
	bool SetNonBlocking() const;
	bool SetBlocking() const;
	bool CheckReadability(const long int secs = 0, const long int microsecs = 0) const;
	bool CheckWritability(const long int secs = 0, const long int microsecs = 0) const;
	bool ShutDownRcv() const;
	bool ShutDownSnd() const;
	unsigned int GetPendingDataSize() const;
	operator SOCKET();
	bool IsInitialized() const;
	bool Close();
	bool Clean();
	~SocketClient();
};
