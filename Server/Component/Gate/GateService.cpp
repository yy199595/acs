//
// Created by zmhy0073 on 2021/12/1.
//

#include"GateService.h"
#include"App/App.h"
#include"Util/MD5.h"
#include"Util/Guid.h"
#include"NetWork/RpcGateClient.h"
#include"Component/Service/UserSubService.h"
#include"Component/Scene/EntityMgrComponent.h"
#include"Component/Gate/GateComponent.h"
#include"Component/Gate/GateClientComponent.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Listener/TcpServerComponent.h"
namespace Sentry
{
	bool GateService::OnInitService(ServiceMethodRegister& methodRegister)
	{
		methodRegister.Bind("Ping", &GateService::Ping);
		methodRegister.Bind("Allot", &GateService::Allot);
		LOG_CHECK_RET_FALSE(this->mGateComponent = this->GetComponent<GateComponent>());
		LOG_CHECK_RET_FALSE(this->mGateClientComponent = this->GetComponent<GateClientComponent>());
		LOG_CHECK_RET_FALSE(this->GetApp()->GetConfig().GetListenerAddress("gate", this->mGateAddress));
		return true;
	}

	bool GateService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(LocalServerRpc::LateAwake());
		LOG_CHECK_RET_FALSE(this->GetComponent<GateClientComponent>());
		LOG_CHECK_RET_FALSE(this->mUserService = this->GetComponent<UserSubService>());
		LOG_CHECK_RET_FALSE(this->mTimerComponent = this->GetComponent<TimerComponent>());
		return true;
	}

	XCode GateService::Ping(long long userId)
	{
		return XCode::Failure;
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