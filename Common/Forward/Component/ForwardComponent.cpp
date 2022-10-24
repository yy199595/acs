//
// Created by zmhy0073 on 2022/10/14.
//

#include"ForwardComponent.h"
#include"Service/RpcService.h"
#include"Component/LocationComponent.h"
#include"Component/NetThreadComponent.h"
#include"Component/ForwardMessageComponent.h"
namespace Sentry
{
    bool ForwardComponent::LateAwake()
    {
        this->mLocationComponent = this->GetComponent<LocationComponent>();
        this->mMessageComponent = this->GetComponent<ForwardMessageComponent>();
        return this->StartListen("forward");
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
                    message->SetType(Tcp::Type::Response);
                    message->GetHead().Add("code", code);
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
        if(this->mAuthClients.find(address) != this->mAuthClients.end())
        {
            return true;
        }
        message->GetHead().Add("address", address);
        const ServiceNodeInfo * nodeInfo = this->mMessageComponent->OnAuth(message);
        if(nodeInfo == nullptr)
        {
            return false;
        }
        this->mAuthClients.insert(address);
        this->mLocationMap.emplace(nodeInfo->LocationRpc, address);
        return true;
    }

    XCode ForwardComponent::OnRequest(std::shared_ptr<Rpc::Packet> message)
    {
        std::string target;
        long long userId = 0;
        Rpc::Head &head = message->GetHead();
        if (head.Get("id", userId))
        {
            return this->Forward(userId, message);
        }
        else if (head.Get("to", target))
        {
            head.Remove("to");
            return this->Forward(target, message);
        }
        else
        {
			XCode code = this->CallService(message);
			{
				if (message->GetHead().Has("rpc"))
				{
#ifdef __DEBUG__
					head.Remove("func");
#endif
					head.Add("code", code);
					return this->OnResponse(message);
				}
			}
        }
        return XCode::Successful;
    }

	XCode ForwardComponent::CallService(std::shared_ptr<Rpc::Packet> message)
	{
		std::string fullName;
		LOG_RPC_CHECK_ARGS(message->GetHead().Get("func", fullName));
		const RpcMethodConfig * rpcMethodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
		if(rpcMethodConfig == nullptr)
		{
			return XCode::CallArgsError;
		}
		RpcService * rpcService = this->mApp->GetService(rpcMethodConfig->Service);
		if(rpcService == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		return rpcService->Invoke(rpcMethodConfig->Method, message);
	}

    XCode ForwardComponent::Forward(long long userId, std::shared_ptr<Rpc::Packet> message)
    {
        std::string service, method, address;
        LOG_RPC_CHECK_ARGS(message->GetMethod(service, method));
        LocationUnit *locationUnit = this->mLocationComponent->GetLocationUnit(userId);
        if(locationUnit == nullptr)
        {
            return XCode::NotFindUser;
        }
        if (!locationUnit->Get(service, address))
        {
            return XCode::Failure;
        }
        return this->Forward(address, message);
    }

    XCode ForwardComponent::Forward(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
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
        netClient->SendData(message);
        return XCode::Successful;
    }

    void ForwardComponent::Send(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
        InnerNetClient * innerNetClient = this->GetClient(address);
        if(innerNetClient == nullptr)
        {
            LOG_ERROR("send message to" << address << " error");
            return;
        }
        innerNetClient->SendData(message);
    }

    void ForwardComponent::Send(const std::string &func, const Message *message)
    {
        for(const std::string & addeess : this->mAuthClients)
        {
            std::shared_ptr<Rpc::Packet> request =
                Rpc::Packet::New(Tcp::Type::Request, Tcp::Porto::Protobuf);
            {
                request->WriteMessage(message);
                request->GetHead().Add("func", func);
            }
            this->Send(addeess, request);
        }
    }

    void ForwardComponent::Send(const std::string &address, const std::string &func, const Message *message)
    {
        InnerNetClient * netClient = this->GetClient(address);
        if(netClient == nullptr)
        {
            return;
        }
        std::shared_ptr<Rpc::Packet> request =
            Rpc::Packet::New(Tcp::Type::Request, Tcp::Porto::Protobuf);
        {
            request->WriteMessage(message);
            request->GetHead().Add("func", func);
        }
        netClient->SendData(request);
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
			return XCode::Failure;
		}
		message->SetType(Tcp::Type::Response);
        message->GetHead().Remove("resp");
		InnerNetClient * netClient = this->GetClient(address);
		if(netClient == nullptr)
		{
			return XCode::NetWorkError;
		}
		netClient->SendData(message);
		return XCode::Successful;
	}
}