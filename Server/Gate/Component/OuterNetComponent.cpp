//
// Created by mac on 2021/11/28.
//

#include"OuterNetComponent.h"
#include"App/App.h"
#include"Client/OuterNetClient.h"
#include"OuterNetMessageComponent.h"
#include"Component/InnerNetMessageComponent.h"
#include"Component/NetThreadComponent.h"
#ifdef __DEBUG__
#include"String/StringHelper.h"
#include"google/protobuf/util/json_util.h"
#include"Config/ServiceConfig.h"
#endif
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
        this->mNetComponent = this->GetComponent<NetThreadComponent>();
		LOG_CHECK_RET_FALSE(this->mTimerComponent = App::Get()->GetTimerComponent());
		LOG_CHECK_RET_FALSE(this->mOuterMessageComponent = this->GetComponent<OuterNetMessageComponent>());
		return true;
	}

	void OuterNetComponent::OnMessage(const std::string& address, std::shared_ptr<Rpc::Data> message)
    {
        LOG_CHECK_RET(message->GetHead().Has("func"));
        switch ((Tcp::Type) message->GetType())
        {
            case Tcp::Type::Request:
                if (this->mOuterMessageComponent->OnRequest(address, message) != XCode::Successful)
                {
                    this->StartClose(address);
                }
                break;
            default:
                this->StartClose(address);
                break;
        }
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
			LOG_WARN("remove client " << address  << " code = " << (int)code);
#endif
            std::shared_ptr<OuterNetClient> gateClient = iter->second;
            if(this->mClientPools.size() < 100)
            {
                this->mClientPools.push(gateClient);
            }
            this->mGateClientMap.erase(iter);
		}
	}

	void OuterNetComponent::SendToAllClient(std::shared_ptr<c2s::rpc::call> message)
	{
		auto iter = this->mGateClientMap.begin();
		for(;iter != this->mGateClientMap.end(); iter++)
		{
			std::shared_ptr<OuterNetClient> proxyClient = iter->second;
			if(proxyClient != nullptr)
			{
                std::shared_ptr<Rpc::Data> request(new Rpc::Data());

                request->SetType(Tcp::Type::Request);
                request->SetProto(Tcp::Porto::Protobuf);
                request->GetHead().Add("func", message->func());
                if(message->has_data())
                {
                    message->data().SerializeToString(request->GetBody());
                }
				proxyClient->SendData(request);
			}
		}
	}

	bool OuterNetComponent::SendData(const std::string &address, std::shared_ptr<Rpc::Data> message)
	{
		std::shared_ptr<OuterNetClient> gateClient = this->GetGateClient(address);
		if(gateClient != nullptr)
		{
			gateClient->SendData(message);
			return true;
		}
		return false;
	}

	std::shared_ptr<OuterNetClient> OuterNetComponent::GetGateClient(const std::string & address)
	{
		auto iter = this->mGateClientMap.find(address);
		return iter != this->mGateClientMap.end() ? iter->second : nullptr;
	}

	void OuterNetComponent::StartClose(const std::string & address)
	{
		std::shared_ptr<OuterNetClient> proxyClient = this->GetGateClient(address);
		if (proxyClient != nullptr)
		{
			proxyClient->StartClose();
		}
	}

	bool OuterNetComponent::AddNewUser(const std::string& address, long long userId)
	{
		auto iter = this->mGateClientMap.find(address);
		if(iter == this->mGateClientMap.end())
		{
			LOG_ERROR("not find user address = [" << address << "]");
			return false;
		}
		auto iter1 = this->mUserAddressMap.find(address);
		if(iter1 != this->mUserAddressMap.end())
		{
			LOG_ERROR("user [" << userId << "] have login");
			return false;
		}
		this->mUserAddressMap.emplace(address, userId);
		this->mClientAddressMap.emplace(userId, address);
		LOG_DEBUG(userId << " add to gate address = " << "[" << address << "]");
		return true;
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