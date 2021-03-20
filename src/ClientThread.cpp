#include "ClientThread.h"
#include <iostream>
#include <vector>
#include <codecvt>
#include <string>
#include <fstream>
#include <filesystem>
#include "SimpleHTTP.h"
#include "SimpleHTML.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::ifstream;
using std::to_string;

string string_to_utf8(const std::string& str)
{
	vector<wchar_t> widestr(BUFFER_SZ, 0);
	int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), widestr.data(), BUFFER_SZ - 1);
	if (!count)
		return "";
	vector<char> multibyte(BUFFER_SZ, 0);
	count = WideCharToMultiByte(CP_UTF8, 0, widestr.data(), count, multibyte.data(), BUFFER_SZ - 1, 0, 0);
	return string(multibyte.begin(), multibyte.begin() + count);
}

bool HandleRequest(SocketClient& cl, const string path)
{
	SimpleHTTP http;
	SimpleHTML html;
	vector<char> data;

	std::filesystem::path p = std::filesystem::u8path(path);
	if (path == NOT_FOUND)
	{
		http.SetContentType(simplehttp::ContentType::TEXT_HTML);
		html.SetTitle("Hmmm...");
		html.SetGeneralMessage("ERROR 404: Content not found");

		const string res = string_to_utf8(html.BuildHTML());
		data.insert(data.end(), res.begin(), res.begin() + res.size());
	}
	else if (path.length() == 0)
	{
		vector<char> letters(BUFFER_SZ, 0);
		const unsigned int resSize = (unsigned int)GetLogicalDriveStringsA(BUFFER_SZ - 2, letters.data());
		for (unsigned int i = 0; i < resSize; i++)
		{
			string tmp = string();
			while (letters[i] != L'\0')
			{
				tmp.push_back(letters[i]);
				i++;
			}
			html.AddLinkDirectory(tmp);
		}
		html.SetTitle("Access From PC home page");
		http.SetContentType(simplehttp::ContentType::TEXT_HTML);
		const string res = string_to_utf8(html.BuildHTML());
		data.insert(data.end(), res.begin(), res.begin() + res.size());
	}
	else if (std::filesystem::is_directory(p))
	{
		html.SetTitle("Contents");
		http.SetContentType(simplehttp::ContentType::TEXT_HTML);

		try
		{
			for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(p, std::filesystem::directory_options::skip_permission_denied))
			{
				entry.is_directory() ? html.AddLinkDirectory(entry.path().u8string()) : html.AddLinkDownloadable(entry.path().u8string());
			}
			const string res = string_to_utf8(html.BuildHTML());
			data.insert(data.end(), res.begin(), res.begin() + res.size());
		}
		catch (...)
		{

		}
	}
	else
	{
		ifstream input(p, std::ios_base::binary);
		if (!input.is_open())
			return false;

		while (input.good())
		{
			vector<char> buffer(BUFFER_SZ, 0);
			input.read(buffer.data(), BUFFER_SZ - 1);
			data.insert(data.end(), buffer.begin(), buffer.begin() + (unsigned int)input.gcount());
		}

		const string ext = p.extension().string();
		if (_stricmp(ext.c_str(), ".jpg") == 0 || _stricmp(ext.c_str(), ".jpeg") == 0)
		{
			http.SetContentType(simplehttp::ContentType::IMAGE_JPEG);
			http.SetContentDisposition(simplehttp::ContentDisposition::INLINE);
		}
		else if (_stricmp(ext.c_str(), ".gif") == 0)
		{
			http.SetContentType(simplehttp::ContentType::IMAGE_GIF);
			http.SetContentDisposition(simplehttp::ContentDisposition::INLINE);
		}
		else if (_stricmp(ext.c_str(), ".png") == 0)
		{
			http.SetContentType(simplehttp::ContentType::IMAGE_PNG);
			http.SetContentDisposition(simplehttp::ContentDisposition::INLINE);

		}
		else if (_stricmp(ext.c_str(), ".html") == 0)
		{
			http.SetContentType(simplehttp::ContentType::TEXT_HTML);
		}
		else if(_stricmp(ext.c_str(), ".txt") == 0)
		{
			http.SetContentType(simplehttp::ContentType::TEXT_PLAIN);
			http.SetContentDisposition(simplehttp::ContentDisposition::INLINE);
		}
		else
		{
			http.SetContentDisposition(simplehttp::ContentDisposition::INLINE);
		}
	}

	http.SetStatus(path == NOT_FOUND ? simplehttp::Status::NOT_FOUND : simplehttp::Status::OK);
	http.SetConnection(simplehttp::Connection::KEEP_ALIVE);
	http.SetContentLength(data.size());

	const string response = http.BuildHTTPResponse();
	if (response.size() == 0)
		return false;

	if (cl.Send(response.c_str(), response.size()) != response.size())
		return false;

	return cl.Send(data.data(), data.size()) == data.size();
}

void ConsoleOut(const string str)
{
	cout << str << endl;
}


void ClientThread(const SOCKET socket, const string IP)
{
	SocketClient client(socket);
	ConsoleOut("New client: " + IP);
	while (true)
	{
		const SimpleHTTPResult res = SimpleHTTP::ReceiveHTTPFromClient(client);
		if (res.Method == simplehttp::Method::NONE)
			break;

		const string& fristLine = res.HTTP.substr(0, res.HTTP.find("\r\n"));
		ConsoleOut(IP + "\t" + string(fristLine.begin(), fristLine.end()));

		if (res.Method == simplehttp::Method::GET)
		{
			if (!HandleRequest(client, res.FileName))
				HandleRequest(client, NOT_FOUND);
		}
		else if (res.Method == simplehttp::Method::POST)
		{
			vector<char> buffer(res.ContentLength);
			const int count = client.RecieveNBytes(buffer.data(), res.ContentLength);;

			SimpleHTTPPostDataResult postData = SimpleHTTP::ExtractHTTPPostData(buffer, res.Boundary);

			if (postData.Count > 0)
			{
				std::ofstream out(postData.Filename, std::ios_base::trunc | std::ios_base::binary);
				out.write(buffer.data() + postData.DataBegin, postData.Count);
				HandleRequest(client, "");
			}
			else
			{
				HandleRequest(client, NOT_FOUND);
			}
		}
		else if (res.Method == simplehttp::Method::PUT)
		{
			client.DiscardNBytes(res.ContentLength);
			HandleRequest(client, NOT_FOUND);
		}
	}
	ConsoleOut("Disconnected: " + IP);
}