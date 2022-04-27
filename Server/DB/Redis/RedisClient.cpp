//
// Created by zmhy0073 on 2022/1/15.
//
#include"RedisClient.h"
#include"Pool/MessagePool.h"
#include"Util/FileHelper.h"
#include"Component/Scene/LoggerComponent.h"
namespace Sentry
{
    RedisClient::RedisClient(std::shared_ptr<SocketProxy> socket, const RedisConfig * config)
        : mNetworkThread(socket->GetThread()), mConfig(config)
    {
        this->mDataSize = 0;
        this->mLineCount = 0;
        this->mSocket = socket;
		this->mLock = std::make_shared<CoroutineLock>();
	}

    bool RedisClient::StartConnect()
    {
        std::shared_ptr<TaskSource<bool>> taskSource(new TaskSource<bool>());
#ifdef ONLY_MAIN_THREAD
		this->ConnectRedis(taskSource);
#else
		this->mNetworkThread.Invoke(&RedisClient::ConnectRedis, this, taskSource);
#endif
        if(!taskSource->Await())
		{
			return false;
		}
		if(!this->mConfig->Password.empty())
		{
			std::shared_ptr<RedisResponse> response = std::make_shared<RedisResponse>();
			std::shared_ptr<RedisRequest> request = RedisRequest::Make("AUTH", this->mConfig->Password);
			if(this->Run(request, response) != XCode::Successful)
			{
				return false;
			}
			return response->IsOk();
		}
		return true;
    }

    void RedisClient::ConnectRedis(std::shared_ptr<TaskSource<bool>> taskSource)
    {
        AsioTcpSocket &tcpSocket = this->mSocket->GetSocket();
        auto address = asio::ip::make_address_v4(this->mConfig->Ip);
        asio::ip::tcp::endpoint endPoint(address, this->mConfig->Port);
        tcpSocket.async_connect(endPoint, [taskSource, this]
                (const asio::error_code &code) {
            if (code)
            {
				CONSOLE_LOG_ERROR(code.message());
				taskSource->SetResult(false);
                return;
            }
            this->mLineCount = 0;
            taskSource->SetResult(true);
        });
    }

    XCode RedisClient::Run(std::shared_ptr<RedisRequest> command, std::shared_ptr<RedisResponse> response)
	{
		AutoCoroutineLock lock(this->mLock);
		this->mLastOperatorTime = Helper::Time::GetNowSecTime();
		std::shared_ptr<TaskSource<XCode>> sendTaskSource(new TaskSource<XCode>());
#ifdef __DEBUG__
		std::string json = command->ToJson();
		LOG_WARN(json);
#endif
#ifdef ONLY_MAIN_THREAD
		this->SendCommand(command, taskSource);
#else
		this->mNetworkThread.Invoke(&RedisClient::SendCommand, this, command, sendTaskSource);
#endif
		if (sendTaskSource->Await() != XCode::Successful)
		{
			return XCode::NetWorkError;
		}
		return this->WaitRedisResponse(response);
	}

     XCode RedisClient::WaitRedisResponse(std::shared_ptr<RedisResponse> response)
    {
        this->mDataSize = 0;
        this->mResponse = response;
        this->mReadTaskSource = std::make_shared<TaskSource<XCode>>();
#ifdef ONLY_MAIN_THREAD
		this->StartReceive();
#else
		this->mNetworkThread.Invoke(&RedisClient::StartReceive, this);
#endif
        return mReadTaskSource->Await();
    }

    void RedisClient::StartReceive()
    {
        assert(this->mResponse);
        assert(this->mReadTaskSource);
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

    void RedisClient::OnReceive(const asio::error_code &code, size_t size)
    {
        if (code)
        {
			CONSOLE_LOG_ERROR(code.message());
			this->mReadTaskSource->SetResult(XCode::NetWorkError);
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
		if(!this->mReadTaskSource->IsComplete())
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
            this->mDataSize = 0;
            readStream.ignore(2);
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
        this->mLastOperatorTime = Helper::Time::GetNowSecTime();
		this->mReadTaskSource->SetResult(XCode::Successful);
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
                return this->mDataSize <= 0 ? 0 : 1;
            case '*': //数组
                this->mResponse->AddValue(RedisRespType::REDIS_ARRAY);
                return std::stoi(lineData);
        }
        return 0;
    }
	bool RedisClient::LoadLuaScript(const string& path, std::string & key)
	{
		std::string content;
		if(!Helper::File::ReadTxtFile(path, content))
		{
			LOG_ERROR("read " << path << " failure");
			return false;
		}
		std::shared_ptr<RedisResponse> response = make_shared<RedisResponse>();
		std::shared_ptr<RedisRequest> request = RedisRequest::Make("script", "load", content);
		if(this->Run(request, response) != XCode::Successful)
		{
			LOG_ERROR("redis net work error load script failure");
			return false;
		}
		if(response->HasError())
		{
			LOG_ERROR(response->GetValue());
			return false;
		}
		key = response->GetValue();
		return true;
	}

	void RedisClient::SendCommand(std::shared_ptr<RedisRequest> request, std::shared_ptr<TaskSource<XCode>> taskSource)
	{
		std::iostream io(&this->mSendDataBuffer);
		request->GetCommand(io);
		AsioTcpSocket &tcpSocket = this->mSocket->GetSocket();
		asio::async_write(tcpSocket, this->mSendDataBuffer, [this, taskSource]
				(const asio::error_code & code, size_t size)
		{
			if(code)
			{
				CONSOLE_LOG_ERROR(code.message());
				taskSource->SetResult(XCode::NetWorkError);
				return;
			}
			taskSource->SetResult(XCode::Successful);
		});
	}
}
