#pragma once

#include <string>
#include <vector>

using std::wstring;
using std::vector;

class SimpleHTML
{
private:
	wstring Title;
	wstring GeneralMessage;
	vector<wstring> LinkDirectory;
	vector<wstring> LinkDownloadable;
public:
	SimpleHTML()
	{
		Clear();
	}
	void SetTitle(const wstring& title)
	{
		Title = title;
	}
	void SetGeneralMessage(const wstring& message)
	{
		GeneralMessage = message;
	}
	void AddLinkDirectory(const wstring& link)
	{
		LinkDirectory.push_back(link);
	}
	void AddLinkDownloadable(const wstring& link)
	{
		LinkDownloadable.push_back(link);
	}
	void Clear()
	{
		Title = wstring();
		LinkDirectory = vector<wstring>();
		LinkDownloadable = vector<wstring>();
	}
	wstring BuildHTML()
	{
		wstring ret = L"<!DOCTYPE html>\r\n";
		ret += L"<html>\r\n";

		ret += L"<head>\r\n";
		ret += L"<meta charset=\"UTF-8\"/>\r\n";
		ret += L"<meta name=viewport content=\"initial-scale=1, minimum-scale=1, width=device-width\">\r\n";

		if (Title.length())
			ret += L"<title>" + Title + L"</title>\r\n";
		ret += L"</head>\r\n";

		ret += L"<body>\r\n";

		ret += L"<a href=\"/\">Home</a><br>\r\n";

		if (GeneralMessage.length())
			ret += L"<p>" + GeneralMessage + L"</p>\r\n";

		const unsigned int countDir = (unsigned int)LinkDirectory.size();
		if (countDir)
		{
			ret += L"<p>Directories</p>\r\n";
			for (unsigned int i = 0; i < countDir; i++)
			{
				ret += L"<a href=\"/" + LinkDirectory[i] + L"\">" + LinkDirectory[i] + L"</a><br>\r\n";
			}
		}

		const unsigned int countDown = (unsigned int)LinkDownloadable.size();
		if (countDown)
		{
			ret += L"<p>Downloadable</p>\r\n";
			for (unsigned int i = 0; i < countDown; i++)
			{
				ret += L"<a href=\"/" + LinkDownloadable[i] + L"\">" + LinkDownloadable[i] + L"</a><br>\r\n";
			}
		}

		ret += L"<p>Send something to PC</p>\r\n";
		ret += L"<form id=\"uploadbanner\" enctype=\"multipart/form-data\" method=\"post\" action=\"#\">\r\n";
		ret += L"<input id=\"fileupload\" name=\"myfile\" type = \"file\" />\r\n";
		ret += L"<input type=\"submit\" value=\"Send\" id=\"submit\" />\r\n";
		ret += L"</form>\r\n";

		ret += L"</body>\r\n";
		ret += L"</html>\r\n";
		return ret;
	}
};