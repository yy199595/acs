#include"TcpRpcClientContext.h"
#include"Component/ClientComponent.h"
#include"Network/Proto/RpcProtoMessage.h"
namespace Client
{
	TcpRpcClientContext::TcpRpcClientContext(std::shared_ptr<SocketProxy> socket, ClientComponent * component)
        : Tcp::TcpContext(socket)
	{
		this->mClientComponent = component;
	}

	void TcpRpcClientContext::SendToServer(std::shared_ptr<c2s::rpc::request> request)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> networkData =
                std::make_shared<Tcp::Rpc::RpcProtoMessage>(MESSAGE_TYPE::MSG_RPC_CLIENT_REQUEST, request);
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
            this->ReceiveLength();
        }
        this->SendFromMessageQueue();
    }

    void TcpRpcClientContext::OnReceiveLength(const asio::error_code &code, int length)
    {
        if (code || length <= 0)
        {
#ifdef __DEBUG__
            CONSOLE_LOG_ERROR(code.message());
#endif
            return;
        }
        this->ReceiveMessage(length);
    }

    void TcpRpcClientContext::OnReceiveMessage(const asio::error_code &code, std::istream & readStream)
    {
        if (code)
        {
#ifdef __DEBUG__
            CONSOLE_LOG_ERROR(code.message());
#endif
            return;
        }
        switch ((MESSAGE_TYPE) readStream.get())
        {
            case MESSAGE_TYPE::MSG_RPC_CALL_CLIENT:
                this->OnRequest(readStream);
                break;
            case MESSAGE_TYPE::MSG_RPC_RESPONSE:
                this->OnResponse(readStream);
                break;
        }
        this->ReceiveLength();
    }

	bool TcpRpcClientContext::OnRequest(std::istream & istream1)
    {
        std::shared_ptr<c2s::rpc::call> request(new c2s::rpc::call());
        if (!request->ParseFromIstream(&istream1))
        {
			CONSOLE_LOG_ERROR("parse request message error");
            return false;
        }
#ifdef ONLY_MAIN_THREAD
        this->mClientComponent->OnRequest(request);
#else
        asio::io_service & io = App::Get()->GetThread();
        io.post(std::bind(&ClientComponent::OnRequest, this->mClientComponent, request));
#endif
        return true;
    }

	bool TcpRpcClientContext::OnResponse(std::istream & istream1)
    {
        std::shared_ptr<c2s::rpc::response> response(new c2s::rpc::response());
        if (!response->ParseFromIstream(&istream1))
        {
			CONSOLE_LOG_ERROR("parse response message error");
			return false;
        }
        long long taskId = response->rpc_id();
#ifdef ONLY_MAIN_THREAD
        this->mClientComponent->OnResponse(taskId, response);
#else
        asio::io_service & io = App::Get()->GetThread();
        io.post(std::bind(&ClientComponent::OnResponse, this->mClientComponent, taskId, response));
#endif
        return true;
    }

}