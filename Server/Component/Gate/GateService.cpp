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

namespace Sentry
{
	bool GateService::OnInitService(ServiceMethodRegister& methodRegister)
	{
		methodRegister.Bind("Ping", &GateService::Ping);
		methodRegister.Bind("Allot", &GateService::Allot);
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

	XCode GateService::Ping(long long userId)
	{
		DataMgrComponent * dataMgrComponent = this->GetComponent<DataMgrComponent>();
		std::shared_ptr<db_account::tab_user_account> userAccount(new db_account::tab_user_account());
		return dataMgrComponent->Get(1996, userAccount);
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
}

namespace Sentry
{
	void GateService::BroadCast(const std::string& func)
	{
		std::shared_ptr<s2s::GateBroadCast::Request> request
			= std::make_shared<s2s::GateBroadCast::Request>();
		request->set_func(func);
		for(const std::string & address : this->mRemoteAddressList)
		{
			this->Call(address, "BroadCast", *request);
		}
	}

	void GateService::BroadCast(const std::string& func, const Message& message)
	{
		std::shared_ptr<s2s::GateBroadCast::Request> request
			= std::make_shared<s2s::GateBroadCast::Request>();
		request->set_func(func);
		request->mutable_data()->PackFrom(message);
		for(const std::string & address : this->mRemoteAddressList)
		{
			this->Call(address, "BroadCast", *request);
		}
	}
}