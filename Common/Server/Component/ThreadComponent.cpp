#include"ThreadComponent.h"
#include"Entity/Actor/App.h"
#include"Network/Tcp/Socket.h"
#include"Core/System/System.h"
#include"Log/Output/ConsoleOutput.h"
#include"Util/Tools/String.h"
#include"Util/Tools/TimeHelper.h"
namespace acs
{

	ThreadComponent::ThreadComponent()
	{
		this->mIndex = -1;
	}

    bool ThreadComponent::Awake()
	{
		size_t count = 2;
		std::unique_ptr<json::r::Value> jsonObject;
		if(ServerConfig::Inst()->Get("core", jsonObject))
		{
			if(!jsonObject->Get("thread", count))
			{
				count = std::thread::hardware_concurrency();
			}
		}
		for (int index = 0; index < count; index++)
		{
			std::shared_ptr<custom::AsioThread> netThread = std::make_shared<custom::AsioThread>();
			{
				netThread->Start(index + 1, "net");
				this->mNetThreads.emplace_back(netThread);
			}
		}
		printf("thread count = %d\n", count);
		return true;
	}

	void ThreadComponent::CloseThread()
	{
#ifndef ONLY_MAIN_THREAD
		while(!this->mNetThreads.empty())
		{
			std::shared_ptr<custom::AsioThread> netThread = this->mNetThreads.front();
			{
				netThread->Stop();
			}
			this->mNetThreads.clear();
		}
#endif
	}

	void ThreadComponent::OnRecord(json::w::Document& document)
	{
		long long nowTime = help::Time::NowSec();
		std::lock_guard<std::mutex> lock(this->mLock);
		std::unique_ptr<json::w::Value> jsonValue = document.AddObject("thread");
		for(const std::shared_ptr<custom::AsioThread> & netThread : this->mNetThreads)
		{
			std::string key = std::to_string(netThread->GetId());
			long long invite = nowTime - netThread->GetLastTime();
			jsonValue->Add(key.c_str(), invite);
		}
		jsonValue->Add("count", this->mNetThreads.size());
	}

    Asio::Context& ThreadComponent::GetContext()
	{
#ifdef ONLY_MAIN_THREAD
		return this->mApp->GetContext();
#else
		std::lock_guard<std::mutex> lock(this->mLock);
		size_t index = this->mIndex++;
		if(index >= this->mNetThreads.size())
		{
			index = 0;
			this->mIndex = 0;
		}
		std::shared_ptr<custom::AsioThread> & netThread = this->mNetThreads.at(index);
		{
			return netThread->Context();
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
