#include"TcpRpcClient.h"
#include"Component/ClientComponent.h"
#include<iostream>
constexpr size_t HeadCount = sizeof(char) + sizeof(int);
namespace Client
{
	TcpRpcClient::TcpRpcClient(std::shared_ptr<SocketProxy> socket, ClientComponent * component)
        : RpcClientContext(socket, SocketType::LocalSocket)
	{
		this->mClientComponent = component;
	}

    std::shared_ptr<TaskSource<bool>> TcpRpcClient::SendToGate(std::shared_ptr<c2s::Rpc_Request> request)
    {
        this->mSendTask = make_shared<TaskSource<bool>>();
        this->SendData(std::make_shared<NetworkData>(RPC_TYPE_REQUEST, request));
        return this->mSendTask;
    }

    void TcpRpcClient::OnClientError(XCode code)
    {

    }

    std::shared_ptr<TaskSource<bool>> TcpRpcClient::ConnectAsync()
    {
        if(!this->StartConnect())
        {
            return nullptr;
        }
        this->mConnectTask = std::make_shared<TaskSource<bool>>();
        return this->mConnectTask;
    }

    void TcpRpcClient::OnSendData(XCode code, std::shared_ptr<NetworkData> message)
    {
        std::move(this->mSendTask)->SetResult(code == XCode::Successful);
    }

    void TcpRpcClient::OnConnect(XCode code)
    {
        std::move(this->mConnectTask)->SetResult(code == XCode::Successful);
    }

	bool TcpRpcClient::OnReceiveMessage(char type, const char* buffer, size_t size)
	{
		switch(type)
		{
		case RPC_TYPE_REQUEST:
			return this->OnRequest(buffer, size);
		case RPC_TYPE_RESPONSE:
			return this->OnResponse(buffer, size);
		case RPC_TYPE_CALL_CLIENT:
			return this->OnCall(buffer, size);
		}
		return false;
	}

	bool TcpRpcClient::OnCall(const char* buffer, size_t size)
	{
		return true;
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