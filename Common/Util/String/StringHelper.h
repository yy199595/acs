
#pragma once
#include<regex>
#include <string>
#include <vector>
namespace Helper
{
    namespace Str
    {
        extern std::string EmptyStr;

        extern const std::string & Empty();

        //转大写
        extern void Toupper(std::string & str);

        //转小写
        extern void Tolower(std::string & str);

        extern std::string CreateNewToken();

        extern void ClearBlank(std::string &input);

        extern std::string RandomString(size_t size = 64);

        extern bool GetFileName(const std::string &path, std::string & name);

        extern void FormatJson(const std::string &json, std::string & format);

        extern void ReplaceString(std::string &outstring, const std::string& str1, const std::string& str2);

        extern size_t Split(const std::string &targetString, const char * cc, std::vector<std::string> &ret);
    }

    namespace Str
    {
		extern bool IsRpcAddr(const std::string & address);
		extern bool IsHttpAddr(const std::string & address);
        extern bool SplitAddr(const std::string& address, std::string& ip, unsigned short& port);
    }
}// namespace StringHelper
