#include"TcpRpcClient.h"
#include"Object/App.h"
#include"ClientComponent.h"
#include"Scene/ThreadPoolComponent.h"
#include"Coroutine/TaskComponent.h"
#include<iostream>
constexpr size_t HeadCount = sizeof(char) + sizeof(int);
namespace Client
{
	TcpRpcClient::TcpRpcClient(SocketProxy * socket, ClientComponent * component)
		: RpcClient(socket, SocketType::LocalSocket)
	{
		this->mClientComponent = component;
	}

    void TcpRpcClient::OnSendData(XCode code, const Message *)
    {

    }


	bool TcpRpcClient::StartSendProtoData(const c2s::Rpc_Request * request)
	{
		if (!this->mSocketProxy->IsOpen())
		{
			return false;
		}
		if (this->mNetWorkThread.IsCurrentThread())
		{
			this->SendProtoData(request);
			return true;
		}
		this->mNetWorkThread.Invoke(&TcpRpcClient::SendProtoData, this, request);
		return true;
	}

    std::shared_ptr<TaskSource<bool>> TcpRpcClient::ConnectAsync(const std::string &ip, unsigned short port)
    {
        auto address = asio::ip::make_address_v4(ip);
        asio::ip::tcp::endpoint endPoint(address, port);
        AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();
        std::shared_ptr<TaskSource<bool>> connectTask = std::make_shared<TaskSource<bool>>();
        LOG_DEBUG(this->mSocketProxy->GetName() << " start connect " << this->GetAddress());
        nSocket.async_connect(endPoint, [this, connectTask](const asio::error_code &err)
        {
            if (err)
            {
                connectTask->SetResult(false);
                std::cout << "connect error : " << err.message() << std::endl;
                return;
            }
            connectTask->SetResult(true);
        });
        return connectTask;
    }

	void TcpRpcClient::OnClose(XCode code)
    {
        long long id = this->GetSocketId();
        MainTaskScheduler &taskScheduller = App::Get().GetTaskScheduler();
        taskScheduller.Invoke(&ClientComponent::OnCloseSocket, this->mClientComponent, id, code);
    }

	XCode TcpRpcClient::OnRequest(const char * buffer, size_t size)
	{
		 auto request = new c2s::Rpc_Request();
		if (!request->ParseFromArray(buffer, size))
		{
			return XCode::ParseMessageError;
		}
		MainTaskScheduler & taskScheduller = App::Get().GetTaskScheduler();
		taskScheduller.Invoke(&ClientComponent::OnRequest, this->mClientComponent, request);
		return XCode::Successful;
	}

	XCode TcpRpcClient::OnResponse(const char * buffer, size_t size)
	{
		 auto response = new c2s::Rpc_Response();
		if (!response->ParseFromArray(buffer, size))
		{
			return XCode::ParseMessageError;
		}
		MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&ClientComponent::OnResponse, this->mClientComponent, response);
		return XCode();
	}

	void TcpRpcClient::SendProtoData(const c2s::Rpc_Request * request)
	{
        this->SendData(RPC_TYPE_REQUEST, request);
	}
}