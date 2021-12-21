#include "StringHelper.h"
#include "MD5.h"
#include "MathHelper.h"
#include <regex>
#include <sstream>
namespace Helper
{
    std::string String::EmptyStr = "";

    const std::string & String::Empty()
    {
        return EmptyStr;
    }

    void String::SplitString(const std::string &targetString, const std::string delim, std::vector<std::string> &ret)
    {
        ret.clear();
        if (targetString.length() == 0)
            return;
        std::regex re{delim};

        ret = std::vector<std::string>{
                std::sregex_token_iterator(targetString.begin(), targetString.end(), re, -1),
                std::sregex_token_iterator()};
    }

    void String::ReplaceString(std::string &outstring, const std::string str1, const std::string str2)
    {
        size_t pos = outstring.find(str1);
        while (pos != std::string::npos)
        {
            outstring.replace(pos, str1.length(), str2);
            pos = outstring.find(str1);
        }
    }

    extern void String::ClearBlank(std::string &input)
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

    std::string String::GetFileName(const std::string &path)
    {
        size_t pos = path.find_last_of('\\');
        if (pos != std::string::npos)
        {
            return path.substr(pos + 1, path.size());
        }
        pos = path.find_last_of('/');
        return path.substr(pos + 1, path.size());
    }

    std::string String::FormatJson(const std::string &json)
    {
        auto getLevelStr = [](int level, std::string &str) {
            for (int i = 0; i < level; i++)
            {
                str += "\t"; //这里可以\t换成你所需要缩进的空格数
            }
        };

        int level = 0;
        std::string format;
        for (string::size_type index = 0; index < json.size(); index++)
        {
            char c = json[index];

            if (level > 0 && '\n' == json[json.size() - 1])
            {
                getLevelStr(level, format);
            }

            switch (c)
            {
                case '{':
                case '[':
                    format = format + c + "\n";
                    level++;
                    getLevelStr(level, format);
                    break;
                case ',':
                    format = format + c + "\n";
                    getLevelStr(level, format);
                    break;
                case '}':
                case ']':
                    format += "\n";
                    level--;
                    getLevelStr(level, format);
                    format += c;
                    break;
                default:
                    format += c;
                    break;
            }
        }
        return format;
    }

    std::string String::RandomString(size_t size)
    {
        std::stringstream ss;
        const static std::string buffer = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890";
        for (size_t index = 0; index < size; index++)
        {
            int pos = Math::Random<int>(0, (int) buffer.size());
            ss << buffer[pos];
        }
        return ss.str();
    }

    std::string String::CreateNewToken()
    {
        const int size = Math::Random<int>(30, 100);
        return Md5::GetMd5(RandomString(size));
    }

    bool String::ParseIpAddress(const std::string &address, std::string &ip, unsigned short &port)
    {
        size_t pos = address.find(":");
        if (pos == std::string::npos)
        {
            return false;
        }
        ip = address.substr(0, pos);
        port = (unsigned short) std::stoul(address.substr(pos + 1));
        return true;
    }
}// namespace StringHelper
