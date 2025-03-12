#include"ThreadComponent.h"
#include"Entity/Actor/App.h"
#include"Network/Tcp/Socket.h"
#include"Core/System/System.h"
#include"Log/Output/ConsoleOutput.h"
#include"Util/Tools/String.h"
#include"Util/Tools/TimeHelper.h"
namespace acs
{

    bool ThreadComponent::Awake()
	{
		int threadCount = 1;
#ifdef __OS_LINUX__
		std::unique_ptr<json::r::Value> jsonObject;
		threadCount = std::thread::hardware_concurrency();
		if(ServerConfig::Inst()->Get("core", jsonObject))
		{
			jsonObject->Get("thread", threadCount);
		}
#endif
		for (int index = 0; index < threadCount; index++)
		{
			std::unique_ptr<custom::AsioThread> t = std::make_unique<custom::AsioThread>();
			{
				t->Start(index + 1, "net");
				this->mNetThreads.emplace(std::move(t));
			}
		}
		printf("thread count = %d\n", threadCount);
		return true;
	}

	void ThreadComponent::CloseThread()
	{
#ifndef ONLY_MAIN_THREAD
		while(!this->mNetThreads.empty())
		{
			std::unique_ptr<custom::AsioThread> & t = this->mNetThreads.front();
			{
				t->Stop();
				this->mNetThreads.pop();
			}
		}
#endif
	}

    Asio::Context& ThreadComponent::GetContext()
	{
#ifdef ONLY_MAIN_THREAD
		return this->mApp->GetContext();
#else
		custom::AsioThread * asioThread = nullptr;
		std::unique_ptr<custom::AsioThread> t = std::move(this->mNetThreads.front());
		{
			asioThread = t.get();
			this->mNetThreads.pop();
			this->mNetThreads.emplace(std::move(t));
			return asioThread->Context();
		}
#endif
	}

	tcp::Socket * ThreadComponent::CreateSocket()
	{
#ifdef ONLY_MAIN_THREAD
		asio::io_service & io = this->mApp->GetContext();
		return new tcp::Socket(io);
#else
		Asio::Context& io = this->GetContext();
		return new tcp::Socket(io);
#endif
	}

#ifdef __ENABLE_OPEN_SSL__

	tcp::Socket * ThreadComponent::CreateSocket(Asio::ssl::Context& ssl)
	{
#ifdef ONLY_MAIN_THREAD
		asio::io_service & io = this->mApp->GetContext();
		return new tcp::Socket(io, ssl);
#else
		Asio::Context& io = this->GetContext();
		return new tcp::Socket(io, ssl);
#endif
	}
#endif

	tcp::Socket * ThreadComponent::CreateSocket(const std::string& addr)
	{
		std::string ip;
		unsigned short port = 0;
		if(!help::Str::SplitAddr(addr, ip, port))
		{
			return nullptr;
		}
		return this->CreateSocket(ip, port);
	}

	tcp::Socket * ThreadComponent::CreateSocket(const std::string& ip, unsigned short port)
	{
		tcp::Socket * sock = this->CreateSocket();
		{
			sock->Init(ip, port);
		}
		return sock;
	}

	ThreadComponent::~ThreadComponent() noexcept
	{
#ifndef ONLY_MAIN_THREAD
		this->CloseThread();
#endif
	}


}
