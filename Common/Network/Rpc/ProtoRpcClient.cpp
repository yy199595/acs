#include"ProtoRpcClient.h"
#include<Core/App.h>
#include<Util/TimeHelper.h>
#include<ProtoRpc/ProtoRpcClientComponent.h>
#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
namespace GameKeeper
{
	ProtoRpcClient::ProtoRpcClient(ProtoRpcClientComponent *component, SocketProxy * socket, SocketType type)
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

	bool ProtoRpcClient::StartSendProtocol(char type, const Message * message)
	{
        if(!this->IsOpen())
        {
            return false;
        }
		if (this->mNetWorkThread.IsCurrentThread())
		{
			this->SendProtocol(type, message);
			return true;
		}
        this->mNetWorkThread.Invoke(&ProtoRpcClient::SendProtocol, this, type, message);
        return true;
	}

	void ProtoRpcClient::OnClose(XCode code)
	{
        long long id = this->GetSocketId();
		MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&ProtoRpcClientComponent::OnCloseSocket, this->mTcpComponent, id, code);
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
        taskScheduler.Invoke(&ProtoRpcClientComponent::OnRequest, mTcpComponent, requestData);
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
        taskScheduler.Invoke(&ProtoRpcClientComponent::OnResponse, mTcpComponent, responseData);
		return XCode::Successful;
	}

	void ProtoRpcClient::SendProtocol(char type, const Message * message)
	{
        LocalObject<Message> lock(message);
		const int body = message->ByteSize();
		size_t head = sizeof(char) + sizeof(int);
		char * buffer = new char[head + body];
       

		buffer[0] = type;
		memcpy(buffer + sizeof(char), &body, sizeof(body));
		if (!message->SerializePartialToArray(buffer + head, body))
		{
            return;
		}
		this->AsyncSendMessage(std::move(buffer), head + body);
	}

    void ProtoRpcClient::OnConnect(XCode code)
    {
        long long id = this->mSocketProxy->GetSocketId();
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&ProtoRpcClientComponent::OnConnectAfter, this->mTcpComponent, id, code);
    }
}