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
        asio::io_service & t = this->mSocket->GetThread();
		t.post(std::bind(&ServerClientContext::CloseSocket, this, XCode::NetActiveShutdown));
#endif
	}

	void ServerClientContext::SendToServer(std::shared_ptr<com::Rpc_Response> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> responseMessage
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>(MESSAGE_TYPE::MSG_RPC_RESPONSE, message);
#ifdef ONLY_MAIN_THREAD
		this->Send(responseMessage);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&ServerClientContext::Send, this, responseMessage));
#endif
	}

	void ServerClientContext::SendToServer(std::shared_ptr<com::Rpc_Request> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> requestMessage
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>(MESSAGE_TYPE::MSG_RPC_REQUEST,message);
#ifdef ONLY_MAIN_THREAD
		this->Send(requestMessage);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&ServerClientContext::Send, this, requestMessage));
#endif
	}

	void ServerClientContext::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
	{
		if(code && this->mSocket->IsRemote())
		{
			this->Connect();
		}
        else
        {
            this->SendFromMessageQueue();
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
		asio::io_service & taskScheduler = App::Get()->GetThread();
		taskScheduler.post(std::bind(&RpcClientComponent::OnCloseSocket, this->mTcpComponent, address, code));
#endif
	}

    void ServerClientContext::OnReceiveLength(const asio::error_code &code, int length)
    {
        if(code || length <= 0)
        {
            CONSOLE_LOG_ERROR(code.message());
            this->CloseSocket(XCode::NetWorkError);
            return;
        }
        this->ReceiveMessage(length);
    }

    void ServerClientContext::OnReceiveMessage(const asio::error_code &code, std::istream & readStream)
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

	bool ServerClientContext::OnRequest(std::istream & istream1)
	{
		std::shared_ptr<com::Rpc_Request> requestData(new com::Rpc_Request());
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

	bool ServerClientContext::OnResponse(std::istream & istream1)
	{
		std::shared_ptr<com::Rpc_Response> responseData(new com::Rpc_Response());
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

	void ServerClientContext::StartReceive()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveLength();
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&ServerClientContext::ReceiveLength, this));
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
        this->ReceiveLength();
        this->SendFromMessageQueue();
	}
}