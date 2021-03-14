#pragma once
#include <string>
#include <MyHeaders/SocketClient.h>

using std::wstring;

void ClientThread(const SOCKET socket, const wstring IP);