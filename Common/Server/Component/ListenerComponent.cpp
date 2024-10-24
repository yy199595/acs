#include"ListenerComponent.h"
#include"Network/Tcp/Socket.h"
#include"Entity/Actor/App.h"
#include"Network/Tcp/Asio.h"
#include"Core/System/System.h"
#include"Core/Thread/ThreadSync.h"
#include"Util/Tools/TimeHelper.h"
#include "Util/Tools/Math.h"
#include"Server/Component/ThreadComponent.h"

namespace acs
{
	ListenData::ListenData(const ListenConfig& conf)
#ifdef __ENABLE_OPEN_SSL__
		: SslCtx(asio::ssl::context::tlsv12), Config(conf)
	{
		if (!this->Config.Cert.empty())
		{
			const std::string& key = this->Config.Key;
			const std::string& cert = this->Config.Cert;
			this->SslCtx.use_certificate_chain_file(cert);
			this->SslCtx.use_private_key_file(key, asio::ssl::context::pem);
		}
	}
#else
	{}
#endif

	tcp::Socket* ListenData::Create(class ThreadComponent* component)
	{
#ifdef __ENABLE_OPEN_SSL__
		if (!this->Config.Cert.empty())
		{
			return component->CreateSocket(this->SslCtx);
		}
#endif
		return component->CreateSocket();
	}

	ListenerComponent::ListenerComponent()
	{
		this->mOffsetPort = 0;
		this->mAcceptor = nullptr;
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

	bool ListenerComponent::Awake()
	{
		std::vector<const char*> keys;
		std::unique_ptr<json::r::Value> jsonObj;
		const ServerConfig* config = ServerConfig::Inst();
		{
			LOG_CHECK_RET_FALSE(config->Get("core", jsonObj));
			LOG_CHECK_RET_FALSE(jsonObj->Get("host", this->mHost));
		}
		os::System::GetEnv("port", this->mOffsetPort);
		LOG_CHECK_RET_FALSE(config->Get("listen", jsonObj));
		LOG_CHECK_RET_FALSE(jsonObj->GetKeys(keys) > 0);
		for (const char* key : keys)
		{
			std::string value;
			std::unique_ptr<json::r::Value> jsonData;
			if (!jsonObj->Get(key, jsonData))
			{
				return false;
			}
			ListenConfig listenConfig;
			{
				listenConfig.Port = 0;
				listenConfig.Name = key;
				listenConfig.MaxConn = 0;
			}
			jsonData->Get("port", listenConfig.Port);
			jsonData->Get("max_conn", listenConfig.MaxConn);
			jsonData->Get("protocol", listenConfig.Protocol);
			jsonData->Get("component", listenConfig.Component);
#ifdef __ENABLE_OPEN_SSL__
			jsonData->Get("key", listenConfig.Key);
			jsonData->Get("cert", listenConfig.Cert);
#endif
			if (listenConfig.Port > 0 && !listenConfig.Component.empty())
			{
				listenConfig.Port += this->mOffsetPort;
				listenConfig.Addr = fmt::format("{}://{}:{}", listenConfig.Protocol, this->mHost, listenConfig.Port);
				this->mListenConfigs.emplace(listenConfig.Name, listenConfig);
			}
		}
		//this->mThread.Start(0, "listen");
		return true;
	}

	bool ListenerComponent::LateAwake()
	{
		std::unique_ptr<json::r::Value> sslObj;
		std::unique_ptr<json::r::Value> listenObj;
		this->mThreadComponent = this->GetComponent<ThreadComponent>();

		for (const auto& item : this->mListenConfigs)
		{
			const ListenConfig& config = item.second;
			if (config.Port > 0 && !this->StartListen(config))
			{
				LOG_ERROR("listen [{}] error", config.Addr);
				return false;
			}
		}
		auto iter = this->mListenDatas.begin();
		for (; iter != this->mListenDatas.end(); iter++)
		{
			const std::string& name = iter->first;
			LOG_INFO("[{}] listen [{}] ok", name, iter->second->Config.Addr);
		}
		//this->mThreadComponent->AddThread(&this->mThread);
		return true;
	}

	bool ListenerComponent::StartListen(const ListenConfig& config)
	{
		Asio::Context& io = this->mThreadComponent->GetContext();
		std::unique_ptr<ListenData> listenData = std::make_unique<ListenData>(config);
		{
			listenData->Acceptor = std::make_unique<Asio::Acceptor>(io);
			listenData->Listener = this->GetComponent<ITcpListen>(config.Component);
		}
		if (listenData->Listener == nullptr)
		{
			LOG_ERROR("not [{}] component {}", listenData->Config.Name, listenData->Config.Component);
			return false;
		}
#ifdef ONLY_MAIN_THREAD
		Asio::EndPoint ep(asio::ip::address(), config.Port);
		while (!listenData->Acceptor->is_open())
		{
			try
			{
				listenData->Acceptor->open(ep.protocol());
				listenData->Acceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
				listenData->Acceptor->bind(ep);
				listenData->Acceptor->listen();
			}
			catch (std::system_error& error)
			{
				listenData->Acceptor->close();
				LOG_ERROR("listen [{}] {}", config.Addr, error.what());
			}
		}
#else
		custom::ThreadSync<bool> threadSync;
		io.post([&threadSync, listenInfo = listenData.get(), this, config]
			{
				Asio::EndPoint ep(asio::ip::address(), config.Port);
				while (!listenInfo->Acceptor->is_open())
				{
					try
					{
						listenInfo->Acceptor->open(ep.protocol());
						listenInfo->Acceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
						listenInfo->Acceptor->bind(ep);
						listenInfo->Acceptor->listen();
					}
					catch (std::system_error& error)
					{
						listenInfo->Acceptor->close();
						std::this_thread::sleep_for(std::chrono::seconds(3));
						CONSOLE_LOG_ERROR("listen [{}] {}", config.Addr, error.what());
					}
				}
				threadSync.SetResult(listenInfo->Acceptor->is_open());
			});
		if (!threadSync.Wait())
		{
			return false;
		}
#endif
		LOG_CHECK_RET_FALSE(this->mApp->AddListen(config.Name, config.Addr));
		LOG_CHECK_RET_FALSE(listenData->Listener->OnListenOk(config.Name.c_str()));
		io.post(std::bind(&ListenerComponent::AcceptConnect, this, listenData.get()));
		this->mListenDatas.emplace(config.Name, std::move(listenData));
		return true;
	}

	void ListenerComponent::AcceptConnect(acs::ListenData* listenData)
	{
		if (listenData->Acceptor == nullptr)
		{
			return;
		}
		tcp::Socket* sock = listenData->Create(this->mThreadComponent);
		listenData->Acceptor->async_accept(sock->Get(), [sock, this, listenData](const Asio::Code& code)
			{
				do
				{
					
					if (code.value() != Asio::OK)
					{
						sock->Destory();
						break;
					}
					sock->Init();
#ifdef __ENABLE_OPEN_SSL__
					if (sock->IsOpenSsl())
					{
						Asio::Code code;
						Asio::ssl::Socket& ssl = sock->SslSocket();
						ssl.handshake(asio::ssl::stream_base::server, code);
						if (code.value() != Asio::OK)
						{
							sock->Destory();
							break;
						}
					}
#endif
#ifdef ONLY_MAIN_THREAD
					this->OnAcceptSocket(listenData, sock);
#else
					Asio::Context& io = this->mApp->GetContext();
					asio::post(io, std::bind(&ListenerComponent::OnAcceptSocket, this, listenData, sock));
#endif			
				} while (false);
				if(listenData->Acceptor == nullptr)
				{
					return;
				}
#ifdef ONLY_MAIN_THREAD
				this->AcceptConnect(listenData);
#else
				const Asio::Executor& executor = listenData->Acceptor->get_executor();
				asio::post(executor, std::bind(&ListenerComponent::AcceptConnect, this, listenData));
#endif
			});
	}

	void ListenerComponent::OnAcceptSocket(ListenData* listenData, tcp::Socket* sock)
	{
		if (!listenData->Listener->OnListen(sock))
		{
			sock->Destory();
		}
	}

	bool ListenerComponent::StartListen(const char* name)
	{
		auto iter = this->mListenConfigs.find(name);
		if (iter == this->mListenConfigs.end())
		{
			return false;
		}
		return this->StartListen(iter->second);
	}

	bool ListenerComponent::StopListen(const char* name)
	{
		auto iter = this->mListenDatas.find(name);
		if(iter == this->mListenDatas.end())
		{
			return false;
		}
		if(!this->StopListen(iter->second.get()))
		{
			return false;
		}
		this->mListenDatas.erase(iter);
		return true;
	}

	bool ListenerComponent::StopListen(acs::ListenData* listenData)
	{
#ifdef ONLY_MAIN_THREAD
		Asio::Code code;
		listenData->Acceptor->close(code);
		LOG_WARN("stop listen [{}]", listenData->Config.Addr);
		return code.value() == Asio::OK;
#else
		custom::ThreadSync<bool> threadSync;
		const Asio::Executor & executor = listenData->Acceptor->get_executor();
		asio::post(executor, [listenData, &threadSync] ()
		{
			Asio::Code code;
			listenData->Acceptor->close(code);
			threadSync.SetResult(code.value() == Asio::OK);
		});
		LOG_DEBUG("stop listen [{}]", listenData->Config.Addr);
		return threadSync.Wait();
#endif
	}

	void ListenerComponent::OnDestroy()
	{
		auto iter = this->mListenDatas.begin();
		for(; iter != this->mListenDatas.end(); iter++)
		{
			this->StopListen(iter->second.get());
		}
		this->mListenDatas.clear();
	}
}
