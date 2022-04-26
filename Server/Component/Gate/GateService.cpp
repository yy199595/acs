//
// Created by zmhy0073 on 2021/12/1.
//

#include"GateService.h"
#include"App/App.h"
#include"Util/MD5.h"
#include"NetWork/RpcGateClient.h"
#include"Component/Gate/GateComponent.h"
#include"Component/Common/DataMgrComponent.h"
#include"Component/Service/UserSubService.h"
#include"Component/Gate/GateClientComponent.h"
#include"Network/Listener/NetworkListener.h"
#include"Component/Gate/GateProxyComponent.h"
#include"Component/Redis/MainRedisComponent.h"
namespace Sentry
{
	bool GateService::OnInitService(ServiceMethodRegister& methodRegister)
	{
		methodRegister.Bind("Ping", &GateService::Ping);
		methodRegister.Bind("Allot", &GateService::Allot);
		methodRegister.Bind("BroadCast", &GateService::BroadCast);
		methodRegister.Bind("CallClient", &GateService::CallClient);
		LOG_CHECK_RET_FALSE(this->mUserService = this->GetComponent<UserSubService>());
		LOG_CHECK_RET_FALSE(this->mGateComponent = this->GetComponent<GateComponent>());
		LOG_CHECK_RET_FALSE(this->mGateClientComponent = this->GetComponent<GateClientComponent>());
		LOG_CHECK_RET_FALSE(this->GetApp()->GetConfig().GetListenerAddress("gate", this->mGateAddress));
		return true;
	}

	bool GateService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(LocalServerRpc::LateAwake());
		LOG_CHECK_RET_FALSE(this->mTimerComponent = this->GetComponent<TimerComponent>());
		return true;
	}

	void GateService::GetAllAddress(std::vector<std::string>& gateAddress)
	{
		for(const std::string & address : this->mRemoteAddressList)
		{
			gateAddress.emplace_back(address);
		}
	}

	XCode GateService::Ping(long long userId)
	{
		MainRedisComponent * redisComponent = this->GetComponent<MainRedisComponent>();
		if(redisComponent->Lock("Ping"))
		{
			LOG_ERROR(userId << " get redis lock successful");
			this->GetApp()->GetTaskComponent()->Sleep(5000);
			GateProxyComponent * gateProxyComponent = this->GetComponent<GateProxyComponent>();
			gateProxyComponent->Call(userId, "TaskComponent.Update");
			DataMgrComponent * dataMgrComponent = this->GetComponent<DataMgrComponent>();
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

	XCode GateService::Allot(const s2s::AddressAllot::Request& request, s2s::AddressAllot::Response & response)
	{
		long long userId = request.user_id();
		const std::string& token = request.login_token();
		if(this->mGateComponent->AddUserToken(token, userId))
		{
			response.set_address(this->mGateAddress);
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
}