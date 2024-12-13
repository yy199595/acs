//
// Created by leyi on 2023/7/27.
//

#include"Client.h"
#include"Entity/Actor/App.h"
#include"Util/Tools/TimeHelper.h"
#include"Core/Thread/ThreadSync.h"

namespace redis
{
	Client::Client(tcp::Socket* socket, const Config& config, Component* com)
		: tcp::Client(socket, 0), mConfig(config),
		  mAddress(config.Address), mComponent(com)
	{
		this->mRequest = nullptr;
		this->mResponse = nullptr;
	}

	void Client::StartReceive()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReadLine();
#else
		Asio::Socket& sock = this->mSocket->Get();
		const Asio::Executor& executor = sock.get_executor();
		asio::post(executor, [this]() { this->ReadLine(); });
#endif
	}

	bool Client::Start()
	{
#ifdef ONLY_MAIN_THREAD
		Asio::Code code;
		if(!this->ConnectSync(code))
		{
			return false;
		}
		return this->InitRedisClient(this->mConfig.Password);
#else
		custom::ThreadSync<bool> threadSync;
		Asio::Socket& sock = this->mSocket->Get();
		const Asio::Executor& executor = sock.get_executor();
		asio::post(executor, [&threadSync, this, &sock]
		{
			Asio::Code code;
			if (!this->ConnectSync(code))
			{
				threadSync.SetResult(false);
				return;
			}
			if (this->InitRedisClient(this->mConfig.Password))
			{
				threadSync.SetResult(true);
				return;
			}
			threadSync.SetResult(false);
		});
		return threadSync.Wait();
#endif
	}

	void Client::OnConnect(bool result, int count)
	{
		if (result)
		{
			this->mSocket->SetOption(tcp::OptionType::NoDelay, true);
			this->mSocket->SetOption(tcp::OptionType::KeepAlive, true);
			if (this->InitRedisClient(this->mConfig.Password))
			{
				if(this->mRequest != nullptr)
				{
					this->Write(*this->mRequest);
				}
			}
#ifdef ONLY_MAIN_THREAD
			this->mComponent->OnConnectOK(this->mConfig.Index);
#else
			Asio::Socket& sock = this->mSocket->Get();
			const Asio::Executor& executor = sock.get_executor();
			asio::post(executor, [this]()
			{
				this->mComponent->OnConnectOK(this->mConfig.Index);
			});
#endif
		}
		else if(this->mRequest != nullptr)
		{
			this->OnResponse();
		}

	}

	bool Client::InitRedisClient(const std::string& pwd)
	{
		assert(this->mSendBuffer.size() == 0);
		assert(this->mRecvBuffer.size() == 0);
		if (!pwd.empty())  //验证密码
		{
			std::unique_ptr<redis::Request> authCommand = redis::Request::Make("AUTH", this->mConfig.Password);
			std::unique_ptr<redis::Response> response = this->ReadResponse(std::move(authCommand));
			if (response == nullptr || !response->IsOk())
			{
				this->mSocket->Close();
				CONSOLE_LOG_ERROR("auth redis user failure {}", response->ToString());
				return false;
			}
		}
		return true;
	}

	void Client::Send(std::unique_ptr<Request> command)
	{
#ifdef ONLY_MAIN_THREAD
		this->mRequest = std::move(command);
		this->Write(*this->mRequest);
#else
		assert(!command->GetCommand().empty());
		Asio::Socket& sock = this->mSocket->Get();
		const Asio::Executor& executor = sock.get_executor();
		asio::post(executor, [this, request = command.release()]
		{
			this->Write(*request);
			this->mRequest.reset(request);
		});
#endif

	}

	void Client::OnSendMessage()
	{
		this->ReadLine();
		//LOG_WARN("redis cmd : {}", this->mRequest->ToString());
	}

	void Client::OnSendMessage(const Asio::Code& code)
	{
		this->ClearRecvStream();
		this->ClearSendStream();
		this->Connect(5);
	}

	void Client::OnReadError(const Asio::Code& code)
	{
		if(this->mRequest != nullptr)
		{
			if(this->mResponse != nullptr)
			{
				this->mResponse->SetError(code.message());
				{
					this->OnResponse();
				}
			}
		}
	}

	void Client::OnReceiveLine(std::istream& is, size_t size)
	{
		if (this->mResponse == nullptr)
		{
			this->mResponse = std::make_unique<redis::Response>();
		}
		this->OnReceiveOnce(this->mResponse->OnRecvLine(is, size));
	}

	void Client::OnReceiveOnce(int flag)
	{
		switch (flag)
		{
			case tcp::ReadDone:
				this->OnResponse();
				break;
			case tcp::ReadError:
			{
				this->ClearRecvStream();
				this->OnResponse();
				break;
			}
			case tcp::ReadOneLine:
				this->ReadLine();
				break;
			default:
				this->ReadSome();
				break;
		}
	}

	void Client::OnReceiveMessage(std::istream& is, size_t size, const Asio::Code &)
	{
		if (this->mResponse == nullptr)
		{
			return;
		}
		this->OnReceiveOnce(this->mResponse->OnRecvMessage(is, size));
	}

	std::unique_ptr<redis::Response> Client::Sync(std::unique_ptr<redis::Request> request)
	{
#ifdef ONLY_MAIN_THREAD
		return this->ReadResponse(std::move(request));
#else
		custom::ThreadSync<bool> threadSync;
		std::unique_ptr<redis::Response> response;
		Asio::Socket& sock = this->mSocket->Get();
		const Asio::Executor& executor = sock.get_executor();
		asio::post(executor, [this, &request, &threadSync, &response]
		{
			response = this->ReadResponse(std::move(request));
			threadSync.SetResult(true);
		});
		threadSync.Wait();
		return response;
#endif
	}

	std::unique_ptr<redis::Response> Client::ReadResponse(std::unique_ptr<redis::Request> request)
	{
		std::unique_ptr<redis::Response> redisResponse = std::make_unique<redis::Response>();
		if (!this->SendSync(*request))
		{
			redisResponse->SetError("sync send redis cmd fai");
			LOG_ERROR("sync send redis cmd fail : {}", request->ToString());
			return redisResponse;
		}
		size_t readSize = 0;
		std::istream is(&this->mRecvBuffer);
	READ_LINE:
		if (!this->RecvLineSync(readSize))
		{
			redisResponse->SetError("recv line message fail");
			return redisResponse;
		}
		int length = redisResponse->OnRecvLine(is, readSize);
		if (length == -1)
		{
			goto READ_LINE;
		}
		else if (length > 0)
		{
			if (!this->RecvSync(length, readSize))
			{
				redisResponse->SetError("recv len message fail");
				return redisResponse;
			}
			if (redisResponse->OnRecvMessage(is, readSize) == -1)
			{
				goto READ_LINE;
			}
		}
		return redisResponse;
	}

	void Client::OnResponse()
	{
		int id = this->mConfig.Id;
		if (this->mResponse == nullptr)
		{
			this->mResponse = std::make_unique<redis::Response>();
			this->mResponse->SetError("unknown error");
		}
#ifdef ONLY_MAIN_THREAD
		this->mComponent->OnMessage(id, this->mRequest.release(), this->mResponse.release());
#else
		Asio::Context& t = acs::App::GetContext();
		t.post([this, req = this->mRequest.release(), id, res = this->mResponse.release()]
		{
			this->mComponent->OnMessage(id, req, res);
		});
#endif
	}
}