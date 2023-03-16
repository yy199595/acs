//
// Created by zmhy0073 on 2021/12/1.
//

#include"Gate.h"
#include"App/App.h"
#include"Md5/MD5.h"
#include"Client/OuterNetClient.h"
#include"Component/OuterNetComponent.h"
#include"Component/NodeMgrComponent.h"
#include"Component/OuterNetMessageComponent.h"
namespace Sentry
{
    Gate::Gate()
    {
        this->mOuterComponent = nullptr;
    }
    void Gate::Init()
    {
        this->mApp->AddComponent<OuterNetComponent>();
        this->mApp->AddComponent<OuterNetMessageComponent>();
    }

	int Gate::Ping(long long userId)
	{
		return XCode::Successful;
	}

	bool Gate::OnStart()
	{
        BIND_COMMON_RPC_METHOD(Gate::Ping);
		BIND_COMMON_RPC_METHOD(Gate::Allocation);
        const ServerConfig * config = ServerConfig::Inst();
		this->mOuterComponent = this->GetComponent<OuterNetComponent>();
        LOG_CHECK_RET_FALSE(config->GetLocation("gate", this->mAddress));
        return true;
	}

    void Gate::OnClose()
    {
		this->GetComponent<OuterNetComponent>()->StopListen();
    }

	int Gate::Allocation(long long userId, s2s::allot::response &response)
    {
        std::string token;
        if(!this->mOuterComponent->MakeToken(userId, token))
        {
            return XCode::Failure;
        }
        response.set_token(token);
        response.set_address(this->mAddress);
        return XCode::Successful;
    }


}