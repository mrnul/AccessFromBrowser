#include "ClientThread.h"
#include <iostream>
#include <vector>
#include <codecvt>
#include <string>
#include <fstream>
#include <filesystem>
#include "SimpleHTTP.h"
#include "SimpleHTML.h"

using std::wcout;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::wstring;
using std::ifstream;
using std::to_string;

string wstring_to_utf8(const std::wstring& str)
{
	vector<char> multibytestr(BUFFER_SZ, 0);
	const int count = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, str.data(), str.length(), multibytestr.data(), multibytestr.size() - 1, 0, 0);
	return string(multibytestr.begin(), multibytestr.begin() + count);
}

bool HandleRequest(SocketClient& cl, const wstring path)
{
	SimpleHTTP http;
	SimpleHTML html;
	vector<char> data;

	std::filesystem::path p = path;
	if (path == NOT_FOUND)
	{
		http.SetContentType(simplehttp::ContentType::TEXT_HTML);
		html.SetTitle(L"Hmmm...");
		html.SetGeneralMessage(L"ERROR 404: Content not found");

		const string res = wstring_to_utf8(html.GetHTML());
		data.insert(data.end(), res.begin(), res.begin() + res.size());
	}
	else if (path.length() == 0)
	{
		vector<wchar_t> letters(BUFFER_SZ, 0);
		const unsigned int resSize = (unsigned int)GetLogicalDriveStringsW(BUFFER_SZ - 2, letters.data());
		for (unsigned int i = 0; i < resSize; i++)
		{
			wstring tmp = wstring();
			while (letters[i] != L'\0')
			{
				tmp.push_back(letters[i]);
				i++;
			}
			html.AddLinkDirectory(tmp);
		}
		html.SetTitle(L"Access From PC home page");
		http.SetContentType(simplehttp::ContentType::TEXT_HTML);
		const string res = wstring_to_utf8(html.GetHTML());
		data.insert(data.end(), res.begin(), res.begin() + res.size());
	}
	else if (std::filesystem::is_directory(p))
	{
		html.SetTitle(L"Contents");
		http.SetContentType(simplehttp::ContentType::TEXT_HTML);

		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(p, std::filesystem::directory_options::skip_permission_denied))
		{
			entry.is_directory() ? html.AddLinkDirectory(entry.path().wstring()) : html.AddLinkDownloadable(entry.path().wstring());
		}
		const string res = wstring_to_utf8(html.GetHTML());
		data.insert(data.end(), res.begin(), res.begin() + res.size());
	}
	else
	{
		ifstream input(path, std::ios_base::binary);
		if (!input.is_open())
			return false;

		while (input.good())
		{
			vector<char> buffer(BUFFER_SZ, 0);
			input.read(buffer.data(), BUFFER_SZ - 1);
			data.insert(data.end(), buffer.begin(), buffer.begin() + (unsigned int)input.gcount());
		}

		const wstring ext = p.extension().wstring();
		if (ext == L".jpg" || ext == L".jpeg")
		{
			http.SetContentType(simplehttp::ContentType::IMAGE_JPEG);
			http.SetContentDisposition(simplehttp::ContentDisposition::INLINE);
		}
		else if (ext == L".gif")
		{
			http.SetContentType(simplehttp::ContentType::IMAGE_GIF);
			http.SetContentDisposition(simplehttp::ContentDisposition::INLINE);
		}
		else if (ext == L".png")
		{
			http.SetContentType(simplehttp::ContentType::IMAGE_PNG);
			http.SetContentDisposition(simplehttp::ContentDisposition::INLINE);

		}
		else if (ext == L".html")
		{
			http.SetContentType(simplehttp::ContentType::TEXT_HTML);
		}
		else
		{
			http.SetContentType(simplehttp::ContentType::TEXT_PLAIN);
			http.SetContentDisposition(simplehttp::ContentDisposition::INLINE);
		}
	}

	http.SetStatus(path == NOT_FOUND ? simplehttp::Status::NOT_FOUND : simplehttp::Status::OK);
	http.SetConnection(simplehttp::Connection::KEEP_ALIVE);
	http.SetContentLength(data.size());

	const string response = http.GetHTTPResponse();
	if (response.size() == 0)
		return false;

	if (cl.Send(response.c_str(), response.size()) != response.size())
		return false;

	return cl.Send(data.data(), data.size()) == data.size();
}

void WCout(const wstring str)
{
	wcout << str << endl;
}


void ClientThread(const SOCKET socket, const wstring IP)
{
	SocketClient client(socket);
	string httpRequest = "";
	WCout(L"New client: " + IP);
	while (true)
	{
		vector<char> buffer(BUFFER_SZ, 0);

		if (client.Recieve(buffer.data(), BUFFER_SZ - 1) <= 0)
			break;

		httpRequest += buffer.data();
		if (SimpleHTTP::Is_Complete(httpRequest))
		{
			const wstring fod = SimpleHTTP::ExtractFileOrDirectoryFromRequest(httpRequest);

			if (!HandleRequest(client, fod))
				HandleRequest(client, NOT_FOUND);

			httpRequest.clear();
		}
	}
	WCout(L"Disconnected: " + IP);
}