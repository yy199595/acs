#include"TcpRpcClient.h"
#include"Component/ClientComponent.h"
#include<iostream>
constexpr size_t HeadCount = sizeof(char) + sizeof(int);
namespace Client
{
	TcpRpcClient::TcpRpcClient(std::shared_ptr<SocketProxy> socket, ClientComponent * component)
        : RpcClient(socket, SocketType::LocalSocket)
	{
		this->mClientComponent = component;
	}

    std::shared_ptr<TaskSource<bool>> TcpRpcClient::SendToGate(std::shared_ptr<c2s::Rpc_Request> request)
    {
        this->mSendTask = make_shared<TaskSource<bool>>();
        this->SendData(RPC_TYPE_REQUEST, request);
        return this->mSendTask;
    }

    void TcpRpcClient::OnClientError(XCode code)
    {

    }

    std::shared_ptr<TaskSource<bool>> TcpRpcClient::ConnectAsync(const std::string &ip, unsigned short port)
    {
        if(!this->StartConnect(ip, port))
        {
            return nullptr;
        }
        this->mConnectTask = std::make_shared<TaskSource<bool>>();
        return this->mConnectTask;
    }

    void TcpRpcClient::OnSendData(XCode code, std::shared_ptr<Message>)
    {
        if(this->mSendTask)
        {
            this->mSendTask->SetResult(code == XCode::Successful);
        }
    }

    void TcpRpcClient::OnConnect(XCode code)
    {
        if(this->mConnectTask)
        {
            this->mConnectTask->SetResult(code == XCode::Successful);
            std::move(this->mConnectTask);
        }
    }

	XCode TcpRpcClient::OnRequest(const char * buffer, size_t size)
    {
        std::shared_ptr<c2s::Rpc_Request> request(new c2s::Rpc_Request());
        if (!request->ParseFromArray(buffer, size))
        {
            return XCode::ParseMessageError;
        }
        this->mClientComponent->OnRequest(request);
        return XCode::Successful;
    }

	XCode TcpRpcClient::OnResponse(const char * buffer, size_t size)
    {
        std::shared_ptr<c2s::Rpc_Response> response(new c2s::Rpc_Response());
        if (!response->ParseFromArray(buffer, size))
        {
            return XCode::ParseMessageError;
        }
        this->mClientComponent->OnResponse(response);
        return XCode::Successful;
    }

}