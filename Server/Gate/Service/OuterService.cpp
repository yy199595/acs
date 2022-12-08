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
    OuterService::OuterService()
    {
        this->mOuterComponent = nullptr;
    }
    void OuterService::Init()
    {
        this->mApp->AddComponent<OuterNetComponent>();
        this->mApp->AddComponent<OuterNetMessageComponent>();
    }

	bool OuterService::OnStart()
	{
        BIND_COMMON_RPC_METHOD(OuterService::Allot);
        const ServerConfig * config = ServerConfig::Inst();
        ServerConfig::Inst()->GetLocation("gate", this->mAddress);
        this->mOuterComponent = this->GetComponent<OuterNetMessageComponent>();
        LOG_CHECK_RET_FALSE(config->GetLocation("gate", this->mAddress));
        return true;
	}

    bool OuterService::OnClose()
    {
		return this->GetComponent<OuterNetComponent>()->StopListen();
    }

	XCode OuterService::Ping(long long userId)
	{
		LOG_ERROR(userId << " ping gate server");
		return XCode::Failure;
	}

	XCode OuterService::Allot(const com::type::int64 &request, s2s::allot::response &response)
    {
        std::string token;
        long long userId = request.value();
        if(!this->mOuterComponent->CreateToken(userId, token))
        {
            return XCode::Failure;
        }
        response.set_token(token);
        response.set_address(this->mAddress);
        return XCode::Successful;
    }
}