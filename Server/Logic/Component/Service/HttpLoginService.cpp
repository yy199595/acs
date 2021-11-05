//
// Created by zmhy0073 on 2021/11/5.
//

#include "HttpLoginService.h"

namespace GameKeeper
{
    bool HttpLoginService::Awake()
    {
        this->Add("Login", &HttpLoginService::Login, this);
        return true;
    }

    XCode HttpLoginService::Login(RapidJsonReader &request, RapidJsonWriter &response)
    {
        std::string account;
        std::string password;
        if(!request.TryGetValue("account", account))
        {
            return XCode::Failure;
        }
        if(!request.TryGetValue("password", password))
        {
            return XCode::Failure;
        }
        GKDebugLog(account << "   " << password);
        return XCode::Successful;
    }
}