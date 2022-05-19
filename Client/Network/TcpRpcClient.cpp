#include"TcpRpcClient.h"
#include"Component/ClientComponent.h"
#include"Network/Proto/RpcProtoMessage.h"
namespace Client
{
	TcpRpcClient::TcpRpcClient(std::shared_ptr<SocketProxy> socket, ClientComponent * component)
        : Tcp::TcpContext(socket)
	{
		this->mClientComponent = component;
		this->SetBufferCount(2048, 2048);
	}

	void TcpRpcClient::SendToServer(std::shared_ptr<c2s::Rpc_Request> request)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> networkData =
			std::make_shared<Tcp::Rpc::RpcProtoMessage>(RPC_TYPE::RPC_TYPE_CLIENT_REQUEST, request);
#ifdef ONLY_MAIN_THREAD
		this->Send(networkData);
#else
		this->mNetworkThread.Invoke(&TcpRpcClient::Send, this, networkData);
#endif
	}


    std::shared_ptr<TaskSource<bool>> TcpRpcClient::ConnectAsync()
    {
		this->mConnectTask = std::make_shared<TaskSource<bool>>();
#ifdef ONLY_MAIN_THREAD
		this->Connect();
#else
		this->mNetworkThread.Invoke(&TcpRpcClient::Connect, this);
#endif
        return this->mConnectTask;
    }

    void TcpRpcClient::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
    {

    }

    void TcpRpcClient::OnConnect(const asio::error_code& error)
    {
		if(error)
		{
			this->mConnectTask->SetResult(false);
			return;
		}
		this->mConnectTask->SetResult(true);
    }

	bool TcpRpcClient::OnRecvMessage(const asio::error_code& code, const char* message, size_t size)
	{
		if(code)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(code.message());
#endif
			return false;
		}
		size_t length = size - 1;
		const char * str = message + 1;
		switch((RPC_TYPE)message[0])
		{
		case RPC_TYPE::RPC_TYPE_REQUEST:
			return this->OnRequest(str, length);
		case RPC_TYPE::RPC_TYPE_RESPONSE:
			return this->OnResponse(str, length);
		case RPC_TYPE::RPC_TYPE_CALL_CLIENT:
			return this->OnCall(str, length);
		}
		return false;
	}

	bool TcpRpcClient::OnCall(const char* buffer, size_t size)
	{
		return true;
	}

	void TcpRpcClient::StartReceive()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveHead();
#else
		this->mNetworkThread.Invoke(&TcpRpcClient::ReceiveHead, this);
#endif
	}

	bool TcpRpcClient::OnRequest(const char * buffer, size_t size)
    {
        std::shared_ptr<c2s::Rpc_Request> request(new c2s::Rpc_Request());
        if (!request->ParseFromArray(buffer, size))
        {
            return false;
        }
        this->mClientComponent->OnRequest(request);
        return true;
    }

	bool TcpRpcClient::OnResponse(const char * buffer, size_t size)
    {
        std::shared_ptr<c2s::Rpc_Response> response(new c2s::Rpc_Response());
        if (!response->ParseFromArray(buffer, size))
        {
            return false;
        }
        this->mClientComponent->OnResponse(response);
        return true;
    }

}