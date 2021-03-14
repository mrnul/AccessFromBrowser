#include "ClientThread.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include "SimpleHTTP.h"

using std::wcout;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::wstring;
using std::ifstream;
using std::to_string;

constexpr unsigned int BUFFER_SZ = 512;

bool HandleRequest(SocketClient& cl, const wstring file)
{
	ifstream input(file, std::ios_base::binary);
	if (!input.is_open())
		return false;

	vector<char> data;

	while (input.good())
	{
		char buffer[BUFFER_SZ] = {};
		input.read(buffer, BUFFER_SZ - 1);
		const int count = (int)input.gcount();

		for (int i = 0; i < count; i++)
		{
			data.push_back(buffer[i]);
		}
	}

	SimpleHTTP http;

	std::filesystem::path filePath = file;
	if (filePath.extension().wstring() == L".jpg" || filePath.extension().wstring() == L".jpeg")
	{
		http.SetContentType(simplehttp::ConentType::IMAGE_JPEG);
	}
	else if (filePath.extension().wstring() == L".html")
	{
		http.SetContentType(simplehttp::ConentType::TEXT_HTML);
	}
	else
	{
		http.SetContentType(simplehttp::ConentType::TEXT_PLAIN);
	}

	http.SetStatus(file == L"not_found.html" ? simplehttp::Status::NOT_FOUND : simplehttp::Status::OK);
	http.SetConnection(simplehttp::Connection::KEEP_ALIVE);
	http.SetContentLength(data.size());

	const string response = http.GetHTTPResponse();

	if (response.size() == 0)
		return false;

	if (cl.Send(response.c_str(), response.size()) != response.size())
		return false;

	return cl.Send(data.data(), data.size()) != data.size();
}

void ThreadSafeCout(const string str)
{
	cout << str << endl;
}

void ThreadSafeWCout(const wstring wstr)
{
	wcout << wstr << endl;
}


void ClientThread(const SOCKET socket, const wstring IP)
{
	SocketClient client(socket);
	string httpRequest = "";
	bool shouldQuit = false;

	ThreadSafeWCout(L"New: " + IP);

	while (true)
	{
		char buffer[BUFFER_SZ] = {};

		if (client.Recieve(buffer, BUFFER_SZ - 1) <= 0)
			break;

		httpRequest += buffer;
		if (SimpleHTTP::Is_Complete(httpRequest))
		{
			const wstring fod = SimpleHTTP::ExtractFileOrDirectoryFromRequest(httpRequest);

			ThreadSafeWCout(IP + L"\tRequested\t" + fod);

			if (!HandleRequest(client, fod))
				HandleRequest(client, L"not_found.html");

			httpRequest = "";
		}
	}

	ThreadSafeWCout(L"Disconnected: " + IP);
}