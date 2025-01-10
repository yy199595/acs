#include"ListenerComponent.h"

#include <utility>
#include"Network/Tcp/Socket.h"
#include"Entity/Actor/App.h"
#include"Network/Tcp/Asio.h"
#include"Core/System/System.h"
#include"Core/Thread/ThreadSync.h"
#include"Util/Tools/TimeHelper.h"
#include"Server/Component/ThreadComponent.h"

namespace acs
{

	ListenerComponent::ListenerComponent()
#ifdef __ENABLE_OPEN_SSL__
		: mSslCtx(asio::ssl::context::tlsv12)
#endif
	{
		this->mAcceptor = nullptr;
		this->mTcpListen = nullptr;
		this->mThreadComponent = nullptr;
	}

	ListenerComponent::~ListenerComponent()
	{
		if (this->mAcceptor != nullptr)
		{
			Asio::Code err;
			this->mAcceptor->close(err);
		}
	}

	bool ListenerComponent::StartListen(const ListenConfig& config)
	{
		this->mThreadComponent = this->GetComponent<ThreadComponent>();
		if(this->mThreadComponent == nullptr)
		{
			return false;
		}
		Asio::Context& io = this->mThreadComponent->GetContext();
		this->mTcpListen = this->GetComponent<ITcpListen>(config.Component);
		if(this->mTcpListen == nullptr)
		{
			LOG_ERROR("not find listen => {}", config.Component);
			return false;
		}
		this->mAcceptor = std::make_unique<Asio::Acceptor>(io);
#ifdef __ENABLE_OPEN_SSL__
		if (!config.Cert.empty())
		{
			const std::string& key = config.Key;
			const std::string& cert = config.Cert;
			this->mSslCtx.use_certificate_chain_file(cert);
			this->mSslCtx.use_private_key_file(key, asio::ssl::context::pem);
		}
#endif
#ifdef ONLY_MAIN_THREAD
		Asio::EndPoint ep(asio::ip::address(), config.Port);
		while (!this->mAcceptor->is_open())
		{
			try
			{
				this->mAcceptor->open(ep.protocol());
				this->mAcceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
				this->mAcceptor->bind(ep);
				this->mAcceptor->listen();
			}
			catch (std::system_error& error)
			{
				this->mAcceptor->close();
				LOG_ERROR("listen [{}] {}", config.Addr, error.what());
			}
		}
#else
		custom::ThreadSync<bool> threadSync;
		asio::post(io, [&threadSync, this, config]
			{
				Asio::EndPoint ep(asio::ip::address(), config.Port);
				while (!this->mAcceptor->is_open())
				{
					try
					{
						this->mAcceptor->open(ep.protocol());
						this->mAcceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
						this->mAcceptor->bind(ep);
						this->mAcceptor->listen();
					}
					catch (std::system_error& error)
					{
						this->mAcceptor->close();
						std::this_thread::sleep_for(std::chrono::seconds(3));
						CONSOLE_LOG_ERROR("listen [{}] {}", config.Addr, error.what());
					}
				}
				threadSync.SetResult(this->mAcceptor->is_open());
			});
		if (!threadSync.Wait())
		{
			return false;
		}
#endif
		this->mConfig = config;
		asio::post(io, [this] { this->Accept(); });
		return true;
	}

	void ListenerComponent::Accept()
	{
		if (this->mAcceptor == nullptr)
		{
			return;
		}
		tcp::Socket* sock = nullptr;
#ifdef __ENABLE_OPEN_SSL__
		if (!this->mConfig.Cert.empty())
		{
			sock = this->mThreadComponent->CreateSocket(this->mSslCtx);
		}
#endif
		if (this->mConfig.Cert.empty())
		{
			sock = this->mThreadComponent->CreateSocket();
		}

		auto callback = [this, sock](const Asio::Code& code)
		{
			do
			{
				if (code.value() != Asio::OK)
				{
					sock->Destroy();
					break;
				}
				sock->Init();
#ifdef __ENABLE_OPEN_SSL__
				if (sock->IsOpenSsl())
				{
					Asio::Code code1;
					Asio::ssl::Socket& ssl = sock->SslSocket();
					ssl.handshake(asio::ssl::stream_base::server, code1);
					if (code1.value() != Asio::OK)
					{
						sock->Destroy();
						break;
					}
				}
#endif
#ifdef ONLY_MAIN_THREAD
				this->OnAcceptSocket(sock);
#else
				Asio::Context& io = this->mApp->GetContext();
				asio::post(io, [this, sock] { this->OnAcceptSocket(sock); });
#endif
			} while (false);
			if (this->mAcceptor == nullptr)
			{
				return;
			}
#ifdef ONLY_MAIN_THREAD
			this->Accept();
#else
			const Asio::Executor& executor = this->mAcceptor->get_executor();
			asio::post(executor, [this] { this->Accept(); });
#endif
		};

		this->mAcceptor->async_accept(sock->Get(), callback);
	}

	void ListenerComponent::OnAcceptSocket(tcp::Socket* sock)
	{
		if (!this->mTcpListen->OnListen(sock))
		{
			sock->Destroy();
		}
	}

	bool ListenerComponent::StopListen()
	{
#ifdef ONLY_MAIN_THREAD
		Asio::Code code;
		this->mAcceptor->close(code);
		LOG_WARN("stop listen [{}]", this->mConfig.Addr);
		return code.value() == Asio::OK;
#else
		custom::ThreadSync<bool> threadSync;
		const Asio::Executor & executor = this->mAcceptor->get_executor();
		asio::post(executor, [&threadSync, this] ()
		{
			Asio::Code code;
			this->mAcceptor->close(code);
			threadSync.SetResult(code.value() == Asio::OK);
		});
		LOG_DEBUG("stop listen [{}]", this->mConfig.Addr);
		return threadSync.Wait();
#endif
	}
}
