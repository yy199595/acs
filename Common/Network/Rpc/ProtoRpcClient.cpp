#include"ProtoRpcClient.h"
#include<Core/App.h>
#include<Util/TimeHelper.h>
#include<Component/Scene/ProtoRpcComponent.h>
#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
namespace GameKeeper
{
	ProtoRpcClient::ProtoRpcClient(ProtoRpcComponent *component, SocketProxy * socket, SocketType type)
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

	void ProtoRpcClient::CloseSocket(XCode code)
	{
		this->mSocketProxy->Close();
        long long id = this->GetSocketId();
		MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&ProtoRpcComponent::OnCloseSocket, this->mTcpComponent, id, code);
	}

	bool ProtoRpcClient::OnRequest(const char * buffer, size_t size)
	{
		auto requestData = new com::Rpc_Request();
		if (!requestData->ParseFromArray(buffer, size))
		{
			delete requestData;
			return false;
		}
        long long id = this->GetSocketId();
		MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&ProtoRpcComponent::OnRequest, mTcpComponent, id, requestData);
		return true;
	}

	bool ProtoRpcClient::OnResponse(const char * buffer, size_t size)
	{
		auto responseData = new com::Rpc_Response();
		if (!responseData->ParseFromArray(buffer, size))
		{
			delete responseData;
			return false;
		}
        long long id = this->mSocketProxy->GetSocketId();
		MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&ProtoRpcComponent::OnResponse, mTcpComponent, id, responseData);
		return true;
	}

	void ProtoRpcClient::SendProtocol(char type, const Message * message)
	{
#ifdef __DEBUG__
		std::string json;
		util::MessageToJsonString(*message, &json);
		if (type == RPC_TYPE_REQUEST)
		{
			GKDebugLog("send request message " << json);
		}
		else if (type == RPC_TYPE_RESPONSE)
		{
			GKDebugLog("send response message " << json);
		}
#endif
		unsigned int body = message->ByteSizeLong();
		size_t head = sizeof(char) + sizeof(unsigned int);
		char * messageBuffer = new char[head + body];

		messageBuffer[0] = type;
        LocalObject<Message> lock(message);
		memcpy(messageBuffer + sizeof(char), &body, sizeof(TCP_HEAD));
		if (!message->SerializePartialToArray(messageBuffer + head, body))
		{
#ifdef __DEBUG__
			std::string json;
			util::MessageToJsonString(*message, &json);
			GKDebugError("Serialize " << "failure : " << json);
#endif // __DEBUG__
			delete[] messageBuffer;
			return;
		}
		this->AsyncSendMessage(messageBuffer, head + body);
	}

    void ProtoRpcClient::OnConnect(XCode code)
    {
        long long id = this->mSocketProxy->GetSocketId();
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&ProtoRpcComponent::OnConnectAfter, this->mTcpComponent, id, code);
    }

    void ProtoRpcClient::OnSendAfter(XCode code, const char *buffer, size_t size)
    {
        delete []buffer;
    }
}