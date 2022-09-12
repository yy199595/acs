//
// Created by mac on 2021/11/28.
//

#include"OuterNetComponent.h"
#include"App/App.h"
#include"NetWork/OuterNetClient.h"
#include"OuterNetMessageComponent.h"
#include"Component/Rpc/InnerNetMessageComponent.h"
#include"Component/Scene/NetThreadComponent.h"
#ifdef __DEBUG__
#include"Util/StringHelper.h"
#include"google/protobuf/util/json_util.h"
#include"Global/ServiceConfig.h"
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

	void OuterNetComponent::OnMessage(const std::string& address, std::shared_ptr<Tcp::RpcMessage> message)
	{		
		switch ((Tcp::Type)message->GetType())
		{
		case Tcp::Type::Request:
			if (!this->OnRequest(address, *message))
			{
				this->StartClose(address);
			}
			break;
		default:
			this->StartClose(address);
			break;
		}
	}

	bool OuterNetComponent::OnRequest(const std::string& address, const Tcp::RpcMessage& message)
	{
		int len = 0;
		const char* data = message.GetData(len);
		std::shared_ptr<c2s::rpc::request> request
			= std::make_shared<c2s::rpc::request>();
		if (!request->ParseFromArray(data, len))
		{
			return false;
		}
		request->set_address(address);
		return this->mOuterMessageComponent->OnRequest(request) == XCode::Successful;
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
        this->mOuterMessageComponent->OnConnect(address);
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

	bool OuterNetComponent::SendToClient(const std::string & address, std::shared_ptr<c2s::rpc::response> message)
	{
		std::shared_ptr<OuterNetClient> outerNetClient = this->GetGateClient(address);
		if (outerNetClient == nullptr)
		{
			return false;
		}
        outerNetClient->SendToClient(message);
		return true;
	}

	void OuterNetComponent::SendToAllClient(std::shared_ptr<c2s::rpc::call> message)
	{
		auto iter = this->mGateClientMap.begin();
		for(;iter != this->mGateClientMap.end(); iter++)
		{
			std::shared_ptr<OuterNetClient> proxyClient = iter->second;
			if(proxyClient != nullptr)
			{
				proxyClient->SendToClient(message);
			}
		}
	}

	bool OuterNetComponent::SendToClient(const std::string& address, std::shared_ptr<c2s::rpc::call> message)
	{
		std::shared_ptr<OuterNetClient> gateClient = this->GetGateClient(address);
		if(gateClient != nullptr)
		{
			gateClient->SendToClient(message);
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