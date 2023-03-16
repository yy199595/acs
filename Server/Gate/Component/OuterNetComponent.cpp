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
		this->mMaxHandlerCount = 500;
		this->mOuterMessageComponent = this->GetComponent<OuterNetMessageComponent>();
		ServerConfig::Inst()->GetMember("message", "outer", this->mMaxHandlerCount);
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
		switch (message->GetType())
		{
		case Tcp::Type::Auth:
			if (!this->OnAuth(message))
			{
				this->StartClose(address);
				CONSOLE_LOG_ERROR("[" << address << "] auth failure");
			}
			return;
		case Tcp::Type::Ping:
			message->SetContent("hello");
			message->SetType(Tcp::Type::Response);
			this->Send(address, message);
			return;
		}
		long long userId = 0;
		if (!this->GetUserId(address, userId))
		{
			LOG_ERROR("not auth client message : [" << address << "]");
			return;
		}
		message->GetHead().Add("id", userId);
		this->mMessages.push(std::move(message));
	}

	void OuterNetComponent::OnFrameUpdate(float t)
	{
		for (int index = 0; index < this->mMaxHandlerCount && !this->mMessages.empty(); index++)
		{
			std::shared_ptr<Rpc::Packet> message = this->mMessages.front();
			{
				long long userId = 0;
				message->GetHead().Get("id", userId);
				int code = this->mOuterMessageComponent->OnMessage(userId, message);
				if(code != XCode::Successful && message->GetHead().Has("rpc"))
				{
					message->Clear();
					message->GetHead().Add("code", code);
					this->Send(message->From(), std::move(message));
				}
			}
			this->mMessages.pop();
		}
	}

	bool OuterNetComponent::Send(const std::string& address, const std::shared_ptr<Rpc::Packet>& message)
	{
		auto iter = this->mGateClientMap.find(address);
		if(iter == this->mGateClientMap.end())
		{
			return false;
		}
		iter->second->SendData(message);
		return true;
	}

    bool OuterNetComponent::OnAuth(std::shared_ptr<Rpc::Packet> message)
	{
		std::string token;
		Rpc::Head & head = message->GetHead();
		if(!head.Get("token", token))
		{
			return false;
		}
		auto iter = this->mClientTokens.find(token);
		if(iter == this->mClientTokens.end())
		{
			LOG_ERROR("not find client token:" << token);
			return false;
		}
		long long userId = iter->second;
		this->mClientTokens.erase(iter);
		const std::string &address = message->From();
		this->mAddressUserMap.emplace(address, userId);
		this->mUserAddressMap.emplace(userId, address);
		int code = this->mOuterMessageComponent->OnLogin(userId);
		const std::string & error = CodeConfig::Inst()->GetDesc(code);
		LOG_ERROR("user id : " << userId << "login result = " << error);
		{
			head.Remove(token);
			head.Add("code", code);
			head.Add("error", error);
		}
		return this->Send(address, message);
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
        auto iter1 = this->mAddressUserMap.find(address);
        if(iter1 != this->mAddressUserMap.end())
        {
			long long userId = iter1->second;
            this->mAddressUserMap.erase(iter1);
			auto iter2 = this->mUserAddressMap.find(userId);
			if(iter2 != this->mUserAddressMap.end())
			{
				this->mUserAddressMap.erase(iter2);
			}
			this->mOuterMessageComponent->OnLogout(userId);
		}
    }

    void OuterNetComponent::OnDestroy()
    {

    }

    bool OuterNetComponent::Send(long long userId, const std::shared_ptr<Rpc::Packet>& message)
    {
		auto iter = this->mUserAddressMap.find(userId);
		if(iter == this->mUserAddressMap.end())
		{
			CONSOLE_LOG_ERROR("send message to user:" << userId << " failure");
			return false;
		}
		return this->Send(iter->second, message);
    }

	void OuterNetComponent::StartClose(const std::string & address)
	{
		auto iter = this->mGateClientMap.find(address);
		if(iter == this->mGateClientMap.end())
		{
			return;
		}
		iter->second->StartClose();
	}

    void OuterNetComponent::OnRecord(Json::Writer &document)
    {
		document.Add("sum").Add(this->mSumCount);
		document.Add("wait").Add(this->mWaitCount);
        document.Add("auth").Add(this->mAddressUserMap.size());
        document.Add("client").Add(this->mGateClientMap.size());
    }
	bool OuterNetComponent::MakeToken(long long id, std::string& token)
	{
		long long now = Helper::Time::NowSecTime();
		token = Helper::Md5::GetMd5(fmt::format("{0}:{1}", now, id));
		if(this->mClientTokens.find(token) != this->mClientTokens.end())
		{
			return false;
		}
		this->mClientTokens.emplace(token, id);
		return true;
	}
	bool OuterNetComponent::GetUserId(const std::string& address, long long & userId) const
	{
		auto iter = this->mAddressUserMap.find(address);
		if(iter == this->mAddressUserMap.end())
		{
			return false;
		}
		userId = iter->second;
		return true;
	}
	size_t OuterNetComponent::Broadcast(const std::shared_ptr<Rpc::Packet>& message)
	{
		size_t count = 0;
		auto iter = this->mAddressUserMap.begin();
		for(; iter != this->mAddressUserMap.end(); iter++)
		{
			const std::string & address = iter->first;
			if(this->Send(address, message->Clone()))
			{
				count++;
			}
		}
		return count;
	}
}