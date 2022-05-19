//
// Created by zmhy0073 on 2021/12/1.
//

#include"GateService.h"
#include"App/App.h"
#include"Util/MD5.h"
#include"NetWork/GateClientContext.h"
#include"Component/Gate/GateComponent.h"
#include"Component/Common/DataMgrComponent.h"
#include"Component/Gate/GateClientComponent.h"
#include"Network/Listener/NetworkListener.h"
#include"Component/Gate/GateProxyComponent.h"
#include"Component/User/UserSyncComponent.h"
#include"Component/Redis/MainRedisComponent.h"
namespace Sentry
{
	bool GateService::OnInitEvent(ServiceEventRegister& methodRegister)
	{
		return this->SubUserEvent();
	}

	bool GateService::OnStartService(ServiceMethodRegister& methodRegister)
	{
		methodRegister.BindAddress("Ping", &GateService::Ping);
		methodRegister.BindAddress("Auth", &GateService::Auth);
		methodRegister.Bind("BroadCast", &GateService::BroadCast);
		methodRegister.Bind("CallClient", &GateService::CallClient);
		methodRegister.Bind("QueryAddress", &GateService::QueryAddress);
		LOG_CHECK_RET_FALSE(this->mGateComponent = this->GetComponent<GateComponent>());
		LOG_CHECK_RET_FALSE(this->mRedisComponent = this->GetComponent<MainRedisComponent>());
		LOG_CHECK_RET_FALSE(this->mGateClientComponent = this->GetComponent<GateClientComponent>());
		return true;
	}

	bool GateService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(ServiceComponent::LateAwake());
		this->mSyncComponent = this->GetComponent<UserSyncComponent>();
		LOG_CHECK_RET_FALSE(this->mTimerComponent = this->GetComponent<TimerComponent>());
		return this->GetConfig().GetListenerAddress("rpc", this->mAddress);
	}

	XCode GateService::Ping(const std::string & address)
	{
		LOG_ERROR(address << " ping gate server");
		return XCode::Failure;
	}

	XCode GateService::CallClient(long long userId, c2s::Rpc::Call& request)
	{
		std::string address;
		if(!this->mGateClientComponent->GetUserAddress(userId, address))
		{
			return XCode::NotFindUser;
		}
		std::shared_ptr<c2s::Rpc::Call> message(new c2s::Rpc::Call());

		message->set_func(request.func());
		message->mutable_data()->PackFrom(request.data());
		if(!this->mGateClientComponent->SendToClient(address, message))
		{
			return XCode::NetWorkError;
		}
		return XCode::Successful;
	}

	XCode GateService::QueryAddress(com::Type::String& response)
	{
		std::string address;
		if (this->GetConfig().GetListenerAddress("gate", address))
		{
			response.set_str(address);
			return XCode::Successful;
		}
		return XCode::Failure;
	}

	XCode GateService::BroadCast(const s2s::GateBroadCast::Request& request)
	{
		std::shared_ptr<c2s::Rpc::Call> message(new c2s::Rpc::Call());
		message->set_func(request.func());
		message->mutable_data()->PackFrom(request.data());
		this->mGateClientComponent->SendToAllClient(message);
		return XCode::Successful;
	}

	XCode GateService::Auth(const std::string & address, const c2s::GateAuth::Request & request)
	{
		Json::Writer jsonWriter1;
		LOGIC_THROW_ERROR(!request.token().empty());
		jsonWriter1.AddMember("token", request.token());
		long long userId = this->mSyncComponent->GetUserId(request.token());
		if (userId != 0)
		{
			this->mSyncComponent->SetUserState(userId, 1);
			this->mGateClientComponent->AddNewUser(address, userId);
			this->mSyncComponent->SeAddress(userId, this->GetName(), this->mAddress);
			return XCode::Successful;
		}
		return XCode::Failure;
	}
}