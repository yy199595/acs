//
// Created by yjz on 2022/4/23.
//

#include"GateHelperComponent.h"
#include"Service/Gate.h"
#include"Lua/LuaParameter.h"
#include"Config/ClusterConfig.h"
#include"Component/LocationComponent.h"
#include"Component/InnerNetComponent.h"
#include"Component/ComponentFactory.h"
namespace Sentry
{
	bool GateHelperComponent::LateAwake()
	{
		LOG_CHECK_FATAL(this->GetComponent<Gate>());
        this->mInnerComponent = this->GetComponent<InnerNetComponent>();
		this->mLocationComponent = this->GetComponent<LocationComponent>();
		const std::string name = ComponentFactory::GetName<Gate>();
		return ClusterConfig::Inst()->GetServerName(name, this->mGateServerName);
	}

	bool GateHelperComponent::GetLocation(long long userId, std::string& address)
	{		
		LocationUnit * locationUnit = this->mLocationComponent->GetUnit(userId);
		if(locationUnit == nullptr)
		{
			return false;
		}
		return locationUnit->Get(this->mGateServerName, address);
	}


	int GateHelperComponent::Call(long long userId, const std::string& func)
	{
        std::string address;
		if(!this->GetLocation(userId, address))
		{
			return XCode::NotFindUser;
		}

        std::shared_ptr<Rpc::Packet> data
            = Rpc::Packet::New(Tcp::Type::Forward, Tcp::Porto::Protobuf);
        data->GetHead().Add("id", userId);
        data->GetHead().Add("func", func);
        if(!this->mInnerComponent->Send(address, data))
        {
            return XCode::SendMessageFail;
        }
        return XCode::Successful;
	}

	int GateHelperComponent::Call(long long userId, const std::string& func, const Message& message)
	{
        std::string address;
        if(!this->GetLocation(userId, address))
		{
			return XCode::NotFindUser;
		}
        std::shared_ptr<Rpc::Packet> data
            = Rpc::Packet::New(Tcp::Type::Forward, Tcp::Porto::Protobuf);

        data->GetHead().Add("func", func);
        data->GetHead().Add("id", userId);
        if(!data->WriteMessage(&message))
        {
            return XCode::SerializationFailure;
        }
        this->mInnerComponent->Send(address, data);
        return XCode::Successful;
	}

	int GateHelperComponent::LuaCall(long long userId, const std::string func, std::shared_ptr<Message> message)
	{
        std::string address;
		if(!this->GetLocation(userId, address))
		{
			return XCode::NotFindUser;
		}

        std::shared_ptr<Rpc::Packet> data
            = Rpc::Packet::New(Tcp::Type::Forward, Tcp::Porto::Protobuf);

        data->GetHead().Add("func", func);
        data->GetHead().Add("id", userId);
        if(!data->WriteMessage(message.get()))
        {
            return XCode::SerializationFailure;
        }
        this->mInnerComponent->Send(address, data);
        return XCode::Successful;
	}

	int GateHelperComponent::BroadCast(const std::string& func)
	{
        std::vector<std::string> locations;
		const std::string name = ComponentFactory::GetName<Gate>();
		if(!this->mLocationComponent->GetServers(name, locations))
		{
			return XCode::Failure;
		}

        std::shared_ptr<Rpc::Packet> data =
            Rpc::Packet::New(Tcp::Type::Broadcast, Tcp::Porto::Protobuf);
        data->GetHead().Add("func", func);
        for(const std::string & address : locations)
        {
            this->mInnerComponent->Send(address, data);
        }
		return XCode::Successful;
	}

	int GateHelperComponent::BroadCast(const std::string& func, const Message& message)
	{
		std::vector<std::string> locations;		
		if(!this->mLocationComponent->GetServers(this->mGateServerName, locations))
		{
			return XCode::Failure;
		}

        std::shared_ptr<Rpc::Packet> data =
            Rpc::Packet::New(Tcp::Type::Broadcast, Tcp::Porto::Protobuf);
        data->GetHead().Add("func", func);
        if(!data->WriteMessage(&message))
        {
            return XCode::SerializationFailure;
        }
        for(const std::string & address : locations)
        {
            this->mInnerComponent->Send(address, data);
        }
        return XCode::Successful;
	}

	int GateHelperComponent::LuaBroadCast(const char * func, std::shared_ptr<Message> message)
	{
		std::vector<std::string> locations;		
		if(!this->mLocationComponent->GetServers(this->mGateServerName, locations))
		{
			return XCode::Failure;
		}

        std::shared_ptr<Rpc::Packet> data =
            Rpc::Packet::New(Tcp::Type::Broadcast, Tcp::Porto::Protobuf);
        data->GetHead().Add("func", func);
        if(!data->WriteMessage(message.get()))
        {
            return XCode::SerializationFailure;
        }
        for(const std::string & address : locations)
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