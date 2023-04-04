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
namespace Tendo
{
    bool System::Init(int argc, char **argv)
    {
        if (System::IsInit)
        {
            return false;
        }
#ifdef __DEBUG__
        if (argc < 2)
        {
            System::IsInit = true;
            System::mExePath = argv[0];
            System::mConfigPath = "./config/config.json";
            System::mWorkPath = fmt::format("{0}/", getcwd(nullptr, 0));
            return true;
        }
#endif
        System::IsInit = true;
        System::mExePath = argv[0];
        System::mConfigPath = argv[1];
        System::mWorkPath = fmt::format("{0}/", getcwd(nullptr, 0));
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
}