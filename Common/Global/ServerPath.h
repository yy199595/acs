//
// Created by zmhy0073 on 2021/10/26.
//
#include<string>
#ifndef SENTRY_SERVERPATH_H
#define SENTRY_SERVERPATH_H

namespace Sentry
{
    class ServerPath
    {
    public:
        ServerPath(int argc, char ** argv);
    public:
        const std::string & GetLogPath() { return this->mLogsPath;}
        const std::string & GetWorkPath() { return this->mWorkPath;}
        const std::string & GetConfigPath() { return this->mConfigPath;}
    private:
        std::string mLogsPath;
        std::string mWorkPath;
        std::string mConfigPath;
    };
}

#endif //SENTRY_SERVERPATH_H
