#include "SocketServer.h"

SocketServer::SocketServer()
{
	Socket = 0;
	Init();
}

bool SocketServer::Init()
{
	WSADATA wsaData;
	Initialized = WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
	return Initialized;
}

bool SocketServer::CreateAndBind(const char* host, const char* port)
{
	ADDRINFOA hints = {};
	ADDRINFOA* res;

	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_TCP;

	if (GetAddrInfoA(host, port, &hints, &res) != 0)
	{
		FreeAddrInfoA(res);
		return false;
	}

	Socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (Socket == INVALID_SOCKET)
	{
		Socket = 0;
		FreeAddrInfoA(res);
		return false;
	}

	if (bind(Socket, res->ai_addr, res->ai_addrlen) == SOCKET_ERROR)
	{
		Close();
		FreeAddrInfoA(res);
		return false;
	}

	FreeAddrInfoA(res);
	return true;
}

bool SocketServer::GetLocalMachineIP(char* ip) const
{
	wchar_t hostname[512] = {};

	GetHostNameW(hostname, 512);

	ADDRINFOW* result;
	ADDRINFOW hints = {};

	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (GetAddrInfoW(hostname, 0, &hints, &result) != 0)
		return false;

	for (ADDRINFOW* ptr = result; ptr != 0; ptr = ptr->ai_next)
	{
		SOCKADDR_IN* tmp = (SOCKADDR_IN*)ptr->ai_addr;
		if (inet_ntop(AF_INET, &tmp->sin_addr, ip, 512) == 0)
			return false;
	}

	return true;
}

bool SocketServer::GetClientAddressName(SOCKET client, char* address, char* name, char* port) const
{
	SOCKADDR_IN addr1 = {};
	int len1 = sizeof(addr1);
	bool allOK = true;

	if (getpeername(client, (SOCKADDR*)&addr1, &len1) != 0)
		allOK = false;

	if (address)
	{
		if (inet_ntop(AF_INET, &addr1.sin_addr, address, 512) == 0)
			allOK = false;
	}

	if (name || port)
	{
		if (getnameinfo((SOCKADDR*)&addr1, sizeof(addr1), name, 512, port, 512, 0) != 0)
			allOK = false;
	}

	return allOK;
}

bool SocketServer::Listen()
{
	if (listen(Socket, SOMAXCONN) == SOCKET_ERROR)
	{
		Close();
		return false;
	}
	return true;
}

SOCKET SocketServer::Accept(char* address, char* name, char* port) const
{
	SOCKADDR_IN addr = {};
	int len = sizeof(addr);
	const SOCKET res = accept(Socket, (SOCKADDR*)&addr, &len);

	if (res == SOCKET_ERROR)
		return 0;

	GetClientAddressName(res, address, name, port);

	return res;
}

bool SocketServer::IsInitialized() const
{
	return Initialized;
}

bool SocketServer::SetNonBlocking(const bool nonBlocking) const
{
	u_long iMode = nonBlocking ? 1 : 0;
	return ioctlsocket(Socket, FIONBIO, &iMode) == 0;
}

bool SocketServer::CheckReadability(const long int secs, const long int microsecs) const
{
	timeval tv = { secs, microsecs };
	fd_set fd;
	FD_ZERO(&fd);
	FD_SET(Socket, &fd);
	//msdn: first parameter is ignored
	return select(0, &fd, 0, 0, &tv) > 0;
}

bool SocketServer::CheckWritability(const long int secs, const long int microsecs) const
{
	timeval tv = { secs, microsecs };
	fd_set fd;
	FD_ZERO(&fd);
	FD_SET(Socket, &fd);
	//msdn: first parameter is ignored
	return select(0, 0, &fd, 0, &tv) > 0;
}

SocketServer::operator SOCKET()
{
	return Socket;
}

bool SocketServer::Close()
{
	if (!Socket)
		return true;

	if (closesocket(Socket) == 0)
	{
		Socket = 0;
		return true;
	}
	return false;
}

bool SocketServer::Clean()
{
	if (!Initialized)
		return true;

	if (WSACleanup() != 0)
		return false;

	Initialized = false;
	return true;
}

SocketServer::~SocketServer()
{
	Close();
	Clean();
}