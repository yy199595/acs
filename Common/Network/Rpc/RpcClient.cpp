#include"RpcClient.h"
#include<Util/TimeHelper.h>
#include<Component/Scene/RpcComponent.h>
#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
namespace GameKeeper
{
    RpcClient::RpcClient(RpcComponent *component)
            :mTcpComponent(component)
    {      		
		this->mLastOperTime = 0;
		this->mSocketProxy = nullptr;
		this->mReceiveMsgBuffer = new char[TCP_BUFFER_COUNT];
    }

    RpcClient::~RpcClient()
    {
        delete this->mSocketProxy;
        delete[]this->mReceiveMsgBuffer;
    }

	void RpcClient::SetSocket(SocketProxy * socketProxy)
    {
        delete this->mSocketProxy;
        this->mSocketProxy = socketProxy;     
        if (this->mSocketProxy->IsOpen())
        {
            this->StartReceive();
			AsioTcpSocket &nScoket = this->mSocketProxy->GetSocket();
            this->mAddress = nScoket.remote_endpoint().address().to_string()
                             + ":" + std::to_string(nScoket.remote_endpoint().port());
        }
    }


	void RpcClient::StartClose()
	{		
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		if (nThread.IsCurrentThread())
		{
			this->CloseSocket(XCode::NetActiveShutdown);
			return;
		}
		nThread.AddTask(&RpcClient::CloseSocket, this, XCode::NetActiveShutdown);
		
	}

	void RpcClient::StartReceive()
	{		
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		if (nThread.IsCurrentThread())
		{
			this->ReceiveMessage();
			return;
		}
		nThread.AddTask(&RpcClient::ReceiveMessage, this);
		
	}

	void RpcClient::StartSendProtocol(char type, const Message * message)
	{
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		if (nThread.IsCurrentThread())
		{
			this->SendProtocol(type, message);
			return;
		}
		nThread.AddTask(&RpcClient::SendProtocol, this, type, message);
	}


	void RpcClient::ReceiveMessage()
	{
		this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
        const size_t count = sizeof(unsigned int) + sizeof(char);
        socket.async_read_some(asio::buffer(this->mReceiveMsgBuffer, count),
			[this](const asio::error_code &error_code, const std::size_t t)
		{
			if (error_code)
			{
				GKDebugError(error_code.message());
				this->CloseSocket(XCode::NetReceiveFailure);		
			}
			else
			{
				size_t packageSize = 0;
				memcpy(&packageSize, this->mReceiveMsgBuffer, t);
				if (packageSize >= 1024 * 10) //最大为10k
				{
					this->CloseSocket(XCode::NetBigDataShutdown);
					return;
				}
                int type = (int)this->mReceiveMsgBuffer[sizeof(unsigned int)];
                if(type == RPC_TYPE_REQUEST || type == RPC_TYPE_RESPONSE)
                {
                    this->ReadMessageBody(packageSize, type);
                }
                else
                {
                    this->CloseSocket(XCode::NetReceiveFailure);
                }
			}
			this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		});
	}

	void RpcClient::CloseSocket(XCode code)
	{		
		if (this->mSocketProxy->IsOpen())
		{
			asio::error_code err;
			this->mSocketProxy->GetSocket().close(err);
		}
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		nThread.AddTask(&RpcComponent::OnCloseSession, this->mTcpComponent, this, code);
	}

	void RpcClient::ReadMessageBody(size_t allSize, int type)
	{	
		if (!this->mSocketProxy->IsOpen())
		{
			this->CloseSocket(XCode::NetWorkError);
			return;
		}
		char *nMessageBuffer = this->mReceiveMsgBuffer;
		if (allSize > TCP_BUFFER_COUNT)
		{
			nMessageBuffer = new char[allSize];
		}
		this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();

		asio::error_code code;
		if (allSize < nSocket.available(code))
		{
			this->CloseSocket(XCode::NetReceiveFailure);
			return;
		}
		nSocket.async_read_some(asio::buffer(nMessageBuffer, allSize),
			[this, nMessageBuffer, type](const asio::error_code &error_code,
				const std::size_t size) {
			if (error_code)
			{
				GKDebugError(error_code.message());
				this->CloseSocket(XCode::NetReceiveFailure);			
			}
			else
			{
                if(type == RPC_TYPE_REQUEST)
                {
					com::Rpc_Request * request = new com::Rpc_Request();
                    if(!request->ParseFromArray(nMessageBuffer, size))
                    {
						delete request;
                        this->CloseSocket(XCode::NetWorkError);
                        GKDebugError(this->GetAddress() << " parse request message error");
                    }
                    else  //通知主线程处理
                    {
                        request->set_socketid(this->GetSocketProxy().GetSocketId());
                        NetWorkThread & netThread = this->mSocketProxy->GetThread();
                        netThread.AddTask(&RpcComponent::OnRequest, this->mTcpComponent, this, request);
                    }
                }
                else if(type == RPC_TYPE_RESPONSE)
                {
					com::Rpc_Response * response = new com::Rpc_Response();
                    if(!response->ParseFromArray(nMessageBuffer, size))
                    {
						delete response;
                        this->CloseSocket(XCode::NetWorkError);
                        GKDebugError(this->GetAddress() << " parse response message error");
                    }
                    else  //通知主线程处理
                    {
                        NetWorkThread & netThread = this->mSocketProxy->GetThread();
                        netThread.AddTask(&RpcComponent::OnResponse, this->mTcpComponent, this, response);
                    }
                }
			}
			if (nMessageBuffer != this->mReceiveMsgBuffer)
			{
				delete[]nMessageBuffer;
			}
			this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		});
	}

	void RpcClient::SendProtocol(char type, const Message * message)
	{
		this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();
		if (!this->mSocketProxy->IsOpen())
		{
			this->CloseSocket(XCode::HttpNetWorkError);
#ifdef __DEBUG__
			std::string json;
			util::MessageToJsonString(*message, &json);
			GKDebugError("send to " << this->GetAddress() << " failure : " << json);
#endif // __DEBUG__
		}
		else
		{			
			size_t size = message->ByteSizeLong();
			char * buffer = new char[size + 1];
			if (!message->SerializePartialToArray(buffer + 1, size))
			{
#ifdef __DEBUG__
				std::string json;
				util::MessageToJsonString(*message, &json);
				GKDebugError("Serialize " << "failure : " << json);
#endif // __DEBUG__
				delete[] buffer;
			}
			buffer[0] = type;
			nSocket.async_send(asio::buffer(buffer, size),
				[buffer, this](const asio::error_code &error_code, std::size_t size)
			{
				XCode code = XCode::Successful;
				if (error_code)
				{
					code = XCode::NetSendFailure;
					this->CloseSocket(XCode::NetSendFailure);
				}
				delete[]buffer;
				this->mLastOperTime = TimeHelper::GetSecTimeStamp();
			});
			delete message;
		}		
	}
}