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

    bool ProtoRpcClient::SendToServer(std::shared_ptr<com::Rpc_Response> message)
    {
        if(!this->IsOpen())
        {
            return false;
        }
        if(this->mNetWorkThread.IsCurrentThread())
        {
            this->SendData(RPC_TYPE_RESPONSE, message);
            return true;
        }
        this->mNetWorkThread.Invoke(&ProtoRpcClient::SendData, this, RPC_TYPE_RESPONSE, message);
        return true;
    }

    bool ProtoRpcClient::SendToServer(std::shared_ptr<com::Rpc_Request> message)
    {
        if(!this->IsOpen())
        {
            return false;
        }
        if(this->mNetWorkThread.IsCurrentThread())
        {
            this->SendData(RPC_TYPE_REQUEST, message);
            return true;
        }
        this->mNetWorkThread.Invoke(&ProtoRpcClient::SendData, this, RPC_TYPE_REQUEST, message);
        return true;
    }

    void ProtoRpcClient::OnSendData(XCode code, std::shared_ptr<Message> message)
    {
        if (code != XCode::Successful)
        {
            long long id = this->GetSocketId();
            MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
            taskScheduler.Invoke(&RpcClientComponent::OnSendFailure, this->mTcpComponent, id, message);
        }
    }

	void ProtoRpcClient::OnClose(XCode code)
	{
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
        requestData->set_socketid(this->GetSocketId());
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

    void ProtoRpcClient::OnConnect(XCode code)
    {
        long long id = this->mSocketProxy->GetSocketId();
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&RpcClientComponent::OnConnectAfter, this->mTcpComponent, id, code);
    }
}