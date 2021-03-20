#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

class SimpleHTML
{
private:
	string Title;
	string GeneralMessage;
	vector<string> LinkDirectory;
	vector<string> LinkDownloadable;
public:
	SimpleHTML()
	{
		Clear();
	}
	void SetTitle(const string& title)
	{
		Title = title;
	}
	void SetGeneralMessage(const string& message)
	{
		GeneralMessage = message;
	}
	void AddLinkDirectory(const string& link)
	{
		LinkDirectory.push_back(link);
	}
	void AddLinkDownloadable(const string& link)
	{
		LinkDownloadable.push_back(link);
	}
	void Clear()
	{
		Title = string();
		LinkDirectory = vector<string>();
		LinkDownloadable = vector<string>();
	}
	string BuildHTML()
	{
		string ret = "<!DOCTYPE html>\r\n";
		ret += "<html>\r\n";

		ret += "<head>\r\n";
		ret += "<meta charset=\"UTF-8\"/>\r\n";
		ret += "<meta name=viewport content=\"initial-scale=1, minimum-scale=1, width=device-width\">\r\n";

		if (Title.length())
			ret += "<title>" + Title + "</title>\r\n";
		ret += "</head>\r\n";

		ret += "<body>\r\n";

		ret += "<a href=\"/\">Home</a><br>\r\n";

		if (GeneralMessage.length())
			ret += "<p>" + GeneralMessage + "</p>\r\n";

		const unsigned int countDir = (unsigned int)LinkDirectory.size();
		if (countDir)
		{
			ret += "<p>Directories</p>\r\n";
			for (unsigned int i = 0; i < countDir; i++)
			{
				ret += "<a href=\"/" + LinkDirectory[i] + "\">" + LinkDirectory[i] + "</a><br>\r\n";
			}
		}

		const unsigned int countDown = (unsigned int)LinkDownloadable.size();
		if (countDown)
		{
			ret += "<p>Downloadable</p>\r\n";
			for (unsigned int i = 0; i < countDown; i++)
			{
				ret += "<a href=\"/" + LinkDownloadable[i] + "\">" + LinkDownloadable[i] + "</a><br>\r\n";
			}
		}

		ret += "<p>Send something to PC</p>\r\n";
		ret += "<form id=\"uploadbanner\" enctype=\"multipart/form-data\" method=\"post\" action=\"#\">\r\n";
		ret += "<input id=\"fileupload\" name=\"myfile\" type = \"file\" />\r\n";
		ret += "<input type=\"submit\" value=\"Send\" id=\"submit\" />\r\n";
		ret += "</form>\r\n";

		ret += "</body>\r\n";
		ret += "</html>\r\n";
		return ret;
	}
};