//
// Created by zmhy0073 on 2022/1/15.
//
#include"RedisClientContext.h"
#include"Util/FileHelper.h"
#include"Component/Scene/LoggerComponent.h"
#include"Component/Redis/RedisComponent.h"
namespace Sentry
{
	RedisClientContext::RedisClientContext(std::shared_ptr<SocketProxy> socket,
		const RedisConfig& config, RedisComponent* component)
		: Tcp::TcpContext(socket, 1024 * 100), mConfig(config), mRedisComponent(component)
	{
		this->mSocket = socket;
		if (!config.Password.empty())
		{
			this->mAuthCommand = std::make_shared<RedisRequest>("AUTH");
			this->mAuthCommand->AddParameter(config.Password);
		}
		if (config.Index != 0)
		{
			this->mSelectCommand = std::make_shared<RedisRequest>("SELECT");
			this->mSelectCommand->AddParameter(config.Index);
		}
		printf("create new redis client %s\n", config.Name.c_str());
	}

	RedisClientContext::~RedisClientContext() noexcept
	{
		LOG_WARN("remove redis client " << this->mConfig.Name << "[" << this->mConfig.Address << "]");
	}

	void RedisClientContext::OnConnect(const asio::error_code& error, int count)
	{
		if (error)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(error.message());
			CONSOLE_LOG_ERROR("connect redis server [" <<
													   this->mConfig.Address << "] failure count = "
													   << count);
#endif
			AsioContext& context = this->mSocket->GetThread();
			this->mTimer = std::make_shared<asio::steady_timer>(context, std::chrono::seconds(5));
			this->mTimer->async_wait(std::bind(&RedisClientContext::Connect, this->shared_from_this()));
			return;
		}
#ifdef __DEBUG__
		CONSOLE_LOG_ERROR("connect redis server [" << this->mConfig.Name << "=>" << this->mConfig.Address << "] successful");
#endif
		if (this->mTimer != nullptr)
		{
			asio::error_code code;
			this->mTimer->cancel(code);
		}
		if (!this->mCommands.empty())
		{
			this->Send(this->mCommands.front());
		}
	}

	void RedisClientContext::SendCommand(std::shared_ptr<RedisRequest> command)
	{
#ifdef ONLY_MAIN_THREAD
		this->AddCommandQueue(command);
#else
		this->mNetworkThread.Invoke(&RedisClientContext::AddCommandQueue, this, command);
#endif
	}

	void RedisClientContext::StartReceiveMessage()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveLine();
#else
		this->mNetworkThread.Invoke(&RedisClientContext::ReceiveLine, this);
#endif
	}

	void RedisClientContext::AddCommandQueue(std::shared_ptr<RedisRequest> command)
	{
		if (this->mCommands.empty())
		{
			this->Send(command);
		}
		this->mCommands.emplace_back(command);
	}

	void RedisClientContext::OnReceiveLine(const asio::error_code& code, asio::streambuf& buffer)
	{
		if (code)
		{
			CONSOLE_LOG_ERROR(code.message());
		}
		if (this->mCurResponse == nullptr)
		{
			this->mCurResponse = std::make_shared<RedisResponse>();
		}
		std::iostream os(&buffer);
		int lenght = this->mCurResponse->OnRecvLine(os);
		if (lenght == 0)
		{
			this->OnReadComplete();
			this->mCurResponse = nullptr;
		}
		else if (lenght == -1)
		{
			this->ReceiveLine();
		}
		else
		{
			this->ReceiveMessage(lenght);
		}
	}

	void RedisClientContext::OnReceiveMessage(const asio::error_code& code, asio::streambuf& buffer)
	{
		std::iostream os(&buffer);
		if (this->mCurResponse->OnRecvMessage(os) == 0)
		{
			this->OnReadComplete();
			this->mCurResponse = nullptr;
			return;
		}
		this->ReceiveLine();
	}

	void RedisClientContext::OnReadComplete()
	{
		long long taskId = 0;
		SharedRedisClient self = this->Cast<RedisClientContext>();
		if (!this->mCommands.empty())
		{
			if (this->mCommands.front() == this->mAuthCommand
				&& !this->mCurResponse->IsOk()) //验证失败
			{
				while (!this->mCommands.empty())
				{
					std::shared_ptr<RedisRequest> request = this->mCommands.front();
					if (request->GetTaskId() != 0)
					{
						taskId = request->GetTaskId();
#ifdef ONLY_MAIN_THREAD
						this->mRedisComponent->OnResponse(self, taskId,
							std::make_shared<RedisResponse>(*this->mCurResponse));
#else
						std::shared_ptr<RedisResponse> response1 = std::make_shared<RedisResponse>(*this->mCurResponse);
						this->mNetworkThread.Invoke(&RedisComponent::OnResponse, this->mRedisComponent, self, taskId, response1);
#endif
					}
					this->mCommands.pop_front();
				}
				return;
			}
			taskId = this->mCommands.front()->GetTaskId();
			this->mCommands.pop_front();
			if (!this->mCommands.empty())
			{
				this->Send(this->mCommands.front());
			}
		}
#ifdef ONLY_MAIN_THREAD
		this->mRedisComponent->OnResponse(self, taskId, this->mCurResponse);
#else
		this->mNetworkThread.Invoke(&RedisComponent::OnResponse, this->mRedisComponent, self, taskId, this->mCurResponse);
#endif
	}

    void RedisClientContext::OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message)
    {
		std::string json = dynamic_pointer_cast<RedisRequest>(message)->ToJson();
		if (code)
        {
			this->ConnectServer();
			CONSOLE_LOG_ERROR("send redis cmd failure : " << json);
			return;
        }
		//CONSOLE_LOG_ERROR("send redis cmd successful : " << json);
		this->ReceiveLine();
    }
	void RedisClientContext::ConnectServer()
	{
		this->Connect();
		if(this->mSelectCommand != nullptr)
		{
			this->mCommands.push_front(this->mSelectCommand);
		}
		if(this->mAuthCommand != nullptr)
		{
			this->mCommands.push_front(this->mAuthCommand);
		}
	}
}
