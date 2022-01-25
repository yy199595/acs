#include"RpcClient.h"
#include"Object/App.h"
#include"Util/TimeHelper.h"
#include<iostream>
namespace Sentry
{
    RpcClient::RpcClient(std::shared_ptr<SocketProxy> socket, SocketType type)
            : mType(type), mSocketProxy(socket),
              mContext(socket->GetContext()),
              mNetWorkThread(socket->GetThread())
    {
        this->mIp.clear();
        this->mIsConnect = false;
        this->mSocketId = socket->GetSocketId();
        this->mLastOperTime = Helper::Time::GetSecTimeStamp();
    }

    void RpcClient::Clear()
    {
        this->mSocketId = 0;
        this->mLastOperTime = 0;
        this->mSocketProxy->Close();
    }

    void RpcClient::StartReceive()
    {
        this->mLastOperTime = Helper::Time::GetSecTimeStamp();
        if (mNetWorkThread.IsCurrentThread())
        {
            this->ReceiveHead();
            return;
        }
        mNetWorkThread.Invoke(&RpcClient::ReceiveHead, this);
    }

    void RpcClient::ReceiveHead()
    {
        assert(this->mNetWorkThread.IsCurrentThread());
        const size_t count = sizeof(int) + sizeof(char);
        AsioTcpSocket &socket = this->mSocketProxy->GetSocket();
        std::shared_ptr<RpcClient> self = this->shared_from_this();
        auto cb = [self, this](const asio::error_code &code, const std::size_t t) {
            self->mLastOperTime = Helper::Time::GetSecTimeStamp();
            if (code)
            {
                STD_ERROR_LOG(code.message());
                this->OnClientError(XCode::NetWorkError);
                return;
            }

            int length = 0;
            char type = self->mReceiveBuffer[0];
            memcpy(&length, self->mReceiveBuffer + sizeof(char), sizeof(int));
            if (length >= MAX_DATA_COUNT)
            {
                this->mSocketProxy->Close();
                this->OnClientError(XCode::NetBigDataShutdown);
                return;
            }

            switch(type)
            {
                case RPC_TYPE_REQUEST:
                case RPC_TYPE_RESPONSE:
                case RPC_TYPE_CLIENT_REQUEST:
                    this->ReceiveBody(type, length);
                    break;
                default:
                    this->mSocketProxy->Close();
                    this->OnClientError(XCode::UnKnowPacket);
                    break;
            }
        };
        socket.async_read_some(asio::buffer(this->mReceiveBuffer, count), std::move(cb));
    }

    void RpcClient::ReceiveBody(char type, size_t size)
    {
        char *messageBuffer = this->mReceiveBuffer;
        if (size > TCP_BUFFER_COUNT)
        {
            if (size > MAX_DATA_COUNT)
            {
                this->mSocketProxy->Close();
                this->OnClientError(XCode::NetBigDataShutdown);
                return;
            }
            messageBuffer = new char[size];
        }
        this->mLastOperTime = Helper::Time::GetSecTimeStamp();
        AsioTcpSocket &nSocket = this->mSocketProxy->GetSocket();
        std::shared_ptr<RpcClient> self = this->shared_from_this();

        auto cb = [self, this, messageBuffer, type]
                (const asio::error_code & error_code, const size_t size)
        {
            if (error_code)
            {
                this->mSocketProxy->Close();
                this->OnClientError(XCode::NetWorkError);
                return;
            }
            switch (type)
            {
                case RPC_TYPE_REQUEST:
                    this->OnRequest(messageBuffer, size);
                    break;
                case RPC_TYPE_RESPONSE:
                    this->OnResponse(messageBuffer, size);
                    break;
                default:
                    assert(false);
                    break;
            }
            if (messageBuffer != this->mReceiveBuffer)
            {
                delete[] messageBuffer;
            }
            mContext.post(std::bind(&RpcClient::ReceiveHead, this));
        };
        nSocket.async_read_some(asio::buffer(messageBuffer, size), std::move(cb));
    }

    bool RpcClient::StartConnect(const std::string &ip, unsigned short port)
    {
        if(this->IsCanConnection())
        {
            this->mIsConnect = true;
            LOG_CHECK_RET_FALSE(this->mSocketProxy);
            IAsioThread &nThread = this->mSocketProxy->GetThread();
            nThread.Invoke(&RpcClient::ConnectHandler, this, ip, port);
            return true;
        }
        return false;
    }

    void RpcClient::ConnectHandler(const std::string &ip, unsigned short port)
    {
        this->mIp = ip;
        this->mPort = port;
        assert(this->mNetWorkThread.IsCurrentThread());
        AsioTcpSocket &nSocket = this->mSocketProxy->GetSocket();
        std::shared_ptr<RpcClient> self = this->shared_from_this();
        auto address = asio::ip::make_address_v4(ip);
        asio::ip::tcp::endpoint endPoint(address, port);
        LOG_DEBUG(this->mSocketProxy->GetName(), " start connect " , ip, ':', port);
        nSocket.async_connect(endPoint, [self, this](const asio::error_code &err)
        {
            XCode code = XCode::Successful;
            if (err)
            {
                STD_ERROR_LOG(err.message());
                code = XCode::NetConnectFailure;
            }
            this->OnConnect(code);
            this->mIsConnect = false;
            this->mSocketProxy->RefreshState();
        });
    }

    bool RpcClient::IsCanConnection()
    {
        return this->GetSocketType() == SocketType::LocalSocket
               && !this->IsOpen() && !this->mIsConnect;
    }

    void RpcClient::SendData(char type, std::shared_ptr<Message> message)
    {
        assert(this->mNetWorkThread.IsCurrentThread());
        const int body = message->ByteSize();
        const int head = sizeof(char) + sizeof(int);

        char *buffer = this->mSendBuffer;
        if (head + body > TCP_BUFFER_COUNT)
        {
            buffer = new char[head + body];
        }
        buffer[0] = type;
        std::shared_ptr<RpcClient> self = this->shared_from_this();
        memcpy(buffer + sizeof(char), &body, sizeof(int));
        if (!message->SerializePartialToArray(buffer + head, body))
        {
            if (buffer != this->mSendBuffer)
            {
                delete[]buffer;
            }
            this->OnSendData(XCode::SerializationFailure, message);
            return;
        }

        auto cb = [message, this, buffer, type, self]
                (const asio::error_code &error_code, std::size_t size)
        {
            if (buffer != this->mSendBuffer)
            {
                delete[]buffer;
            }
            if (error_code)
            {
                this->mSocketProxy->Close();
                STD_ERROR_LOG(error_code.message());
                this->OnClientError(XCode::NetWorkError);
                this->OnSendData(XCode::NetWorkError, message);
                return;
            }
            this->OnSendData(XCode::Successful, message);
            this->mLastOperTime = Helper::Time::GetSecTimeStamp();
        };

        AsioTcpSocket &nSocket = this->mSocketProxy->GetSocket();
        nSocket.async_send(asio::buffer(buffer, head + body), std::move(cb));
    }
}