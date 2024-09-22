
#pragma once
#include<regex>
#include <string>
#include <vector>
namespace help
{
    namespace Str
    {
        extern std::string EmptyStr;

        extern const std::string & Empty();

		extern bool IsPhoneNumber(const std::string & phone);

		extern size_t Hash(const std::string & str);

        //转大写
        extern std::string Toupper(const std::string & str);

        //转小写
        extern std::string Tolower(const std::string & str);

        extern std::string CreateNewToken();

        extern void ClearBlank(std::string &input);

        extern std::string RandomString(size_t size = 64);

        extern bool GetFileName(const std::string &path, std::string & name);

        extern std::string FormatJson(const std::string &json);

        extern void ReplaceString(std::string &outstring, const std::string& str1, const std::string& str2);

		extern size_t Split(const std::string &target, char cc, std::string & str1, std::string & str2);

		extern size_t Split(const std::string &targetString, char cc, std::vector<std::string> &ret);
    }

    namespace Str
    {
		extern bool IsIpAddress(const std::string & str);
		extern bool IsRpcAddr(const std::string & address);
		extern bool IsHttpAddr(const std::string & address);
		extern bool SplitAddr(const std::string& address, std::string& ip, unsigned short& port);
		extern bool SplitAddr(const std::string& address, std::string & net, std::string& ip, unsigned short& port);
    }
}// namespace StringHelper


namespace help
{
	namespace utf8
	{
		extern size_t Length(const std::string & str);
		extern std::string Sub(const std::string & str, int start, int count);
	}
}