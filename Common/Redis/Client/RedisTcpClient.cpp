//
// Created by zmhy0073 on 2022/1/15.
//
#include"RedisTcpClient.h"

#include"Util/File/FileHelper.h"
#include"Util/String/StringHelper.h"
#include"Entity/Unit/App.h"
namespace Tendo
{
	RedisTcpClient::RedisTcpClient(std::shared_ptr<Tcp::SocketProxy> socket,
		const RedisClientConfig& config, IRpc<RedisResponse>* component)
		: Tcp::TcpContext(std::move(socket), 1024 * 1024), mConfig(config), mComponent(component)
	{
        this->mIndex = 0;
		this->mIsSubClient = false;
        this->mAddress = this->mConfig.Address[0].FullAddress;
        this->mCurResponse = std::make_shared<RedisResponse>();
	}

	void RedisTcpClient::Send(const std::shared_ptr<RedisRequest>& command)
	{
#ifdef ONLY_MAIN_THREAD
		this->Write(command);
#else
        Asio::Context & t = this->mSocket->GetThread();
		t.post(std::bind(&RedisTcpClient::Write, this, command));
#endif
        //CONSOLE_LOG_INFO("async command = " << command->ToJson());
	}

	void RedisTcpClient::OnReceiveLine(const Asio::Code & code, std::istream & is, size_t size)
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

	void RedisTcpClient::OnReceiveMessage(const asio::error_code& code, std::istream & is, size_t size)
	{
		if (this->mCurResponse->OnRecvMessage(is) == 0)
		{
			this->OnReadComplete();
			return;
		}
		this->ReceiveLine();
	}

    void RedisTcpClient::OnReadComplete()
	{
		std::shared_ptr<RedisRequest> request =
				std::static_pointer_cast<RedisRequest>(this->PopMessage());
		if (request != nullptr && request->GetTaskId() > 0)
		{
			if (request->GetCommand() == "SUBSCRIBE")
			{
				this->mIsSubClient = true;
			}
			this->mCurResponse->SetTaskId(request->GetTaskId());
		}
#ifdef ONLY_MAIN_THREAD
		this->mComponent->OnMessage(this->mCurResponse);
#else
		asio::io_service& io = App::Inst()->MainThread();
		io.post(std::bind(&IRpc<RedisResponse>::OnMessage, this->mComponent, this->mCurResponse));
#endif
		this->mCurResponse = std::make_shared<RedisResponse>();
		if (!this->SendFromMessageQueue() && this->mIsSubClient)
		{
			this->ReceiveLine();
		}
	}

    void RedisTcpClient::OnSendMessage(const asio::error_code &code,
			std::shared_ptr<Tcp::ProtoMessage> message)
	{
		if (code)
		{
			if (!this->AuthUser())
			{
				this->mIndex = 0;
				CONSOLE_LOG_FATAL("redis auth failure");
				return;
			}
			this->SendFromMessageQueue();
			return;
		}
		this->ReceiveLine();
	}

	bool RedisTcpClient::AuthUser()
    {
        if (this->mIndex >= this->mConfig.Address.size())
        {
            return false;
        }
        const Net::Address &address =
                this->mConfig.Address[this->mIndex];
        this->mAddress = address.FullAddress;
        this->mSocket->Init(address.Ip, address.Port); 
#ifdef __DEBUG__
        CONSOLE_LOG_DEBUG("start connect redis server [" << address.FullAddress << "]");
#endif
        if (!this->ConnectSync())
        {
            this->mIndex++;
#ifdef __DEBUG__
            CONSOLE_LOG_ERROR("connect redis server [" << address.FullAddress << "] failure");
#endif // __DEBUG__
            return this->AuthUser();
        }
        this->mIndex = 0;
#ifdef __DEBUG__
        CONSOLE_LOG_DEBUG("connect redis server [" << address.FullAddress << "]successful");
#endif
        return this->InitRedisClient(this->mConfig.Password);
    }

    bool RedisTcpClient::InitRedisClient(const std::string & pwd)
    {
        assert(this->mSendBuffer.size() == 0);
        assert(this->mRecvBuffer.size() == 0);
        if (!pwd.empty())  //验证密码
        {
            std::shared_ptr<RedisRequest> authCommand = RedisRequest::Make("AUTH", this->mConfig.Password);
            std::shared_ptr<RedisResponse> response = this->SyncCommand(authCommand);
            if(response == nullptr || !response->IsOk())
            {
				this->mIndex++;
				this->mSocket->Close();
                CONSOLE_LOG_ERROR("auth redis user failure");
				return this->AuthUser();
			}
        }

        //切换数据库
        std::shared_ptr<RedisRequest> selectCommand = RedisRequest::Make("SELECT", this->mConfig.Index);
        std::shared_ptr<RedisResponse> response2 = this->SyncCommand(selectCommand);
        if(response2 == nullptr || !response2->IsOk())
        {
            CONSOLE_LOG_ERROR("auth redis user failure");
            return false;
        }

        if(this->mConfig.FreeClient > 0)
        {
            int s = this->mConfig.FreeClient;
            asio::io_service & io = this->mSocket->GetThread();
            this->mCloseTimer = std::make_shared<asio::steady_timer>(io, std::chrono::seconds(s));
            this->mCloseTimer->async_wait(std::bind(&RedisTcpClient::CloseFreeClient, this));
        }
#ifdef ONLY_MAIN_THREAD
        this->mComponent->OnConnectSuccessful(this->mAddress);
#else
        asio::io_service & taskThread = App::Inst()->MainThread();
        taskThread.post(std::bind(&IRpc<RedisResponse>::OnConnectSuccessful, this->mComponent, this->mAddress));
#endif
        return true;
    }

    std::shared_ptr<RedisResponse> RedisTcpClient::SyncCommand(const std::shared_ptr<RedisRequest>& request)
    {
        assert(this->mRecvBuffer.size() == 0);
        //CONSOLE_LOG_DEBUG("sync command = " << request->ToJson());
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

    void RedisTcpClient::StartPingServer()
    {
        long long nowTime = Helper::Time::NowSecTime();
        if(nowTime - this->GetLastOperTime() >= 10) //十秒没进行操作 ping一下
        {
			this->Write(RedisRequest::Make("PING"));
        }
        asio::io_service &io = this->mSocket->GetThread();
        this->mTimer = std::make_shared<asio::steady_timer>(io, std::chrono::seconds(10));
        this->mTimer->async_wait(std::bind(&RedisTcpClient::StartPingServer, this));
    }

    void RedisTcpClient::CloseFreeClient()
    {
        int second = this->mConfig.FreeClient;
        long long nowTime = Helper::Time::NowSecTime();
        if(nowTime - this->GetLastOperTime() >= second)
        {
            this->mSocket->Close();
            CONSOLE_LOG_INFO("[" << this->mAddress << "] close free client");
        }
        else
        {
            asio::io_service &io = this->mSocket->GetThread();
            this->mCloseTimer = std::make_shared<asio::steady_timer>(io, std::chrono::seconds(second));
            this->mCloseTimer->async_wait(std::bind(&RedisTcpClient::CloseFreeClient, this));
        }
    }
}
