//
// Created by zmhy0073 on 2021/12/1.
//

#include"OuterService.h"
#include"App/App.h"
#include"Md5/MD5.h"
#include"Client/OuterNetClient.h"
#include"Component/OuterNetComponent.h"
#include"Component/GateHelperComponent.h"
#include"Component/OuterNetMessageComponent.h"
namespace Sentry
{

    bool OuterService::Awake()
    {
        this->mTimerComponent = nullptr;
        this->mOuterComponent = nullptr;
        this->mApp->AddComponent<OuterNetComponent>();
        this->mApp->AddComponent<OuterNetMessageComponent>();
        return true;
    }

	bool OuterService::OnStart()
	{
        const ServerConfig * config = ServerConfig::Inst();
        BIND_COMMON_RPC_METHOD(OuterService::Ping);
        BIND_COMMON_RPC_METHOD(OuterService::AllotUser);
        LOG_CHECK_RET_FALSE(config->GetLocation("gate", this->mAddress));
        this->mOuterComponent = this->GetComponent<OuterNetMessageComponent>();
        return true;
	}

    bool OuterService::OnClose()
    {
        this->GetComponent<OuterNetComponent>()->StopListen();
		return true;
    }

	XCode OuterService::Ping(long long userId)
	{
		LOG_ERROR(userId << " ping gate server");
		return XCode::Failure;
	}

	XCode OuterService::AllotUser(const com::type::int64 &request, s2s::allot::response &response)
    {
        long long userId = request.value();
        if(!this->mOuterComponent->CreateToken(userId, *response.mutable_token()))
        {
            return XCode::Failure;
        }
        response.set_address(this->mAddress);
        return XCode::Successful;
    }
}