//
// Created by mac on 2021/11/28.
//

#include"RpcGateComponent.h"
#include"App/App.h"
#include"NetWork/GateMessageClient.h"
#include"GateComponent.h"
#include"Component/Rpc/TcpRpcComponent.h"
#include"Component/Scene/NetThreadComponent.h"
#ifdef __DEBUG__
#include"Util/StringHelper.h"
#include"google/protobuf/util/json_util.h"
#include"Global/ServiceConfig.h"
#endif
namespace Sentry
{
	void RpcGateComponent::Awake()
	{
        this->mNetComponent = nullptr;
        this->mGateComponent = nullptr;
        this->mTimerComponent = nullptr;

    }

	bool RpcGateComponent::LateAwake()
	{
        this->mNetComponent = this->GetComponent<NetThreadComponent>();
		LOG_CHECK_RET_FALSE(this->mTimerComponent = App::Get()->GetTimerComponent());
		LOG_CHECK_RET_FALSE(this->mGateComponent = this->GetComponent<GateComponent>());
		return true;
	}

	bool RpcGateComponent::OnListen(std::shared_ptr<SocketProxy> socket)
    {
        const std::string &address = socket->GetAddress();
        auto iter = this->mGateClientMap.find(address);
        if (iter != this->mGateClientMap.end())
        {
            LOG_FATAL("handler socket error " << socket->GetAddress());
            return false;
        }
        std::shared_ptr<GateMessageClient> gateClient;
        if (!this->mClientPools.empty())
        {
            gateClient = this->mClientPools.front();
            gateClient->Reset(socket);
            this->mClientPools.pop();
        }
        else
        {
            gateClient = std::make_shared<GateMessageClient>(socket, this);
        }

        gateClient->StartReceive();
        this->mGateComponent->OnConnect(address);
        this->mGateClientMap.emplace(address, gateClient);
        return true;
    }

	void RpcGateComponent::OnRequest(std::shared_ptr<c2s::rpc::request> request) //客户端调过来的
	{
		XCode code = this->mGateComponent->OnRequest(request);
		if (code != XCode::Successful)
		{
#ifdef __DEBUG__
			LOG_ERROR("player call " << request->method_name() << " error");
#endif
			this->StartClose(request->address());
		}
	}

	void RpcGateComponent::OnCloseSocket(const std::string & address, XCode code)
	{
		auto iter = this->mGateClientMap.find(address);
		if (iter != this->mGateClientMap.end())
		{
#ifdef __DEBUG__
			LOG_WARN("remove client " << address  << " code = " << (int)code);
#endif
            std::shared_ptr<GateMessageClient> gateClient = iter->second;
            if(this->mClientPools.size() < 100)
            {
                this->mClientPools.push(gateClient);
            }
            this->mGateClientMap.erase(iter);
		}
	}

	bool RpcGateComponent::SendToClient(const std::string & address, std::shared_ptr<c2s::rpc::response> message)
	{
		std::shared_ptr<GateMessageClient> proxyClient = this->GetGateClient(address);
		if (proxyClient == nullptr)
		{
			return false;
		}
		proxyClient->SendToClient(message);
		return true;
	}

	void RpcGateComponent::SendToAllClient(std::shared_ptr<c2s::rpc::call> message)
	{
		auto iter = this->mGateClientMap.begin();
		for(;iter != this->mGateClientMap.end(); iter++)
		{
			std::shared_ptr<GateMessageClient> proxyClient = iter->second;
			if(proxyClient != nullptr)
			{
				proxyClient->SendToClient(message);
			}
		}
	}

	bool RpcGateComponent::SendToClient(const std::string& address, std::shared_ptr<c2s::rpc::call> message)
	{
		std::shared_ptr<GateMessageClient> gateClient = this->GetGateClient(address);
		if(gateClient != nullptr)
		{
			gateClient->SendToClient(message);
			return true;
		}
		return false;
	}

	std::shared_ptr<GateMessageClient> RpcGateComponent::GetGateClient(const std::string & address)
	{
		auto iter = this->mGateClientMap.find(address);
		return iter != this->mGateClientMap.end() ? iter->second : nullptr;
	}

	void RpcGateComponent::StartClose(const std::string & address)
	{
		std::shared_ptr<GateMessageClient> proxyClient = this->GetGateClient(address);
		if (proxyClient != nullptr)
		{
			proxyClient->StartClose();
		}
	}

	void RpcGateComponent::CheckPlayerLogout(const std::string & address)
	{
		std::shared_ptr<GateMessageClient> proxyClient = this->GetGateClient(address);
		if (proxyClient != nullptr)
		{
			long long nowTime = Helper::Time::GetNowSecTime();
			if (nowTime - proxyClient->GetLastOperTime() >= 5)
			{
				proxyClient->StartClose();
				LOG_ERROR(proxyClient->GetAddress() << " logout");
				return;
			}
		}
		this->mTimerComponent->DelayCall(5000, &RpcGateComponent::CheckPlayerLogout, this, address);
	}

	bool RpcGateComponent::AddNewUser(const std::string& address, long long userId)
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

	bool RpcGateComponent::GetUserId(const std::string& address, long long& userId)
	{
		auto iter = this->mUserAddressMap.find(address);
		if(iter != this->mUserAddressMap.end())
		{
			userId = iter->second;
			return true;
		}
		return false;
	}

	bool RpcGateComponent::GetUserAddress(long long userId, std::string& address)
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