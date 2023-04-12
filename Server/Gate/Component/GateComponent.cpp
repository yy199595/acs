//
// Created by yjz on 2022/4/23.
//

#include"GateComponent.h"
#include"Entity/Unit/App.h"
#include"Gate/Service/Gate.h"
#include"Gate/Lua/LuaGate.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Lua/Engine/ClassProxyHelper.h"
#include"Rpc/Component/NodeMgrComponent.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Core/Component/ComponentFactory.h"
namespace Tendo
{
	bool GateComponent::LateAwake()
	{
		this->mNodeComponent = this->GetComponent<NodeMgrComponent>();
        this->mInnerComponent = this->GetComponent<InnerNetComponent>();
		LOG_CHECK_RET_FALSE(this->mGate = this->mApp->GetService<Gate>());
		return true;
	}


	int GateComponent::Send(long long userId, const std::string& func)
	{
        std::string address;
		const std::string & name = this->mGate->GetServer();
        if (!this->mNodeComponent->GetServer(name, userId, address))
        {
            return XCode::NotFindUser;
        }		

        std::shared_ptr<Rpc::Packet> data =
            std::make_shared<Rpc::Packet>();
		{
            data->SetType(Msg::Type::Forward);
			data->GetHead().Add("id", userId);
			data->GetHead().Add("func", func);
		}
        if(!this->mInnerComponent->Send(address, data))
        {
            return XCode::SendMessageFail;
        }
        return XCode::Successful;
	}

	int GateComponent::Send(long long userId, const std::string& func, const Message& message)
	{
        std::string address;
		const std::string & name = this->mGate->GetServer();
        if (!this->mNodeComponent->GetServer(name, userId, address))
		{
			return XCode::NotFindUser;
		}
        std::shared_ptr<Rpc::Packet> data(new Rpc::Packet());
        {
            data->SetType(Msg::Type::Forward);
            data->SetProto(Msg::Porto::Protobuf);
        }
        data->GetHead().Add("func", func);
        data->GetHead().Add("id", userId);
        data->GetHead().Add("pb", message.GetTypeName());
        if(!data->WriteMessage(&message))
        {
            return XCode::SerializationFailure;
        }
        this->mInnerComponent->Send(address, data);
        return XCode::Successful;
	}

	int GateComponent::BroadCast(const std::string& func)
	{
        std::vector<std::string> locations;
		const std::string name = ComponentFactory::GetName<Gate>();
		if(!this->mNodeComponent->GetServers(name, locations))
		{
			return XCode::Failure;
		}

        std::shared_ptr<Rpc::Packet> data(new Rpc::Packet());
        {
            data->SetType(Msg::Type::Broadcast);
            data->SetProto(Msg::Porto::Protobuf);
        }
        data->GetHead().Add("func", func);
        for(const std::string & address : locations)
        {
            this->mInnerComponent->Send(address, data);
        }
		return XCode::Successful;
	}

	int GateComponent::BroadCast(const std::string& func, const Message& message)
	{
		std::vector<std::string> locations;
		const std::string & name = this->mGate->GetServer();
		if(!this->mNodeComponent->GetServers(name, locations))
		{
			return XCode::Failure;
		}

        std::shared_ptr<Rpc::Packet> data(new Rpc::Packet());
        {
            data->SetType(Msg::Type::Broadcast);
            data->SetProto(Msg::Porto::Protobuf);
        }
        data->GetHead().Add("func", func);
        data->GetHead().Add("pb", message.GetTypeName());
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

	void GateComponent::OnLuaRegister(Lua::ClassProxyHelper & luaRegister)
	{
		luaRegister.BeginNewTable("Gate");
		luaRegister.PushExtensionFunction("Send", Lua::Gate::Send);
		luaRegister.PushExtensionFunction("BroadCast", Lua::Gate::BroadCast);
	}
}