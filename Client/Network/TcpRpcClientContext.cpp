#include"TcpRpcClientContext.h"
#include"Component/ClientComponent.h"
#include"Network/Proto/RpcProtoMessage.h"
namespace Client
{
	TcpRpcClientContext::TcpRpcClientContext(std::shared_ptr<SocketProxy> socket, ClientComponent * component)
        : Tcp::TcpContext(socket)
	{
		this->mClientComponent = component;
		this->SetBufferCount(2048, 2048);
	}

	void TcpRpcClientContext::SendToServer(std::shared_ptr<c2s::Rpc_Request> request)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> networkData =
			std::make_shared<Tcp::Rpc::RpcProtoMessage>(RPC_TYPE::RPC_TYPE_CLIENT_REQUEST, request);
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

    void TcpRpcClientContext::OnConnect(const asio::error_code& error)
    {
		if(error)
		{
			this->mConnectTask->SetResult(false);
			return;
		}
		this->mConnectTask->SetResult(true);
    }

	bool TcpRpcClientContext::OnRecvMessage(const asio::error_code& code, const char* message, size_t size)
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
		case RPC_TYPE::RPC_TYPE_CALL_CLIENT:
			return this->OnRequest(str, length);
		case RPC_TYPE::RPC_TYPE_RESPONSE:
			return this->OnResponse(str, length);
		}
		CONSOLE_LOG_FATAL("unknow message type");
		return false;
	}

	void TcpRpcClientContext::StartReceive()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveHead();
#else
		this->mNetworkThread.Invoke(&TcpRpcClientContext::ReceiveHead, this);
#endif
	}

	bool TcpRpcClientContext::OnRequest(const char * buffer, size_t size)
    {
        std::shared_ptr<c2s::Rpc::Call> request(new c2s::Rpc::Call());
        if (!request->ParseFromArray(buffer, size))
        {
			CONSOLE_LOG_ERROR("parse request message error");
            return false;
        }
        this->mClientComponent->OnRequest(request);
        return true;
    }

	bool TcpRpcClientContext::OnResponse(const char * buffer, size_t size)
    {
        std::shared_ptr<c2s::Rpc::Response> response(new c2s::Rpc::Response());
        if (!response->ParseFromArray(buffer, size))
        {
			CONSOLE_LOG_ERROR("parse response message error");
			return false;
        }
        this->mClientComponent->OnResponse(response);
        return true;
    }

}