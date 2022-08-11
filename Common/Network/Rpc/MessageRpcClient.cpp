#include"MessageRpcClient.h"
#include"App/App.h"
#include"Network/Rpc.h"
#include"Network/Proto/RpcProtoMessage.h"
#include<Component/Rpc/RpcClientComponent.h>

#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
namespace Sentry
{
	MessageRpcClient::MessageRpcClient(RpcClientComponent* component,
		std::shared_ptr<SocketProxy> socket)
		: TcpContext(socket), mTcpComponent(component)
	{

	}

	void MessageRpcClient::StartClose()
	{
#ifdef ONLY_MAIN_THREAD
		this->CloseSocket(XCode::NetActiveShutdown);
#else
        asio::io_service & t = this->mSocket->GetThread();
		t.post(std::bind(&MessageRpcClient::CloseSocket, this, XCode::NetActiveShutdown));
#endif
	}

	void MessageRpcClient::SendToServer(std::shared_ptr<com::rpc::response> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> responseMessage
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>(MESSAGE_TYPE::MSG_RPC_RESPONSE, message);
#ifdef ONLY_MAIN_THREAD
		this->Send(responseMessage);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&MessageRpcClient::Send, this, responseMessage));
#endif
	}

	void MessageRpcClient::SendToServer(std::shared_ptr<com::rpc::request> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> requestMessage
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>(MESSAGE_TYPE::MSG_RPC_REQUEST,message);
#ifdef ONLY_MAIN_THREAD
		this->Send(requestMessage);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&MessageRpcClient::Send, this, requestMessage));
#endif
	}

	void MessageRpcClient::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
	{
		if(code && this->mSocket->IsRemote())
		{
			this->Connect();
		}
        else
        {
            this->PopMessage();
            this->SendFromMessageQueue();
        }
	}

	void MessageRpcClient::CloseSocket(XCode code)
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
		asio::io_service & taskScheduler = App::Get()->GetThread();
		taskScheduler.post(std::bind(&RpcClientComponent::OnCloseSocket, this->mTcpComponent, address, code));
#endif
	}

    void MessageRpcClient::OnReceiveLength(const asio::error_code &code, int length)
    {
        if(code || length <= 0)
        {
            CONSOLE_LOG_ERROR(code.message());
            this->CloseSocket(XCode::NetWorkError);
            return;
        }
        this->ReceiveMessage(length);
    }

    void MessageRpcClient::OnReceiveMessage(const asio::error_code &code, std::istream & readStream, size_t size)
    {
        if (code)
        {
            CONSOLE_LOG_ERROR(code.message());
            this->CloseSocket(XCode::NetWorkError);
            return;
        }
        switch ((MESSAGE_TYPE) readStream.get())
        {
            case MESSAGE_TYPE::MSG_RPC_REQUEST:
                this->OnRequest(readStream);
                break;
            case MESSAGE_TYPE::MSG_RPC_RESPONSE:
                this->OnResponse(readStream);
                break;
            default:
                CONSOLE_LOG_FATAL("unknow message type");
                return;
        }
        this->ReceiveLength();
    }

	bool MessageRpcClient::OnRequest(std::istream & istream1)
	{
		std::shared_ptr<com::rpc::request> requestData(new com::rpc::request());
		if (!requestData->ParsePartialFromIstream(&istream1))
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
		asio::io_service & taskScheduler = App::Get()->GetThread();
		taskScheduler.post(std::bind(&RpcClientComponent::OnRequest, mTcpComponent, requestData));
#endif

		return true;
	}

	bool MessageRpcClient::OnResponse(std::istream & istream1)
	{
		std::shared_ptr<com::rpc::response> responseData(new com::rpc::response());
		if (!responseData->ParseFromIstream(&istream1))
		{
#ifdef __NET_ERROR_LOG__
			CONSOLE_LOG_ERROR("parse server response message error");
#endif
			return false;
		}
#ifdef ONLY_MAIN_THREAD
		this->mTcpComponent->OnResponse(responseData);
#else
		asio::io_service & taskScheduler = App::Get()->GetThread();
		taskScheduler.post(std::bind(&RpcClientComponent::OnResponse, mTcpComponent, responseData));
#endif
		return true;
	}

	void MessageRpcClient::StartReceive()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveLength();
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&MessageRpcClient::ReceiveLength, this));
#endif
	}

	void MessageRpcClient::OnConnect(const asio::error_code& error, int count)
	{
		if(error)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(error.message());
#endif
			AsioContext & context = this->mSocket->GetThread();
			this->mTimer = std::make_shared<asio::steady_timer>(context, std::chrono::seconds(5));
			this->mTimer->async_wait(std::bind(std::bind(&MessageRpcClient::Connect, this->shared_from_this())));
			return;
		}
        this->ReceiveLength();
        this->SendFromMessageQueue();
	}
}