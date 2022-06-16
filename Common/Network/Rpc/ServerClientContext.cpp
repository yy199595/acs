#include"ServerClientContext.h"
#include"App/App.h"
#include"Network/Rpc.h"
#include"Network/Proto/RpcProtoMessage.h"
#include<Component/Rpc/RpcClientComponent.h>

#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
namespace Sentry
{
	ServerClientContext::ServerClientContext(RpcClientComponent* component,
		std::shared_ptr<SocketProxy> socket)
		: TcpContext(socket), mTcpComponent(component)
	{
		this->mConnectCount = 0;
		this->SetBufferCount(1024 * 2, 1024 * 20);
		this->mConnectLock = std::make_shared<CoroutineLock>();
	}

	void ServerClientContext::StartClose()
	{
#ifdef ONLY_MAIN_THREAD
		this->CloseSocket(XCode::NetActiveShutdown);
#else
		this->mNetworkThread.Invoke(&ServerClientContext::CloseSocket, this, XCode::NetActiveShutdown);
#endif
	}

	void ServerClientContext::SendToServer(std::shared_ptr<com::Rpc_Response> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> responseMessage
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>(MESSAGE_TYPE::MSG_RPC_RESPONSE, message);
#ifdef ONLY_MAIN_THREAD
		this->Send(responseMessage);
#else
		this->mNetworkThread.Invoke(&ServerClientContext::Send, this, responseMessage);
#endif
	}

	void ServerClientContext::SendToServer(std::shared_ptr<com::Rpc_Request> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> requestMessage
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>(MESSAGE_TYPE::MSG_RPC_REQUEST,message);
#ifdef ONLY_MAIN_THREAD
		this->Send(requestMessage);
#else
		this->mNetworkThread.Invoke(&ServerClientContext::Send, this, requestMessage);
#endif
	}

	void ServerClientContext::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
	{

	}

	void ServerClientContext::CloseSocket(XCode code)
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

    void ServerClientContext::OnReceiveHead(const asio::error_code &code, const char *message, size_t size)
    {
        if(code || message == nullptr || size == 0)
        {
            this->CloseSocket(XCode::NetWorkError);
            return;
        }
        size_t lenght = 0;
        memcpy(&lenght, message, size);
        this->ReceiveBody(lenght);
    }

    void ServerClientContext::OnReceiveBody(const asio::error_code &code, const char *message, size_t size)
    {
        if (code || message == nullptr || size == 0)
        {
#ifdef __NET_ERROR_LOG__
            CONSOLE_LOG_ERROR(code.message());
#endif
            this->CloseSocket(XCode::NetWorkError);
            return;
        }
        switch ((MESSAGE_TYPE)message[0])
        {
            case MESSAGE_TYPE::MSG_RPC_REQUEST:
                this->ReceiveHead(sizeof(int));
                this->OnRequest(message + 1, size - 1);
                break;
            case MESSAGE_TYPE::MSG_RPC_RESPONSE:
                this->ReceiveHead(sizeof(int));
                this->OnResponse(message + 1, size - 1);
                break;
        }
    }

	bool ServerClientContext::OnRequest(const char* buffer, size_t size)
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

	bool ServerClientContext::OnResponse(const char* buffer, size_t size)
	{
		std::shared_ptr<com::Rpc_Response> responseData(new com::Rpc_Response());
		if (!responseData->ParseFromArray(buffer, size))
		{
#ifdef __NET_ERROR_LOG__
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

	bool ServerClientContext::StartConnectAsync()
	{
		AutoCoroutineLock lock(this->mConnectLock);
		if(this->mSocket->IsOpen())
		{
			return true;
		}
#ifdef ONLY_MAIN_THREAD
		this->Connect();
#else
		this->mNetworkThread.Invoke(&ServerClientContext::Connect, this);
#endif
		this->mConnectTask = std::make_shared<TaskSource<XCode>>();
		return mConnectTask->Await() == XCode::Successful;
	}

	void ServerClientContext::StartReceive()
	{
        size_t size = sizeof(int);
#ifdef ONLY_MAIN_THREAD
		this->ReceiveHead(size);
#else
		this->mNetworkThread.Invoke(&ServerClientContext::ReceiveHead, this, size);
#endif
	}

	void ServerClientContext::OnConnect(const asio::error_code& error)
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
		this->ReceiveHead(sizeof(int));
		this->mConnectTask->SetResult(code);
	}
}