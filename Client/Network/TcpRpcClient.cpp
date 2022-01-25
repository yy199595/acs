#include"TcpRpcClient.h"
#include"Object/App.h"
#include"ClientComponent.h"
#include<iostream>
constexpr size_t HeadCount = sizeof(char) + sizeof(int);
namespace Client
{
	TcpRpcClient::TcpRpcClient(ClientComponent * component)
	{
		this->mClientComponent = component;
	}


    std::shared_ptr<TaskSource<bool>> TcpRpcClient::ConnectAsync(const std::string &ip, unsigned short port)
    {
        auto address = asio::ip::make_address_v4(ip);
        asio::ip::tcp::endpoint endPoint(address, port);
        MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        this->mTcpSocket = std::make_shared<AsioTcpSocket>(taskScheduler.GetContext());
        std::shared_ptr<TaskSource<bool>> connectTask = std::make_shared<TaskSource<bool>>();
        this->mTcpSocket->async_connect(endPoint, [this, connectTask](const asio::error_code &err)
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
	}
}