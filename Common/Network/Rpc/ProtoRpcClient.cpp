#include"ProtoRpcClient.h"
#include"App/App.h"
#include<Component/Rpc/RpcClientComponent.h>
#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
namespace Sentry
{
	ProtoRpcClient::ProtoRpcClient(RpcClientComponent* component,
		std::shared_ptr<SocketProxy> socket, SocketType type)
		: RpcClient(socket, type), mTcpComponent(component)
	{
		this->mConnectCount = 0;
	}

	void ProtoRpcClient::StartClose()
	{
#ifdef ONLY_MAIN_THREAD
		this->OnClientError(XCode::NetActiveShutdown);
#else
		this->mNetWorkThread.Invoke(&ProtoRpcClient::OnClientError, this, XCode::NetActiveShutdown);
#endif
	}

	void ProtoRpcClient::SendToServer(std::shared_ptr<com::Rpc_Response> message)
	{
		std::shared_ptr<NetworkData> networkData(
			new NetworkData(RPC_TYPE_RESPONSE, message));
#ifdef ONLY_MAIN_THREAD
		this->SendData(networkData);
#else
		this->mNetWorkThread.Invoke(&ProtoRpcClient::SendData, this, networkData);
#endif
	}

	void ProtoRpcClient::SendToServer(std::shared_ptr<com::Rpc_Request> message)
	{
		std::shared_ptr<NetworkData> networkData(
			new NetworkData(RPC_TYPE_REQUEST, message));
#ifdef ONLY_MAIN_THREAD
		this->SendData(networkData);
#else
		this->mNetWorkThread.Invoke(&ProtoRpcClient::SendData, this, networkData);
#endif
	}

	void ProtoRpcClient::OnSendData(XCode code, std::shared_ptr<NetworkData> message)
	{
		if (code != XCode::Successful)
		{
			MainTaskScheduler& taskScheduler = App::Get()->GetTaskScheduler();
		}
	}

	void ProtoRpcClient::OnClientError(XCode code)
	{
		if (code == XCode::NetActiveShutdown) //主动关闭不需要通知回主线
		{
			this->mSocketProxy->Close();
			return;
		}
		const std::string & address = this->GetAddress();
#ifdef ONLY_MAIN_THREAD
		this->mTcpComponent->OnCloseSocket(address, code);
#else
		MainTaskScheduler & taskScheduler = App::Get()->GetTaskScheduler();
		taskScheduler.Invoke(&RpcClientComponent::OnCloseSocket, this->mTcpComponent, address, code);
#endif
	}

	bool ProtoRpcClient::OnReceiveMessage(char type, const char* buffer, size_t size)
	{
		switch(type)
		{
		case RPC_TYPE_REQUEST:
			return this->OnRequest(buffer, size);
		case RPC_TYPE_RESPONSE:

			return this->OnResponse(buffer, size);
		}
		return false;
	}

	bool ProtoRpcClient::OnRequest(const char* buffer, size_t size)
	{
		std::shared_ptr<com::Rpc_Request> requestData(new com::Rpc_Request());
		if (!requestData->ParseFromArray(buffer, size))
		{
			return false;
		}
		requestData->set_address(this->GetAddress());
#ifdef ONLY_MAIN_THREAD
		this->mTcpComponent->OnRequest(requestData);
#else
		MainTaskScheduler & taskScheduler = App::Get()->GetTaskScheduler();
		taskScheduler.Invoke(&RpcClientComponent::OnRequest, mTcpComponent, requestData);
#endif

		return true;
	}

	bool ProtoRpcClient::OnResponse(const char* buffer, size_t size)
	{
		std::shared_ptr<com::Rpc_Response> responseData(new com::Rpc_Response());
		if (!responseData->ParseFromArray(buffer, size))
		{
			return false;
		}
#ifdef ONLY_MAIN_THREAD
		this->mTcpComponent->OnResponse(responseData);
#else
		MainTaskScheduler & taskScheduler = App::Get()->GetTaskScheduler();
		taskScheduler.Invoke(&RpcClientComponent::OnResponse, mTcpComponent, responseData);
#endif
		return true;
	}

	std::shared_ptr<TaskSource<bool>> ProtoRpcClient::ConnectAsync()
	{
		std::shared_ptr<TaskSource<bool>> taskSource
			= std::make_shared<TaskSource<bool>>();
		this->mConnectTasks.emplace(taskSource);
		this->StartConnect();
		return taskSource;
	}

	void ProtoRpcClient::OnConnect(XCode code)
	{
		this->mConnectCount++;
		bool res = code == XCode::Successful;
		while(!this->mConnectTasks.empty())
		{
			this->mConnectTasks.front()->SetResult(res);
			this->mConnectTasks.pop();
		}
		const std::string & address = this->GetAddress();
#ifdef ONLY_MAIN_THREAD
		this->mTcpComponent->OnConnectAfter(address, code);
#else
		MainTaskScheduler &taskScheduler = App::Get()->GetTaskScheduler();
		taskScheduler.Invoke(&RpcClientComponent::OnConnectAfter, this->mTcpComponent, address, code);
#endif
	}
}