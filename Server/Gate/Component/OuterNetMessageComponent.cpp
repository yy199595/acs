//
// Created by mac on 2021/11/28.
//

#include"OuterNetMessageComponent.h"
#include"Client/OuterNetClient.h"
#include"Config/ServiceConfig.h"
#include"OuterNetComponent.h"
#include"Md5/MD5.h"
#include"Service/User.h"
#include"Component/InnerNetComponent.h"
#include"Service/PhysicalService.h"
#include"Component/ProtoComponent.h"
#include"Component/NodeMgrComponent.h"

#include"Component/InnerNetMessageComponent.h"

#include"Config/ClusterConfig.h"
namespace Sentry
{
    OuterNetMessageComponent::OuterNetMessageComponent()
    {
        this->mNodeComponent = nullptr;
        this->mInnerMessageComponent = nullptr;
    }


	bool OuterNetMessageComponent::LateAwake()
	{
		this->mNodeComponent = this->GetComponent<NodeMgrComponent>();
        this->mInnerMessageComponent = this->GetComponent<InnerNetMessageComponent>();
		return true;
	}

	int OuterNetMessageComponent::OnMessage(long long userId, std::shared_ptr<Rpc::Packet> message)
	{
		switch (message->GetType())
		{
		case Tcp::Type::Ping:
			break;
		case Tcp::Type::Request:
			return this->OnRequest(userId, std::move(message));
		}
		return XCode::UnKnowPacket;
	}

	int OuterNetMessageComponent::OnRequest(long long userId, std::shared_ptr<Rpc::Packet> message)
	{
		const std::string & fullName = message->GetHead().GetStr("func");
		const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
		if (methodConfig == nullptr || methodConfig->Type != "Client")
		{
			return XCode::CallFunctionNotExist;
		}
        if(!methodConfig->Request.empty() &&  message->GetSize() == 0)
        {
            return XCode::CallArgsError;
        }
        std::string target;
		const std::string & server = methodConfig->Server;
        if (!this->mNodeComponent->GetServer(server, userId, target))
        {
            return XCode::NotFindUser;
        }
		message->GetHead().Add("id", userId);
		if(!this->mInnerMessageComponent->Send(target, message))
		{
			return XCode::NetWorkError;
		}
		return XCode::Successful;
	}

	int OuterNetMessageComponent::OnLogin(long long userId)
	{
		static std::string func("Login");
		std::vector<const NodeConfig *> configs;
		ClusterConfig::Inst()->GetNodeConfigs(configs);
		RpcService * rpcService = this->mApp->GetService<User>();
		for(const NodeConfig * nodeConfig : configs)
		{
			if(nodeConfig->IsAuthAllot())
			{
				std::string address;
				s2s::user::login message;
				const std::string & server = nodeConfig->GetName();
				if(!this->mNodeComponent->GetServer(server, address))
				{
					return XCode::AddressAllotFailure;
				}
				message.set_user_id(userId);
				rpcService->Send(address, func, message);
				this->mNodeComponent->AddRpcServer(server, userId, address);
			}
		}
		return XCode::Successful;
	}

	int OuterNetMessageComponent::OnLogout(long long userId)
	{
		static std::string func("Logout");
		std::unordered_map<std::string, std::string> servers;
		if(!this->mNodeComponent->GetServer(userId, servers))
		{
			return XCode::Failure;
		}
		RpcService * rpcService = this->mApp->GetService<User>();
		if(rpcService == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		s2s::user::logout message;
		message.set_user_id(userId);
		for(const auto & info : servers)
		{
			const std::string & address = info.second;
			rpcService->Send(address, func, message);
		}
		return 0;
	}
}