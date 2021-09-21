﻿#include "LoginService.h"
#include <Scene/MysqlComponent.h>
#include <Scene/RedisComponent.h>

namespace Sentry
{
    LoginService::LoginService()
    {
    }

    bool LoginService::Awake()
    {


        return LocalServiceComponent::Awake();
    }

    void LoginService::Start()
    {
    }

    XCode LoginService::Login(long long operId, shared_ptr<c2s::UserVerify_Request> LoginData)
    {
        return XCode::Failure;
    }

    XCode LoginService::Register(long long operId, shared_ptr<c2s::UserRegister_Request> registerData)
    {
        return XCode::Successful;
    }
}// namespace Sentry