#include"StringHelper.h"
#include"Md5/MD5.h"
#include<regex>
#include"Math/MathHelper.h"
#include"google/protobuf/message.h"
namespace Helper
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

    size_t Str::Split(const std::string &targetString, const char * cc, std::vector<std::string>& ret)
    {
        google::protobuf::SplitStringUsing(targetString, cc, &ret);
        return ret.size();
    }

    void Str::ReplaceString(std::string &outstring, const std::string str1, const std::string str2)
    {
        size_t pos = outstring.find(str1);
        while (pos != std::string::npos)
        {
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
        size_t pos = path.find_last_of('\\');
        if (pos != std::string::npos)
        {
            name = path.substr(pos + 1, path.size());
			return true;
        }
        pos = path.find_last_of('/');
		if(pos != std::string::npos)
		{
			name = path.substr(pos + 1, path.size());
			return true;
		}
		return false;
    }

    void Str::FormatJson(const std::string &json, std::string & format)
    {
        auto getLevelStr = [](int level, std::string &str) {
            for (int i = 0; i < level; i++)
            {
                str += "\t"; //这里可以\t换成你所需要缩进的空格数
            }
        };

        int level = 0;
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
    }

    std::string Str::RandomString(size_t size)
    {
		char x = 0;
		std::unique_ptr<char[]> buffer(new char[size]);
		for(size_t index = 0; index < size; index++)
		{
#ifdef __OS_WIN__
            buffer[index] = std::rand() & 0XFF;
#else
            buffer[index] = random() & 0XFF;
#endif // __OS_WIN__

			x ^= buffer[index];
		}
		if(x == 0)
		{
			buffer[x] |= 1;
		}
		return std::string(buffer.get(), size);
    }

    std::string Str::CreateNewToken()
    {
        const int size = Math::Random<int>(30, 100);
        return Md5::GetMd5(RandomString(size));
    }
}// namespace StringHelper

namespace Helper
{
    bool Str::SplitAddr(const std::string& address, std::string& ip, unsigned short& port)
    {
		if(Str::IsRpcAddr(address))
		{
			size_t pos = address.find(':');
			if (pos == std::string::npos)
			{
				return false;
			}
			ip = address.substr(0, pos);
			port = (unsigned short)std::stoul(address.substr(pos + 1));
			return true;
		}
		else if(Str::IsHttpAddr(address))
		{
			std::regex pattern("(http://)?([^:/]+)(:\\d+)?");
			std::smatch match;
			if (std::regex_search(address, match, pattern))
			{
				ip = match[2];
				port = std::stol(match[3].str().substr(1));
				return true;
			}
		}
		return false;
    }

	bool Str::IsRpcAddr(const std::string& address)
	{
		std::regex pattern(R"(^\d{1,3}(\.\d{1,3}){3}:\d{1,5}$)");
		return std::regex_match(address, pattern);
	}

	bool Str::IsHttpAddr(const std::string& address)
	{
		std::regex pattern(R"((http|https)://[\w\-_]+(\.[\w\-_]+)+([\w\-\.,@?^=%&:/~\+#]*[\w\-\@?^=%&/~\+#])?)");
		return std::regex_match(address, pattern);
	}
}