#include"TcpRpcClientContext.h"
#include"Component/ClientComponent.h"
#include"Network/Proto/RpcProtoMessage.h"
namespace Client
{
	TcpRpcClientContext::TcpRpcClientContext(std::shared_ptr<SocketProxy> socket, ClientComponent * component)
        : Tcp::TcpContext(socket)
	{
		this->mClientComponent = component;
        this->mState = Tcp::DecodeState::Head;
    }

	void TcpRpcClientContext::SendToServer(std::shared_ptr<c2s::rpc::request> request)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> networkData =
                std::make_shared<Tcp::Rpc::RpcProtoMessage>();

        networkData->mMessage = request;
        networkData->mType = Tcp::Type ::Request;
#ifdef ONLY_MAIN_THREAD
        this->Send(networkData);
#else
        asio::io_service & io = this->mSocket->GetThread();
        io.post(std::bind(&TcpRpcClientContext::Send, this, networkData));
#endif
	}

    void TcpRpcClientContext::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
    {
        if(code)
        {
            CONSOLE_LOG_DEBUG(code.message());
            if(!this->ConnectSync())
            {
                CONSOLE_LOG_FATAL("connect server error");
                return;
            }
            this->SendFromMessageQueue();
            this->ReceiveMessage(RPC_PACK_HEAD_LEN);
        }
        else
        {
            this->PopMessage();
            this->SendFromMessageQueue();
        }
    }

    void TcpRpcClientContext::OnReceiveMessage(const asio::error_code &code, std::istream & readStream, size_t)
    {
        if (code)
        {
#ifdef __DEBUG__
            CONSOLE_LOG_ERROR(code.message());
#endif
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
                this->mMessage->DecodeBody(readStream);
                const std::string & address = this->mSocket->GetAddress();
#ifdef ONLY_MAIN_THREAD
                this->mClientComponent->OnMessage(address, std::move(this->mMessage));
#else
                asio::io_service & io = App::Get()->GetThread();
                io.post(std::bind(&ClientComponent::OnMessage,
                                  this->mClientComponent, address, std::move(this->mMessage)));
#endif
            }
                break;
        }
        this->ReceiveMessage(RPC_PACK_HEAD_LEN);
    }
}