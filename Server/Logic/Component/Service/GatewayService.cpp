//
// Created by zmhy0073 on 2021/10/11.
//

#include "GatewayService.h"
#include <Object/GameObject.h>
#include <Scene/RedisComponent.h>
#include <Scene/GatewayComponent.h>
#include <Scene/GameObjectComponent.h>
namespace Sentry
{
    bool GatewayService::Awake()
    {
        __add_method(GatewayService::Login);
        __add_method(GatewayService::Logout);
        this->mRedisComponent = this->GetComponent<RedisComponent>();
        this->mGateComponent = this->GetComponent<GatewayComponent>();
        this->mGameObjComponent = this->GetComponent<GameObjectComponent>();
        return true;
    }

    XCode GatewayService::Login(long long id, c2s::GateLogin_Request & request)
    {
        std::string queryData;
        const std::string & token = request.token();
        const std::string address = this->GetCurAddress();
        if(!this->mRedisComponent->GetValue(token,queryData))
        {
            return XCode::Failure;
        }
        long long userId = std::stoll(queryData);
        GameObject * gameObject = new GameObject(userId, address);
        // 给连接到网关的对象加组件
        if(!this->mGameObjComponent->Add(gameObject))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

    XCode GatewayService::Logout(long long id, c2s::GateLogout_Request & request)
    {
        const std::string address = this->GetCurAddress();
        GameObject * gameObject = this->mGameObjComponent->Find(address);
        if(gameObject == nullptr)
        {
            return XCode::Failure;
        }
        this->mGameObjComponent->Del(gameObject);
        return XCode::Successful;
    }
}