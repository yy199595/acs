//
// Created by zmhy0073 on 2022/1/15.
//
#include"RedisClient.h"
namespace GameKeeper
{
    RedisClient::RedisClient(std::shared_ptr<SocketProxy> socket)
        : mNetworkThread(socket->GetThread())
    {
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
        AsioTcpSocket  & tcpSocket = this->mSocket->GetSocket();
        auto address = asio::ip::make_address_v4(this->mIp);
        asio::ip::tcp::endpoint endPoint(address, this->mPort);
        tcpSocket.async_connect(endPoint, [taskSource, this]
            (const asio::error_code & code)
        {
            if(code)
            {
                this->mIsOpen = false;
                STD_ERROR_LOG(code.message());
                taskSource->SetResult(false);
                return;
            }
            this->mIsOpen = true;
            this->mLineCount = 0;
            this->StartReceive();
            taskSource->SetResult(true);
            const std::string str = "AUTH 199595yjz.\r\n";
            const std::string str1 = "SADD yjz2 1 2 3 4 5 6 7 yjz2 yjz3 yjz7 .038\r\n";
            const std::string str2 = "SMEMBERS yjz2\r\n";

            this->SendCommand(str);
            this->SendCommand(str1);
            this->SendCommand(str2);

        });
    }

    void RedisClient::SendCommand(const std::string &data)
    {
        AsioTcpSocket &tcpSocket = this->mSocket->GetSocket();
        tcpSocket.async_send(asio::buffer(data.c_str(), data.size()),
                             [this](const asio::error_code & code, size_t size)
        {
            if(code)
            {
                this->mIsOpen = false;
                STD_ERROR_LOG(code.message());
            }
        });
    }

    void RedisClient::StartReceive()
    {
        if(this->mMessageBuffer.size() > 0)
        {
            asio::error_code code;
            this->OnReceive(code, this->mMessageBuffer.size());
            return;
        }
        AsioTcpSocket &tcpSocket = this->mSocket->GetSocket();
        auto cb = std::bind(&RedisClient::OnReceive, this, args1, args2);
        asio::async_read(tcpSocket, this->mMessageBuffer,
                         asio::transfer_at_least(1), std::move(cb));
    }

    void RedisClient::OnReceive(const asio::error_code &code, size_t size)
    {
        if (code)
        {
            this->mIsOpen = false;
            STD_ERROR_LOG(code.message());
            return;
        }
        std::string lineData;
        std::iostream iostream(&this->mMessageBuffer);
        if (this->mLineCount == 0)
        {
            this->mDataCount = 0;
            char type = iostream.get();
            if (std::getline(iostream, lineData))
            {
                lineData.pop_back();
                this->mDataCount = this->OnReceiveFirstLine(type, lineData);
                if (this->mDataCount == 0)
                {
                    this->OnReceiveComplete();
                    return;
                }
            }
        }
        for (; this->mLineCount < this->mDataCount;)
        {
            if (!std::getline(iostream, lineData))
            {
                break;
            }
            this->mLineCount++;
            lineData.pop_back();
            this->mLineArray.emplace_back(std::move(lineData));
            if (this->mLineCount >= this->mDataCount)
            {
                this->OnReceiveComplete();
                return;
            }
        }
        AsioContext &io = this->mSocket->GetContext();
        io.post(std::bind(&RedisClient::StartReceive, this));
    }

    void RedisClient::OnReceiveComplete()
    {
        this->mLineCount = 0;
        this->mDataCount = 0;
        STD_ERROR_LOG("finish a command" << (int)this->mRedisRespType);
        AsioContext &io = this->mSocket->GetContext();
        io.post(std::bind(&RedisClient::StartReceive, this));
    }

    int RedisClient::OnReceiveFirstLine(char type, const std::string &lineData)
    {
        switch(type)
        {
            case '+': //字符串类型
                STD_ERROR_LOG("str = " << lineData.data());
                this->mRedisRespType = RedisRespType::REDIS_STRING;
                return 0;
            case '-': //错误
                STD_ERROR_LOG(lineData);
                this->mRedisRespType = RedisRespType::REDIS_ERROR;
                return 0;
            case ':': //整型
                STD_ERROR_LOG("num = " << std::stoll(lineData));
                this->mRedisRespType = RedisRespType::REDIS_NUMBER;
                return 0;
            case '$': //多行字符串
                this->mRedisRespType = RedisRespType::REDIS_STRING_ARRAY;
                break;
            case '*': //数组
                this->mRedisRespType = RedisRespType::REDIS_ARRAY;
                return stoi(lineData) * 2;
        }
        return 0;
    }
}
