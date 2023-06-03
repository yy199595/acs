//
// Created by mac on 2021/11/28.
//

#include"OuterNetComponent.h"
#include"Gate/Client/OuterNetTcpClient.h"
#include"Server/Component/ThreadComponent.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Util/Md5/MD5.h"
#include"Server/Config/CodeConfig.h"
#include"Util/Json/JsonWriter.h"
#include"Gate/Service/Gate.h"
#include"Entity/Actor/App.h"
#include"Entity/Actor/PlayerActor.h"
namespace Tendo
{
	bool OuterNetComponent::Awake()
	{
		this->mSumCount = 0;
		this->mWaitCount = 0;
        return true;
    }

	bool OuterNetComponent::LateAwake()
	{
		this->mMaxHandlerCount = 500;
		this->mInnerNetComponent = this->GetComponent<InnerNetComponent>();
		ServerConfig::Inst()->GetMember("message", "outer", this->mMaxHandlerCount);
		return true;
	}

    void OuterNetComponent::Complete()
    {
        if(!this->StartListen("gate"))
        {
            LOG_FATAL("listen gate failure");
            return;
        }
        LOG_INFO("wait client connect to gate ....");
    }

	void OuterNetComponent::OnTimeout(const std::string& address)
	{

	}

	bool OuterNetComponent::BindClient(const std::string & address, long long userId)
	{
		auto iter1 = this->mAddressUserMap.find(address);
		if(iter1 != this->mAddressUserMap.end())
		{
			return false;
		}
		this->mAddressUserMap.emplace(address, userId);
		this->mUserAddressMap.emplace(userId, address);
		return true;
	}

	void OuterNetComponent::OnMessage(std::shared_ptr<Msg::Packet> message)
	{
		this->mSumCount++;
		const std::string& address = message->From();
		switch (message->GetType())
		{
			case Msg::Type::Ping:
			{
				message->SetContent("hello");
				message->SetType(Msg::Type::Response);
				this->Send(address, message);
				break;
			}
			default:
			{
				long long userId = 0;
				if (this->GetUserId(address, userId))
				{
					message->GetHead().Add("id", userId);
				}
				this->mMessages.push(std::move(message));
				break;
			}
		}
	}

	void OuterNetComponent::OnFrameUpdate(float t)
	{
		for (int index = 0; index < this->mMaxHandlerCount && !this->mMessages.empty(); index++)
		{
			std::shared_ptr<Msg::Packet> message = this->mMessages.front();
			{
				long long userId = 0;
				message->GetHead().Get("id", userId);
				int code = this->OnRequest(userId, message);
				if(code != XCode::Successful && message->GetHead().Has("rpc"))
				{
					message->Clear();
					message->GetHead().Remove("func");
					message->SetType(Msg::Type::Response);
					message->GetHead().Add("code", code);
					this->Send(message->From(), message);
				}
			}
			this->mMessages.pop();
		}
	}

	int OuterNetComponent::OnRequest(long long userId, std::shared_ptr<Msg::Packet>& message)
	{
		const std::string & fullName = message->GetHead().GetStr("func");
		const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
		if (methodConfig == nullptr || !methodConfig->IsClient || !methodConfig->IsOpen)
		{
			return XCode::CallFunctionNotExist;
		}
		std::string target;
		const std::string & server = methodConfig->Server;
		Actor * player = this->mApp->ActorMgr()->GetActor(userId);
		if(player == nullptr)
		{
			return XCode::NotFindUser;
		}
		if(!player->GetAddress(server, target))
		{
			return XCode::NotFoundPlayerRpcAddress;
		}
		message->GetHead().Add("id", userId);
		message->GetHead().Add("cli", message->From());
#ifdef __DEBUG__
		int rpcId = 0;
		if (message->ConstHead().Get("rpc", rpcId))
		{
			long long now = Helper::Time::NowSecTime();
			this->mRecords.emplace(rpcId, now);
		}
#endif // __DEBUG__
		this->mWaitCount++;
		if(!this->mInnerNetComponent->Send(target, message))
		{
			return XCode::NetWorkError;
		}
		return XCode::Successful;
	}

	bool OuterNetComponent::Send(const std::string& address, const std::shared_ptr<Msg::Packet>& message)
	{
#ifdef __DEBUG__
		int rpcId = 0;
		const Msg::Head& head = message->ConstHead();
		if (message->GetType() == Msg::Type::Response
			&& head.Get("rpc", rpcId))
		{
			std::string func;
			head.Get("func", func);
			auto iter1 = this->mRecords.find(rpcId);
			if (iter1 != this->mRecords.end())
			{
				long long time = iter1->second;
				long long t = Helper::Time::NowSecTime() - time;
				LOG_DEBUG("client call [" << func << "] use time [" << t << "ms]");
				this->mRecords.erase(iter1);
			}
		}
#endif
		if(message->GetType() == Msg::Type::Response)
		{
			this->mWaitCount--;
		}
		auto iter = this->mGateClientMap.find(address);
		if (iter == this->mGateClientMap.end())
		{
			return false;
		}
		iter->second->SendData(message);
		return true;
	}

    void OuterNetComponent::OnListen(std::shared_ptr<Tcp::SocketProxy> socket)
    {
        const std::string &address = socket->GetAddress();
        auto iter = this->mGateClientMap.find(address);
        if (iter != this->mGateClientMap.end())
        {
            LOG_FATAL("handler socket error " << socket->GetAddress());
            return;
        }
        std::shared_ptr<OuterNetTcpClient> outerNetClient;
        if (!this->mClientPools.empty())
        {
            outerNetClient = this->mClientPools.front();
            outerNetClient->Reset(socket);
            this->mClientPools.pop();
        }
        else
        {
            outerNetClient = std::make_shared<OuterNetTcpClient>(socket, this);
        }
        outerNetClient->StartReceive(10);
        this->mGateClientMap.emplace(address, outerNetClient);
    }

	void OuterNetComponent::OnCloseSocket(const std::string & address, int code)
    {
#ifdef __DEBUG__
		LOG_WARN("remove client [" << address << "]" << CodeConfig::Inst()->GetDesc(code));
#endif
        auto iter = this->mGateClientMap.find(address);
        if (iter != this->mGateClientMap.end())
        {
            std::shared_ptr<OuterNetTcpClient> gateClient = iter->second;
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
		}
    }

    void OuterNetComponent::OnDestroy()
    {
		this->StopListen();
		auto iter = this->mGateClientMap.begin();
		for(; iter != this->mGateClientMap.end(); iter++)
		{
			iter->second->StartClose();
		}
    }

    bool OuterNetComponent::Send(long long userId, const std::shared_ptr<Msg::Packet>& message)
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
	size_t OuterNetComponent::Broadcast(const std::shared_ptr<Msg::Packet>& message)
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
	bool OuterNetComponent::StopClient(long long userId)
	{
		auto iter1 = this->mUserAddressMap.find(userId);
		if(iter1 == this->mUserAddressMap.end())
		{
			return false;
		}
		const std::string & address = iter1->second;
		auto iter2 = this->mAddressUserMap.find(iter1->second);
		if(iter2 != this->mAddressUserMap.end())
		{
			this->mAddressUserMap.erase(iter2);
		}
		this->StartClose(address);
		this->mUserAddressMap.erase(iter1);
		return true;
	}
}