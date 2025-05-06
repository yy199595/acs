#include"ListenerComponent.h"

#include <utility>
#include"Network/Tcp/Socket.h"
#include"Entity/Actor/App.h"
#include"Network/Tcp/Asio.h"
#include"Core/System/System.h"
#ifndef ONLY_MAIN_THREAD
#include"Core/Thread/ThreadSync.h"
#endif
#include"Util/Tools/TimeHelper.h"
#include"Server/Component/ThreadComponent.h"

namespace acs
{

	ListenerComponent::ListenerComponent()
#ifdef __ENABLE_OPEN_SSL__
		: mSslCtx(asio::ssl::context::tlsv12_server)
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
		this->mTcpListen = this->GetComponent<ITcpListen>(config.component);
		if(this->mTcpListen == nullptr)
		{
			LOG_ERROR("not find listen => {}", config.component);
			return false;
		}
		std::string ip;
		this->mExecutor = io.get_executor();
		this->mAcceptor = std::make_unique<Asio::Acceptor>(io);
#ifdef __ENABLE_OPEN_SSL__
		if (!config.cert.empty() && !config.key.empty())
		{
			try
			{
				const std::string& key = config.key;
				const std::string& cert = config.cert;
				this->mSslCtx.use_certificate_chain_file(cert);
				this->mSslCtx.use_private_key_file(key, asio::ssl::context::pem);
				this->mSslCtx.set_options(asio::ssl::context::default_workarounds |
						asio::ssl::context::no_sslv2 | asio::ssl::context::no_sslv3 |
						asio::ssl::context::no_tlsv1 | asio::ssl::context::no_tlsv1_1 |
						asio::ssl::context::single_dh_use | asio::ssl::context::no_compression);
			}
			catch (std::system_error & error)
			{
				LOG_ERROR("listen [{}] => {}", config.name, error.what());
				return false;
			}
		}
#endif
#ifdef ONLY_MAIN_THREAD
		Asio::EndPoint ep(asio::ip::address(), config.port);
		while (!this->mAcceptor->is_open())
		{
			try
			{
				this->mAcceptor->open(ep.protocol());
				this->mAcceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
#if !defined(__OS_WIN__) && defined(SO_REUSEPORT)
				int one = 1;
				setsockopt(this->mAcceptor->native_handle(), SOL_SOCKET,  SO_REUSEPORT, &one, sizeof(one));
#endif
				this->mAcceptor->bind(ep);
				this->mAcceptor->listen();
			}
			catch (std::system_error& error)
			{
				this->mAcceptor->close();
				CONSOLE_LOG_ERROR("listen [{}] {}", config.address, error.what());
			}
		}
#else
		custom::ThreadSync<bool> threadSync;
		asio::post(io, [&threadSync, this, config]
			{
				Asio::EndPoint ep(asio::ip::address(), config.port);
				while (!this->mAcceptor->is_open())
				{
					try
					{
						this->mAcceptor->open(ep.protocol());
						this->mAcceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
#if !defined(__OS_WIN__) && defined(SO_REUSEPORT)
						int one = 1;
						setsockopt(this->mAcceptor->native_handle(), SOL_SOCKET,  SO_REUSEPORT, &one, sizeof(one));
#endif
						this->mAcceptor->bind(ep);
						if(config.max_conn == 0)
						{
							this->mAcceptor->listen();
							break;
						}
						this->mAcceptor->listen(config.max_conn);
					}
					catch (std::system_error& error)
					{
						this->mAcceptor->close();
						std::this_thread::sleep_for(std::chrono::seconds(3));
						CONSOLE_LOG_ERROR("listen [{}] {}", config.address, error.what());
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
		asio::post(this->mExecutor, [this] { this->Accept(); });
		return true;
	}

	tcp::Socket* ListenerComponent::CreateSocket()
	{
#ifdef __ENABLE_OPEN_SSL__
		if (!this->mConfig.cert.empty() && !this->mConfig.key.empty())
		{
			return this->mThreadComponent->CreateSocket(this->mSslCtx);
		}
#endif
		return this->mThreadComponent->CreateSocket();
	}

	void ListenerComponent::Accept()
	{
		if (this->mAcceptor == nullptr)
		{
			return;
		}
		tcp::Socket* sock = this->CreateSocket();
		auto callback = [this, sock](const Asio::Code& code)
		{
			do
			{
				this->Accept();
				if (code.value() != Asio::OK)
				{
					sock->Destroy();
					break;
				}
				sock->Init();
				//CONSOLE_LOG_INFO("{} listen => {}", this->GetName(), sock->GetAddress())
#ifdef __ENABLE_OPEN_SSL__
				if (sock->IsOpenSsl())
				{
					Asio::ssl::Socket& ssl = sock->SslSocket();
					ssl.async_handshake(asio::ssl::stream_base::server,
							[sock, this](const Asio::Code& code1)
							{
								if (code1.value() != Asio::OK)
								{
									sock->Destroy();
									return;
								}
								Asio::Context& io = this->mApp->GetContext();
								asio::post(io, [this, sock] { this->OnAcceptSocket(sock); });
							});
					break;
				}
#endif
				Asio::Context& io = this->mApp->GetContext();
				asio::post(io, [this, sock] { this->OnAcceptSocket(sock); });
			}
			while (false);
		};
#ifdef __ENABLE_OPEN_SSL__
		if(sock->IsOpenSsl())
		{
			Asio::ssl::Socket & sslSocket = sock->SslSocket();
			this->mAcceptor->async_accept(sslSocket.lowest_layer(), callback);
			return;
		}
#endif
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
		LOG_WARN("stop listen [{}]", this->mConfig.address);
		return code.value() == Asio::OK;
#else
		custom::ThreadSync<bool> threadSync;
		asio::post(this->mExecutor, [&threadSync, this] ()
		{
			Asio::Code code;
			this->mAcceptor->close(code);
			threadSync.SetResult(code.value() == Asio::OK);
		});
		return threadSync.Wait();
#endif
	}
}
