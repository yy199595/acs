#include"ProtoRpcClient.h"
#include<Core/App.h>
#include<Rpc/RpcClientComponent.h>
#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
namespace GameKeeper
{
	ProtoRpcClient::ProtoRpcClient(RpcClientComponent *component,
                                   std::shared_ptr<SocketProxy> socket, SocketType type)
		:RpcClient(socket, type), mTcpComponent(component)
	{
        this->mConnectCount = 0;
        this->mConnectTimer = nullptr;
	}

	void ProtoRpcClient::StartClose()
	{
		if (this->mNetWorkThread.IsCurrentThread())
		{
			this->CloseSocket(XCode::NetActiveShutdown);
			return;
		}
        this->mNetWorkThread.Invoke(&ProtoRpcClient::CloseSocket, this, XCode::NetActiveShutdown);
	}

    void ProtoRpcClient::SendToServer(std::shared_ptr<com::Rpc_Response> message)
    {
        if(this->mNetWorkThread.IsCurrentThread())
        {
            this->SendData(RPC_TYPE_RESPONSE, message);
            return;
        }
        this->mNetWorkThread.Invoke(&ProtoRpcClient::Send, this, RPC_TYPE_RESPONSE, message);
    }

    void ProtoRpcClient::SendToServer(std::shared_ptr<com::Rpc_Request> message)
    {
        if(this->mNetWorkThread.IsCurrentThread())
        {
            this->SendData(RPC_TYPE_REQUEST, message);
            return;
        }
        this->mNetWorkThread.Invoke(&ProtoRpcClient::Send, this, RPC_TYPE_REQUEST, message);
        return;
    }

    void ProtoRpcClient::Send(char type, std::shared_ptr<Message> message)
    {
        if(this->IsOpen())
        {
            this->SendData(type, message);
            return;
        }
        if(this->IsCanConnection())
        {
            this->ConnectInSecond(1);
        }
        this->mMessageQueue.emplace(message);
    }

    void ProtoRpcClient::OnSendData(XCode code, std::shared_ptr<Message> message)
    {
        if (code != XCode::Successful)
        {
            long long id = this->GetSocketId();
            MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
            taskScheduler.Invoke(&RpcClientComponent::OnSendFailure, this->mTcpComponent, id, message);
        }
        this->SendFromQueue();
    }

	void ProtoRpcClient::OnClose(XCode code)
	{
        if(this->GetSocketType() == SocketType::LocalSocket)
        {
            if(code == XCode::NetWorkError)
            {
                this->ConnectInSecond(3);
                return;
            }
        }
        long long id = this->GetSocketId();
		MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&RpcClientComponent::OnCloseSocket, this->mTcpComponent, id, code);
	}

	XCode ProtoRpcClient::OnRequest(const char * buffer, size_t size)
	{
        std::shared_ptr<com::Rpc_Request> requestData(new com::Rpc_Request());
        if (!requestData->ParseFromArray(buffer, size))
		{
			return XCode::ParseRequestDataError;
		}
        requestData->set_socket_id(this->GetSocketId());
		MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&RpcClientComponent::OnRequest, mTcpComponent, requestData);
		return XCode::Successful;
	}

	XCode ProtoRpcClient::OnResponse(const char * buffer, size_t size)
	{
		std::shared_ptr<com::Rpc_Response> responseData(new com::Rpc_Response());
		if (!responseData->ParseFromArray(buffer, size))
		{
			return XCode::ParseResponseDataError;
		}
		MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&RpcClientComponent::OnResponse, mTcpComponent, responseData);
		return XCode::Successful;
	}

    bool ProtoRpcClient::ConnectInSecond(int second)
    {
        if(this->mConnectTimer != nullptr)
        {
            this->mConnectTimer->cancel();
            delete this->mConnectTimer;
            this->mConnectTimer = nullptr;
        }
        AsioContext & io = this->GetSocketProxy()->GetContext();
        this->mConnectTimer = new asio::steady_timer(io, chrono::seconds(second));
        mConnectTimer->async_wait(std::bind(&ProtoRpcClient::ReConnection, this));
    }

    void ProtoRpcClient::OnConnect(XCode code)
    {
        this->mConnectCount++;
        if(code != XCode::Successful && this->mConnectCount < 3)
        {
            this->ConnectInSecond(3);
            return;
        }
        this->SendFromQueue();
        this->mConnectCount = 0;
        delete this->mConnectTimer;
        this->mConnectTimer = nullptr;
        long long id = this->mSocketProxy->GetSocketId();
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&RpcClientComponent::OnConnectAfter, this->mTcpComponent, id, code);
    }

    bool ProtoRpcClient::SendFromQueue()
    {
        if(!this->mMessageQueue.empty())
        {
            std::shared_ptr<Message> message = this->mMessageQueue.front();
            char type = dynamic_pointer_cast<com::Rpc_Request>(message)
                    != nullptr ? RPC_TYPE_REQUEST : RPC_TYPE_RESPONSE;

            this->SendData(type, message);
            this->mMessageQueue.pop();
            return true;
        }
        return false;
    }
}