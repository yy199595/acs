#include"ProtoRpcClient.h"
#include"Object/App.h"
#include<Rpc/RpcClientComponent.h>
#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
namespace Sentry
{
	ProtoRpcClient::ProtoRpcClient(RpcClientComponent *component,
                                   std::shared_ptr<SocketProxy> socket, SocketType type)
		:RpcClient(socket, type), mTcpComponent(component)
	{
        this->mConnectCount = 0;
	}

	void ProtoRpcClient::StartClose()
	{
#ifdef ONLY_MAIN_THREAD
      this->OnClientError(XCode::NetActiveShutdown);
#else
        this->mNetWorkThread.Invoke(&ProtoRpcClient::OnClientError, this, XCode::NetActiveShutdown);
#endif
	}

    void ProtoRpcClient::SendToServer(std::shared_ptr<com::Rpc_Response> message)
    {
        std::shared_ptr<NetworkData> networkData(
                new NetworkData(RPC_TYPE_RESPONSE, message));
#ifdef ONLY_MAIN_THREAD
        this->SendData(networkData);
#else
        this->mNetWorkThread.Invoke(&ProtoRpcClient::SendData, this, networkData);
#endif
    }

    void ProtoRpcClient::SendToServer(std::shared_ptr<com::Rpc_Request> message)
    {
        std::shared_ptr<NetworkData> networkData(
                new NetworkData(RPC_TYPE_REQUEST, message));
#ifdef ONLY_MAIN_THREAD
        this->SendData(networkData);
#else
        this->mNetWorkThread.Invoke(&ProtoRpcClient::SendData, this, networkData);
#endif
    }

    void ProtoRpcClient::OnSendData(XCode code, std::shared_ptr<NetworkData> message)
    {
        if(code != XCode::Successful)
        {
            long long id = this->GetSocketId();
            MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        }
    }

	void ProtoRpcClient::OnClientError(XCode code)
    {
        if (code == XCode::NetActiveShutdown) //主动关闭不需要通知回主线
        {
            this->mSocketProxy->Close();
            return;
        }
        long long id = this->GetSocketId();
#ifdef ONLY_MAIN_THREAD
        this->mTcpComponent->OnCloseSocket(id, code);
#else
        MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&RpcClientComponent::OnCloseSocket, this->mTcpComponent, id, code);
#endif
    }

	XCode ProtoRpcClient::OnRequest(const char * buffer, size_t size)
	{
        std::shared_ptr<com::Rpc_Request> requestData(new com::Rpc_Request());
        if (!requestData->ParseFromArray(buffer, size))
		{
			return XCode::ParseRequestDataError;
		}
        requestData->set_socket_id(this->GetSocketId());
#ifdef ONLY_MAIN_THREAD
    this->mTcpComponent->OnRequest(requestData);
#else
        MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&RpcClientComponent::OnRequest, mTcpComponent, requestData);
#endif

		return XCode::Successful;
	}

	XCode ProtoRpcClient::OnResponse(const char * buffer, size_t size)
	{
		std::shared_ptr<com::Rpc_Response> responseData(new com::Rpc_Response());
		if (!responseData->ParseFromArray(buffer, size))
		{
			return XCode::ParseResponseDataError;
		}
#ifdef ONLY_MAIN_THREAD
        this->mTcpComponent->OnResponse(responseData);
#else
        MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&RpcClientComponent::OnResponse, mTcpComponent, responseData);
#endif
		return XCode::Successful;
	}

    std::shared_ptr<TaskSource<bool>> ProtoRpcClient::ConnectAsync(const std::string &ip, unsigned short port)
    {
        if(!this->StartConnect(ip, port))
        {
            return nullptr;
        }
        this->mConnectTaskSource = std::make_shared<TaskSource<bool>>();
        return this->mConnectTaskSource;
    }

    void ProtoRpcClient::OnConnect(XCode code)
    {
        this->mConnectCount++;
        if(this->mConnectTaskSource != nullptr)
        {
            this->mConnectTaskSource->SetResult(code == XCode::Successful);
        }
        long long id = this->mSocketProxy->GetSocketId();
#ifdef ONLY_MAIN_THREAD
        this->mTcpComponent->OnConnectAfter(id, code);
#else
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&RpcClientComponent::OnConnectAfter, this->mTcpComponent, id, code);
#endif
    }
}