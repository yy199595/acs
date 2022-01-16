//
// Created by zmhy0073 on 2022/1/15.
//
#include"RedisClient.h"
namespace GameKeeper
{
    RedisClient::RedisClient(std::shared_ptr<SocketProxy> socket)
        : mNetworkThread(socket->GetThread())
    {
        this->mIsSend = false;
        this->mDataSize = 0;
        this->mLineCount = 0;
        this->mSocket = socket;
        this->mTempReadBuffer = nullptr;
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
            if (code) {
                this->mIsOpen = false;
                STD_ERROR_LOG(code.message());
                taskSource->SetResult(false);
                return;
            }
            this->mIsOpen = true;
            this->mLineCount = 0;
            this->StartReceive();
            taskSource->SetResult(true);

            std::shared_ptr<std::list<std::string>> command1(new std::list<std::string>);
            command1->emplace_back("AUTH");
            command1->emplace_back("199595yjz.");
            this->SendCommand(command1);
            const std::string str = "AUTH 199595yjz.\r\n";

            db::db_account_tab_user_account account;
            account.set_account("646585122@qq.com");
            account.set_lastlogin_time(12345678);
            account.set_lastlogin_time(Helper::Time::GetSecTimeStamp());
            account.set_phone_num(13716061995);


            std::string data = account.SerializeAsString();

            //this->SendCommand("set user " + account.SerializeAsString());
            std::shared_ptr<std::list<std::string>>
                    redisCommands(new std::list<std::string>);
            redisCommands->emplace_back("SET");
            redisCommands->emplace_back("user");
            redisCommands->emplace_back(account.SerializeAsString());


//            std::ostream ostream1(&this->mSendDataBuffer);
//            ostream1 << "*" << redisCommands->size() << "\r\n";
//            for (const std::string &str: *redisCommands) {
//                ostream1 << "$" << str.size() << "\r\n";
//                ostream1 << str << "\r\n";
//            }
            //this->SendCommand(this->mSendDataBuffer);

            std::shared_ptr<std::list<std::string>> command2(new std::list<std::string>);
            command2->emplace_back("SUBSCRIBE");
            command2->emplace_back("chat");

            this->SendCommand(command2);

            std::shared_ptr<std::list<std::string>> command3(new std::list<std::string>);

            command3->emplace_back("SMEMBERS");
            command3->emplace_back("127.0.0.1:7789");
            this->SendCommand(command3);


            //this->SendCommand("SUBSCRIBE chat\r\n");
        });
    }

    void RedisClient::SendCommand()
    {
        if(this->mSendQueue.empty())
        {
            return;
        }
        auto data = this->mSendQueue.front();
        std::iostream io(&this->mSendDataBuffer);
        io << "*" << data->size() << "\r\n";
        for(const std::string & cmd : *data)
        {
            io << "$" << cmd.size() << "\r\n" << cmd << "\r\n";
        }

        AsioTcpSocket &tcpSocket = this->mSocket->GetSocket();
        asio::async_write(tcpSocket, this->mSendDataBuffer, [this]
                (const asio::error_code & code, size_t size)
        {
            this->mIsSend = false;
            this->mSendQueue.pop();
            if(!this->mSendQueue.empty())
            {
                this->SendCommand();
            }
        });

    }

    void RedisClient::SendCommand(std::shared_ptr<std::list<std::string>> data)
    {
        this->mSendQueue.emplace(data);
        if(this->mIsSend)
        {
            return;
        }
        this->mIsSend = true;
        this->SendCommand();
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
        std::iostream readStream(&this->mMessageBuffer);
        switch(this->mRedisRespType)
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
        AsioContext &io = this->mSocket->GetContext();
        io.post(std::bind(&RedisClient::StartReceive, this));
    }

    void RedisClient::OnDecodeHead(std::iostream &readStream)
    {
        std::string lineData;
        this->mLineArray.clear();
        char type = readStream.get();
        if (std::getline(readStream, lineData))
        {
            lineData.pop_back(); //拿掉\r
            this->mDataCount = this->OnReceiveFirstLine(type, lineData);
            if(this->mDataCount == 0)
            {
                this->OnReceiveComplete();
            }
        }
    }

    void RedisClient::OnDecodeBinString(std::iostream &readStream)
    {
        if(this->mMessageBuffer.size() >= this->mDataSize)
        {
            char cc = 0;
            this->mLineArray.emplace_back(std::string());
            std::string &data = this->mLineArray.back();
            for (size_t index = 0; index < this->mDataSize; index++)
            {
                readStream.get(cc);
                data.push_back(cc);
            }
            this->mLineCount++;
            this->mDataSize = 0;
            readStream.ignore(2);
            if(this->mLineCount >= this->mDataCount)
            {
                this->OnReceiveComplete();
            }
        }
    }

    void RedisClient::OnDecodeArray(std::iostream &readStream)
    {
        if (this->mMessageBuffer.size() > 0 && this->mDataSize == 0) {
            std::string lineData;
            char type = readStream.get();
            if (type == '$' && std::getline(readStream, lineData)) {
                lineData.pop_back();
                this->mDataSize = std::stoi(lineData);
            }
        }
        this->OnDecodeBinString(readStream);
    }

    void RedisClient::OnReceiveComplete()
    {
        this->mDataSize = 0;
        this->mLineCount = 0;
        this->mDataCount = 0;
        this->mRedisRespType = RedisRespType::REDIS_NONE;
        for(size_t index =0 ;index<this->mLineArray.size();index++)
        {
            std::string &data = this->mLineArray[index];
            std::cout << "[ " << index << ":" << data.size() <<  " ]" << "{" << data << "}"<< std::endl;
        }
        this->mLineArray.clear();
        std::cout << "****************" << std::endl;
        AsioContext &io = this->mSocket->GetContext();
        io.post(std::bind(&RedisClient::StartReceive, this));
    }

    int RedisClient::OnReceiveFirstLine(char type, const std::string &lineData)
    {
        switch(type)
        {
            case '+': //字符串类型
                //STD_ERROR_LOG("str = " << lineData.data());
                this->mRedisRespType = RedisRespType::REDIS_STRING;
                this->mLineArray.emplace_back(std::move(lineData));
                break;
            case '-': //错误
                STD_ERROR_LOG(lineData);
                this->mLineArray.emplace_back(std::move(lineData));
                this->mRedisRespType = RedisRespType::REDIS_ERROR;
                break;
            case ':': //整型
                //STD_ERROR_LOG("num = " << std::stoll(lineData));
                this->mLineArray.emplace_back(std::move(lineData));
                this->mRedisRespType = RedisRespType::REDIS_NUMBER;
                break;
            case '$': //二进制字符串
                this->mDataSize = std::stoi(lineData);
                this->mRedisRespType = RedisRespType::REDIS_BIN_STRING;
                return 1;
            case '*': //数组
                this->mRedisRespType = RedisRespType::REDIS_ARRAY;
                return std::stoi(lineData);
        }
        return 0;
    }
}
