//
// Created by zmhy0073 on 2021/12/1.
//

#include"GateService.h"
#include"App/App.h"
#include"Util/MD5.h"
#include"NetWork/GateRpcClientContext.h"
#include"Component/Gate/GateComponent.h"
#include"Component/Common/DataMgrComponent.h"
#include"Component/Service/UserInfoSyncService.h"
#include"Component/Gate/GateClientComponent.h"
#include"Network/Listener/NetworkListener.h"
#include"Component/Gate/GateProxyComponent.h"
#include"Component/Redis/MainRedisComponent.h"
namespace Sentry
{
	bool GateService::OnInitEvent(ServiceEventRegister& methodRegister)
	{
		return this->SubUserEvent();
	}

	bool GateService::OnInitService(ServiceMethodRegister& methodRegister)
	{
		methodRegister.Bind("Ping", &GateService::Ping);
		methodRegister.Bind("Auth", &GateService::Auth);
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
		LOG_CHECK_RET_FALSE(LocalServiceComponent::LateAwake());
		this->GetConfig().GetListenerAddress("rpc", this->mAddress);
		LOG_CHECK_RET_FALSE(this->mTimerComponent = this->GetComponent<TimerComponent>());
		return true;
	}

	XCode GateService::Ping(long long userId)
	{
		MainRedisComponent* redisComponent = this->GetComponent<MainRedisComponent>();
		if(redisComponent->Lock("Ping"))
		{
			LOG_ERROR(userId << " get redis lock successful");
			GateProxyComponent* gateProxyComponent = this->GetComponent<GateProxyComponent>();
			gateProxyComponent->Call(userId, "TaskComponent.Update");
			DataMgrComponent* dataMgrComponent = this->GetComponent<DataMgrComponent>();
			std::shared_ptr<db_account::tab_user_account> userAccount(new db_account::tab_user_account());
			redisComponent->UnLock("Ping");
			return dataMgrComponent->Get(userId, userAccount);
		}
		LOG_WARN(userId << " get redis lock failure");
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

	XCode GateService::Auth(const c2s::GateAuth::Request & request)
	{
		Json::Writer jsonWriter1;
		std::string userIdStr = "";
		jsonWriter1.AddMember("token", request.token());
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		if(!this->mRedisComponent->CallLua("user.get_token", jsonWriter1,
			response) || !response->GetMember("user_id", userIdStr))
		{
			return XCode::Failure;
		}
		Json::Writer jsonWriter2;
		long long userId = std::stoll(userIdStr);
		jsonWriter2.AddMember("user_id", userId);
		jsonWriter2.AddMember("address", this->mAddress);
		jsonWriter2.AddMember("service", this->GetName());
		this->mRedisComponent->CallLua("user.set_address", jsonWriter2);

		Json::Writer jsonWriter3;
		jsonWriter3.AddMember("state", 1);
		jsonWriter3.AddMember("user_id", userId);
		this->mRedisComponent->CallLua("user.set_state", jsonWriter3);
		return this->mGateClientComponent->AddNewUser(request.address(), userId)
			   ? XCode::Successful : XCode::Failure;
	}
}