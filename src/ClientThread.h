#pragma once
#include <string>
#include "SocketClient.h"

using std::wstring;
constexpr unsigned int BUFFER_SZ = 1024 * 128; // 128 KB
const wstring NOT_FOUND = wstring(4, 0);

void ClientThread(const SOCKET socket, const wstring IP);