#include"TcpRpcClient.h"
#include"Core/App.h"
#include"ClientComponent.h"
#include"Scene/TaskPoolComponent.h"
#include"Coroutine/CoroutineComponent.h"
#include<iostream>
constexpr size_t HeadCount = sizeof(char) + sizeof(int);
namespace Client
{
	TcpRpcClient::TcpRpcClient(SocketProxy * socket, ClientComponent * component)
		: RpcClient(socket, SocketType::LocalSocket)
	{
		this->mCoroutineId = 0;
		this->mClientComponent = component;
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

	bool TcpRpcClient::AwaitConnect(const std::string & ip, unsigned short port)
	{
        this->mIsConnectSuccessful = false;
		auto address = asio::ip::make_address_v4(ip);
		asio::ip::tcp::endpoint endPoint(address, port);
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();
        CoroutineComponent * corComponent = App::Get().GetCorComponent();
		LOG_DEBUG(this->mSocketProxy->GetName() << " start connect " << this->GetAddress());
		nSocket.async_connect(endPoint, [this, corComponent](const asio::error_code &err)
		{
            this->mIsConnectSuccessful = true;
			if (err)
			{
                this->mIsConnectSuccessful = false;
                std::cout << "connect error : " << err.message() << std::endl;
			}
			MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
            taskScheduler.Invoke(&CoroutineComponent::Resume, corComponent, this->mCoroutineId);
		});
        corComponent->WaitForYield(this->mCoroutineId);
		return this->mIsConnectSuccessful;
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
		MainTaskScheduler & taskScheduller = App::Get().GetTaskScheduler();
		taskScheduller.Invoke(&ClientComponent::OnResponse, this->mClientComponent, response);
		return XCode();
	}

	void TcpRpcClient::SendProtoData(const c2s::Rpc_Request * request)
	{
		int size = request->ByteSize();
		char * buffer = new char[HeadCount + size];

		buffer[0] = RPC_TYPE_REQUEST;
		memcpy(buffer + sizeof(char), &size, sizeof(int));
		if (!request->SerializePartialToArray(buffer + HeadCount, 2048))
		{
			std::cerr << "serialize message failure";
			return;
		}
		this->AsyncSendMessage(buffer, HeadCount + size);
	}
}