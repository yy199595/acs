//
// Created by zmhy0073 on 2022/1/15.
//
#include"RedisClientContext.h"
#include"Util/FileHelper.h"
#include"Component/Scene/LoggerComponent.h"
namespace Sentry
{
    RedisClientContext::RedisClientContext(std::shared_ptr<SocketProxy> socket, const RedisConfig * config)
        : Tcp::TcpContext(socket), mConfig(config)
    {
        this->mDataSize = 0;
        this->mLineCount = 0;
        this->mSocket = socket;
		this->mConnectLock = std::make_shared<CoroutineLock>();
		this->mCommandLock = std::make_shared<CoroutineLock>();
	}

	RedisClientContext::~RedisClientContext() noexcept
	{
		LOG_WARN("remove redis client " << this->mConfig->Name << "[" << this->mConfig->Address << "]");
	}

    XCode RedisClientContext::StartConnect()
    {
		AutoCoroutineLock lock(this->mConnectLock);
		if(this->mSocket->IsOpen())
		{
			return XCode::Successful;
		}
		this->mConnectTaskSource.Clear();
#ifdef ONLY_MAIN_THREAD
		this->Connect();
#else
		this->mNetworkThread.Invoke(&RedisClientContext::Connect, this);
#endif
		XCode code = this->mConnectTaskSource.Await();
		if(code == XCode::Successful && !this->mConfig->Password.empty())
		{
			std::shared_ptr<RedisResponse> response = std::make_shared<RedisResponse>();
			std::shared_ptr<RedisRequest> request = RedisRequest::Make("AUTH", this->mConfig->Password);
			if(this->Run(request, response) != XCode::Successful)
			{
				return XCode::RedisSocketError;
			}
			return response->IsOk() ? XCode::Successful : XCode::RedisAuthFailure;
		}
		return code;
    }

	void RedisClientContext::OnConnect(const asio::error_code& error)
	{
		if(error)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(error.message());
#endif
			this->mConnectTaskSource.SetResult(XCode::NetConnectFailure);
			return;
		}
		this->mConnectTaskSource.SetResult(XCode::Successful);
	}

	XCode RedisClientContext::Run(std::shared_ptr<RedisRequest> command)
	{
		if(!this->mSocket->IsOpen())
		{
			return XCode::NetWorkError;
		}
		AutoCoroutineLock lock(this->mCommandLock);
		this->mSendTaskSource.Clear();
#ifdef ONLY_MAIN_THREAD
		this->Send(command);
#else
		this->mNetworkThread.Invoke(&RedisClientContext::Send, this, command);
#endif
		return this->mSendTaskSource.Await();
	}

    XCode RedisClientContext::Run(std::shared_ptr<RedisRequest> command, std::shared_ptr<RedisResponse> response)
	{
		if(!this->mSocket->IsOpen())
		{
			return XCode::NetWorkError;
		}
		AutoCoroutineLock lock(this->mCommandLock);
		this->mSendTaskSource.Clear();
#ifdef ONLY_MAIN_THREAD
		this->Send(command);
#else
		this->mNetworkThread.Invoke(&RedisClientContext::Send, this, command);
#endif
		if(this->mSendTaskSource.Await() != XCode::Successful)
		{
			return XCode::NetWorkError;
		}
		XCode code = this->WaitRedisResponse(response);
		if(code == XCode::Successful && response->HasError())
		{
			std::string error;
			if(response->GetString(error))
			{
				LOG_ERROR(error);
			}
			return XCode::RedisInvokeError;
		}
		return code;
	}

     XCode RedisClientContext::WaitRedisResponse(std::shared_ptr<RedisResponse> response)
    {
		if(!this->mSocket->IsOpen())
		{
			return XCode::NetWorkError;
		}
        this->mDataSize = 0;
        this->mResponse = response;
		this->mReadTaskSource.Clear();
#ifdef ONLY_MAIN_THREAD
		this->StartReceive();
#else
		this->mNetworkThread.Invoke(&RedisClientContext::StartReceive, this);
#endif
        return mReadTaskSource.Await();
    }

    void RedisClientContext::StartReceive()
    {
        assert(this->mResponse);
        if(this->mRecvDataBuffer.size() > this->mDataSize)
        {
            asio::error_code code;
            this->OnReceive(code, this->mRecvDataBuffer.size());
            return;
        }
        AsioTcpSocket &tcpSocket = this->mSocket->GetSocket();
        auto cb = std::bind(&RedisClientContext::OnReceive, this, args1, args2);
        asio::async_read(tcpSocket, this->mRecvDataBuffer,
                         asio::transfer_at_least(1), std::move(cb));
    }

    void RedisClientContext::OnReceive(const asio::error_code &code, size_t size)
    {
        if (code)
        {
			this->mSocket->Close();
#ifdef __NET_ERROR_LOG__
			CONSOLE_LOG_ERROR(code.message());
#endif
			this->mReadTaskSource.SetResult(XCode::NetWorkError);
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
		if(this->mLineCount >= this->mDataCount)
		{
			this->OnComplete();
			return;
		}
		if(!this->mReadTaskSource.IsComplete())
		{
			this->StartReceive();
		}
    }

    void RedisClientContext::OnDecodeHead(std::iostream &readStream)
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

    void RedisClientContext::OnDecodeBinString(std::iostream &readStream)
    {
        if(this->mRecvDataBuffer.size() >= this->mDataSize)
        {
            readStream.readsome(this->mReadTempBuffer, this->mDataSize);
            this->mResponse->AddValue(this->mReadTempBuffer, this->mDataSize);

            this->mLineCount++;
            this->mDataSize = 0;
            readStream.ignore(2);
        }
    }

    void RedisClientContext::OnDecodeArray(std::iostream &readStream)
    {
        if (this->mRecvDataBuffer.size() > 0)
		{
            std::string lineData;
            char type = readStream.get();
            if (type == '$' && std::getline(readStream, lineData))
			{
                lineData.pop_back();
                this->mDataSize = std::stoi(lineData);
				this->OnDecodeBinString(readStream);
			}
			else if(type == ':' && std::getline(readStream, lineData))
			{
				lineData.pop_back();
				this->mLineCount++;
				this->mResponse->AddValue(std::stoll(lineData));
				//readStream.ignore(2);
			}
        }
    }

    void RedisClientContext::OnComplete()
    {
        this->mDataSize = 0;
        this->mLineCount = 0;
        this->mDataCount = 0;
		this->mReadTaskSource.SetResult(XCode::Successful);
    }

    int RedisClientContext::OnReceiveFirstLine(char type, const std::string &lineData)
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
	bool RedisClientContext::LoadLuaScript(const string& path, std::string & key)
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
		std::string error;
		if(response->HasError() && response->GetString(error))
		{
			LOG_ERROR(error);
			return false;
		}
		return response->GetString(key);
	}

	void RedisClientContext::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
	{
		if(code)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(code.message());
#endif
			this->mSendTaskSource.SetResult(XCode::NetWorkError);
			return;
		}
		this->mSendTaskSource.SetResult(XCode::Successful);
	}
}
