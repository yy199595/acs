//
// Created by mac on 2021/11/28.
//

#include"OuterNetComponent.h"
#include"App/App.h"
#include"Client/OuterNetClient.h"
#include"OuterNetMessageComponent.h"
#include"Component/RedisDataComponent.h"
#include"Component/NetThreadComponent.h"
#include"Component/InnerNetMessageComponent.h"

#ifdef __DEBUG__
#include"String/StringHelper.h"
#include"Config/ServiceConfig.h"
#endif
#include"Md5/MD5.h"
#include"Component/UnitMgrComponent.h"
namespace Sentry
{
	void OuterNetComponent::Awake()
	{
        this->mNetComponent = nullptr;
        this->mTimerComponent = nullptr;
        this->mOuterMessageComponent = nullptr;
    }

	bool OuterNetComponent::LateAwake()
	{
        this->mTimerComponent = this->GetApp()->GetTimerComponent();
        this->mNetComponent = this->GetComponent<NetThreadComponent>();
        LOG_CHECK_RET_FALSE(this->mRedisComponent = this->GetComponent<RedisDataComponent>());
		LOG_CHECK_RET_FALSE(this->mOuterMessageComponent = this->GetComponent<OuterNetMessageComponent>());
		return true;
	}

	void OuterNetComponent::OnMessage(const std::string& address, std::shared_ptr<Rpc::Data> message)
    {
        switch ((Tcp::Type) message->GetType())
        {
            case Tcp::Type::Auth:
                if(!this->OnAuth(address, message))
                {
                    this->StartClose(address);
                }
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

    bool OuterNetComponent::OnAuth(const std::string &address, std::shared_ptr<Rpc::Data> message)
    {
        long long userId = 0;
        if (this->GetUserId(address, userId)) //已经验证过了
        {
            return true;
        }
        std::string token;
        const Rpc::Head & head = message->GetHead();
        LOG_CHECK_RET_FALSE(head.Get("token", token));

        auto iter = this->mTokens.find(token);
        if (iter == this->mTokens.end())
        {
            LOG_ERROR(address << " auth failure not find token");
            return false;
        }
        this->mTokens.erase(iter);
        message->GetHead().Remove("token");
        this->OnAuthSuccessful(iter->second, address);
        message->GetHead().Add("code", XCode::Successful);
        this->mOuterMessageComponent->OnResponse(address, message);
        return true;
    }

    bool OuterNetComponent::OnRequest(const std::string &address, std::shared_ptr<Rpc::Data> message)
    {
        long long userId = 0;
        if (!this->GetUserId(address, userId)) //没有验证
        {
            return false;
        }
        message->GetHead().Add("client", address);
        XCode code = this->mOuterMessageComponent->OnRequest(userId, message);
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

    void OuterNetComponent::OnAuthSuccessful(long long userId, const std::string &address)
    {
        // 分配服务器

        this->mUserAddressMap.emplace(address, userId);
        this->mClientAddressMap.emplace(userId, address);
        CONSOLE_LOG_INFO(userId << " auth successful ......");
    }

    std::string OuterNetComponent::CreateToken(long long userId, float second)
    {
        long long nowTime = Helper::Time::GetNowSecTime();
        std::string rand = fmt::format("{0}:{1}", userId, nowTime);
        const std::string token = Helper::Md5::GetMd5(rand);
        this->mTokens.emplace(token, userId);
        return token;
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
            LOG_WARN("remove client " << address << " code = " << (int) code);
#endif
            std::shared_ptr<OuterNetClient> gateClient = iter->second;
            if (this->mClientPools.size() < 100)
            {
                this->mClientPools.push(gateClient);
            }
            this->mGateClientMap.erase(iter);
        }
        auto iter1 = this->mUserAddressMap.find(address);
        if (iter1 != this->mUserAddressMap.end())
        {
            long long userId = iter1->second;
            auto iter2 = this->mClientAddressMap.find(userId);
            if (iter2 != this->mClientAddressMap.end())
            {
                this->mClientAddressMap.erase(iter2);
            }
            this->mUserAddressMap.erase(iter1);
        }
    }

    void OuterNetComponent::OnStopListen()
    {
        auto iter = this->mGateClientMap.begin();
        for(; iter != this->mGateClientMap.end(); iter++)
        {
            iter->second->StartClose();
        }
        this->mTokens.clear();
        this->mGateClientMap.clear();
        this->mUserAddressMap.clear();
    }

    bool OuterNetComponent::SendData(std::shared_ptr<Rpc::Data> message)
    {
        message->SetType(Tcp::Type::Request);
        auto iter = this->mGateClientMap.begin();
        for(;iter != this->mGateClientMap.end(); iter++)
        {
            std::shared_ptr<OuterNetClient> proxyClient = iter->second;
            if (proxyClient != nullptr)
            {
                proxyClient->SendData(message);
            }
        }
        return true;
    }

    bool OuterNetComponent::SendData(long long userId, std::shared_ptr<Rpc::Data> message)
    {
        std::string address;
        message->SetType(Tcp::Type::Request);
        if(!this->GetUserAddress(userId, address))
        {
            CONSOLE_LOG_ERROR("send message to user:" << userId << " failure");
            return false;
        }
        return this->SendData(address, message);
    }

	bool OuterNetComponent::SendData(const std::string &address, std::shared_ptr<Rpc::Data> message)
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

	bool OuterNetComponent::GetUserId(const std::string& address, long long& userId)
	{
		auto iter = this->mUserAddressMap.find(address);
		if(iter != this->mUserAddressMap.end())
		{
			userId = iter->second;
			return true;
		}
		return false;
	}

	bool OuterNetComponent::GetUserAddress(long long userId, std::string& address)
	{
		auto iter = this->mClientAddressMap.find(userId);
		if(iter != this->mClientAddressMap.end())
		{
			address = iter->second;
			return true;
		}
		return false;
	}
}