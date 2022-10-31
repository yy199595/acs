//
// Created by mac on 2021/11/28.
//

#include"OuterNetComponent.h"
#include"App/App.h"
#include"Client/OuterNetClient.h"
#include"OuterNetMessageComponent.h"
#include"Component/NetThreadComponent.h"
#include"Component/InnerNetMessageComponent.h"

#ifdef __DEBUG__
#include"String/StringHelper.h"
#include"Config/ServiceConfig.h"
#endif
#include"Md5/MD5.h"
#include"Config/CodeConfig.h"
#include"Config/ServerConfig.h"
#include"Component/UnitMgrComponent.h"
namespace Sentry
{
	bool OuterNetComponent::Awake()
	{
        this->mNetComponent = nullptr;
        this->mTimerComponent = nullptr;
        this->mOuterMessageComponent = nullptr;
        return true;
    }

	bool OuterNetComponent::LateAwake()
	{
        this->mTimerComponent = this->mApp->GetTimerComponent();
        this->mNetComponent = this->GetComponent<NetThreadComponent>();
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

	void OuterNetComponent::OnMessage(const std::string& address, std::shared_ptr<Rpc::Packet> message)
    {
        switch ((Tcp::Type) message->GetType())
        {
            case Tcp::Type::Auth:
                if(!this->OnAuth(address, message))
                {
                    this->StartClose(address);
                }
                break;
            case Tcp::Type::Ping:
                break;
            case Tcp::Type::Request:
                if (!this->OnRequest(address, message))
                {
                    this->StartClose(address);
                }
                break;
            default:
                this->StartClose(address);
                break;
        }
    }

    bool OuterNetComponent::OnAuth(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
        XCode code = this->mOuterMessageComponent->OnAuth(address, message);
        if(code != XCode::Successful)
        {
            return false;
        }
        this->mAuthClients.insert(address);
        message->GetHead().Remove("token");
        message->GetHead().Add("code", XCode::Successful);
        this->mOuterMessageComponent->OnResponse(address, message);
        return true;
    }

    bool OuterNetComponent::OnRequest(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
        message->GetHead().Add("client", address);
        XCode code = this->mOuterMessageComponent->OnRequest(address, message);
        if (code != XCode::Successful)
        {
            message->Clear();
            message->GetHead().Remove("address");
            message->GetHead().Add("code", code);
            this->mOuterMessageComponent->OnResponse(address, message);
            return false;
        }
        return true;
    }

	bool OuterNetComponent::OnListen(std::shared_ptr<SocketProxy> socket)
    {
        const std::string &address = socket->GetAddress();
        auto iter = this->mGateClientMap.find(address);
        if (iter != this->mGateClientMap.end())
        {
            LOG_FATAL("handler socket error " << socket->GetAddress());
            return false;
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

        outerNetClient->StartReceive();
        this->mGateClientMap.emplace(address, outerNetClient);
        return true;
    }

	void OuterNetComponent::OnCloseSocket(const std::string & address, XCode code)
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

    void OuterNetComponent::OnDestory()
    {

    }


    bool OuterNetComponent::IsAuth(const std::string &address)
    {
        auto iter = this->mAuthClients.find(address);
        return iter != this->mAuthClients.end();
    }

    bool OuterNetComponent::SendData(std::shared_ptr<Rpc::Packet> message)
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

    bool OuterNetComponent::SendData(long long userId, std::shared_ptr<Rpc::Packet> message)
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

	bool OuterNetComponent::SendData(const std::string &address, std::shared_ptr<Rpc::Packet> message)
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
}