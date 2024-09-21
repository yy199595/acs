//
// Created by zmhy0073 on 2022/10/13.
//

#ifndef APP_SYSTEM_H
#define APP_SYSTEM_H

#endif //APP_SYSTEM_H
#include<string>
#include<unordered_map>
namespace os
{
	class SystemInfo
	{
	public:
		double cpu = 0;
		long long use_memory = 0;
		long long max_memory = 0;
	};

    class System
    {
    public:
        static bool Init(int argc, char **argv);
		static bool GetSystemInfo(SystemInfo & systemInfo);
		static std::string FormatPath(const std::string & path);
        static const std::string & WorkPath() { return System::mWorkPath; }
	public:
        static bool HasEnv(const std::string& k);
		static bool GetEnv(const std::string & k, int & v);
		static bool GetEnv(const std::string & k, std::string & v);
		static void SetEnv(const std::string & k, const std::string & v);
		static bool ReadFile(const std::string & path, std::string & content);
		static bool AddValue(const std::string& key, const std::string& value);
	public:
		static void LuaSetEnv(const char * key, const char * val);
	private:
        static bool SubValue(std::string& value);
	private:
        static std::string mWorkPath;
		static std::unordered_map<std::string, std::string> mSubValues;
	};
}
