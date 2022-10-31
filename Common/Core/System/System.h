//
// Created by zmhy0073 on 2022/10/13.
//

#ifndef APP_SYSTEM_H
#define APP_SYSTEM_H

#endif //APP_SYSTEM_H
#include<string>
namespace Sentry
{
    class System
    {
    public:
        static void Init(char **argv);
        static const std::string FormatPath(const std::string & path);
        static const std::string & Name() { return System::mName; }
        static const std::string & ExePath() { return System::mExePath; }
        static const std::string & WorkPath() { return System::mWorkPath; }
        static const std::string & ConfigPath() { return System::mConfigPath;}
    private:
        static std::string mName;
        static std::string mExePath;
        static std::string mWorkPath;
        static std::string mConfigPath;
    };
}
