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
		if(code && this->mSocket->IsRemote())
		{
			this->Connect();
			this->mSendQueues.emplace(message);
		}
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

    void ServerClientContext::OnReceiveMessage(const asio::error_code &code, asio::streambuf &buffer)
    {
        if (code || buffer.size() == 0)
        {
			CONSOLE_LOG_ERROR(code.message() << " " << buffer.size());
            this->CloseSocket(XCode::NetWorkError);
            return;
        }
        if (this->mReadState == ReadType::HEAD)
        {
            this->mReadState = ReadType::BODY;
            this->ReceiveMessage(this->GetLength(buffer));
        }
        else if (this->mReadState == ReadType::BODY)
        {
			std::iostream os(&buffer);
			assert(buffer.size() > 0);
            this->mReadState = ReadType::HEAD;
            switch ((MESSAGE_TYPE) os.get())
            {
                case MESSAGE_TYPE::MSG_RPC_REQUEST:
                    this->OnRequest(os);
                    this->ReceiveMessage(sizeof(int));
                    break;
                case MESSAGE_TYPE::MSG_RPC_RESPONSE:
                    this->OnResponse(os);
                    this->ReceiveMessage(sizeof(int));
                    break;
            }
        }
    }

	bool ServerClientContext::OnRequest(std::iostream & os)
	{
		std::shared_ptr<com::Rpc_Request> requestData(new com::Rpc_Request());
		if (!requestData->ParseFromIstream(&os))
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

	bool ServerClientContext::OnResponse(std::iostream & os)
	{
		std::shared_ptr<com::Rpc_Response> responseData(new com::Rpc_Response());
		if (!responseData->ParseFromIstream(&os))
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

	void ServerClientContext::StartReceive()
	{
        size_t size = sizeof(int);
        this->mReadState = ReadType::HEAD;
#ifdef ONLY_MAIN_THREAD
		this->ReceiveMessage(size);
#else
		this->mNetworkThread.Invoke(&ServerClientContext::ReceiveMessage, this, size);
#endif
	}

	void ServerClientContext::OnConnect(const asio::error_code& error, int count)
	{
		if(error)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(error.message());
#endif
			AsioContext & context = this->mSocket->GetThread();
			this->mTimer = std::make_shared<asio::steady_timer>(context, std::chrono::seconds(5));
			this->mTimer->async_wait(std::bind(std::bind(&ServerClientContext::Connect, this->shared_from_this())));
			return;
		}
        this->mReadState = ReadType::HEAD;
        this->ReceiveMessage(sizeof(int));
		while(!this->mSendQueues.empty())
		{
			this->Send(this->mSendQueues.front());
			this->mSendQueues.pop();
		}
	}
}