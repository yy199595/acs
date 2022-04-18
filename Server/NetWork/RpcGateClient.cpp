//
// Created by mac on 2021/11/28.
//

#include"RpcGateClient.h"
#include"Protocol/c2s.pb.h"
#include"App/App.h"
#ifdef __DEBUG__
#include"google/protobuf/util/json_util.h"
#endif
#include"Component/Gate/GateClientComponent.h"
namespace Sentry
{
	RpcGateClient::RpcGateClient(std::shared_ptr<SocketProxy> socket, SocketType type,
		GateClientComponent* component)
		: RpcClient(socket, type), mGateComponent(component)
	{
		this->mQps = 0;
		this->mCallCount = 0;
	}

	void RpcGateClient::OnConnect(XCode code)
	{

	}

	XCode RpcGateClient::OnRequest(const char* buffer, size_t size)
	{
		std::shared_ptr<c2s::Rpc_Request> request(new c2s::Rpc_Request());
		if (!request->ParseFromArray(buffer, (int)size))
		{
			return XCode::ParseRequestDataError;
		}
		this->mCallCount++;
		this->mQps += size;
		std::cout << "receive player message count = " << this->mCallCount << std::endl;
		request->set_address(this->GetAddress());
#ifdef ONLY_MAIN_THREAD
		this->mGateComponent->OnRequest(request);
#else
		MainTaskScheduler &mainTaskScheduler = App::Get()->GetTaskScheduler();
		mainTaskScheduler.Invoke(&GateClientComponent::OnRequest, this->mGateComponent, request);
#endif

		return XCode::Successful;
	}

	XCode RpcGateClient::OnResponse(const char* buffer, size_t size) //不处理response消息
	{
		return XCode::UnKnowPacket;
	}

	void RpcGateClient::OnClientError(XCode code)
	{
		if (code == XCode::NetActiveShutdown)
		{
			this->mSocketProxy->Close();
			return;
		}
		const std::string & address = this->GetAddress();
#ifdef ONLY_MAIN_THREAD
		this->mGateComponent->OnCloseSocket(address, code);
#else
		MainTaskScheduler &mainTaskScheduler = App::Get()->GetTaskScheduler();
		mainTaskScheduler.Invoke(&GateClientComponent::OnCloseSocket, this->mGateComponent, address, code);
#endif

	}

	void RpcGateClient::StartClose()
	{
		XCode code = XCode::NetActiveShutdown;
#ifdef ONLY_MAIN_THREAD
		this->OnClientError(code);
#else
		this->mNetWorkThread.Invoke(&RpcGateClient::OnClientError, this, code);
#endif
	}

	bool RpcGateClient::SendToClient(std::shared_ptr<c2s::Rpc_Request> message)
	{
		if(!this->GetSocketProxy()->IsOpen())
		{
			return false;
		}
		std::shared_ptr<NetworkData> networkData(
			new NetworkData(RPC_TYPE_REQUEST, message));
#ifdef ONLY_MAIN_THREAD
		this->SendData(networkData);
#else
		this->mNetWorkThread.Invoke(&RpcGateClient::SendData, this, networkData);
#endif
		return true;
	}

	bool RpcGateClient::SendToClient(std::shared_ptr<c2s::Rpc_Response> message)
	{
		if(!this->GetSocketProxy()->IsOpen())
		{
			return false;
		}
		std::shared_ptr<NetworkData> networkData(
			new NetworkData(RPC_TYPE_RESPONSE, message));
#ifdef ONLY_MAIN_THREAD
		this->SendData(networkData);
#else
		this->mNetWorkThread.Invoke(&RpcGateClient::SendData, this, networkData);
#endif
		return true;
	}

	void RpcGateClient::OnSendData(XCode code, std::shared_ptr<NetworkData> message)
	{
#ifdef __DEBUG__
		if (code != XCode::Successful)
		{

		}
#endif
	}
}