#include"ProtoRpcClient.h"
#include<Core/App.h>
#include<Util/TimeHelper.h>
#include<Component/Scene/ProtoRpcComponent.h>
#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
namespace GameKeeper
{
	ProtoRpcClient::ProtoRpcClient(ProtoRpcComponent *component, SocketProxy * socket)
		:RpcClient(socket), mTcpComponent(component)
	{

	}

	void ProtoRpcClient::StartClose()
	{
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		if (nThread.IsCurrentThread())
		{
			this->CloseSocket(XCode::NetActiveShutdown);
			return;
		}
		nThread.AddTask(&ProtoRpcClient::CloseSocket, this, XCode::NetActiveShutdown);

	}

	void ProtoRpcClient::StartSendProtocol(char type, const Message * message)
	{
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		if (nThread.IsCurrentThread())
		{
			this->SendProtocol(type, message);
			return;
		}
		nThread.AddTask(&ProtoRpcClient::SendProtocol, this, type, message);
	}

	void ProtoRpcClient::CloseSocket(XCode code)
	{
		this->mSocketProxy->Close();
		MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
		taskScheduler.AddMainTask(&ProtoRpcComponent::OnCloseSession, this->mTcpComponent, this->GetSocketId(), code);
	}

	bool ProtoRpcClient::OnRequest(char * buffer, size_t size)
	{
		com::Rpc_Request * requestData = new com::Rpc_Request();
		if (!requestData->ParseFromArray(buffer, size))
		{
			delete requestData;
			return false;
		}
		MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
		taskScheduler.AddMainTask(&ProtoRpcComponent::OnRequest, this->mTcpComponent, this, requestData);
		return true;
	}

	bool ProtoRpcClient::OnResponse(char * buffer, size_t size)
	{
		com::Rpc_Response * responseData = new com::Rpc_Response();
		if (!responseData->ParseFromArray(buffer, size))
		{
			delete responseData;
			return false;
		}
		MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
		taskScheduler.AddMainTask(&ProtoRpcComponent::OnResponse, mTcpComponent, this, responseData);
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
			GKDebugLog("send reaponse message " << json);
		}
#endif
		unsigned int body = message->ByteSizeLong();
		size_t head = sizeof(char) + sizeof(unsigned int);
		char * messageBuffer = new char[head + body];

		messageBuffer[0] = type;
		memcpy(messageBuffer + sizeof(char), &body, sizeof(unsigned int));
		if (!message->SerializePartialToArray(messageBuffer + head, body))
		{
#ifdef __DEBUG__
			std::string json;
			util::MessageToJsonString(*message, &json);
			GKDebugError("Serialize " << "failure : " << json);
#endif // __DEBUG__
			delete message;
			delete[] messageBuffer;
			return;
		}		
		if (!this->AsyncSendMessage(messageBuffer, head + body))
		{
			delete message;
			delete[]messageBuffer;
		}
	}
}