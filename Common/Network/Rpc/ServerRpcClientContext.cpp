#include"ServerRpcClientContext.h"
#include"App/App.h"
#include<Component/Rpc/RpcClientComponent.h>
#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
namespace Sentry
{
	ServerRpcClientContext::ServerRpcClientContext(RpcClientComponent* component,
		std::shared_ptr<SocketProxy> socket, SocketType type)
		: RpcClientContext(socket, type), mTcpComponent(component)
	{
		this->mConnectCount = 0;
		this->mConnectLock = std::make_shared<CoroutineLock>();
	}

	void ServerRpcClientContext::StartClose()
	{
#ifdef ONLY_MAIN_THREAD
		this->OnClientError(XCode::NetActiveShutdown);
#else
		this->mNetWorkThread.Invoke(&ServerRpcClientContext::OnClientError, this, XCode::NetActiveShutdown);
#endif
	}

	void ServerRpcClientContext::SendToServer(std::shared_ptr<com::Rpc_Response> message)
	{
		std::shared_ptr<NetworkData> networkData(
			new NetworkData(RPC_TYPE_RESPONSE, message));
#ifdef ONLY_MAIN_THREAD
		this->SendData(networkData);
#else
		this->mNetWorkThread.Invoke(&ServerRpcClientContext::SendData, this, networkData);
#endif
	}

	void ServerRpcClientContext::SendToServer(std::shared_ptr<com::Rpc_Request> message)
	{
		std::shared_ptr<NetworkData> networkData(
			new NetworkData(RPC_TYPE_REQUEST, message));
#ifdef ONLY_MAIN_THREAD
		this->SendData(networkData);
#else
		this->mNetWorkThread.Invoke(&ServerRpcClientContext::SendData, this, networkData);
#endif
	}

	void ServerRpcClientContext::OnSendData(XCode code, std::shared_ptr<NetworkData> message)
	{
		if (code != XCode::Successful)
		{
			MainTaskScheduler& taskScheduler = App::Get()->GetTaskScheduler();
		}
	}

	void ServerRpcClientContext::OnClientError(XCode code)
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

	bool ServerRpcClientContext::OnReceiveMessage(char type, const char* buffer, size_t size)
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

	bool ServerRpcClientContext::OnRequest(const char* buffer, size_t size)
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

	bool ServerRpcClientContext::OnResponse(const char* buffer, size_t size)
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

	bool ServerRpcClientContext::StartConnectAsync()
	{
		AutoCoroutineLock lock(this->mConnectLock);
		if(this->mSocketProxy->IsOpen())
		{
			return true;
		}
		this->StartConnect();
		this->mConnectTask = std::make_shared<TaskSource<XCode>>();
		return mConnectTask->Await() == XCode::Successful;
	}

	void ServerRpcClientContext::OnConnect(XCode code)
	{
		this->mConnectCount++;
		assert(this->mConnectTask);
		this->mConnectTask->SetResult(code);
		const std::string & address = this->GetAddress();
#ifdef ONLY_MAIN_THREAD
		this->mTcpComponent->OnConnectAfter(address, code);
#else
		MainTaskScheduler &taskScheduler = App::Get()->GetTaskScheduler();
		taskScheduler.Invoke(&RpcClientComponent::OnConnectAfter, this->mTcpComponent, address, code);
#endif
	}
}