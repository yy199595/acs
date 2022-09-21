#include"InnerNetClient.h"
#include"App/App.h"
#include"Client/Rpc.h"
#include"Message/RpcProtoMessage.h"
#include"Component/InnerNetComponent.h"

#ifdef __DEBUG__
#include"google/protobuf/util/json_util.h"
#endif

namespace Sentry
{
	InnerNetClient::InnerNetClient(InnerNetComponent* component,
                                   std::shared_ptr<SocketProxy> socket)
		: TcpContext(socket, 1024 * 1024), mTcpComponent(component)
	{
        this->mState = Tcp::DecodeState::Head;
	}

	void InnerNetClient::StartClose()
	{
#ifdef ONLY_MAIN_THREAD
		this->CloseSocket(XCode::NetActiveShutdown);
#else
        asio::io_service & t = this->mSocket->GetThread();
		t.post(std::bind(&InnerNetClient::CloseSocket, this, XCode::NetActiveShutdown));
#endif
	}

	void InnerNetClient::SendToServer(std::shared_ptr<com::rpc::response> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> response
            = std::make_shared<Tcp::Rpc::RpcProtoMessage>();

        response->mMessage = message;
        response->mType = Tcp::Type::Response;
#ifdef ONLY_MAIN_THREAD
		this->Send(response);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&InnerNetClient::Send, this, response));
#endif
	}

	void InnerNetClient::SendToServer(std::shared_ptr<com::rpc::request> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> request
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>();

        request->mMessage = message;
        request->mType = Tcp::Type::Request;
#ifdef ONLY_MAIN_THREAD
		this->Send(request);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&InnerNetClient::Send, this, request));
#endif
	}

	void InnerNetClient::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
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

	void InnerNetClient::CloseSocket(XCode code)
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
		taskScheduler.post(std::bind(&InnerNetComponent::OnCloseSocket, this->mTcpComponent, address, code));
#endif
	}

    void InnerNetClient::OnReceiveMessage(const asio::error_code &code, std::istream & readStream, size_t size)
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
                this->mMessage = std::make_shared<Tcp::BinMessage>();
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
                this->ReceiveMessage(RPC_PACK_HEAD_LEN);
                const std::string & address = this->mSocket->GetAddress();
#ifdef ONLY_MAIN_THREAD
                this->mTcpComponent->OnMessage(address, std::move(this->mMessage));
#else
                asio::io_service & io = App::Get()->GetThread();
                io.post(std::bind(&InnerNetComponent::OnMessage,
                                  this->mTcpComponent, address, std::move(this->mMessage)));
#endif
            }
                break;
        }
    }

	void InnerNetClient::StartReceive()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveMessage(PackeHeadtLength);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&InnerNetClient::ReceiveMessage, this, RPC_PACK_HEAD_LEN));
#endif
	}

	void InnerNetClient::OnConnect(const asio::error_code& error, int count)
	{
		if(error)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(error.message());
#endif
			AsioContext & context = this->mSocket->GetThread();
			this->mTimer = std::make_shared<asio::steady_timer>(context, std::chrono::seconds(5));
			this->mTimer->async_wait(std::bind(std::bind(&InnerNetClient::Connect, this->shared_from_this())));
			return;
		}
        this->SendFromMessageQueue();
        this->ReceiveMessage(RPC_PACK_HEAD_LEN);
    }
}