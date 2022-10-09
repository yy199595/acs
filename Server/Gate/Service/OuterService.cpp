//
// Created by zmhy0073 on 2021/12/1.
//

#include"OuterService.h"
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

    void OuterService::Awake()
    {
        this->mTimerComponent = nullptr;
        this->mOuterNetComponent = nullptr;
        this->GetApp()->AddComponent<OuterNetComponent>();
        this->GetApp()->AddComponent<OuterNetMessageComponent>();
    }

	bool OuterService::OnStart()
	{
        BIND_ADDRESS_RPC_METHOD(OuterService::Ping);
        BIND_COMMON_RPC_METHOD(OuterService::AllotUser);
        BIND_COMMON_RPC_METHOD(OuterService::BroadCast);
        BIND_COMMON_RPC_METHOD(OuterService::CallClient);
        BIND_COMMON_RPC_METHOD(OuterService::SaveAddress);
        BIND_COMMON_RPC_METHOD(OuterService::QueryAddress);
        this->mOuterNetComponent = this->GetComponent<OuterNetComponent>();
		return this->mOuterNetComponent->StartListen("gate");
	}

    bool OuterService::OnClose()
    {
        this->mOuterNetComponent->StopListen();
    }

	XCode OuterService::Ping(const std::string & address)
	{
		LOG_ERROR(address << " ping gate server");
		return XCode::Failure;
	}

	XCode OuterService::CallClient(long long userId, c2s::rpc::call& request)
	{
		std::string address;
		if(!this->mOuterNetComponent->GetUserAddress(userId, address))
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

		if(!this->mOuterNetComponent->SendData(address, message))
		{
			return XCode::NetWorkError;
		}
		return XCode::Successful;
	}

	XCode OuterService::AllotUser(const com::type::int64 &request, s2s::allot::response &response)
    {
        std::string address;
        long long userId = request.value();
        if (this->GetConfig().GetListener("gate", address))
        {
            std::string token = this->mOuterNetComponent->CreateToken(userId);
            response.set_token(token);
            response.set_address(address);
            return XCode::Successful;
        }
        return XCode::Failure;
    }

	XCode OuterService::QueryAddress(long long userId, const com::type::string& request, com::type::string& response)
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

	XCode OuterService::SaveAddress(long long userId, const s2s::allot::save& request)
	{
		Service * component = this->GetComponent<Service>(request.service());
		if(component == nullptr || !component->HasHost(request.address()))
		{
			return XCode::Failure;
		}
		component->AddHost(request.address(), userId);
		return XCode::Successful;
	}

	XCode OuterService::BroadCast(const s2s::broadcast::request& request)
	{
		std::shared_ptr<c2s::rpc::call> message(new c2s::rpc::call());
		message->set_func(request.func());
		message->mutable_data()->PackFrom(request.data());
		this->mOuterNetComponent->SendToAllClient(message);
		return XCode::Successful;
	}
}