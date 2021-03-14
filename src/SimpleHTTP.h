#pragma once

#include <string>
#include <unordered_map>
#include <Shlwapi.h>

using std::string;
using std::wstring;
using std::unordered_map;
using std::to_string;

#pragma comment(lib, "Shlwapi.lib")

namespace simplehttp
{
	enum class Status
	{
		NONE, OTHER, OK, NOT_FOUND
	};

	enum class ContentType
	{
		NONE, OTHER, TEXT_PLAIN, TEXT_HTML, IMAGE_JPEG, IMAGE_PNG, IMAGE_GIF
	};

	enum class Connection
	{
		NONE, OTHER, CLOSE, KEEP_ALIVE
	};

	enum class ContentDisposition
	{
		NONE, OTHER, ATTACHMENT, INLINE
	};

	namespace mapping
	{
		const unordered_map<Status, string> StatusMap =
		{
			{Status::NONE, ""},
			{Status::OTHER, ""},
			{Status::OK, "HTTP/1.1 200 OK\r\n"},
			{Status::NOT_FOUND, "HTTP/1.1 404 Not Found\r\n"}
		};

		const unordered_map<ContentType, string> ContentTypeMap =
		{
			{ContentType::NONE, ""},
			{ContentType::OTHER, ""},
			{ContentType::TEXT_PLAIN, "Conent-Type: text/plain; charset=utf-8\r\n"},
			{ContentType::TEXT_HTML, "Conent-Type: text/html; charset=utf-8\r\n"},
			{ContentType::IMAGE_JPEG, "Conent-Type: image/jpeg\r\n"},
			{ContentType::IMAGE_PNG, "Conent-Type: image/png\r\n"},
			{ContentType::IMAGE_GIF, "Conent-Type: image/gif\r\n"}
		};

		const unordered_map<ContentDisposition, string> ContentDispositionMap =
		{
			{ContentDisposition::NONE, ""},
			{ContentDisposition::OTHER, ""},
			{ContentDisposition::ATTACHMENT, "Content-Disposition: attachment\r\n"},
			{ContentDisposition::INLINE, "Content-Disposition: inline\r\n"}
		};

		const unordered_map<Connection, string> ConnectionTypeMap
		{
			{Connection::NONE, ""},
			{Connection::OTHER, ""},
			{Connection::CLOSE, "Connection: close\r\n"},
			{Connection::KEEP_ALIVE, "Connection: keep-alive\r\n"}
		};
	}
}

class SimpleHTTP
{
private:
	simplehttp::Status Status;
	simplehttp::ContentType ContentType;
	simplehttp::ContentDisposition ContentDisposition;
	simplehttp::Connection Connection;
	int ContentLength;

public:
	SimpleHTTP()
	{
		Clear();
	}

	void SetStatus(simplehttp::Status status)
	{
		Status = status;
	}
	void SetConnection(simplehttp::Connection connection)
	{
		Connection = connection;
	}
	void SetContentType(simplehttp::ContentType type)
	{
		ContentType = type;
	}
	void SetContentDisposition(simplehttp::ContentDisposition content)
	{
		ContentDisposition = content;
	}
	void SetContentLength(int length)
	{
		ContentLength = length;
	}

	string GetHTTPResponse() const
	{
		if (simplehttp::mapping::StatusMap.at(Status).size() == 0)
			return "";

		string res =
			simplehttp::mapping::StatusMap.at(Status) +
			simplehttp::mapping::ContentDispositionMap.at(ContentDisposition) +
			simplehttp::mapping::ConnectionTypeMap.at(Connection) +
			simplehttp::mapping::ContentTypeMap.at(ContentType);

		if (ContentLength >= 0)
		{
			res += "Content-Length: " + to_string(ContentLength) + "\r\n";
		}

		res += "\r\n";
		return res;
	}

	void Clear()
	{
		Status = simplehttp::Status::NONE;
		ContentType = simplehttp::ContentType::NONE;
		Connection = simplehttp::Connection::NONE;
		ContentDisposition = simplehttp::ContentDisposition::NONE;
		ContentLength = -1;
	}

	wstring static ExtractFileOrDirectoryFromRequest(const string request)
	{
		const string GET_START = "GET /";
		const string GET_END = " HTTP/";

		const int httpGetStartPos = request.find(GET_START);
		const int httpGetEndPos = request.find(GET_END);

		if (httpGetStartPos == string::npos || httpGetEndPos == string::npos)
			return wstring(L"");

		const int offset = httpGetStartPos + GET_START.length();
		const int count = httpGetEndPos - offset;

		if (count < 0)
			return wstring(L"");

		wstring res = wstring(request.begin() + offset, request.begin() + offset + count);
		wstring unescaped(1024 * 1024, 0);
		DWORD size = unescaped.size();
		return UrlUnescapeW(res.data(), unescaped.data(), &size, URL_UNESCAPE_AS_UTF8) == S_OK ? wstring(unescaped.begin(), unescaped.begin() + size) : wstring(L"");
	}

	bool static Is_Complete(const string request)
	{
		const string REQUEST_END = "\r\n\r\n";
		return (request.substr(request.size() - REQUEST_END.length(), REQUEST_END.length()) == REQUEST_END);

	}
};