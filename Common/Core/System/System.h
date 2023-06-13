//
// Created by zmhy0073 on 2022/10/13.
//

#ifndef APP_SYSTEM_H
#define APP_SYSTEM_H

#endif //APP_SYSTEM_H
#include<string>
#include<unordered_map>
namespace Tendo
{
    class System
    {
    public:
        static bool Init(int argc, char **argv);
        static std::string FormatPath(const std::string & path);
        static const std::string & ExePath() { return System::mExePath; }
        static const std::string & WorkPath() { return System::mWorkPath; }
        static const std::string & ConfigPath() { return System::mConfigPath;}
	public: 
		static bool GetEnv(const std::string & k, int & v);
		static bool GetEnv(const std::string & k, std::string & v);
		static bool SetEnv(const std::string & k, const std::string & v);
    public:
        static bool SubValue(std::string& value);
        static bool AddValue(const std::string& key, std::string& value);
	private:
        static bool IsInit;
        static std::string mExePath;
        static std::string mWorkPath;
        static std::string mConfigPath;
		static std::unordered_map<std::string, std::string> mEnvs;
		static std::unordered_map<std::string, std::string> mSubValues;
	};
}
