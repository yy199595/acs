//
// Created by zmhy0073 on 2022/1/19.
//

#include"HttpSession.h"
#include"XCode/XCode.h"
#include"Util/Tools/TimeHelper.h"

namespace http
{
	Session::Session(Component* component, Asio::Context& io)
			: tcp::Client(0), mComponent(component), mMainContext(io)
	{

	}

	Session::~Session() noexcept = default;

	void Session::StartReceiveBody()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReadSome();
#else
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(context, [this, self] { this->ReadSome(5); });
#endif
	}

	void Session::StartReceiveBody(std::unique_ptr<http::Content> content)
	{
		this->mRequest.SetBody(std::move(content));
#ifdef ONLY_MAIN_THREAD
		this->ReadSome();
#else
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(context, [this, self] { this->ReadSome(5); });
#endif
	}

	void Session::StartReceive(int sockId, tcp::Socket* socket, int timeout)
	{
		assert(sockId > 0);
		this->SetSocket(socket);
		this->mRequest.SetSockId(sockId);
#ifdef __DEBUG__
		long long nowTime = help::Time::NowMil();
		this->mRequest.Header().Add("t", nowTime);
#endif
#ifdef ONLY_MAIN_THREAD
		this->ReadLine();
#else
		Asio::Context & context = this->mSocket->GetContext();
		asio::post(context, [this, timeout, self = this->shared_from_this()] {
			this->ReadLine();
			this->StartTimer(timeout, tcp::timeout::read);
		});
#endif
	}

	void Session::StartClose(int code)
	{
#ifdef ONLY_MAIN_THREAD
		this->ClosetClient(code);
#else
		Asio::Socket& sock = this->mSocket->Get();
		const Asio::Executor& exec = sock.get_executor();
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(exec, [this, code, self] { this->ClosetClient(code); });
#endif
	}

	bool Session::StartWriter(HttpStatus status)
	{
		this->mResponse.SetCode(status);
		return this->StartWriter();
	}

	bool Session::StartWriter(HttpStatus status, std::unique_ptr<Content> data)
	{
		this->mResponse.SetCode(status);
		this->mResponse.SetContent(std::move(data));
		return this->StartWriter();
	}

	bool Session::StartWriter()
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
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(this->mSocket->GetContext(), [this, self]
		{
			this->Write(this->mResponse);
		});
#endif
		return true;
	}

	void Session::OnReadError(const Asio::Code& code)
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

	void Session::OnReceiveLine(std::istream& is, size_t size)
	{
		asio::error_code code;
		this->OnReceiveMessage(is, size, code);
	}

	void Session::OnReadPause()
	{
		this->StopTimer();
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
		asio::post(this->mMainContext, [this, req = &this->mRequest, res = &this->mResponse]
		{
			this->mComponent->OnReadHead(req, res);
		});
#endif
	}

	void Session::OnComplete(HttpStatus status)
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
			std::shared_ptr<Client> self = this->shared_from_this();
			asio::post(this->mMainContext, [this, self, req = &this->mRequest, res = &this->mResponse]
			{
				this->mComponent->OnMessage(req, res);
			});
#endif
		}
	}

	void Session::OnReceiveMessage(std::istream& is, size_t size, const Asio::Code&)
	{
		int flag = this->mRequest.OnRecvMessage(is, size);
		if (flag > 0)
		{
			this->ReadLength(flag);
			return;
		}
		switch (flag)
		{
			case tcp::read::done:
				this->OnComplete(HttpStatus::OK);
				break;
			case tcp::read::line:
				this->ReadLine();
				break;
			case tcp::read::some:
				this->ReadSome();
				break;
			case tcp::read::error:
				this->OnComplete(HttpStatus::INTERNAL_SERVER_ERROR);
				break;
			case tcp::read::decode_error:
				this->OnComplete(HttpStatus::BAD_REQUEST);
				break;
			case tcp::read::pause:
				this->OnReadPause();
				break;
			case tcp::read::big_long:
				this->OnComplete(HttpStatus::PAYLOAD_TOO_LARGE);
				break;
			case tcp::read::all:
				this->ReadAll();
				break;
		}
	}

	void Session::OnSendMessage(size_t size)
	{
		if (this->mResponse.Header().KeepAlive())
		{
			this->Clear();
			this->ReadLine(10);
			return;
		}
		this->ClosetClient(XCode::Ok);
	}

	void Session::OnSendMessage(const asio::error_code& error)
	{
		this->ClosetClient(XCode::SendMessageFail);
	}

	void Session::Clear()
	{
		const http::Url & url = this->mRequest.GetUrl();

		this->mPath = url.Path();
		this->mPath.append("&");
		this->mPath.append(url.GetQuery().ToStr());

		this->StopTimer();
		this->ClearBuffer();
		this->mRequest.Clear();
		this->mResponse.Clear();
	}

	void Session::ClosetClient(int code)
	{
		int sockId = this->mRequest.GetSockId();
		if (sockId == 0)
		{
			return;
		}
		this->Clear();
#ifdef ONLY_MAIN_THREAD
		this->mComponent->OnClientError(sockId, code);
#else
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(this->mMainContext, [this, self, sockId, code]()
		{
			this->mComponent->OnClientError(sockId, code);
		});
#endif
		this->mSocket->Close();
		this->mRequest.SetSockId(0);
	}
}