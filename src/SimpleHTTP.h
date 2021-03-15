#pragma once

#include "SocketClient.h"
#include <string>
#include <unordered_map>
#include <Shlwapi.h>
#include <vector>

using std::vector;
using std::string;
using std::wstring;
using std::unordered_map;
using std::to_string;
using std::stoi;

#pragma comment(lib, "Shlwapi.lib")

namespace simplehttp
{
	enum class Field
	{
		CONTENT_LENGTH, CONTENT_TYPE, CONTENT_DISPOSITION, CONNECTION, BOUNDARY, FILENAME
	};

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

	enum class Method
	{
		NONE, OTHER, POST, GET, PUT, HTTP
	};

	namespace mapping
	{
		const unordered_map<Field, string> FieldMap
		{
			{Field::CONTENT_LENGTH, "Content-Length: "},
			{Field::CONTENT_TYPE, "Content-Type: "},
			{Field::CONTENT_DISPOSITION, "Content-Disposition: "},
			{Field::CONNECTION, "Connection: "},
			{Field::BOUNDARY, "boundary="},
			{Field::FILENAME, "filename="}
		};

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
			{ContentType::TEXT_PLAIN, FieldMap.at(Field::CONTENT_TYPE) + "text/plain; charset=utf-8\r\n"},
			{ContentType::TEXT_HTML, FieldMap.at(Field::CONTENT_TYPE) + "text/html; charset=utf-8\r\n"},
			{ContentType::IMAGE_JPEG, FieldMap.at(Field::CONTENT_TYPE) + "image/jpeg\r\n"},
			{ContentType::IMAGE_PNG, FieldMap.at(Field::CONTENT_TYPE) + "image/png\r\n"},
			{ContentType::IMAGE_GIF, FieldMap.at(Field::CONTENT_TYPE) + "image/gif\r\n"}
		};

		const unordered_map<ContentDisposition, string> ContentDispositionMap =
		{
			{ContentDisposition::NONE, ""},
			{ContentDisposition::OTHER, ""},
			{ContentDisposition::ATTACHMENT, FieldMap.at(Field::CONTENT_DISPOSITION) + "attachment\r\n"},
			{ContentDisposition::INLINE, FieldMap.at(Field::CONTENT_DISPOSITION) + "inline\r\n"}
		};

		const unordered_map<Connection, string> ConnectionTypeMap
		{
			{Connection::NONE, ""},
			{Connection::OTHER, ""},
			{Connection::CLOSE, FieldMap.at(Field::CONNECTION) + "close\r\n"},
			{Connection::KEEP_ALIVE, FieldMap.at(Field::CONNECTION) + "keep-alive\r\n"}
		};

		const unordered_map<Method, string> MethodMap
		{
			{Method::NONE, ""},
			{Method::OTHER, ""},
			{Method::POST, "POST /"},
			{Method::GET, "GET /"},
			{Method::PUT, "PUT /"},
			{Method::HTTP, " HTTP/"}
		};
	}
}

struct SimpleHTTPResult
{
	wstring FileName;
	string HTTP;
	string Boundary;
	simplehttp::Method Method;
	int ContentLength;
	SimpleHTTPResult()
	{
		FileName = wstring();
		HTTP = string();
		Boundary = string();
		Method = simplehttp::Method::NONE;
		ContentLength = -1;
	}
};

struct SimpleHTTPPostDataResult
{
	int DataBegin;
	int DataEnd;
	int Count;
	wstring Filename;
	SimpleHTTPPostDataResult()
	{
		DataBegin = DataEnd = -1;
		Count = 0;
		Filename = wstring();
	}
};

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

	string BuildHTTPResponse() const
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
			res += simplehttp::mapping::FieldMap.at(simplehttp::Field::CONTENT_LENGTH) + to_string(ContentLength) + "\r\n";
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

	SimpleHTTPResult static ReceiveHTTPFromClient(const SocketClient& client)
	{
		const string REQUEST_END = "\r\n\r\n";
		const string& GET = simplehttp::mapping::MethodMap.at(simplehttp::Method::GET);
		const string& PUT = simplehttp::mapping::MethodMap.at(simplehttp::Method::PUT);
		const string& POST = simplehttp::mapping::MethodMap.at(simplehttp::Method::POST);

		SimpleHTTPResult res = SimpleHTTPResult();
		string http = string();

		char byte = 0;

		while (true)
		{
			const int count = client.Recieve(&byte, 1);
			if (count <= 0)
				return res;

			http += byte;

			if (http.size() >= REQUEST_END.size())
			{
				if (http.substr(http.size() - REQUEST_END.size(), REQUEST_END.size()) == REQUEST_END)
					break;
			}
		}

		res.HTTP = http;

		if (http.substr(0, GET.size()) == GET)
		{
			res.Method = simplehttp::Method::GET;
		}
		else if (http.substr(0, PUT.size()) == PUT)
		{
			res.Method = simplehttp::Method::PUT;
		}
		else if (http.substr(0, POST.size()) == POST)
		{
			res.Method = simplehttp::Method::POST;
		}
		else
		{
			res.Method = simplehttp::Method::OTHER;
		}

		wstring tmp;

		if (res.Method == simplehttp::Method::POST)
		{
			const int contentLengthPos = http.find(simplehttp::mapping::FieldMap.at(simplehttp::Field::CONTENT_LENGTH));
			if (contentLengthPos != string::npos)
			{
				const int begin = contentLengthPos + simplehttp::mapping::FieldMap.at(simplehttp::Field::CONTENT_LENGTH).size();
				const int end = contentLengthPos + http.substr(contentLengthPos).find("\r\n");
				if (begin < end)
					res.ContentLength = stoi(wstring(http.begin() + begin, http.begin() + end));
			}
			const int contentTypePos = http.find(simplehttp::mapping::FieldMap.at(simplehttp::Field::CONTENT_TYPE));
			if (contentTypePos != string::npos)
			{
				const int begin = http.find(simplehttp::mapping::FieldMap.at(simplehttp::Field::BOUNDARY)) + simplehttp::mapping::FieldMap.at(simplehttp::Field::BOUNDARY).size();
				const int end = contentTypePos + http.substr(contentTypePos).find("\r\n");

				if (begin < end)
					res.Boundary = "--" + string(http.begin() + begin, http.begin() + end);
			}
		}
		else if (res.Method == simplehttp::Method::GET)
		{
			wstring unescaped(1024 * 1024, 0);
			DWORD size = unescaped.size();
			tmp = wstring(http.begin() + simplehttp::mapping::MethodMap.at(res.Method).size(),
				http.begin() + http.find(simplehttp::mapping::MethodMap.at(simplehttp::Method::HTTP)));

			res.FileName = UrlUnescapeW(tmp.data(), unescaped.data(), &size, URL_UNESCAPE_AS_UTF8) == S_OK ? wstring(unescaped.begin(), unescaped.begin() + size) : wstring(L"");
		}
		return res;
	}

	SimpleHTTPPostDataResult static ExtractHTTPPostData(vector<char>& data, const string& boundary)
	{
		SimpleHTTPPostDataResult res = SimpleHTTPPostDataResult();

		const vector<char> name(simplehttp::mapping::FieldMap.at(simplehttp::Field::FILENAME).begin(), simplehttp::mapping::FieldMap.at(simplehttp::Field::FILENAME).end());
		const vector<char> bound(boundary.begin(), boundary.end());
		const vector<char> lineEnd = { '\r', '\n' };
		const vector<char> dataStart = { '\r', '\n' ,'\r', '\n' };

		const vector<char>::iterator nameIt = std::search(data.begin(), data.end(), name.begin(), name.end());
		const vector<char>::iterator lineEndIt = std::search(nameIt, data.end(), lineEnd.begin(), lineEnd.end());
		const vector<char>::iterator boundaryStartIt = std::search(data.begin(), data.end(), bound.begin(), bound.end());
		const vector<char>::iterator dataStartIt = std::search(boundaryStartIt + bound.size(), data.end(), dataStart.begin(), dataStart.end());
		const vector<char>::iterator boundaryEndIt = std::search(dataStartIt, data.end(), bound.begin(), bound.end());

		if (nameIt == data.end() || lineEndIt == data.end() || boundaryStartIt == data.end() || dataStartIt == data.end() || boundaryEndIt == data.end())
			return SimpleHTTPPostDataResult();

		const int nameStart = std::distance(data.begin(), nameIt) + simplehttp::mapping::FieldMap.at(simplehttp::Field::FILENAME).size() + 1; // ignore \"
		const int nameEnd = std::distance(data.begin(), lineEndIt) - 1; // ignore \"

		if (nameStart >= nameEnd)
			return SimpleHTTPPostDataResult();

		res.Filename = wstring(data.begin() + nameStart, data.begin() + nameEnd);
		res.DataBegin = std::distance(data.begin(), dataStartIt) + dataStart.size();
		res.DataEnd = std::distance(data.begin(), boundaryEndIt) - 2; // ignore \r\n

		if (res.DataBegin > res.DataEnd)
			return SimpleHTTPPostDataResult();

		res.Count = res.DataEnd - res.DataBegin;

		return res;
	}
};