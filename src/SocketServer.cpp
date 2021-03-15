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

bool SocketServer::CreateAndBind(const wchar_t* host, const wchar_t* port)
{
	ADDRINFOW hints = {};
	ADDRINFOW* res;

	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_TCP;

	if (GetAddrInfoW(host, port, &hints, &res) != 0)
	{
		FreeAddrInfoW(res);
		return false;
	}

	Socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (Socket == INVALID_SOCKET)
	{
		Socket = 0;
		FreeAddrInfoW(res);
		return false;
	}

	if (bind(Socket, res->ai_addr, res->ai_addrlen) == SOCKET_ERROR)
	{
		Close();
		FreeAddrInfoW(res);
		return false;
	}

	FreeAddrInfoW(res);
	return true;
}

bool SocketServer::GetLocalMachineIP(wchar_t* ip) const
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
		if (InetNtopW(AF_INET, &tmp->sin_addr, ip, 512) == 0)
			return false;
	}

	return true;
}

bool SocketServer::GetClientAddress(SOCKET client, wchar_t* address) const
{
	SOCKADDR_IN addr1 = {};
	int len1 = sizeof(addr1);
	if (getpeername(client, (SOCKADDR*)&addr1, &len1) != 0)
		return false;

	if (InetNtop(AF_INET, &addr1.sin_addr, address, 512) == 0)
		return false;
	return true;
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

SOCKET SocketServer::Accept(wchar_t* IP) const
{
	SOCKADDR_IN addr = {};
	int len = sizeof(addr);
	const SOCKET res = accept(Socket, (SOCKADDR*)&addr, &len);

	if (res == SOCKET_ERROR)
		return 0;

	if (IP)
		InetNtopW(AF_INET, &addr.sin_addr, IP, 512);

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