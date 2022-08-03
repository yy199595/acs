//
// Created by zmhy0073 on 2022/1/15.
//
#include"RedisClientContext.h"
#include"Util/FileHelper.h"
#include"Component/Scene/LoggerComponent.h"
#include"Component/Redis/RedisComponent.h"
namespace Sentry
{
	RedisClientContext::RedisClientContext(std::shared_ptr<SocketProxy> socket, const RedisConfig& config)
		: Tcp::TcpContext(socket, 1024 * 100), mConfig(config), mRedisComponent(nullptr)
	{
		this->mSocket = socket;
        LOG_CHECK_FATAL(this->mRedisComponent = App::Get()->GetComponent<RedisComponent>());
	}

	RedisClientContext::~RedisClientContext() noexcept
	{
		LOG_WARN("remove redis client " << this->mConfig.Name << "[" << this->mConfig.Address << "]");
	}

	void RedisClientContext::SendCommand(std::shared_ptr<RedisRequest> command)
	{
#ifdef ONLY_MAIN_THREAD
		this->Send(command);
#else
        asio::io_service & t = this->mSocket->GetThread();
		t.post(std::bind(&RedisClientContext::Send, this, command));
#endif
	}

	void RedisClientContext::StartReceiveMessage()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveLine();
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&RedisClientContext::ReceiveLine, this));
#endif
	}

	void RedisClientContext::OnReceiveLine(const asio::error_code& code, std::istream & is)
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

	void RedisClientContext::OnReceiveMessage(const asio::error_code& code, std::istream & is)
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

	void RedisClientContext::OnReadComplete()
	{
        std::shared_ptr<RedisRequest> request = dynamic_pointer_cast<RedisRequest>(this->PopMessage());

        SharedRedisClient self = this->Cast<RedisClientContext>();
        long long taskId = request != nullptr ? request->GetTaskId() : 0;

#ifdef ONLY_MAIN_THREAD
		this->mRedisComponent->OnResponse(self, taskId, this->mCurResponse);
#else
		asio::io_service & taskThread = App::Get()->GetThread();
		taskThread.post(std::bind(&RedisComponent::OnResponse,
			this->mRedisComponent, self, taskId, this->mCurResponse));
#endif
        this->SendFromMessageQueue();
	}

    void RedisClientContext::OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message)
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
        this->ReceiveLine();
    }

	bool RedisClientContext::AuthUser()
	{
		if(!this->ConnectSync())
		{
			CONSOLE_LOG_ERROR("connect redis [" << this->mConfig.Address << "] failure");
			return false;
		}
        assert(this->mSendBuffer.size() == 0);
        assert(this->mRecvBuffer.size() == 0);
		if(!this->mConfig.Password.empty())  //验证密码
		{
			std::shared_ptr<RedisRequest> authCommand
					= std::make_shared<RedisRequest>("AUTH");
			authCommand->AddParameter(this->mConfig.Password);
			std::shared_ptr<RedisResponse> response(new RedisResponse());
			if (this->SendSync(authCommand) <= 0 || this->RecvLineSync() <= 0)
			{
				return false;
			}
            std::istream is(&this->mRecvBuffer);
			if (response->OnRecvLine(is) != 0 || !response->IsOk())
			{
				CONSOLE_LOG_ERROR("auth redis user faliure");
				return false;
			}
		}

		//切换数据库
		std::shared_ptr<RedisRequest> selectCommand
			= std::make_shared<RedisRequest>("SELECT");
		selectCommand->AddParameter(this->mConfig.Index);
		if(this->SendSync(selectCommand) <= 0 || this->RecvLineSync() <= 0)
		{
			return false;
		}

        std::istream is(&this->mRecvBuffer);
		std::shared_ptr<RedisResponse> response2(new RedisResponse());
		if(response2->OnRecvLine(is) != 0 || !response2->IsOk())
		{
			CONSOLE_LOG_ERROR("auth redis user faliure");
			return false;
		}
		return true;
	}
}
