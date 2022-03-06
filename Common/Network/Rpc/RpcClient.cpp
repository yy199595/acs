#include"RpcClient.h"
#include"Object/App.h"
#include"Util/TimeHelper.h"
#include<iostream>

namespace Sentry
{
    NetworkData::NetworkData(char type, std::shared_ptr<Message> message)
    {
        this->mType = type;
        this->mMessage = message;
    }

    size_t NetworkData::GetByteSize()
    {
        return this->mMessage->ByteSize() + sizeof(char) + sizeof(int);
    }

    bool NetworkData::WriteToBuffer(std::string & streamBuffer)
    {
        streamBuffer.clear();
        streamBuffer += this->mType;
        int body = this->mMessage->ByteSize();
        streamBuffer.append((char *) &body, sizeof(int));
        return this->mMessage->AppendToString(&streamBuffer);
    }
}

namespace Sentry
{
    RpcClient::RpcClient(std::shared_ptr<SocketProxy> socket, SocketType type)
            : mType(type), mSocketProxy(socket),
              mContext(socket->GetContext()),
              mNetWorkThread(socket->GetThread())
    {
        this->mIp.clear();
        this->mIsConnect = false;
        this->mIsCanSendData = true;
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

            switch(type)
            {
                case RPC_TYPE_REQUEST:
                case RPC_TYPE_RESPONSE:
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
#ifdef ONLY_MAIN_THREAD
            this->ConnectHandler(ip, port);
#else
            IAsioThread &nThread = this->mSocketProxy->GetThread();
            nThread.Invoke(&RpcClient::ConnectHandler, this, ip, port);
#endif
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
            if(code == XCode::Successful)
            {
                this->mSocketProxy->RefreshState();
            }
        });
    }

    bool RpcClient::IsCanConnection()
    {
        return this->GetSocketType() == SocketType::LocalSocket
               && !this->IsOpen() && !this->mIsConnect;
    }

    void RpcClient::SendData(std::shared_ptr<NetworkData> message)
    {
        assert(this->mNetWorkThread.IsCurrentThread());
        if(!this->mIsCanSendData)
        {
            this->mWaitSendQueue.emplace(message);
            return;
        }
        this->mIsCanSendData = false;
        message->WriteToBuffer(this->mSendBuffer);
        std::shared_ptr<RpcClient> self = this->shared_from_this();
        AsioTcpSocket & tcpSocket = this->mSocketProxy->GetSocket();

        size_t size = this->mSendBuffer.size();
        tcpSocket.async_send(asio::buffer(this->mSendBuffer),[this, self, message]
                (const asio::error_code & code, size_t size)
        {
            if (code)
            {
                this->mSocketProxy->Close();
                STD_ERROR_LOG(code.message());
                this->OnClientError(XCode::NetWorkError);
                this->OnSendData(XCode::NetWorkError, message);
                while(!this->mWaitSendQueue.empty())
                {
                    this->OnSendData(XCode::NetWorkError, this->mWaitSendQueue.front());
                    this->mWaitSendQueue.pop();
                }
            }
            else
            {
                this->mIsCanSendData = true;
                this->OnSendData(XCode::Successful, message);
                if (!this->mWaitSendQueue.empty())
                {
                    this->SendData(this->mWaitSendQueue.front());
                    this->mWaitSendQueue.pop();
                }
                this->mLastOperTime = Helper::Time::GetSecTimeStamp();
            }
        });
    }
}