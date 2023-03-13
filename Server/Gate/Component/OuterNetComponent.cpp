//
// Created by mac on 2021/11/28.
//

#include"OuterNetComponent.h"
#include"Client/OuterNetClient.h"
#include"OuterNetMessageComponent.h"
#include"Component/ThreadComponent.h"
#include"Component/InnerNetMessageComponent.h"
#include"Md5/MD5.h"
#include"Config/CodeConfig.h"
#include"Json/JsonWriter.h"
#include"Component/UnitMgrComponent.h"
namespace Sentry
{
	bool OuterNetComponent::Awake()
	{
		this->mSumCount = 0;
		this->mWaitCount = 0;
        this->mOuterMessageComponent = nullptr;
        return true;
    }

	bool OuterNetComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mOuterMessageComponent = this->GetComponent<OuterNetMessageComponent>());
		return true;
	}

    void OuterNetComponent::OnClusterComplete()
    {
        if(!this->StartListen("gate"))
        {
            LOG_FATAL("listen gate failure");
            return;
        }
        LOG_INFO("wait client connect to gate ....");
    }

	void OuterNetComponent::OnMessage(std::shared_ptr<Rpc::Packet> message)
	{
        const std::string& address = message->From();
		switch ((Tcp::Type)message->GetType())
		{
		case Tcp::Type::Auth:
			if (!this->OnAuth(address, message))
			{
				this->StartClose(address);
				CONSOLE_LOG_ERROR("[" << address << "] auth failure");
			}
			break;
		case Tcp::Type::Ping:
			message->SetContent("hello");
			this->SendData(address, message);
			break;
		case Tcp::Type::Request:
			if (!this->OnRequest(address, message))
			{
				this->StartClose(address);
				CONSOLE_LOG_ERROR("[" << address << "] request failure");
			}
			break;
		case Tcp::Type::Response:
			this->OnResponse(address, message);
			break;
		default:
			this->StartClose(address);
			break;
		}
	}

	bool OuterNetComponent::OnResponse(const std::string& address, std::shared_ptr<Rpc::Packet> message)
	{
		this->mWaitCount--;
		OuterNetClient * outerNetClient = this->GetGateClient(address);
		if(outerNetClient == nullptr)
		{
			LOG_ERROR("send to client [" << address << "] failure");
			return false;
		}
		outerNetClient->SendData(message);
		return true;
	}

    bool OuterNetComponent::OnAuth(const std::string &address, std::shared_ptr<Rpc::Packet> message)
	{
		Rpc::Head& head = message->GetHead();
		int code = this->mOuterMessageComponent->OnAuth(address, message);
		if (code != XCode::Successful)
		{
			return false;
		}
		this->mAuthClients.insert(address);

		head.Remove("token");
		message->SetType(Tcp::Type::Response);
		head.Add("code", XCode::Successful);
		return this->SendData(address, message);
	}

    bool OuterNetComponent::OnRequest(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
		this->mSumCount++;
		Rpc::Head & head = message->GetHead();
		if(head.Has("rpc"))
		{
			this->mWaitCount++;
		}
        head.Add("client", address);
        int code = this->mOuterMessageComponent->OnRequest(address, message);
        if (code != XCode::Successful)
        {
            this->StartClose(address);
            return false;
        }
        return true;
    }

    void OuterNetComponent::OnListen(std::shared_ptr<SocketProxy> socket)
    {
        const std::string &address = socket->GetAddress();
        auto iter = this->mGateClientMap.find(address);
        if (iter != this->mGateClientMap.end())
        {
            LOG_FATAL("handler socket error " << socket->GetAddress());
            return;
        }
        std::shared_ptr<OuterNetClient> outerNetClient;
        if (!this->mClientPools.empty())
        {
            outerNetClient = this->mClientPools.front();
            outerNetClient->Reset(socket);
            this->mClientPools.pop();
        }
        else
        {
            outerNetClient = std::make_shared<OuterNetClient>(socket, this);
        }

        outerNetClient->StartReceive(10);
        this->mGateClientMap.emplace(address, outerNetClient);
    }

	void OuterNetComponent::OnCloseSocket(const std::string & address, int code)
    {
        auto iter = this->mGateClientMap.find(address);
        if (iter != this->mGateClientMap.end())
        {
#ifdef __DEBUG__
            LOG_WARN("remove client [" << address << "]" << CodeConfig::Inst()->GetDesc(code));
#endif
            std::shared_ptr<OuterNetClient> gateClient = iter->second;
            if (this->mClientPools.size() < 100)
            {
                this->mClientPools.push(gateClient);
            }
            this->mGateClientMap.erase(iter);
        }
        auto iter1 = this->mAuthClients.find(address);
        if(iter1 != this->mAuthClients.end())
        {
            this->mAuthClients.erase(iter1);
        }
        this->mOuterMessageComponent->OnClose(address);
    }

    void OuterNetComponent::OnDestroy()
    {

    }


    bool OuterNetComponent::IsAuth(const std::string &address)
    {
        auto iter = this->mAuthClients.find(address);
        return iter != this->mAuthClients.end();
    }

    bool OuterNetComponent::SendData(const std::shared_ptr<Rpc::Packet>& message)
    {
        message->SetType(Tcp::Type::Request);
        auto iter = this->mGateClientMap.begin();
        for(;iter != this->mGateClientMap.end(); iter++)
        {
            std::shared_ptr<OuterNetClient> proxyClient = iter->second;
            if (proxyClient != nullptr && this->IsAuth(proxyClient->GetAddress()))
            {
                proxyClient->SendData(message->Clone());
            }
        }
        return true;
    }

    bool OuterNetComponent::SendData(long long userId, const std::shared_ptr<Rpc::Packet>& message)
    {
        std::string address;
        if(!this->mOuterMessageComponent->GetAddress(userId, address))
        {
            CONSOLE_LOG_ERROR("send message to user:" << userId << " failure");
            return false;
        }
        message->SetType(Tcp::Type::Request);
        return this->SendData(address, message);
    }

	bool OuterNetComponent::SendData(const std::string &address, const std::shared_ptr<Rpc::Packet>& message)
	{
        message->GetHead().Remove("address");
		OuterNetClient * outerNetClient = this->GetGateClient(address);
		if(outerNetClient != nullptr)
		{
            outerNetClient->SendData(message);
			return true;
		}
        CONSOLE_LOG_ERROR("send outer message to " << address << " error");
		return false;
	}

	OuterNetClient * OuterNetComponent::GetGateClient(const std::string & address)
	{
		auto iter = this->mGateClientMap.find(address);
		return iter != this->mGateClientMap.end() ? iter->second.get() : nullptr;
	}

	void OuterNetComponent::StartClose(const std::string & address)
	{
		OuterNetClient * proxyClient = this->GetGateClient(address);
		if (proxyClient != nullptr)
		{
			proxyClient->StartClose();
		}
	}

    void OuterNetComponent::OnRecord(Json::Writer &document)
    {
		document.Add("sum").Add(this->mSumCount);
		document.Add("wait").Add(this->mWaitCount);
        document.Add("auth").Add(this->mAuthClients.size());
        document.Add("client").Add(this->mGateClientMap.size());
    }
}