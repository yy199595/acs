//
// Created by zmhy0073 on 2021/12/1.
//

#include"OuterService.h"
#include"App/App.h"
#include"Md5/MD5.h"
#include"Client/OuterNetClient.h"
#include"Component/RedisDataComponent.h"
#include"Component/OuterNetMessageComponent.h"
#include"Component/OuterNetComponent.h"
#include"Component/GateHelperComponent.h"
#include"Component/UserSyncComponent.h"
namespace Sentry
{

    bool OuterService::Awake()
    {
        this->mTimerComponent = nullptr;
        this->mOuterNetComponent = nullptr;
        this->GetApp()->AddComponent<OuterNetComponent>();
        this->GetApp()->AddComponent<OuterNetMessageComponent>();
        return true;
    }

	bool OuterService::OnStart()
	{
        const ServerConfig * config = ServerConfig::Get();
        BIND_COMMON_RPC_METHOD(OuterService::Ping);
        BIND_COMMON_RPC_METHOD(OuterService::AllotUser);
        BIND_COMMON_RPC_METHOD(OuterService::SaveAddress);
        BIND_COMMON_RPC_METHOD(OuterService::QueryAddress);
        LOG_CHECK_RET_FALSE(config->GetLocation("gate", this->mAddress));
        this->mOuterNetComponent = this->GetComponent<OuterNetComponent>();
		return this->mOuterNetComponent->StartListen("gate");
	}

    bool OuterService::OnClose()
    {
        this->mOuterNetComponent->StopListen();
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
        std::string token = this->mOuterNetComponent->CreateToken(userId);
        response.set_token(token);
        response.set_address(this->mAddress);
        return XCode::Successful;
    }

	XCode OuterService::QueryAddress(long long userId, const com::type::string& request, com::type::string& response)
	{
		return XCode::Successful;
	}

	XCode OuterService::SaveAddress(long long userId, const s2s::allot::save& request)
	{
		return XCode::Successful;
	}
}