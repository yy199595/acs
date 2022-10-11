//
// Created by yjz on 2022/4/23.
//

#include"GateAgentComponent.h"
#include"Service/OuterService.h"
#include"Lua/LuaParameter.h"
#include"Component/InnerNetComponent.h"
namespace Sentry
{
	bool GateAgentComponent::LateAwake()
	{
		this->mGateService = this->GetComponent<OuterService>();
        this->mInnerComponent = this->GetComponent<InnerNetComponent>();
		return true;
	}


	XCode GateAgentComponent::Call(long long userId, const std::string& func)
	{
        std::string address;
        if(!this->GetUserAddress(userId, address))
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

    bool GateAgentComponent::RemoveUserAddress(long long userId)
    {
        auto iter = this->mUserHosts.find(userId);
        if(iter == this->mUserHosts.end())
        {
            return false;
        }
        this->mUserHosts.erase(iter);
        return true;
    }

    bool GateAgentComponent::AddUserAddress(long long userId, std::string &address)
    {
        auto iter = this->mUserHosts.find(userId);
        if(iter != this->mUserHosts.end())
        {
            return false;
        }
        this->mUserHosts.emplace(userId, address);
        return true;
    }


    bool GateAgentComponent::GetUserAddress(long long userId, std::string &address)
    {
        auto iter = this->mUserHosts.find(userId);
        if(iter == this->mUserHosts.end())
        {
            return false;
        }
        address = iter->second;
        return true;
    }

	XCode GateAgentComponent::Call(long long userId, const std::string& func, const Message& message)
	{
        std::string address;
        if(!this->GetUserAddress(userId, address))
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

	XCode GateAgentComponent::LuaCall(long long userId, const std::string func, std::shared_ptr<Message> message)
	{
        std::string address;
        if(!this->GetUserAddress(userId, address))
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

	XCode GateAgentComponent::BroadCast(const std::string& func)
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

	XCode GateAgentComponent::BroadCast(const std::string& func, const Message& message)
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

	XCode GateAgentComponent::LuaBroadCast(const char * func, std::shared_ptr<Message> message)
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

	void GateAgentComponent::OnLuaRegister(Lua::ClassProxyHelper & luaRegister)
	{
		luaRegister.BeginRegister<GateAgentComponent>();
		luaRegister.PushMemberFunction("Call", &GateAgentComponent::LuaCall);
		luaRegister.PushMemberFunction("BroadCast", &GateAgentComponent::LuaBroadCast);
	}
}