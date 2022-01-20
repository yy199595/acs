//
// Created by zmhy0073 on 2022/1/15.
//
#include"RedisClient.h"
#include"Pool/MessagePool.h"
namespace Sentry
{
    RedisClient::RedisClient(std::shared_ptr<SocketProxy> socket)
        : mNetworkThread(socket->GetThread())
    {
        this->mDataSize = 0;
        this->mLineCount = 0;
        this->mSocket = socket;
    }

    std::shared_ptr<TaskSource<bool>> RedisClient::ConnectAsync(const std::string &ip, unsigned short port)
    {
        this->mIp = ip;
        this->mPort = port;
        std::shared_ptr<TaskSource<bool>> taskSource(new TaskSource<bool>());
        this->mNetworkThread.Invoke(&RedisClient::ConnectRedis, this, taskSource);
        return taskSource;
    }

    void RedisClient::ConnectRedis(std::shared_ptr<TaskSource<bool>> taskSource)
    {
        AsioTcpSocket &tcpSocket = this->mSocket->GetSocket();
        auto address = asio::ip::make_address_v4(this->mIp);
        asio::ip::tcp::endpoint endPoint(address, this->mPort);
        tcpSocket.async_connect(endPoint, [taskSource, this]
                (const asio::error_code &code) {
            if (code)
            {
                this->OnClientError(code);
                taskSource->SetResult(false);
                return;
            }
            this->mIsOpen = true;
            this->mLineCount = 0;
            taskSource->SetResult(true);
        });
    }

    TaskSourceShared<RedisCmdResponse> RedisClient::InvokeCommand(std::shared_ptr<RedisCmdRequest> command)
    {
        this->mLastOperatorTime = Helper::Time::GetSecTimeStamp();
        std::shared_ptr<TaskSource<bool>> taskSource(new TaskSource<bool>());

        std::iostream io(&this->mSendDataBuffer);
        command->GetCommand(io);

        AsioTcpSocket &tcpSocket = this->mSocket->GetSocket();
        asio::async_write(tcpSocket, this->mSendDataBuffer, [this, taskSource]
                (const asio::error_code & code, size_t size)
        {
            if(code)
            {
                this->OnClientError(code);
                taskSource->SetResult(false);
                return;
            }
            taskSource->SetResult(true);
        });

        if(!taskSource->Await())
        {
            return nullptr;
        }
        return this->WaitRedisMessageResponse();
    }

    TaskSourceShared<RedisCmdResponse> RedisClient::WaitRedisMessageResponse()
    {
        this->mDataSize = 0;
        this->mResponse = std::make_shared<RedisCmdResponse>();
        this->mRespTaskSource = make_shared<TaskSource<std::shared_ptr<RedisCmdResponse>>>();
        this->mNetworkThread.Invoke(&RedisClient::StartReceive, this);
        return mRespTaskSource;
    }

    void RedisClient::StartReceive()
    {
        assert(this->mResponse);
        assert(this->mRespTaskSource);
        if(this->mRecvDataBuffer.size() > this->mDataSize)
        {
            asio::error_code code;
            this->OnReceive(code, this->mRecvDataBuffer.size());
            return;
        }
        AsioTcpSocket &tcpSocket = this->mSocket->GetSocket();
        auto cb = std::bind(&RedisClient::OnReceive, this, args1, args2);
        asio::async_read(tcpSocket, this->mRecvDataBuffer,
                         asio::transfer_at_least(1), std::move(cb));
    }

    void RedisClient::OnClientError(const asio::error_code &code)
    {
        this->mIsOpen = false;
        STD_ERROR_LOG(code.message());
        if(this->mRespTaskSource != nullptr)
        {
            std::move(this->mResponse);
            this->OnComplete();
        }
        asio::error_code err;
        this->mSocket->GetSocket().close(err);
    }

    void RedisClient::OnReceive(const asio::error_code &code, size_t size)
    {
        if (code)
        {
            this->OnClientError(code);
            return;
        }
        std::iostream readStream(&this->mRecvDataBuffer);
        switch(this->mResponse->GetType())
        {
            case RedisRespType::REDIS_NONE:
                this->OnDecodeHead(readStream);
                break;
            case RedisRespType::REDIS_BIN_STRING:
                this->OnDecodeBinString(readStream);
                break;
            case RedisRespType::REDIS_ARRAY:
                this->OnDecodeArray(readStream);
                break;
        }
        if(this->mResponse != nullptr)
        {
            this->StartReceive();
        }
    }

    void RedisClient::OnDecodeHead(std::iostream &readStream)
    {
        std::string lineData;
        char type = readStream.get();
        if (std::getline(readStream, lineData))
        {
            lineData.pop_back(); //拿掉\r
            this->mDataCount = this->OnReceiveFirstLine(type, lineData);
            std::cout << "receive sum line " << this->mDataCount << std::endl;
            if(this->mDataCount == 0)
            {
                this->OnComplete();
            }
        }
    }

    void RedisClient::OnDecodeBinString(std::iostream &readStream)
    {
        if(this->mRecvDataBuffer.size() >= this->mDataSize)
        {
            readStream.readsome(this->mReadTempBuffer, this->mDataSize);
            this->mResponse->AddValue(this->mReadTempBuffer, this->mDataSize);

            this->mLineCount++;
            readStream.ignore(2);
#ifdef __DEBUG__
            std::cout << "current receive line " << this->mLineCount <<
                "[ " << std::string(this->mReadTempBuffer, this->mDataSize) <<"]" << std::endl;
#endif
            this->mDataSize = 0;
            if(this->mLineCount >= this->mDataCount)
            {
                this->OnComplete();
            }
        }
    }

    void RedisClient::OnDecodeArray(std::iostream &readStream)
    {
        if (this->mRecvDataBuffer.size() > 0 && this->mDataSize == 0) {
            std::string lineData;
            char type = readStream.get();
            if (type == '$' && std::getline(readStream, lineData)) {
                lineData.pop_back();
                this->mDataSize = std::stoi(lineData);
            }
        }
        this->OnDecodeBinString(readStream);
    }

    void RedisClient::OnComplete()
    {
        this->mDataSize = 0;
        this->mLineCount = 0;
        this->mDataCount = 0;
        this->mLastOperatorTime = Helper::Time::GetSecTimeStamp();
        this->mRespTaskSource->SetResult(std::move(this->mResponse));
        std::move(this->mRespTaskSource);
    }

    int RedisClient::OnReceiveFirstLine(char type, const std::string &lineData)
    {
        switch(type)
        {
            case '+': //字符串类型
                //STD_ERROR_LOG("str = " << lineData.data());
                this->mResponse->AddValue(lineData);
                this->mResponse->AddValue(RedisRespType::REDIS_STRING);
                break;
            case '-': //错误
                STD_ERROR_LOG(lineData);
                this->mResponse->AddValue(lineData);
                this->mResponse->AddValue(RedisRespType::REDIS_ERROR);
                break;
            case ':': //整型
                //STD_ERROR_LOG("num = " << std::stoll(lineData));
                this->mResponse->AddValue(std::stoll(lineData));
                break;
            case '$': //二进制字符串
                this->mDataSize = std::stoi(lineData);
                this->mResponse->AddValue(RedisRespType::REDIS_BIN_STRING);
                return 1;
            case '*': //数组
                this->mResponse->AddValue(RedisRespType::REDIS_ARRAY);
                return std::stoi(lineData);
        }
        return 0;
    }
}
