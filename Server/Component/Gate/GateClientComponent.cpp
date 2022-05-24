//
// Created by mac on 2021/11/28.
//

#include"GateClientComponent.h"
#include"App/App.h"
#include"NetWork/GateClientContext.h"
#include"GateComponent.h"
#include"Component/Rpc/RpcHandlerComponent.h"
#ifdef __DEBUG__
#include"Util/StringHelper.h"
#include"Pool/MessagePool.h"
#include"google/protobuf/util/json_util.h"
#include"Global/ServiceConfig.h"
#endif
namespace Sentry
{
	void GateClientComponent::Awake()
	{
		this->mRpcComponent = nullptr;
		this->mTimerComponent = nullptr;
		this->mGateComponent = nullptr;
	}

	bool GateClientComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mTimerComponent = App::Get()->GetTimerComponent());
		LOG_CHECK_RET_FALSE(this->mRpcComponent = this->GetComponent<RpcHandlerComponent>());
		LOG_CHECK_RET_FALSE(this->mGateComponent = this->GetComponent<GateComponent>());
		return true;
	}

	void GateClientComponent::OnListen(std::shared_ptr<SocketProxy> socket)
	{
		const std::string & address = socket->GetAddress();
		auto iter = this->mGateClientMap.find(address);
		LOG_CHECK_RET(iter == this->mGateClientMap.end());
		if (this->mBlackList.find(socket->GetIp()) == this->mBlackList.end())
		{
			std::shared_ptr<GateClientContext> gateClient(
				new GateClientContext(socket, this));

			gateClient->StartReceive();
			this->mGateComponent->OnConnect(address);
			this->mGateClientMap.emplace(address, gateClient);
		}
	}

	void GateClientComponent::OnRequest(std::shared_ptr<c2s::Rpc_Request> request) //客户端调过来的
	{
		XCode code = this->mGateComponent->OnRequest(request);
		if (code != XCode::Successful)
		{
#ifdef __DEBUG__
			const ServiceConfig & configCom = this->GetApp()->GetServiceConfig();
			LOG_ERROR("player call " << request->method_name() << " failure error = " << configCom.GetCodeDesc(code));
#endif
			this->StartClose(request->address());
		}
	}

	void GateClientComponent::OnCloseSocket(const std::string & address, XCode code)
	{
		auto iter = this->mGateClientMap.find(address);
		if (iter != this->mGateClientMap.end())
		{
#ifdef __DEBUG__
			const ServiceConfig & configCom = this->GetApp()->GetServiceConfig();
			LOG_WARN("remove " << address  << " code = " << configCom.GetCodeDesc(code));
#endif
			this->mGateClientMap.erase(iter);
		}
	}

	bool GateClientComponent::SendToClient(const std::string & address, std::shared_ptr<c2s::Rpc_Response> message)
	{
		std::shared_ptr<GateClientContext> proxyClient = this->GetGateClient(address);
		if (proxyClient == nullptr)
		{
			return false;
		}
		return proxyClient->SendToClient(message);
	}

	void GateClientComponent::SendToAllClient(std::shared_ptr<c2s::Rpc::Call> message)
	{
		auto iter = this->mGateClientMap.begin();
		for(;iter != this->mGateClientMap.end(); iter++)
		{
			std::shared_ptr<GateClientContext> proxyClient = iter->second;
			if(proxyClient != nullptr)
			{
				proxyClient->SendToClient(message);
			}
		}
	}

	bool GateClientComponent::SendToClient(const std::string& address, std::shared_ptr<c2s::Rpc::Call> message)
	{
		std::shared_ptr<GateClientContext> gateClient = this->GetGateClient(address);
		if(gateClient != nullptr)
		{
			return gateClient->SendToClient(message);
		}
		return false;
	}

	std::shared_ptr<GateClientContext> GateClientComponent::GetGateClient(const std::string & address)
	{
		auto iter = this->mGateClientMap.find(address);
		return iter != this->mGateClientMap.end() ? iter->second : nullptr;
	}

	void GateClientComponent::StartClose(const std::string & address)
	{
		std::shared_ptr<GateClientContext> proxyClient = this->GetGateClient(address);
		if (proxyClient != nullptr)
		{
			proxyClient->StartClose();
		}
	}

	void GateClientComponent::CheckPlayerLogout(const std::string & address)
	{
		std::shared_ptr<GateClientContext> proxyClient = this->GetGateClient(address);
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
		this->mTimerComponent->DelayCall(5000, &GateClientComponent::CheckPlayerLogout, this, address);
	}

	bool GateClientComponent::AddNewUser(const std::string& address, long long userId)
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

	bool GateClientComponent::GetUserId(const std::string& address, long long& userId)
	{
		auto iter = this->mUserAddressMap.find(address);
		if(iter != this->mUserAddressMap.end())
		{
			userId = iter->second;
			return true;
		}
		return false;
	}

	bool GateClientComponent::GetUserAddress(long long userId, std::string& address)
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