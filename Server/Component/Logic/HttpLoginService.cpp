//
// Created by zmhy0073 on 2021/11/5.
//
#include"App/App.h"
#include"HttpLoginService.h"
#include"Util/DirectoryHelper.h"
#include"Util/MD5.h"
namespace Sentry
{
    bool HttpLoginService::Awake()
    {
        return true;
    }

    bool HttpLoginService::LateAwake()
    {
        return true;
    }

    XCode HttpLoginService::Login(const Json::Reader &request, Json::Writer &response)
    {
        std::string account;
        std::string password;
        if (!request.GetMember("account", account))
        {
            return XCode::Failure;
        }
        if (!request.GetMember("password", password))
        {
            return XCode::Failure;
        }
        LOG_DEBUG("{0}   {1}", account, password);
        return XCode::Successful;
    }
}