//
// Created by yjz on 2022/4/23.
//

#include"GateHelperComponent.h"
#include"Service/OuterService.h"
#include"Lua/LuaParameter.h"
#include"Component/InnerNetComponent.h"
namespace Sentry
{
	bool GateHelperComponent::LateAwake()
	{
        this->mInnerComponent = this->GetComponent<InnerNetComponent>();
        LOG_CHECK_RET_FALSE(this->mGateService = this->GetComponent<OuterService>());
		return true;
	}


	XCode GateHelperComponent::Call(long long userId, const std::string& func)
	{
        std::string address;
        if(!this->mGateService->GetLocation(userId, address))
        {
            return XCode::NotFindUser;
        }
        std::shared_ptr<Rpc::Data> data
            = Rpc::Data::New(Tcp::Type::Forward, Tcp::Porto::Protobuf);
        data->GetHead().Add("id", userId);
        data->GetHead().Add("func", func);
        if(!this->mInnerComponent->Send(address, data))
        {
            return XCode::SendMessageFail;
        }
        return XCode::Successful;
	}

	XCode GateHelperComponent::Call(long long userId, const std::string& func, const Message& message)
	{
        std::string address;
        if(!this->mGateService->GetLocation(userId, address))
        {
            return XCode::NotFindUser;
        }
        std::shared_ptr<Rpc::Data> data
            = Rpc::Data::New(Tcp::Type::Forward, Tcp::Porto::Protobuf);

        data->GetHead().Add("func", func);
        data->GetHead().Add("id", userId);
        if(!data->WriteMessage(&message))
        {
            return XCode::SerializationFailure;
        }
        this->mInnerComponent->Send(address, data);
        return XCode::Successful;
	}

	XCode GateHelperComponent::LuaCall(long long userId, const std::string func, std::shared_ptr<Message> message)
	{
        std::string address;
        if(!this->mGateService->GetLocation(userId, address))
        {
            return XCode::NotFindUser;
        }
        std::shared_ptr<Rpc::Data> data
            = Rpc::Data::New(Tcp::Type::Forward, Tcp::Porto::Protobuf);

        data->GetHead().Add("func", func);
        data->GetHead().Add("id", userId);
        if(!data->WriteMessage(message.get()))
        {
            return XCode::SerializationFailure;
        }
        this->mInnerComponent->Send(address, data);
        return XCode::Successful;
	}

	XCode GateHelperComponent::BroadCast(const std::string& func)
	{
        std::vector<std::string> hosts;
        if(!this->mGateService->GetHosts(hosts))
        {
            return XCode::Failure;
        }
        std::shared_ptr<Rpc::Data> data =
            Rpc::Data::New(Tcp::Type::Broadcast, Tcp::Porto::Protobuf);
        data->GetHead().Add("func", func);
        for(const std::string & address : hosts)
        {
            this->mInnerComponent->Send(address, data);
        }
		return XCode::Successful;
	}

	XCode GateHelperComponent::BroadCast(const std::string& func, const Message& message)
	{
        std::vector<std::string> hosts;
        if(!this->mGateService->GetHosts(hosts))
        {
            return XCode::Failure;
        }
        std::shared_ptr<Rpc::Data> data =
            Rpc::Data::New(Tcp::Type::Broadcast, Tcp::Porto::Protobuf);
        data->GetHead().Add("func", func);
        if(!data->WriteMessage(&message))
        {
            return XCode::SerializationFailure;
        }
        for(const std::string & address : hosts)
        {
            this->mInnerComponent->Send(address, data);
        }
        return XCode::Successful;
	}

	XCode GateHelperComponent::LuaBroadCast(const char * func, std::shared_ptr<Message> message)
	{
        std::vector<std::string> hosts;
        if(!this->mGateService->GetHosts(hosts))
        {
            return XCode::Failure;
        }
        std::shared_ptr<Rpc::Data> data =
            Rpc::Data::New(Tcp::Type::Broadcast, Tcp::Porto::Protobuf);
        data->GetHead().Add("func", func);
        if(!data->WriteMessage(message.get()))
        {
            return XCode::SerializationFailure;
        }
        for(const std::string & address : hosts)
        {
            this->mInnerComponent->Send(address, data);
        }
        return XCode::Successful;
	}

	void GateHelperComponent::OnLuaRegister(Lua::ClassProxyHelper & luaRegister)
	{
		luaRegister.BeginRegister<GateHelperComponent>();
		luaRegister.PushMemberFunction("Call", &GateHelperComponent::LuaCall);
		luaRegister.PushMemberFunction("BroadCast", &GateHelperComponent::LuaBroadCast);
	}
}