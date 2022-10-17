//
// Created by zmhy0073 on 2022/10/13.
//

#include"System.h"
#ifdef __OS_WIN__
#include<direct.h>
#else
#include<unistd.h>
#endif // 
#include"Log/CommonLogDef.h"
namespace Sentry
{
    void System::Init(char **argv)
    {
        System::mExePath.append(argv[0]);
        System::mConfigPath.append(argv[1]);
        System::mName.append(argv[2]);
        System::mWorkPath.append(getcwd(NULL, NULL));
        System::mWorkPath += "/";
        CONSOLE_LOG_ERROR("exe path = " << argv[0]);
        CONSOLE_LOG_ERROR("config path = " << argv[1]);
        CONSOLE_LOG_ERROR("server name = " << argv[2]);
        CONSOLE_LOG_ERROR("work path = " << getcwd(NULL, NULL));
    }

    std::string System::mName;
    std::string System::mExePath;
    std::string System::mWorkPath;
    std::string System::mConfigPath;
}