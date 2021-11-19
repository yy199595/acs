#include"RpcClient.h"
#include<Core/App.h>
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
		this->mSocketId = 0;
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
			this->mIp = nScoket.remote_endpoint().address().to_string();
            this->mAddress = this->mIp + ":" + std::to_string(nScoket.remote_endpoint().port());
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
                unsigned int length = 0;
                char type = this->mReceiveMsgBuffer[0];
                memcpy(&length, this->mReceiveMsgBuffer + sizeof(char), sizeof(unsigned int));
                if (length >= MAX_DATA_COUNT) //最大为10k
                {
                    this->CloseSocket(XCode::NetBigDataShutdown);
                    return;
                }

                if (type == RPC_TYPE_REQUEST || type == RPC_TYPE_RESPONSE)
                {
                    this->ReadMessageBody(length, type);
                }
                else
                {
                    GKDebugFatal("receive err type = " << type);
                    this->CloseSocket(XCode::NetReceiveFailure);
                }
            }
			this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		});
	}

	void RpcClient::CloseSocket(XCode code)
	{				
		this->mSocketProxy->Close();
        MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.AddMainTask(&RpcComponent::OnCloseSession, this->mTcpComponent, this->mSocketId, code);
	}

	void RpcClient::ReadMessageBody(unsigned int allSize, int type)
	{
		char *nMessageBuffer = this->mReceiveMsgBuffer;
		if (allSize > TCP_BUFFER_COUNT)
		{
			nMessageBuffer = new char[allSize];
            GKDebugWarning("receive message count = " << allSize);
		}
		this->mLastOperTime = TimeHelper::GetSecTimeStamp();
		AsioTcpSocket & nSocket = this->mSocketProxy->GetSocket();

		asio::error_code code;
		if (allSize > nSocket.available(code))
		{
			this->CloseSocket(XCode::NetReceiveFailure);
            GKDebugError("available data less " << allSize);
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
					auto request = new com::Rpc_Request();
                    if(!request->ParseFromArray(nMessageBuffer, size))
                    {
						delete request;
                        this->CloseSocket(XCode::NetWorkError);
                        GKDebugError(this->GetAddress() << " parse request message error");
                    }
                    else  //通知主线程处理
                    {
						request->set_socketid(this->GetSocketProxy().GetSocketId());
						MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();                                  
						taskScheduler.AddMainTask(&RpcComponent::OnRequest, this->mTcpComponent, this, request);
                    }
                }
                else if(type == RPC_TYPE_RESPONSE)
                {
					auto response = new com::Rpc_Response();
                    if(!response->ParseFromArray(nMessageBuffer, size))
                    {
						delete response;
                        this->CloseSocket(XCode::NetWorkError);
                        GKDebugError(this->GetAddress() << " parse response message error");
                    }
                    else  //通知主线程处理
                    {
						MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
						taskScheduler.AddMainTask(&RpcComponent::OnResponse, this->mTcpComponent, this, response);
                    }
                }
                AsioContext &context = this->mSocketProxy->GetContext();
                context.post(std::bind(&RpcClient::ReceiveMessage, this));
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
#ifdef __DEBUG__
            std::string json;
            util::MessageToJsonString(*message, &json);
            if(type == RPC_TYPE_REQUEST)
            {
                GKDebugLog("send request message " << json);
            }
            else if(type == RPC_TYPE_RESPONSE)
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
				delete[] messageBuffer;
			}
			
			nSocket.async_send(asio::buffer(messageBuffer, head + body),
				[messageBuffer, this](const asio::error_code &error_code, std::size_t size)
			{
				XCode code = XCode::Successful;
				if (error_code)
				{
					code = XCode::NetSendFailure;
                    GKDebugError(error_code.message());
					this->CloseSocket(XCode::NetSendFailure);
				}
				delete[]messageBuffer;
				this->mLastOperTime = TimeHelper::GetSecTimeStamp();
			});
		}		
		delete message;
	}
}