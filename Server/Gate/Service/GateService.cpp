//
// Created by zmhy0073 on 2021/12/1.
//

#include"GateService.h"
#include"App/App.h"
#include"Md5/MD5.h"
#include"Client/OuterNetClient.h"
#include"Component/RedisDataComponent.h"
#include"Component/OuterNetMessageComponent.h"
#include"Component/OuterNetComponent.h"
#include"Component/GateAgentComponent.h"
#include"Component/UserSyncComponent.h"
namespace Sentry
{

	bool GateService::OnStartService()
	{
        BIND_ADDRESS_RPC_METHOD(GateService::Ping);
        BIND_COMMON_RPC_METHOD(GateService::AllotUser);
        BIND_COMMON_RPC_METHOD(GateService::BroadCast);
        BIND_COMMON_RPC_METHOD(GateService::CallClient);
        BIND_COMMON_RPC_METHOD(GateService::SaveAddress);
        BIND_COMMON_RPC_METHOD(GateService::QueryAddress);
        LOG_CHECK_RET_FALSE(this->GetComponent<OuterNetMessageComponent>());
        LOG_CHECK_RET_FALSE(this->mGateClientComponent = this->GetComponent<OuterNetComponent>());
		return true;
	}

	bool GateService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(LocalService::LateAwake());
		this->mSyncComponent = this->GetComponent<UserSyncComponent>();
		LOG_CHECK_RET_FALSE(this->mTimerComponent = this->GetComponent<TimerComponent>());
		return this->GetConfig().GetListener("rpc", this->mAddress);
	}

	XCode GateService::Ping(const std::string & address)
	{
		LOG_ERROR(address << " ping gate server");
		return XCode::Failure;
	}

	XCode GateService::CallClient(long long userId, c2s::rpc::call& request)
	{
		std::string address;
		if(!this->mGateClientComponent->GetUserAddress(userId, address))
		{
			return XCode::NotFindUser;
		}
        std::shared_ptr<Rpc::Data> message(new Rpc::Data());

        message->SetType(Tcp::Type::Request);
        message->SetProto(Tcp::Porto::Protobuf);
        message->GetHead().Add("func", request.func());
        if(request.has_data())
        {
            message->WriteMessage(request.mutable_data());
        }

		if(!this->mGateClientComponent->SendData(address, message))
		{
			return XCode::NetWorkError;
		}
		return XCode::Successful;
	}

	XCode GateService::AllotUser(const com::type::int64 &request, s2s::allot::response &response)
    {
        std::string address;
        long long userId = request.value();
        if (this->GetConfig().GetListener("gate", address))
        {
            std::string token = this->mGateClientComponent->CreateToken(userId);
            response.set_token(token);
            response.set_address(address);
            return XCode::Successful;
        }
        return XCode::Failure;
    }

	XCode GateService::QueryAddress(long long userId, const com::type::string& request, com::type::string& response)
	{
		std::string address;
		Service * component = this->GetComponent<Service>(request.str());
		if(component == nullptr || !component->GetHost(userId, address))
		{
			return XCode::Failure;
		}
		response.set_str(address);
		return XCode::Successful;
	}

	XCode GateService::SaveAddress(long long userId, const s2s::allot::save& request)
	{
		Service * component = this->GetComponent<Service>(request.service());
		if(component == nullptr || !component->HasHost(request.address()))
		{
			return XCode::Failure;
		}
		component->AddHost(request.address(), userId);
		return XCode::Successful;
	}

	XCode GateService::BroadCast(const s2s::broadcast::request& request)
	{
		std::shared_ptr<c2s::rpc::call> message(new c2s::rpc::call());
		message->set_func(request.func());
		message->mutable_data()->PackFrom(request.data());
		this->mGateClientComponent->SendToAllClient(message);
		return XCode::Successful;
	}
}