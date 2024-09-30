//
// Created by zmhy0073 on 2022/1/19.
//

#include"SessionClient.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Util/Tools/TimeHelper.h"

namespace http
{
	SessionClient::SessionClient(Component* component)
			: tcp::TcpClient(1024 * 4), mComponent(component)
	{

	}

	void SessionClient::StartReceiveBody()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReadSome();
#else
		Asio::Socket& sock = this->mSocket->Get();
		const Asio::Executor& exec = sock.get_executor();
		asio::post(exec, [this] { this->ReadSome(5); });
#endif
	}

	void SessionClient::StartReceive(int sockId, tcp::Socket* socket, int timeout)
	{
		assert(sockId > 0);
		this->SetSocket(socket);
		this->mRequest.SetSockId(sockId);
#ifdef __DEBUG__
		long long nowTime = help::Time::NowMil();
		this->mRequest.Header().Add("t", nowTime);
#endif
#ifdef ONLY_MAIN_THREAD
		this->ReadLine(timeout);
#else
		Asio::Socket& sock = this->mSocket->Get();
		const Asio::Executor& exec = sock.get_executor();
		asio::post(exec, [this, timeout, &sock] { this->ReadLine(timeout); });
#endif
	}

	void SessionClient::StartClose(int code)
	{
#ifdef ONLY_MAIN_THREAD
		this->ClosetClient(code);
#else
		Asio::Socket& sock = this->mSocket->Get();
		const Asio::Executor& exec = sock.get_executor();
		asio::post(exec, [this, code] { this->ClosetClient(code); });
#endif
	}

	bool SessionClient::StartWriter(HttpStatus status)
	{
		this->mResponse.SetCode(status);
		return this->StartWriter();
	}

	bool SessionClient::StartWriter()
	{
		if (this->mSocket == nullptr)
		{
			return false;
		}
		const http::Url& url = this->mRequest.GetUrl();
		http::Head& httpHead = this->mResponse.Header();
		if (this->mResponse.Code() == HttpStatus::OK)
		{
			if (this->mRequest.Header().KeepAlive())
			{
				httpHead.SetKeepAlive(true);
				httpHead.Add("Keep-Alive", "timeout=8");
			}
			httpHead.SetKeepAlive(false);
		}
		if (this->mResponse.GetBody() == nullptr)
		{
			httpHead.Add(http::Header::ContentLength, 0);
		}
#ifdef __DEBUG__
		std::string t;
		long long time = 0;
		if (this->mRequest.Header().Del("t", t))
		{
			time = std::stoll(t);
			time = help::Time::NowMil() - time;
		}
		const HttpStatus status = this->mResponse.Code();
		const std::string error = HttpStatusToString(status);
		if (status != HttpStatus::OK)
		{
			LOG_WARN("({}ms) [{}:{}]=>{} ({})", time,
					url.Method(), this->mRequest.GetSockId(), url.ToStr(), error);
		}
#endif
#ifdef ONLY_MAIN_THREAD
		this->Write(this->mResponse);
#else
		Asio::Socket& sock = this->mSocket->Get();
		asio::post(sock.get_executor(), [this]
		{
			this->Write(this->mResponse);
		});
#endif
		return true;
	}

	void SessionClient::OnTimeout(tcp::TimeoutFlag flag)
	{
		switch (flag)
		{
			case tcp::TimeoutFlag::ReadLine:
			case tcp::TimeoutFlag::ReadSome:
			case tcp::TimeoutFlag::ReadCount:
				this->ClosetClient(XCode::ReadDataTimeout);
				break;
			case tcp::Write:
				this->ClosetClient(XCode::SendDataTimeout);
				break;
			default:
				this->ClosetClient(XCode::NetTimeout);
				break;
		}
		//LOG_FATAL("{}", this->mRequest.ToString());
	}

	void SessionClient::OnReadError(const Asio::Code& code)
	{
		switch (code.value())
		{
			case asio::error::eof:
			case asio::error::operation_aborted:
				this->ClosetClient(XCode::Ok);
				break;
			default:
//				LOG_ERROR("receive code : {}", code.message());
//				const std::string & addr = this->mSocket->GetAddress();
//				LOG_ERROR("[{}] : {}", addr, this->mRequest.ToString());
				this->ClosetClient(XCode::NetReadFailure);
				break;
		}
	}

	void SessionClient::OnReceiveLine(std::istream& is, size_t size)
	{
		this->OnReceiveMessage(is, size);
	}

	void SessionClient::OnReadPause()
	{
		if (this->mRequest.Header().Count() == 0)
		{
			this->OnComplete(HttpStatus::INTERNAL_SERVER_ERROR);
			return;
		}
		const std::string& ip = this->mSocket->GetIp();
		this->mRequest.Header().Set(http::Header::RealIp, ip);
#ifdef ONLY_MAIN_THREAD
		this->mComponent->OnReadHead(&this->mRequest, &this->mResponse);
#else
		Asio::Context& t = acs::App::GetContext();
		t.post([this, req = &this->mRequest, res = &this->mResponse]
		{
			this->mComponent->OnReadHead(req, res);
		});
#endif
	}

	void SessionClient::OnComplete(HttpStatus status)
	{
		this->StopTimer();
		if (status != HttpStatus::OK)
		{
			this->mResponse.SetCode(status);
			this->mResponse.Header().SetKeepAlive(false);
			this->Write(this->mResponse);
		}
		else
		{
#ifdef ONLY_MAIN_THREAD
			this->mComponent->OnMessage(&this->mRequest, &this->mResponse);
#else
			Asio::Context& t = acs::App::GetContext();
			t.post([this, req = &this->mRequest, res = &this->mResponse]
			{
				this->mComponent->OnMessage(req, res);
			});
#endif
		}
	}

	void SessionClient::OnReceiveMessage(std::istream& is, size_t size)
	{
		switch (this->mRequest.OnRecvMessage(is, size))
		{
		case tcp::ReadDone:
			this->OnComplete(HttpStatus::OK);
			break;
		case tcp::ReadOneLine:
			this->ReadLine();
			break;
		case tcp::ReadSomeMessage:
			this->ReadSome();
			break;
		case tcp::ReadError:
			this->OnComplete(HttpStatus::INTERNAL_SERVER_ERROR);
			break;
		case tcp::ReadDecodeError:
			this->OnComplete(HttpStatus::BAD_REQUEST);
			break;
		case tcp::ReadPause:
			this->OnReadPause();
			break;
		case tcp::PacketLong:
			this->OnComplete(HttpStatus::PAYLOAD_TOO_LARGE);
			break;
		case tcp::ReadAll:
			this->ReadAll();
			break;
		}
	}

	void SessionClient::OnSendMessage()
	{
		if (this->mResponse.Header().KeepAlive())
		{
			this->Clear();
			this->ReadLine(10);
			return;
		}
		this->ClosetClient(XCode::Ok);
	}

	void SessionClient::OnSendMessage(const asio::error_code& error)
	{
		const std::string& addr = this->mSocket->GetAddress();
		CONSOLE_LOG_DEBUG("send to [{}] code = {}", addr, error.message());
		{
			this->ClosetClient(XCode::SendMessageFail);
		}
	}

	void SessionClient::Clear()
	{
		this->mPath = this->mRequest.GetUrl().Path();
		this->mPath.append("&");
		this->mPath.append(this->mRequest.GetUrl().GetQuery().ToStr());
		this->StopTimer();
		this->ClearBuffer();
		this->mRequest.Clear();
		this->mResponse.Clear();
	}

	void SessionClient::ClosetClient(int code)
	{
		int sockId = this->mRequest.GetSockId();
		if (sockId == 0)
		{
			return;
		}
		this->Clear();
		this->mSocket->Close();
		this->mRequest.SetSockId(0);
		Asio::Socket& sock = this->mSocket->Get();
		asio::post(sock.get_executor(), [this, code, sockId]()
		{
#ifdef ONLY_MAIN_THREAD
			this->mComponent->OnCloseSocket(sockId, code);
#else
			Asio::Context& t = acs::App::GetContext();
			t.post([this, code, sockId] { this->mComponent->OnCloseSocket(sockId, code); });
#endif
		});

	}
}