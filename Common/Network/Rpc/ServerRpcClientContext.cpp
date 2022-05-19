#include"ServerRpcClientContext.h"
#include"App/App.h"
#include"Network/Rpc.h"
#include"Network/Proto/RpcProtoMessage.h"
#include<Component/Rpc/RpcClientComponent.h>

#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
namespace Sentry
{
	ServerRpcClientContext::ServerRpcClientContext(RpcClientComponent* component,
		std::shared_ptr<SocketProxy> socket)
		: TcpContext(socket), mTcpComponent(component)
	{
		this->mConnectCount = 0;
		this->SetBufferCount(1024 * 2, 1024 * 20);
		this->mConnectLock = std::make_shared<CoroutineLock>();
	}

	void ServerRpcClientContext::StartClose()
	{
#ifdef ONLY_MAIN_THREAD
		this->CloseSocket(XCode::NetActiveShutdown);
#else
		this->mNetworkThread.Invoke(&ServerRpcClientContext::CloseSocket, this, XCode::NetActiveShutdown);
#endif
	}

	void ServerRpcClientContext::SendToServer(std::shared_ptr<com::Rpc_Response> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> responseMessage
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>(RPC_TYPE::RPC_TYPE_RESPONSE, message);
#ifdef ONLY_MAIN_THREAD
		this->SendData(networkData);
#else
		this->mNetworkThread.Invoke(&ServerRpcClientContext::Send, this, responseMessage);
#endif
	}

	void ServerRpcClientContext::SendToServer(std::shared_ptr<com::Rpc_Request> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> requestMessage
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>(RPC_TYPE::RPC_TYPE_REQUEST,message);
#ifdef ONLY_MAIN_THREAD
		this->Send(requestMessage);
#else
		this->mNetworkThread.Invoke(&ServerRpcClientContext::Send, this, requestMessage);
#endif
	}

	void ServerRpcClientContext::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
	{

	}

	void ServerRpcClientContext::CloseSocket(XCode code)
	{
		if (code == XCode::NetActiveShutdown) //主动关闭不需要通知回主线
		{
			this->mSocket->Close();
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

	bool ServerRpcClientContext::OnRecvMessage(const asio::error_code& code, const char* message, size_t size)
	{
		if (code || message == nullptr || size == 0)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(code.message());
#endif
			this->CloseSocket(XCode::NetWorkError);
			return false;
		}
		switch ((RPC_TYPE)message[0])
		{
		case RPC_TYPE::RPC_TYPE_REQUEST:
			return this->OnRequest(message + 1, size - 1);
		case RPC_TYPE::RPC_TYPE_RESPONSE:
			return this->OnResponse(message + 1, size - 1);
		}
		return false;
	}

	bool ServerRpcClientContext::OnRequest(const char* buffer, size_t size)
	{
		std::shared_ptr<com::Rpc_Request> requestData(new com::Rpc_Request());
		if (!requestData->ParseFromArray(buffer, size))
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR("parse server request message error");
#endif
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
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR("parse server response message error");
#endif
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
		if(this->mSocket->IsOpen())
		{
			return true;
		}
#ifdef ONLY_MAIN_THREAD
		this->Connect();
#else
		this->mNetworkThread.Invoke(&ServerRpcClientContext::Connect, this);
#endif
		this->mConnectTask = std::make_shared<TaskSource<XCode>>();
		return mConnectTask->Await() == XCode::Successful;
	}

	void ServerRpcClientContext::StartReceive()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveHead();
#else
		this->mNetworkThread.Invoke(&ServerRpcClientContext::ReceiveHead, this);
#endif
	}

	void ServerRpcClientContext::OnConnect(const asio::error_code& error)
	{
		this->mConnectCount++;
		XCode code = XCode::Successful;
		assert(this->mConnectTask);
		if(error)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(error.message());
#endif
			code = XCode::Failure;
		}
		this->ReceiveHead();
		this->mConnectTask->SetResult(code);
	}
}