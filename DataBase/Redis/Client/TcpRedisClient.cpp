//
// Created by zmhy0073 on 2022/1/15.
//
#include"TcpRedisClient.h"
#include"File/FileHelper.h"
#include"Component/LoggerComponent.h"

namespace Sentry
{
	TcpRedisClient::TcpRedisClient(std::shared_ptr<SocketProxy> socket,
		const RedisClientConfig& config, IRpc<RedisResponse>* component)
		: Tcp::TcpContext(socket, 1024 * 1024), mConfig(config), mComponent(component)
	{
		this->mAddress = fmt::format("{0}:{1}", this->mConfig.Name, this->mConfig.Address);
	}

	TcpRedisClient::~TcpRedisClient() noexcept
	{
		LOG_WARN("remove redis client " << this->mConfig.Name << "[" << this->mConfig.Address << "]");
	}

	void TcpRedisClient::Send(std::shared_ptr<RedisRequest> command)
	{
#ifdef ONLY_MAIN_THREAD
		this->Send(command);
#else
        Asio::Context & t = this->mSocket->GetThread();
		t.post(std::bind(&TcpRedisClient::Write, this, command));
#endif
        //CONSOLE_LOG_INFO("async command = " << command->ToJson());
	}

	long long TcpRedisClient::Call(std::shared_ptr<RedisRequest> command)
	{
		return 0;
	}

	void TcpRedisClient::OnReceiveLine(const Asio::Code & code, std::istream & is, size_t size)
	{
		if (code)
		{
			CONSOLE_LOG_ERROR(code.message());
		}
		assert(this->mCurResponse);
		int length = this->mCurResponse->OnRecvLine(is);
		if (length == 0)
		{
			this->OnReadComplete();
			this->mCurResponse = nullptr;
		}
		else if (length == -1)
		{
			this->ReceiveLine();
		}
		else
		{
			this->ReceiveMessage(length);
		}
	}

	void TcpRedisClient::OnReceiveMessage(const asio::error_code& code, std::istream & is, size_t size)
	{
        if(this->mCurResponse == nullptr)
        {
            this->ClearRecvStream();
            return;
        }
		if (this->mCurResponse->OnRecvMessage(is) == 0)
		{
			this->OnReadComplete();
			return;
		}
		this->ReceiveLine();
	}

	void TcpRedisClient::OnReadComplete()
	{
        std::shared_ptr<RedisResponse> response = std::move(this->mCurResponse);
#ifdef ONLY_MAIN_THREAD
		this->mRedisComponent->OnMessage(id, response);
#else
		asio::io_service & io = App::Inst()->MainThread();
		io.post(std::bind(&IRpc<RedisResponse>::OnMessage, this->mComponent, this->mAddress, response));
#endif
		this->SendFromMessageQueue();
	}

    void TcpRedisClient::OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message)
    {
        if (code)
        {
            if (!this->AuthUser())
            {
				CONSOLE_LOG_FATAL("redis auth failure");
				return;
            }
            this->SendFromMessageQueue();
            return;
        }
		assert(this->PopMessage() == message);
		std::shared_ptr<RedisRequest> request = std::static_pointer_cast<RedisRequest>(message);
		this->mCurResponse = std::make_shared<RedisResponse>(request->GetTaskId());
        this->ReceiveLine();
    }

	bool TcpRedisClient::AuthUser()
	{
		if (!this->ConnectSync())
		{
			CONSOLE_LOG_ERROR("connect redis [" << this->mConfig.Address << "] failure");
			return false;
		}
		assert(this->mSendBuffer.size() == 0);
		assert(this->mRecvBuffer.size() == 0);
		if (!this->mConfig.Password.empty())  //验证密码
		{
			std::shared_ptr<RedisRequest> authCommand = RedisRequest::Make("AUTH", this->mConfig.Password);
			std::shared_ptr<RedisResponse> response = this->SyncCommand(authCommand);
            if(response == nullptr || !response->IsOk())
            {
                CONSOLE_LOG_ERROR("auth redis user faliure");
                return false;
            }
		}

		//切换数据库
		std::shared_ptr<RedisRequest> selectCommand = RedisRequest::Make("SELECT", this->mConfig.Index);
        std::shared_ptr<RedisResponse> response2 = this->SyncCommand(selectCommand);
        if(response2 == nullptr || !response2->IsOk())
        {
            CONSOLE_LOG_ERROR("auth redis user faliure");
            return false;
        }

        if(this->mConfig.FreeClient > 0)
        {
            int s = this->mConfig.FreeClient;
            asio::io_service & io = this->mSocket->GetThread();
            this->mCloseTimer = std::make_shared<asio::steady_timer>(io, std::chrono::seconds(s));
            this->mCloseTimer->async_wait(std::bind(&TcpRedisClient::CloseFreeClient, this));
        }
#ifdef ONLY_MAIN_THREAD
		this->mRedisComponent->OnConnectSuccessful(this->mAddress);
#else
		asio::io_service & taskThread = App::Inst()->MainThread();
		taskThread.post(std::bind(&IRpc<RedisResponse>::OnConnectSuccessful, this->mComponent, this->mAddress));
#endif
		return true;
	}

    std::shared_ptr<RedisResponse> TcpRedisClient::SyncCommand(std::shared_ptr<RedisRequest> request)
    {
        assert(this->mRecvBuffer.size() == 0);
        //CONSOLE_LOG_DEBUG("sync command = " << request->ToJson());
        if(this->SendSync(request) <= 0)
        {
            return nullptr;
        }
        std::istream is(&this->mRecvBuffer);
        std::shared_ptr<RedisResponse> redisResponse(new RedisResponse(0));
        READ_LINE:
        if(this->RecvLineSync() <= 0)
        {
            return nullptr;
        }
        int length = redisResponse->OnRecvLine(is);
        if(length == -1)
        {
            goto READ_LINE;
        }
        else if(length > 0)
        {
            if (this->RecvSync(length) <= 0)
            {
                return nullptr;
            }
            if (redisResponse->OnRecvMessage(is) == -1)
            {
                goto READ_LINE;
            }
        }
        return redisResponse;
    }

    void TcpRedisClient::StartPingServer()
    {
        long long nowTime = Helper::Time::GetNowSecTime();
        if(nowTime - this->GetLastOperTime() >= 10) //十秒没进行操作 ping一下
        {
			this->Write(RedisRequest::Make("PING"));
        }
        asio::io_service &io = this->mSocket->GetThread();
        this->mTimer = std::make_shared<asio::steady_timer>(io, std::chrono::seconds(10));
        this->mTimer->async_wait(std::bind(&TcpRedisClient::StartPingServer, this));
    }

    void TcpRedisClient::CloseFreeClient()
    {
        int second = this->mConfig.FreeClient;
        long long nowTime = Helper::Time::GetNowSecTime();
        if(nowTime - this->GetLastOperTime() >= second)
        {
            this->mSocket->Close();
            CONSOLE_LOG_INFO("[" << this->mConfig.Name << "] close free client");
        }
        else
        {
            asio::io_service &io = this->mSocket->GetThread();
            this->mCloseTimer = std::make_shared<asio::steady_timer>(io, std::chrono::seconds(second));
            this->mCloseTimer->async_wait(std::bind(&TcpRedisClient::CloseFreeClient, this));
        }
    }
}
