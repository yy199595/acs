//
// Created by mac on 2021/11/28.
//

#include"GateClientComponent.h"
#include"App/App.h"
#include"NetWork/RpcGateClient.h"
#include"GateComponent.h"
#include"Component/Rpc/RpcComponent.h"
#ifdef __DEBUG__
#include"Util/StringHelper.h"
#include"Pool/MessagePool.h"
#include"google/protobuf/util/json_util.h"
#include"Global/RpcConfig.h"
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
		LOG_CHECK_RET_FALSE(this->mRpcComponent = this->GetComponent<RpcComponent>());
		LOG_CHECK_RET_FALSE(this->mGateComponent = this->GetComponent<GateComponent>());
		return true;
	}

	void GateClientComponent::OnListen(std::shared_ptr<SocketProxy> socket)
	{
		const std::string & address = socket->GetAddress();
		auto iter = this->mGateClientMap.find(address);
		LOG_CHECK_RET(iter == this->mGateClientMap.end());
		const std::string ip = socket->GetSocket().remote_endpoint().address().to_string();
		if (this->mBlackList.find(ip) == this->mBlackList.end())
		{
			std::shared_ptr<RpcGateClient> gateClient(
				new RpcGateClient(socket, SocketType::RemoteSocket, this));

			gateClient->StartReceive();
			this->mGateClientMap.emplace(address, gateClient);
			this->mGateComponent->OnConnect(socket->GetSocketId());
		}
	}

	void GateClientComponent::OnRequest(std::shared_ptr<c2s::Rpc_Request> request) //客户端调过来的
	{
		XCode code = this->mGateComponent->OnRequest(request);
		if (code != XCode::Successful)
		{
			std::shared_ptr<c2s::Rpc_Response> responseMessage(new c2s::Rpc_Response());
#ifdef __DEBUG__
			const RpcConfig & configCom = this->GetApp()->GetRpcConfig();
			LOG_ERROR("player call" << request->method_name() << "failure error = " << configCom.GetCodeDesc(code));
#endif
			responseMessage->set_code((int)code);
			responseMessage->set_rpc_id(request->rpc_id());
			this->SendToClient(request->address(), responseMessage);
		}
	}

	void GateClientComponent::OnCloseSocket(const std::string & address, XCode code)
	{
		auto iter = this->mGateClientMap.find(address);
		if (iter != this->mGateClientMap.end())
		{
#ifdef __DEBUG__
			const RpcConfig & configCom = this->GetApp()->GetRpcConfig();
			LOG_WARN("remove player session code = " << configCom.GetCodeDesc(code));
#endif
			this->mGateClientMap.erase(iter);
		}
	}

	bool GateClientComponent::SendToClient(const std::string & address, std::shared_ptr<c2s::Rpc_Response> message)
	{
		auto proxyClient = this->GetGateClient(address);
		if (proxyClient == nullptr)
		{
			return false;
		}
		return proxyClient->SendToClient(message);
	}

	std::shared_ptr<RpcGateClient> GateClientComponent::GetGateClient(const std::string & address)
	{
		auto iter = this->mGateClientMap.find(address);
		return iter != this->mGateClientMap.end() ? iter->second : nullptr;
	}

	void GateClientComponent::StartClose(const std::string & address)
	{
		auto proxyClient = this->GetGateClient(address);
		if (proxyClient != nullptr)
		{
			proxyClient->StartClose();
		}
	}

	void GateClientComponent::CheckPlayerLogout(const std::string & address)
	{
		auto proxyClient = this->GetGateClient(address);
		if (proxyClient != nullptr)
		{
			long long nowTime = Helper::Time::GetNowSecTime();
			if (nowTime - proxyClient->GetLastOperatorTime() >= 5)
			{
				proxyClient->StartClose();
				LOG_ERROR(proxyClient->GetAddress() << " logout");
				return;
			}
		}
		this->mTimerComponent->AsyncWait(5000, &GateClientComponent::CheckPlayerLogout, this, address);
	}
}