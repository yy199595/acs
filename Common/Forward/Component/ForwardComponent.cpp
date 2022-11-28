//
// Created by zmhy0073 on 2022/10/14.
//

#include"ForwardComponent.h"
#include"Helper/Helper.h"
#include"Service/RpcService.h"
#include"Config/ClusterConfig.h"
#include"Component/LocationComponent.h"
#include"Component/NetThreadComponent.h"
namespace Sentry
{
    bool ForwardComponent::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->StartListen("forward"));
        this->mLocationComponent = this->GetComponent<LocationComponent>();
		this->mLocalHost = this->GetListenAddress();
        return true;
    }

    void ForwardComponent::StartClose(const std::string &address)
    {
        InnerNetClient * netClient = this->GetClient(address);
        if(netClient != nullptr)
        {
            netClient->StartClose();
        }
    }

    bool ForwardComponent::OnListen(std::shared_ptr<SocketProxy> socket)
    {
        std::shared_ptr<InnerNetClient> netClient = std::make_shared<InnerNetClient>(this, socket);
        {
            netClient->StartReceive();
            this->mClients.emplace(socket->GetAddress(), netClient);
        }
        return true;
    }

    InnerNetClient *ForwardComponent::GetClient(const std::string &address)
    {
        auto iter = this->mClients.find(address);
        return iter != this->mClients.end() ? iter->second.get() : nullptr;
    }


    void ForwardComponent::OnCloseSocket(const std::string &address, XCode code)
    {
        auto iter = this->mClients.find(address);
        if(iter != this->mClients.end())
        {
            this->mClients.erase(iter);
        }
        auto iter1 = this->mAuthClients.find(address);
        if(iter1 != this->mAuthClients.end()) //TODO 通知其他验证过的客户端
        {
            this->mAuthClients.erase(iter1);
        }
    }

    void ForwardComponent::OnMessage(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
        if(message->GetType() != (int)Tcp::Type::Auth && !this->IsAuth(address))
        {
            this->StartClose(address);
            CONSOLE_LOG_ERROR(address << " not auth");
            return;
        }
        Rpc::Head &head = message->GetHead();
        switch ((Tcp::Type) message->GetType())
        {
            case Tcp::Type::Auth:
            {
                if(!this->OnAuth(address, message))
                {
                    this->StartClose(address);
                    return;
                }
            }
                break;
            case Tcp::Type::Ping:
            {
                message->SetType(Tcp::Type::Response);
                head.Add("code", XCode::Successful);
                this->Send(address, message);
            }
                break;
            case Tcp::Type::Request:
            {
                head.Add("resp", address);
                XCode code = this->OnRequest(message);
                if (code != XCode::Successful)
                {
                    head.Add("code", code);
                    message->SetType(Tcp::Type::Response);
                    this->Send(address, message);
                }
            }
                break;
            case Tcp::Type::Broadcast:
                for (const std::string & location: this->mAuthClients)
                {
                    this->Send(location, message->Clone());
                }
                break;
            case Tcp::Type::Response:
                this->OnResponse(message);
                break;
            default:
                this->StartClose(address);
                CONSOLE_LOG_FATAL(address << " unknow message type");
                break;
        }
    }

    bool ForwardComponent::OnAuth(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
        if (this->mAuthClients.find(address) != this->mAuthClients.end())
        {
            return true;
        }
        std::unique_ptr<ServiceNodeInfo> serverNode(new ServiceNodeInfo());
        {
            const Rpc::Head &head = message->GetHead();
            head.Get("rpc", serverNode->LocationRpc);
            head.Get("http", serverNode->LocationHttp);
            LOG_CHECK_RET_FALSE(head.Get("name", serverNode->SrvName));
            LOG_CHECK_RET_FALSE(head.Get("user", serverNode->UserName));
            LOG_CHECK_RET_FALSE(head.Get("passwd", serverNode->PassWord));
            if (serverNode->LocationRpc.empty() && serverNode->LocationHttp.empty())
            {
                return false;
            }
            if (ClusterConfig::Inst()->GetConfig(serverNode->SrvName) == nullptr)
            {
                LOG_ERROR("not find cluster config : " << serverNode->SrvName);
                return false;
            }
        }
		this->mLocationComponent->AddRpcServer(serverNode->SrvName, serverNode->LocationRpc);
		this->mLocationComponent->AddHttpServer(serverNode->SrvName, serverNode->LocationHttp);

		s2s::cluster::join registerInfo;
		const std::string fullName("InnerService.Join");
        for (const std::string &location: this->mAuthClients)
        {
			s2s::cluster::join data;
			s2s::cluster::server * server = data.add_list();
			server->set_name(serverNode->SrvName);
			server->set_rpc(serverNode->LocationRpc);
			server->set_http(serverNode->LocationHttp);
            RpcPacket request = PacketHelper::MakeRpcPacket(fullName);
            {
                request->SetType(Tcp::Type::Request);
                request->SetProto(Tcp::Porto::Protobuf);
                request->WriteMessage(&data);
            }
            this->Send(location, request);
			s2s::cluster::server * registerServer = registerInfo.add_list();
			const ServiceNodeInfo * nodeInfo = this->GetServerInfo(location);
			{
				registerServer->set_name(nodeInfo->SrvName);
				registerServer->set_rpc(nodeInfo->LocationRpc);
				registerServer->set_http(nodeInfo->LocationHttp);
			}
        }
		s2s::cluster::server * registerServer = registerInfo.add_list();
		{
			registerServer->set_rpc(this->mLocalHost);
			registerServer->set_name(ServerConfig::Inst()->Name());
		}
		RpcPacket request = PacketHelper::MakeRpcPacket(fullName);
		{
			request->SetType(Tcp::Type::Request);
			request->SetProto(Tcp::Porto::Protobuf);
			request->WriteMessage(&registerInfo);
			this->Send(address, request);
		}

		this->mAuthClients.insert(address);
        this->mLocationMap.emplace(serverNode->LocationRpc, address);
        this->mNodeInfos.emplace(address, std::move(serverNode));
        return true;
    }

    const ServiceNodeInfo *ForwardComponent::GetServerInfo(const std::string &address) const
    {
        auto iter = this->mNodeInfos.find(address);
        return iter != this->mNodeInfos.end() ? iter->second.get() : nullptr;
    }

    XCode ForwardComponent::OnRequest(std::shared_ptr<Rpc::Packet> message)
    {
        std::string target;
        long long userId = 0;
        Rpc::Head &head = message->GetHead();
        if (head.Get("id", userId)) //根据玩家id中转
        {
            return this->Forward(userId, message);
        }
        else if (head.Get("to", target)) //根据地址中转
        {
            head.Remove("to");
            return this->Forward(target, message);
        }
        else
        {
            std::string fullName;
            LOG_RPC_CHECK_ARGS(head.Get("func", fullName));
            const MethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
            if(methodConfig == nullptr)
            {
                return XCode::CallArgsError;
            }
            RpcService * rpcService = this->mApp->GetService(methodConfig->Service);
            if(rpcService == nullptr || (!rpcService->IsStartService()))
            {
                return XCode::CallServiceNotFound;
            }
            XCode code = rpcService->Invoke(methodConfig->Method, message);
            {
                head.Add("code", code);
                message->SetType(Tcp::Type::Response);
                this->OnResponse(message);
            }
        }
        return XCode::Successful;
    }

    XCode ForwardComponent::Forward(long long userId, std::shared_ptr<Rpc::Packet> message)
    {
        std::string service, method, address, server;
        LOG_RPC_CHECK_ARGS(message->GetMethod(service, method));
#ifdef __DEBUG__
        CONSOLE_LOG_DEBUG("forward message user id = "
            << userId << " func = " << service << "." << method);
#endif
        if (!ClusterConfig::Inst()->GetServerName(service, server))
        {
            return XCode::CallServiceNotFound;
        }
        LocationUnit *locationUnit = this->mLocationComponent->GetUnit(userId);
        if(locationUnit == nullptr)
        {
            return XCode::NotFindUser;
        }
        if (!locationUnit->Get(server, address))
        {
            return XCode::Failure;
        }
        return this->Forward(address, message);
    }

    XCode ForwardComponent::Forward(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
#ifdef __DEBUG__
        std::string func;
        message->GetHead().Get("func", func);
        CONSOLE_LOG_DEBUG("forward message address = " << address << " func = " << func);
#endif
        auto iter = this->mLocationMap.find(address);
        if(iter == this->mLocationMap.end())
        {
            return XCode::NetWorkError;
        }
        InnerNetClient * netClient = this->GetClient(iter->second);
        if(netClient == nullptr)
        {
            return XCode::NetWorkError;
        }
		netClient->Send(message);
        return XCode::Successful;
    }

    bool ForwardComponent::Send(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
        InnerNetClient * innerNetClient = this->GetClient(address);
        if(innerNetClient == nullptr)
        {
            auto iter = this->mLocationMap.find(address);
            if(iter == this->mLocationMap.end())
            {
                return false;
            }
            innerNetClient = this->GetClient(iter->second);
            if(innerNetClient == nullptr)
            {
                return false;
            }
        }
		innerNetClient->Send(message);
        return true;
    }

    bool ForwardComponent::Send(std::shared_ptr<Rpc::Packet> message)
    {
        if(this->mAuthClients.empty())
        {
            return false;
        }
        for(const std::string & address : this->mAuthClients)
        {
            if(this->mAuthClients.size() == 1)
            {
                this->Send(address,  message);
                return true;
            }
            this->Send(address,  message->Clone());
        }
        return false;
    }

    bool ForwardComponent::IsAuth(const std::string &address) const
    {
        auto iter = this->mAuthClients.find(address);
        return iter != this->mAuthClients.end();
    }

	XCode ForwardComponent::OnResponse(std::shared_ptr<Rpc::Packet> message)
	{
		std::string address;
		if(!message->GetHead().Get("resp", address))
		{
            LOG_ERROR("not find head field : resp");
			return XCode::Failure;
		}
		message->SetType(Tcp::Type::Response);
        message->GetHead().Remove("resp");
		InnerNetClient * netClient = this->GetClient(address);
		if(netClient == nullptr)
		{
			return XCode::NetWorkError;
		}
		netClient->Send(message);
		return XCode::Successful;
	}
}