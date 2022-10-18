//
// Created by zmhy0073 on 2022/10/14.
//

#include"ForwardComponent.h"
#include"Component/LocationComponent.h"
#include"Component/NetThreadComponent.h"
namespace Sentry
{
    bool ForwardComponent::LateAwake()
    {
        this->mLocationComponent = this->GetComponent<LocationComponent>();
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
		Rpc::Head & head = message->GetHead();
        switch ((Tcp::Type) message->GetType())
        {
            case Tcp::Type::Auth:
			{
				if(!this->OnAuth(address, message))
				{
					this->StartClose(address);
					LOG_ERROR(address << " auth failure");
				}
			}
                break;
            case Tcp::Type::Request:
            {
				if(head.Has("rpc"))
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
            {
				if(this->OnBroadcast(message) != XCode::Successful)
				{
					std::string func;
					if(head.Get("func", func))
					{
						LOG_ERROR("broadcast message error : " << func);
					}
				}
            }
                break;
            case Tcp::Type::Response:
			{
				if(this->OnResponse(message) != XCode::Successful)
				{

				}
			}
                break;
        }
    }

	bool ForwardComponent::OnAuth(const std::string& address, std::shared_ptr<Rpc::Packet> message)
	{
		if(this->mAuthClients.find(address) != this->mAuthClients.end())
		{
			return true;
		}
		std::string user, passwd, location, name;
		const Rpc::Head & head = message->GetHead();
		LOG_CHECK_RET_FALSE(head.Get("user", user));
        LOG_CHECK_RET_FALSE(head.Get("name", name));
        LOG_CHECK_RET_FALSE(head.Get("passwd", passwd));
        LOG_CHECK_RET_FALSE(head.Get("location", location));
        this->mAuthClients.insert(address);
		return true;
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
            return this->SendData(target, message)
                   ? XCode::Successful : XCode::NetWorkError;
        }
        else if (head.Get("to", target))
        {
            return this->SendData(target, message)
                   ? XCode::Successful : XCode::NetWorkError;
        }

        return XCode::Successful;
    }

    bool ForwardComponent::SendData(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
        InnerNetClient * netClient = this->GetOrCreateClient(address);
        if(netClient == nullptr)
        {
            return false;
        }
        netClient->SendData(message);
        return true;
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

	XCode ForwardComponent::OnBroadcast(std::shared_ptr<Rpc::Packet> message)
	{
		if(this->mInnerClients.empty())
		{
			return XCode::NetWorkError;
		}
		auto iter = this->mInnerClients.begin();
		for(; iter != this->mInnerClients.end(); iter++)
		{
			InnerNetClient * netClient = iter->second.get();
            if(netClient != nullptr)
            {
                netClient->SendData(message);
            }
		}
		return XCode::Successful;
	}

	XCode ForwardComponent::OnResponse(std::shared_ptr<Rpc::Packet> message)
	{
		std::string address;
		if(!message->GetHead().Get("resp", address))
		{
			return XCode::Failure;
		}
		InnerNetClient * netClient = this->GetClient(address);
		if(netClient == nullptr)
		{
			return XCode::NetWorkError;
		}
		netClient->SendData(message);
		return XCode::Successful;
	}
}