#include "NetworkHelper.h"
#include <regex>
namespace GameKeeper
{
	bool NetworkHelper::IsIp(const std::string & ip)
	{
		std::regex regExpress(
			R"((?=(\b|\D))(((\d{1,2})|(1\d{1,2})|(2[0-4]\d)|(25[0-5]))\.){3}((\d{1,2})|(1\d{1,2})|(2[0-4]\d)|(25[0-5]))(?=(\b|\D)))");
		return std::regex_match(ip, regExpress);
	}

	bool NetworkHelper::ParseHttpUrl(const std::string & url, std::string & host, std::string & port, std::string & path)
	{
		std::cmatch what;
		std::string protocol;
		std::regex pattern("(http|https)://([^/ :]+):?([^/ ]*)(/.*)?");	
		if (std::regex_match(url.c_str(), what, pattern))
		{
			host = std::string(what[2].first, what[2].second);		
			path = std::string(what[4].first, what[4].second);
			protocol = std::string(what[1].first, what[1].second);
			port = std::string(what[3].first, what[3].second);

			if (0 == port.length())
			{
				port = "http" == protocol ? "80" : "443";
			}
			return true;
		}
		return false;
		

		const static std::string http = "http://";
		const static std::string https = "https://";

		std::string content;
		size_t pos = url.find(http);
		if (pos != std::string::npos)
		{
			content = url.substr(http.size());
		}
		else
		{
			pos = url.find(https);
			if (pos != std::string::npos)
			{
				content = url.substr(https.size());
			}
		}
		if (content.empty())
		{
			return false;
		}

		port = "80";
		size_t pos1 = content.find(':');
		size_t pos2 = content.find('/');
		if (pos2 != std::string::npos)
		{
			path = content.substr(pos2);
			host = content.substr(0, pos2);
			if (pos1 != std::string::npos)
			{
				host = content.substr(0, pos1);
				port = std::to_string(std::stoul(content.substr(pos1 + 1, pos2 - pos1 - 1)));
			}
		}
		else
		{
			path = "/";
			host = content;
			if (pos1 != std::string::npos)
			{
				host = content.substr(0, pos1);
				port = std::to_string(std::stoul(content.substr(pos1 + 1, pos2 - pos1 - 1)));
			}
		}
		return true;
	}
}
