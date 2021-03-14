#pragma once
#include <string>
#include <unordered_map>

using std::string;
using std::wstring;
using std::unordered_map;
using std::to_string;

namespace simplehttp
{
	enum class Status
	{
		NONE, OTHER, OK, NOT_FOUND
	};

	enum class ConentType
	{
		NONE, OTHER, TEXT_PLAIN, TEXT_HTML, IMAGE_JPEG, IMAGE_PNG
	};

	enum class Connection
	{
		NONE, OTHER, CLOSE, KEEP_ALIVE
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

		const unordered_map<ConentType, string> ContentTypeMap =
		{
			{ConentType::NONE, ""},
			{ConentType::OTHER, ""},
			{ConentType::TEXT_PLAIN, "Conent-Type: text/plain; charset=utf-8\r\n"},
			{ConentType::TEXT_HTML, "Conent-Type: text/html; charset=utf-8\r\n"},
			{ConentType::IMAGE_JPEG, "Conent-Type: image/jpeg\r\n"},
			{ConentType::IMAGE_PNG, "Conent-Type: image/png\r\n"}
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
	simplehttp::ConentType ContentType;
	simplehttp::Connection Connection;
	int ContentLength;

public:
	SimpleHTTP()
	{
		Status = simplehttp::Status::NONE;
		ContentType = simplehttp::ConentType::NONE;
		Connection = simplehttp::Connection::NONE;
		ContentLength = -1;
	}

	void SetStatus(simplehttp::Status status)
	{
		Status = status;
	}
	void SetConnection(simplehttp::Connection connection)
	{
		Connection = connection;
	}
	void SetContentType(simplehttp::ConentType type)
	{
		ContentType = type;
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
			simplehttp::mapping::ConnectionTypeMap.at(Connection) +
			simplehttp::mapping::ContentTypeMap.at(ContentType);

		if (ContentLength >= 0)
		{
			res += "Content-Length: " + to_string(ContentLength) + "\r\n";
		}

		res += "\r\n";
		return res;
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

		wstring ret = L"";
		for (int i = 0; i < count; i++)
		{
			ret += request[i + offset];
		}

		return ret;
	}

	bool static Is_Complete(const string request)
	{
		const string REQUEST_END = "\r\n\r\n";
		return (request.substr(request.size() - REQUEST_END.length(), REQUEST_END.length()) == REQUEST_END);
		
	}
};