//
// Created by zmhy0073 on 2022/1/15.
//
#include"TcpRedisClient.h"
#include"Util/FileHelper.h"
#include"Component/Scene/LoggerComponent.h"
#include"Component/Redis/RedisComponent.h"
namespace Sentry
{
	TcpRedisClient::TcpRedisClient(std::shared_ptr<SocketProxy> socket, const RedisConfig& config, RedisComponent * component)
		: Tcp::TcpContext(socket, 1024 * 1024), mConfig(config), mRedisComponent(component)
	{
		this->mSocket = socket;
	}

	TcpRedisClient::~TcpRedisClient() noexcept
	{
		LOG_WARN("remove redis client " << this->mConfig.Name << "[" << this->mConfig.Address << "]");
	}

	void TcpRedisClient::SendCommand(std::shared_ptr<RedisRequest> command)
	{
#ifdef ONLY_MAIN_THREAD
		this->Send(command);
#else
        asio::io_service & t = this->mSocket->GetThread();
		t.post(std::bind(&TcpRedisClient::Send, this, command));
#endif
        CONSOLE_LOG_INFO("async command = " << command->ToJson());
	}

	void TcpRedisClient::OnReceiveLine(const asio::error_code& code, std::istream & is, size_t size)
	{
		if (code)
		{
			CONSOLE_LOG_ERROR(code.message());
		}
		if (this->mCurResponse == nullptr)
		{
			this->mCurResponse = std::make_shared<RedisResponse>();
		}
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
			this->mCurResponse = nullptr;
			return;
		}
		this->ReceiveLine();
	}

	void TcpRedisClient::OnReadComplete()
	{
        long long taskId = 0;
        std::shared_ptr<Tcp::ProtoMessage> message = this->PopMessage();
        if(message != nullptr)
        {
            std::shared_ptr<RedisRequest> request = dynamic_pointer_cast<RedisRequest>(message);
            if(request != nullptr && request->GetTaskId() == 0)
            {
                std::move(this->mCurResponse);
                if(!this->SendFromMessageQueue())
                {
                    if(!this->mConfig.Channels.empty())
                    {
                        this->ReceiveLine();
                    }
                }
                return;
            }
            taskId = request->GetTaskId();
        }
        std::shared_ptr<RedisResponse> response = std::move(this->mCurResponse);
        if(response->HasError())
        {
            CONSOLE_LOG_ERROR(response->GetString());
        }
#ifdef ONLY_MAIN_THREAD
		this->mRedisComponent->OnResponse(taskId, response);
#else
		asio::io_service & taskThread = App::Get()->GetThread();
		taskThread.post(std::bind(&RedisComponent::OnResponse, this->mRedisComponent, taskId, response));
#endif
        if(!this->SendFromMessageQueue())
        {
            if(!this->mConfig.Channels.empty())
            {
                this->ReceiveLine();
            }
        }
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
            CONSOLE_LOG_DEBUG(this->mConfig.Address << " redis client auth successful");
            return;
        }
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

        if(!this->LoadScript())
        {
            CONSOLE_LOG_ERROR("load lua script error");
            return false;
        }
        if(!this->SubChannel())
        {
            CONSOLE_LOG_ERROR("sub redis channel error");
            return false;
        }
        if(this->mConfig.FreeClient > 0 && this->mConfig.Channels.empty())
        {
            int s = this->mConfig.FreeClient;
            asio::io_service & io = this->mSocket->GetThread();
            this->mCloseTimer = std::make_shared<asio::steady_timer>(io, std::chrono::seconds(s));
            this->mCloseTimer->async_wait(std::bind(&TcpRedisClient::CloseFreeClient, this));
        }
		return true;
	}

    bool TcpRedisClient::LoadScript()
    {
        for (const std::string& path : this->mConfig.LuaFiles)
        {
            std::string content;
            if (!Helper::File::ReadTxtFile(path, content))
            {
                CONSOLE_LOG_ERROR("load lua script error");
                return false;
            }
            std::shared_ptr<RedisRequest> loadRequest = RedisRequest::Make("SCRIPT", "LOAD", content);
            std::shared_ptr<RedisResponse> response = this->SyncCommand(loadRequest);
            if(response == nullptr || response->GetType() != RedisRespType::REDIS_BIN_STRING)
            {
                return false;
            }
            if (response->HasError())
            {
                CONSOLE_LOG_ERROR(response->GetString());
                return false;
            }
            const std::string& md5 = response->GetString();
#ifdef ONLY_MAIN_THREAD
            this->mRedisComponent->OnLoadScript(path, md5);
#else
            asio::io_service& io = App::Get()->GetThread();
            io.post(std::bind(&RedisComponent::OnLoadScript, this->mRedisComponent, path, md5));
#endif
        }
        return true;
    }

    std::shared_ptr<RedisResponse> TcpRedisClient::SyncCommand(std::shared_ptr<RedisRequest> request)
    {
        assert(this->mRecvBuffer.size() == 0);
        CONSOLE_LOG_DEBUG("sync command = " << request->ToJson());
        if(this->SendSync(request) <= 0)
        {
            return nullptr;
        }
        std::istream is(&this->mRecvBuffer);
        std::shared_ptr<RedisResponse> redisResponse(new RedisResponse());
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

    bool TcpRedisClient::SubChannel()
    {
        if(this->mConfig.Channels.empty())
        {
            return true;
        }
        for (const std::string &channel: this->mConfig.Channels)
        {
            std::shared_ptr<RedisRequest> request = RedisRequest::Make("SUBSCRIBE", channel);
            std::shared_ptr<RedisResponse> redisResponse = this->SyncCommand(request);
            if (redisResponse == nullptr || redisResponse->GetArraySize() != 3)
            {
                CONSOLE_LOG_INFO("sub " << channel << " failure");
                return false;
            }
            if (!redisResponse->Get(2)->IsNumber())
            {
                CONSOLE_LOG_INFO("sub " << channel << " failure");
                return false;
            }
            if (redisResponse->Get(2)->Cast<RedisNumber>()->GetValue() <= 0)
            {
                return false;
            }
            CONSOLE_LOG_INFO("sub " << channel << " successful");
        }
        asio::io_service &io = this->mSocket->GetThread();
        this->mTimer = std::make_shared<asio::steady_timer>(io, std::chrono::seconds(10));
        this->mTimer->async_wait(std::bind(&TcpRedisClient::StartPingServer, this));
        return true;
    }

    void TcpRedisClient::StartPingServer()
    {
        long long nowTime = Helper::Time::GetNowSecTime();
        if(nowTime - this->GetLastOperTime() >= 10) //十秒没进行操作 ping一下
        {
            this->Send(RedisRequest::Make("PING"));
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
