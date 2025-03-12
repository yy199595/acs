#include"String.h"
#include<regex>
#include<sstream>
#include <codecvt>
#include"Random.h"
namespace help
{
    std::string Str::EmptyStr;

    const std::string & Str::Empty()
    {
        return EmptyStr;
    }

    void Str::Tolower(std::string &str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

	void Str::Toupper(std::string &str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	}

	size_t Str::Hash(const std::string& str)
	{
		std::hash<std::string> hash;
		return hash(str);
	}



    size_t Str::Split(const std::string &targetString, char cc, std::vector<std::string>& ret)
    {
        std::string item;
        std::stringstream ss(targetString);
        while (std::getline(ss, item, cc)) 
        {
            ret.emplace_back(item);
        }
        return ret.size();
    }

	size_t Str::Split(const std::string& target, char cc, std::string& str1, std::string& str2)
	{
		size_t pos = target.find(cc);
		if (pos == std::string::npos)
		{
			return -1;
		}
		str2.clear();
		str1.assign(target.c_str(), pos);
		if(target.size() > pos + 1)
		{
			str2.assign(target.c_str() + pos + 1);
		}
		return 0;
	}

    void Str::ReplaceString(std::string &outstring, const std::string& str1, const std::string& str2)
    {
		int index = 0;
        size_t pos = outstring.find(str1);
        while (pos != std::string::npos && index < 100)
        {
			index++;
            outstring.replace(pos, str1.length(), str2);
            pos = outstring.find(str1);
        }
    }

    extern void Str::ClearBlank(std::string &input)
    {
        input.erase(std::remove(input.begin(), input.end(), ' '), input.end());
    }

    bool Str::GetFileName(const std::string &path, std::string & name)
    {
		size_t pos = path.find_last_of('/');
		if(pos != std::string::npos)
		{
			name = path.substr(pos + 1, path.size());
			return true;
		}
		pos = path.find_last_of('\\');
        if (pos != std::string::npos)
        {
            name = path.substr(pos + 1, path.size());
			return true;
        }

		return false;
    }

    std::string Str::FormatJson(const std::string &json)
    {
		std::string format;
        auto getLevelStr = [](int level, std::string &str) {
            for (int i = 0; i < level; i++)
            {
                str += "    "; //这里可以\t换成你所需要缩进的空格数
            }
        };

        int level = 0;
        for (size_t index = 0; index < json.size(); index++)
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

    std::string Str::RandomString(int size)
    {
		std::string result;
		result.resize(size);
		static const std::string STR_BUFFER("123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM");
		for(size_t index = 0; index < size; index++)
		{
			int max = (int)STR_BUFFER.size();
			int num = help::Rand::Random<int>(0, max - 1);
			result[index] = (char)STR_BUFFER[num];
		}
		return result;
    }

}

namespace help
{
    bool Str::SplitAddr(const std::string& address, std::string & net, std::string& ip, unsigned short& port)
    {
		std::smatch match;
		std::regex pattern(R"((\w+)://(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):(\d+))");
		if (!std::regex_search(address, match, pattern))
		{
			return false;
		}
		net = match[1].str();
		ip = match[2].str();
		port = (unsigned short)std::stoi(match[3].str());
		return true;
    }

	bool Str::SplitAddr(const std::string& address, std::string& ip, unsigned short& port)
	{
		std::smatch match;
		std::regex pattern(R"((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):(\d+))");
		if (!std::regex_search(address, match, pattern))
		{
			return false;
		}
		ip = match[1].str();
		port = (unsigned short)std::stoi(match[2].str());
		return true;
	}

	bool Str::IsIpAddress(const std::string& str)
	{
		std::regex pattern(R"(^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$)");
		return std::regex_match(str, pattern);
	}

	bool Str::IsRpcAddr(const std::string& address)
	{
		//std::regex pattern(R"(^\d{1,3}(\.\d{1,3}){3}:\d{1,5}$)");
		std::regex pattern(R"(tcp://[\w\-_]+(\.[\w\-_]+)+([\w\-\.,@?^=%&:/~\+#]*[\w\-\@?^=%&/~\+#])?)");
		return std::regex_match(address, pattern);
	}

	bool Str::IsHttpAddr(const std::string& address)
	{
		std::regex pattern(R"((http|https)://[\w\-_]+(\.[\w\-_]+)+([\w\-\.,@?^=%&:/~\+#]*[\w\-\@?^=%&/~\+#])?)");
		return std::regex_match(address, pattern);
	}

	bool Str::IsPhoneNumber(const std::string& phoneNumber)
	{
		if(phoneNumber.empty())
		{
			return false;
		}
		std::regex pattern("^1[3456789]\\d{9}$");
		return std::regex_match(phoneNumber, pattern);
	}
}

namespace help
{
	size_t utf8::Length(const std::string& str)
	{
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
		std::u32string utf32_string = converter.from_bytes(str);
		return utf32_string.size();
	}

	std::string utf8::Sub(const std::string& utf8_string, int start, int length)
	{
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
		std::u32string utf32_string = converter.from_bytes(utf8_string);

		// 计算截取位置
		auto begin_it = utf32_string.begin() + start;
		auto end_it = utf32_string.begin() + std::min(start + length, static_cast<int>(utf32_string.size()));

		// 截取子字符串
		std::u32string substr(begin_it, end_it);

		// 将子字符串转换回 UTF-8 编码
		return converter.to_bytes(substr);
	}
}