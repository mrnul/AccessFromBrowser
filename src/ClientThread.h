#pragma once
#include <string>
#include "SocketClient.h"

using std::string;
constexpr unsigned int BUFFER_SZ = 1024 * 128; // 128 KB
const string NOT_FOUND = string(4, 0);

void ClientThread(const SOCKET socket, const string IP);