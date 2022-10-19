//
// Created by zmhy0073 on 2022/10/14.
//

#include"ForwardComponent.h"
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

    InnerNetClient *ForwardComponent::GetClient(const std::string &address)
    {
        auto iter = this->mInnerClients.find(address);
        return iter != this->mInnerClients.end() ? iter->second.get() : nullptr;
    }


    void ForwardComponent::OnCloseSocket(const std::string &address, XCode code)
    {
        auto iter = this->mInnerClients.find(address);
        if(iter != this->mInnerClients.end())
        {
            this->mInnerClients.erase(iter);
        }
    }

    void ForwardComponent::OnMessage(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
        Rpc::Head &head = message->GetHead();
        switch ((Tcp::Type) message->GetType())
        {
            case Tcp::Type::Auth:
            {
                head.Add("address", address);
                if (this->mMessageComponent->OnAuth(message) != XCode::Successful)
                {
                    this->StartClose(address);
                    LOG_ERROR(address << " auth failure");
                    return;
                }
                this->mAuthClients.insert(address);
            }
                break;
            case Tcp::Type::Ping:
            {
                if (!this->IsAuth(address))
                {
                    this->StartClose(address);
                    return;
                }
                head.Add("resp", address);
                message->SetType(Tcp::Type::Response);
                head.Add("code", XCode::Successful);
                this->OnResponse(message);
            }
                break;
            case Tcp::Type::Request:
            {
                if (head.Has("rpc"))
                {
                    head.Add("resp", address);
                }
                XCode code = this->OnRequest(message);
                if (code != XCode::Successful)
                {
                    InnerNetClient *netClient = this->GetClient(address);
                    if (netClient != nullptr)
                    {
                        message->SetType(Tcp::Type::Response);
                        message->GetHead().Add("code", code);
                        netClient->SendData(message);
                    }
                }
            }
                break;
            case Tcp::Type::Broadcast:
                for (const std::string &address: this->mAuthClients)
                {
                    InnerNetClient *netClient = this->GetClient(address);
                    if (netClient != nullptr)
                    {
                        netClient->SendData(message->Clone());
                    }
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

    XCode ForwardComponent::OnRequest(std::shared_ptr<Rpc::Packet> message)
    {
        std::string target;
        long long userId = 0;
        const Rpc::Head &head = message->GetHead();
        if (message->GetHead().Get("id", userId))
        {
            std::string service, method;
            LOG_RPC_CHECK_ARGS(message->GetMethod(service, method));
            LocationUnit *locationUnit = this->mLocationComponent->GetLocationUnit(userId);
            if (locationUnit == nullptr || !locationUnit->Get(service, target))
            {
                return XCode::NotFindUser;
            }
            InnerNetClient * netClient = this->GetOrCreateClient(target);
            if(netClient == nullptr)
            {
                return XCode::NetWorkError;
            }
            netClient->SendData(message);
            return XCode::Successful;
        }
        else if (head.Get("to", target))
        {
            InnerNetClient * netClient = this->GetOrCreateClient(target);
            if(netClient == nullptr)
            {
                return XCode::NetWorkError;
            }
            netClient->SendData(message);
            return XCode::Successful;
        }
        else
        {
            XCode code = this->mMessageComponent->OnMessage(message);
            {
                if (message->GetHead().Has("rpc"))
                {
                    message->GetHead().Add("code", code);
                    this->OnResponse(message);
                }
            }
        }
        return XCode::Successful;
    }

    void ForwardComponent::Send(const std::string &func, const Message *message)
    {
        for(const std::string & address : this->mAuthClients)
        {
            InnerNetClient * netClient = this->GetClient(address);
            if(netClient != nullptr)
            {
                std::shared_ptr<Rpc::Packet> request =
                    Rpc::Packet::New(Tcp::Type::Request, Tcp::Porto::Protobuf);
                {
                    request->WriteMessage(message);
                    request->GetHead().Add("func", func);
                }
                netClient->SendData(request);
            }
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

    bool ForwardComponent::OnListen(std::shared_ptr<SocketProxy> socket)
    {
        std::shared_ptr<InnerNetClient> netClient =
            std::make_shared<InnerNetClient>(this, socket);
        if(netClient != nullptr)
        {
            netClient->StartReceive();
            const std::string & address = socket->GetAddress();
            this->mInnerClients.emplace(address, netClient);
        }
        return true;
    }

	InnerNetClient* ForwardComponent::GetOrCreateClient(const std::string& address)
	{
		InnerNetClient * netClient = this->GetClient(address);
		if(netClient != nullptr)
		{
			return netClient;
		}
		NetThreadComponent * threadComponent = this->GetComponent<NetThreadComponent>();
		std::shared_ptr<SocketProxy> socketProxy = threadComponent->CreateSocket(address);
		if(socketProxy == nullptr)
		{
			return nullptr;
		}
		std::unique_ptr<InnerNetClient> newNetClient(new InnerNetClient(this, socketProxy));
		this->mInnerClients.emplace(address, std::move(newNetClient));
		return this->GetClient(address);
	}

	XCode ForwardComponent::OnResponse(std::shared_ptr<Rpc::Packet> message)
	{
		std::string address;
		if(!message->GetHead().Get("resp", address))
		{
			return XCode::Failure;
		}
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