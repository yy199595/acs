//
// Created by zmhy0073 on 2022/10/13.
//

#ifndef APP_SYSTEM_H
#define APP_SYSTEM_H

#endif //APP_SYSTEM_H
#include<string>
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
    private:
        static bool IsInit;       
        static std::string mExePath;
        static std::string mWorkPath;
        static std::string mConfigPath;
    };
}
