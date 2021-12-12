//
// Created by zmhy0073 on 2021/11/5.
//
#include<Core/App.h>
#include "HttpLoginService.h"
#include<Util/DirectoryHelper.h>
#include<Util/MD5.h>
namespace GameKeeper
{
    bool HttpLoginService::Awake()
    {
        this->Add("Login", &HttpLoginService::Login, this);
        return true;
    }

    bool HttpLoginService::LateAwake()
    {
        return true;
    }

    XCode HttpLoginService::Login(const RapidJsonReader &request, RapidJsonWriter &response)
    {
        std::string account;
        std::string password;
        if (!request.TryGetValue("account", account))
        {
            return XCode::Failure;
        }
        if (!request.TryGetValue("password", password))
        {
            return XCode::Failure;
        }
        LOG_DEBUG(account << "   " << password);
        return XCode::Successful;
    }
}