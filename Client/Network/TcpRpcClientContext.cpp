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

	void TcpRpcClientContext::SendToServer(std::shared_ptr<c2s::Rpc_Request> request)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> networkData =
			std::make_shared<Tcp::Rpc::RpcProtoMessage>(MESSAGE_TYPE::MSG_RPC_CLIENT_REQUEST, request);
#ifdef ONLY_MAIN_THREAD
		this->Send(networkData);
#else
		this->mNetworkThread.Invoke(&TcpRpcClientContext::Send, this, networkData);
#endif
	}


    std::shared_ptr<TaskSource<bool>> TcpRpcClientContext::ConnectAsync()
    {
		this->mConnectTask = std::make_shared<TaskSource<bool>>();
#ifdef ONLY_MAIN_THREAD
		this->Connect();
#else
		this->mNetworkThread.Invoke(&TcpRpcClientContext::Connect, this);
#endif
        return this->mConnectTask;
    }

    void TcpRpcClientContext::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
    {

    }

    void TcpRpcClientContext::OnConnect(const asio::error_code& error, int count)
    {
		if(error)
		{
			this->mConnectTask->SetResult(false);
			return;
		}
		this->mConnectTask->SetResult(true);
    }

    void TcpRpcClientContext::OnReceiveMessage(const asio::error_code &code, asio::streambuf &buffer)
    {
        if (code || buffer.size() == 0)
        {
#ifdef __DEBUG__
            CONSOLE_LOG_ERROR(code.message());
#endif
            return;
        }
        if(this->mReadState == ReadType::HEAD)
        {
            this->mReadState = ReadType::BODY;
            this->ReceiveMessage(this->GetLength(buffer));
        }
        else if(this->mReadState == ReadType::BODY)
        {
			std::iostream os(&buffer);
            this->mReadState = ReadType::HEAD;
            switch ((MESSAGE_TYPE)os.get())
            {
                case MESSAGE_TYPE::MSG_RPC_CALL_CLIENT:
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

	void TcpRpcClientContext::StartReceive()
	{
        this->mReadState = ReadType::HEAD;
#ifdef ONLY_MAIN_THREAD
		this->ReceiveMessage(sizeof(int));
#else
		this->mNetworkThread.Invoke(&TcpRpcClientContext::ReceiveMessage, this, sizeof(int));
#endif
	}

	bool TcpRpcClientContext::OnRequest(std::iostream & os)
    {
        std::shared_ptr<c2s::Rpc::Call> request(new c2s::Rpc::Call());
        if (!request->ParseFromIstream(&os))
        {
			CONSOLE_LOG_ERROR("parse request message error");
            return false;
        }
        this->mClientComponent->OnRequest(request);
        return true;
    }

	bool TcpRpcClientContext::OnResponse(std::iostream & os)
    {
        std::shared_ptr<c2s::Rpc::Response> response(new c2s::Rpc::Response());
        if (!response->ParseFromIstream(&os))
        {
			CONSOLE_LOG_ERROR("parse response message error");
			return false;
        }
        this->mClientComponent->OnResponse(response);
        return true;
    }

}