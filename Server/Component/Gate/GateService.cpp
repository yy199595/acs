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
#include"Component/Gate/GateAgentComponent.h"
#include"Component/User/UserSyncComponent.h"
#include"Component/Redis/MainRedisComponent.h"
namespace Sentry
{

	bool GateService::OnStartService(ServiceMethodRegister& methodRegister)
	{
        LOG_CHECK_RET_FALSE(this->GetComponent<GateComponent>());
        methodRegister.BindAddress("Ping", &GateService::Ping);
		methodRegister.BindAddress("Auth", &GateService::Auth);
        methodRegister.Bind("AllotUser", &GateService::AllotUser);
        methodRegister.Bind("BroadCast", &GateService::BroadCast);
		methodRegister.Bind("CallClient", &GateService::CallClient);
		methodRegister.Bind("SaveAddress", &GateService::SaveAddress);
		methodRegister.Bind("QueryAddress", &GateService::QueryAddress);
		LOG_CHECK_RET_FALSE(this->mGateClientComponent = this->GetComponent<GateClientComponent>());
		return true;
	}

	bool GateService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(LocalRpcService::LateAwake());
		this->mSyncComponent = this->GetComponent<UserSyncComponent>();
		LOG_CHECK_RET_FALSE(this->mTimerComponent = this->GetComponent<TimerComponent>());
		return this->GetConfig().GetListener("rpc", this->mAddress);
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

	XCode GateService::AllotUser(const com::Type::Int64 &request, s2s::Allot::Response &response)
    {
        std::string address;
        long long userId = request.value();
        if (this->GetConfig().GetListener("gate", address))
        {
            std::string str = std::to_string(userId);
            str.append(std::to_string(Helper::Time::GetNowSecTime()));
            const std::string token = Helper::Md5::GetMd5(str);

            response.set_token(token);
            response.set_address(address);
            this->mUserTokens.emplace(response.token(), userId);
            this->mTimerComponent->DelayCall(15, [this, token]() {
                auto iter = this->mUserTokens.find(token);
                if (iter != this->mUserTokens.end())
                {
                    this->mUserTokens.erase(iter);
                }
            });
            return XCode::Successful;
        }
        return XCode::Failure;
    }

	XCode GateService::QueryAddress(long long userId, const com::Type::String& request, com::Type::String& response)
	{
		std::string address;
		ServiceComponent * component = this->GetComponent<ServiceComponent>(request.str());
		if(component == nullptr || !component->GetAddressProxy().GetAddress(userId, address))
		{
			return XCode::Failure;
		}
		response.set_str(address);
		return XCode::Successful;
	}

	XCode GateService::SaveAddress(long long userId, const s2s::Allot::Save& request)
	{
		ServiceComponent * component = this->GetComponent<ServiceComponent>(request.service());
		if(component == nullptr || !component->GetAddressProxy().HasAddress(request.address()))
		{
			return XCode::Failure;
		}
		component->GetAddressProxy().AddUserAddress(userId, request.address());
		return XCode::Successful;
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
        auto iter = this->mUserTokens.find(request.token());
        if (iter == this->mUserTokens.end())
        {
            return XCode::Failure;
        }
        long long userId = iter->second;
        this->mUserTokens.erase(iter);
        this->mSyncComponent->SetUserState(userId, 1);
        this->mGateClientComponent->AddNewUser(address, userId);
        this->mSyncComponent->SetAddress(userId, this->GetName(), this->mAddress, true);
        return XCode::Successful;
    }
}