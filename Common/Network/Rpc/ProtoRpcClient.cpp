#include"ProtoRpcClient.h"
#include<Core/App.h>
#include<Rpc/RpcClientComponent.h>
#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
namespace GameKeeper
{
	ProtoRpcClient::ProtoRpcClient(RpcClientComponent *component, SocketProxy * socket, SocketType type)
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

    bool ProtoRpcClient::SendToServer(const com::Rpc_Response *message)
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

    bool ProtoRpcClient::SendToServer(const com::Rpc_Request *message)
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

    void ProtoRpcClient::OnSendData(XCode code, const Message * message)
    {
        if (code != XCode::Successful)
        {
            long long id = this->GetSocketId();
            this->mNetWorkThread.Invoke(&RpcClientComponent::OnSendFailure, this->mTcpComponent, id, message);
            return;
        }
        delete message;
    }

	void ProtoRpcClient::OnClose(XCode code)
	{
        long long id = this->GetSocketId();
		MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&RpcClientComponent::OnCloseSocket, this->mTcpComponent, id, code);
	}

	XCode ProtoRpcClient::OnRequest(const char * buffer, size_t size)
	{
		auto requestData = new com::Rpc_Request();
		if (!requestData->ParseFromArray(buffer, size))
		{
			delete requestData;
			return XCode::ParseRequestDataError;
		}
        requestData->set_socketid(this->GetSocketId());
		MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&RpcClientComponent::OnRequest, mTcpComponent, requestData);
		return XCode::Successful;
	}

	XCode ProtoRpcClient::OnResponse(const char * buffer, size_t size)
	{
		auto responseData = new com::Rpc_Response();
		if (!responseData->ParseFromArray(buffer, size))
		{
			delete responseData;
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