#include"MessageRpcClient.h"
#include"App/App.h"
#include"Network/Rpc.h"
#include"Network/Proto/RpcProtoMessage.h"
#include<Component/Rpc/RpcServerComponent.h>

#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
namespace Sentry
{
	MessageRpcClient::MessageRpcClient(RpcServerComponent* component,
                                       std::shared_ptr<SocketProxy> socket)
		: TcpContext(socket, 1024 * 1024), mTcpComponent(component)
	{
        this->mState = Tcp::DecodeState::Head;
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
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> response
            = std::make_shared<Tcp::Rpc::RpcProtoMessage>();

        response->mMessage = message;
        response->mType = MESSAGE_TYPE::MSG_RPC_RESPONSE;
#ifdef ONLY_MAIN_THREAD
		this->Send(response);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&MessageRpcClient::Send, this, response));
#endif
	}

	void MessageRpcClient::SendToServer(std::shared_ptr<com::rpc::request> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> request
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>();

        request->mMessage = message;
        request->mType = MESSAGE_TYPE::MSG_RPC_REQUEST;
#ifdef ONLY_MAIN_THREAD
		this->Send(request);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&MessageRpcClient::Send, this, request));
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
		taskScheduler.post(std::bind(&RpcServerComponent::OnCloseSocket, this->mTcpComponent, address, code));
#endif
	}

    void MessageRpcClient::OnReceiveMessage(const asio::error_code &code, std::istream & readStream, size_t size)
    {
        if (code)
        {
            CONSOLE_LOG_ERROR(code.message());
            this->CloseSocket(XCode::NetWorkError);
            return;
        }
        switch(this->mState)
        {
            case Tcp::DecodeState::Head:
            {
                this->mState = Tcp::DecodeState::Body;
                this->mMessage = std::make_shared<Tcp::RpcMessage>();
                int len = this->mMessage->DecodeHead(readStream);
                this->ReceiveMessage(len);
            }
                break;
            case Tcp::DecodeState::Body:
            {
                this->mState = Tcp::DecodeState::Head;
                if (!this->mMessage->DecodeBody(readStream))
                {
                    this->CloseSocket(XCode::UnKnowPacket);
                    return;
                }
                this->OnDecodeComplete();
                this->ReceiveMessage(sizeof(int) + 2);
            }
                break;
        }
    }

    void MessageRpcClient::OnDecodeComplete()
    {
        const std::string & address = this->mSocket->GetAddress();
#ifdef ONLY_MAIN_THREAD
        this->mTcpComponent->OnMessage(address, std::move(this->mMessage));
#else
        asio::io_service & taskScheduler = App::Get()->GetThread();
        taskScheduler.post(std::bind(&RpcServerComponent::OnMessage, mTcpComponent, address, std::move(this->mMessage)));
#endif
    }

	void MessageRpcClient::StartReceive()
	{
        int len = sizeof(int) + 2;
#ifdef ONLY_MAIN_THREAD
		this->ReceiveMessage(len);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&MessageRpcClient::ReceiveMessage, this, len));
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
        this->SendFromMessageQueue();
        this->ReceiveMessage(sizeof(int) + 2);
    }
}