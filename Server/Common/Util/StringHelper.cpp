#include"StringHelper.h"
#include<regex>
#include<sstream>
#include"MD5.h"
#include"MathHelper.h"
namespace StringHelper
{
	void SplitString(const std::string & targetString, const std::string delim, std::vector<std::string>& ret)
	{
		if (targetString.length() == 0)
			return;
		std::regex re{ delim };
		
		ret = std::vector<std::string>
		{
			std::sregex_token_iterator(targetString.begin(), targetString.end(), re, -1),
				std::sregex_token_iterator()
		};
	}
	void ReplaceString(std::string & outstring, const std::string str1, const std::string str2)
	{
		size_t pos = outstring.find(str1);
		while (pos != std::string::npos)
		{
			outstring.replace(pos, str1.length(), str2);
			pos = outstring.find(str1);
		}
	}

	extern void ClearBlank(std::string & input)
	{
		int index = 0;
		if (!input.empty())
		{
			while ((index = input.find(' ', index)) != std::string::npos)
			{
				input.erase(index, 1);
			}
		}
	}
	std::string GetFileName(const std::string & path)
	{
		size_t pos = path.find_last_of('\\');
		if (pos != std::string::npos)
		{		
			return path.substr(pos + 1, path.size());
		}
		pos = path.find_last_of('/');
		return path.substr(pos + 1, path.size());

	}
	std::string RandomString(size_t size)
	{
		std::stringstream ss;
		const static std::string buffer = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890";
		for (size_t index = 0; index < size; index++)
		{
			int pos = MathHelper::Random<int>(0, (int)buffer.size());
			ss << buffer[pos];
		}
		return ss.str();
	}

	std::string CreateNewToken()
	{
		const int size = MathHelper::Random<int>(30, 100);
		MD5 md5(RandomString(size));
		return md5.toString();
	}

	bool ParseIpAddress(const std::string & address, std::string & ip, unsigned short & port)
	{
		size_t pos = address.find(":");
		if (pos == std::string::npos)
		{
			return false;
		}
		ip = address.substr(0, pos);
		port = (unsigned short)std::stoul(address.substr(pos + 1));
		return true;
	}
}