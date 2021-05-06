
#pragma once
#include<string>
#include<vector>
namespace StringHelper
{
	extern void ClearBlank(std::string & input);
	extern std::string GetFileName(const std::string & path);
	extern void RandomString(std::string & outString, size_t size = 64);
	extern bool ParseIpAddress(const std::string & address, std::string & ip, unsigned short & port);
	extern void ReplaceString(std::string & outstring, const std::string str1, const std::string str2);
	extern void SplitString(const std::string & targetString, const std::string cc, std::vector<std::string> & ret);
}
