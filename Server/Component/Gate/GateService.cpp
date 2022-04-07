//
// Created by zmhy0073 on 2021/12/1.
//

#include"GateService.h"
#include"App/App.h"
#include"Util/MD5.h"
#include"Util/Guid.h"
#include"NetWork/RpcGateClient.h"
#include"Component/Scene/EntityMgrComponent.h"
#include"Component/Gate/GateClientComponent.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Listener/TcpServerComponent.h"

namespace Sentry
{
	bool GateService::Awake()
	{
		BIND_RPC_FUNCTION(GateService::Ping);
		BIND_RPC_FUNCTION(GateService::Login);
		return true;
	}

	bool GateService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mTimerComponent = this->GetComponent<TimerComponent>());
		TcpServerComponent* tcpServerComponent = this->GetComponent<TcpServerComponent>();
		LOG_CHECK_RET_FALSE(tcpServerComponent->GetTcpConfig("gate"));

		this->mGateComponent = this->GetComponent<GateClientComponent>();
		this->mEntityComponent = this->GetComponent<EntityMgrComponent>();
		if (this->mGateComponent == nullptr)
		{
			this->mGateComponent = this->mEntity->GetOrAddComponent<GateClientComponent>();
			LOG_CHECK_RET_FALSE(this->mGateComponent->LateAwake());
		}

		if (this->mEntityComponent == nullptr)
		{
			this->mEntityComponent = this->mEntity->GetOrAddComponent<EntityMgrComponent>();
			LOG_CHECK_RET_FALSE(this->mEntityComponent->LateAwake());
		}
		return true;
	}

	XCode GateService::Ping()
	{

		return XCode::Failure;
	}

	XCode GateService::Login(const c2s::GateLogin::Request& request)
	{

		return XCode::Successful;
	}

	void GateService::OnTokenTimeout(const std::string& token)
	{
		auto iter = this->mTokenMap.find(token);
		if (iter != this->mTokenMap.end())
		{
			this->mTokenMap.erase(iter);
		}
	}
}