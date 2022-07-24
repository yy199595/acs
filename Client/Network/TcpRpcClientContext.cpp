#include"TcpRpcClientContext.h"
#include"Component/ClientComponent.h"
#include"Network/Proto/RpcProtoMessage.h"
namespace Client
{
	TcpRpcClientContext::TcpRpcClientContext(std::shared_ptr<SocketProxy> socket, ClientComponent * component)
        : Tcp::TcpContext(socket)
	{
		this->mClientComponent = component;
	}

	void TcpRpcClientContext::SendToServer(std::shared_ptr<c2s::Rpc_Request> request)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> networkData =
			std::make_shared<Tcp::Rpc::RpcProtoMessage>(MESSAGE_TYPE::MSG_RPC_CLIENT_REQUEST, request);

        this->Send(networkData);
	}


    bool TcpRpcClientContext::StartConnect()
    {
        return this->ConnectSync();
    }

    void TcpRpcClientContext::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
    {

    }

    void TcpRpcClientContext::OnReceiveMessage(const asio::error_code &code, asio::streambuf &buffer, size_t size)
    {
        if (code || size == 0)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(code.message());
#endif
			return;
		}
        if(this->mReadState == ReadType::HEAD)
        {
            this->mReadState = ReadType::BODY;
            this->ReceiveMessage(this->GetLength(buffer));
        }
        else if(this->mReadState == ReadType::BODY)
        {
            char * dataBuffer = this->mDataBuffer;
            if(sizeof(this->mDataBuffer) < size)
            {
                dataBuffer = new char[size];
            }
            std::istream & readStream = this->GetReadStream();
            this->mReadState = ReadType::HEAD;
            switch ((MESSAGE_TYPE)readStream.get())
            {
                case MESSAGE_TYPE::MSG_RPC_CALL_CLIENT:
                    readStream.readsome(dataBuffer, size);
                    this->OnRequest(dataBuffer, size);
                    this->ReceiveMessage(sizeof(int));
                    break;
                case MESSAGE_TYPE::MSG_RPC_RESPONSE:
                    readStream.readsome(dataBuffer, size);
                    this->OnResponse(dataBuffer, size);
                    this->ReceiveMessage(sizeof(int));
                    break;
            }
            if(dataBuffer != this->mDataBuffer)
            {
                delete[] dataBuffer;
            }
        }
    }

	void TcpRpcClientContext::StartReceive()
	{
        this->mReadState = ReadType::HEAD;
		this->ReceiveMessage(sizeof(int));
	}

	bool TcpRpcClientContext::OnRequest(const char * data, size_t size)
    {
        std::shared_ptr<c2s::Rpc::Call> request(new c2s::Rpc::Call());
        if (!request->ParseFromArray(data, size))
        {
			CONSOLE_LOG_ERROR("parse request message error");
            return false;
        }
        this->mClientComponent->OnRequest(request);
        return true;
    }

	bool TcpRpcClientContext::OnResponse(const char * data, size_t size)
    {
        std::shared_ptr<c2s::Rpc::Response> response(new c2s::Rpc::Response());
        if (!response->ParseFromArray(data, size))
        {
			CONSOLE_LOG_ERROR("parse response message error");
			return false;
        }
        long long taskId = response->rpc_id();
        this->mClientComponent->OnResponse(taskId, response);
        return true;
    }

}