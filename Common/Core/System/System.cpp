//
// Created by zmhy0073 on 2022/10/13.
//

#include"System.h"
#ifdef __OS_WIN__
#include<direct.h>
#else
#include<unistd.h>
#endif // __OS_WIN__
#include"spdlog/fmt/fmt.h"
#include"Util/String/StringHelper.h"
namespace Tendo
{
	bool System::Init(int argc, char** argv)
	{
		if (System::IsInit || argc < 2)
		{
			return false;
		}
		System::IsInit = true;
		System::mExePath = argv[0];
		System::mConfigPath = argv[1];
		System::mWorkPath = fmt::format("{0}/", getcwd(nullptr, 0));
		Helper::Str::ReplaceString(System::mWorkPath, "\\", "/");
		if (System::mWorkPath.back() == '/')
		{
			System::mWorkPath.pop_back();
		}
		std::vector<std::string> result;
		for (int index = 2; index < argc; index++)
		{
			const std::string line(argv[index]);
			if (Helper::Str::Split(line, '=', result) != 2)
			{
				return false;
			}
			System::SetEnv(result[0], result[1]);
		}		
		return System::AddValue("WORK_PATH", mWorkPath);
	}

	bool System::AddValue(const std::string& key, std::string& value)
	{
		char buffer[256] = { 0 };
		sprintf(buffer, "${%s}", key.c_str());
		mSubValues[std::string(buffer)] = value;
		return true;
	}

	bool System::GetEnv(const std::string& k, int& v)
	{
		std::string value;
		if (!System::GetEnv(k, value))
		{
			return false;
		}
		v = std::stoi(value);
		return true;
	}

	bool System::GetEnv(const std::string& k, std::string& v)
	{
		auto iter = System::mEnvs.find(k);
		if (iter == System::mEnvs.end())
		{
			return false;
		}
		v = iter->second;
		return true;
	}

	bool System::SubValue(std::string& value)
	{
		std::smatch match;
		std::regex re(R"(\$?\{[^}]*\})");   // 匹配 ${...} 或 $${...} 格式的字符串
		if (std::regex_search(value, match, re))
		{
			auto iter = mSubValues.begin();
			for (; iter != mSubValues.end(); iter++)
			{
				const std::string& key = iter->first;
				const std::string& val = iter->second;
				Helper::Str::ReplaceString(value, key, val);
			}
			return true;
		}
		return false;
	}

	bool System::SetEnv(const std::string& k, const std::string& v)
	{
		auto iter = System::mEnvs.find(k);
		if(iter != System::mEnvs.end())
		{
			return false;
		}
		System::mEnvs.emplace(k, v);
		return true;
	}

    std::string System::FormatPath(const std::string &path)
    {
        return System::mWorkPath + path;
    }

    bool System::IsInit = false;
    std::string System::mExePath;
    std::string System::mWorkPath;
    std::string System::mConfigPath;
	std::unordered_map<std::string, std::string> System::mEnvs;
	std::unordered_map<std::string, std::string> System::mSubValues;
}