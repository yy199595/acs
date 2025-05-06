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
#ifndef ONLY_MAIN_THREAD
		this->mIndex = -1;
#endif
		REGISTER_JSON_CLASS_FIELD(thread::Config, count);
		REGISTER_JSON_CLASS_FIELD(thread::Config, monitor);
	}

#ifndef ONLY_MAIN_THREAD
    bool ThreadComponent::Awake()
	{
		std::unique_ptr<json::r::Value> jsonObject;
		ServerConfig::Inst()->Get("thread", this->mConfig);
		for (int index = 0; index < this->mConfig.count; index++)
		{
			std::unique_ptr<custom::AsioThread> netThread = std::make_unique<custom::AsioThread>();
			{
				netThread->Start(index + 1, "net");
				this->mNetThreads.emplace_back(std::move(netThread));
			}
		}
		if(this->mConfig.monitor > 0)
		{
			this->mThread = std::make_unique<std::thread>();
			std::thread(&ThreadComponent::OnMonitor, this).swap(*this->mThread);
		}
		printf("thread count = %d\n", this->mConfig.count);
		return true;
	}
#endif
	void ThreadComponent::CloseThread()
	{
#ifndef ONLY_MAIN_THREAD
		while(!this->mNetThreads.empty())
		{
			std::unique_ptr<custom::AsioThread> & netThread = this->mNetThreads.front();
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
		std::unique_ptr<json::w::Value> jsonValue = document.AddObject("thread");
		jsonValue->AddObject("main")->Add("event_count", this->mApp->GetEventCount());
#ifndef ONLY_MAIN_THREAD
		std::lock_guard<std::mutex> lock(this->mLock);
		for(const std::unique_ptr<custom::AsioThread> & netThread : this->mNetThreads)
		{
			std::string key = std::to_string(netThread->GetId());
			long long invite = nowTime - netThread->GetLastTime();
			std::unique_ptr<json::w::Value> jsonObject = jsonValue->AddObject(key.c_str());
			{
				jsonObject->Add("invite", invite);
				jsonObject->Add("event_count", netThread->GetEventCount());
			}
		}
		jsonValue->Add("count", this->mNetThreads.size());
#endif
	}
#ifndef ONLY_MAIN_THREAD
	void ThreadComponent::OnMonitor()
	{
		Asio::Context & main = this->mApp->GetContext();
		std::chrono::seconds sleep(this->mConfig.monitor);
		while(this->mConfig.monitor > 0)
		{
			std::this_thread::sleep_for(sleep);
			long long nowTime = help::Time::NowSec();
			std::lock_guard<std::mutex> lock(this->mLock);
			for(const std::unique_ptr<custom::AsioThread> & netThread : this->mNetThreads)
			{
				long long invite = nowTime - netThread->GetLastTime();
				if(invite >= this->mConfig.monitor * 2)
				{
#ifdef __OS_WIN__
					int id = netThread->GetId();
					asio::post(main, [id, invite] {
						LOG_FATAL("thread:{} update invite => {}", id, invite);
					});
#else

#endif
				}
			}
		}
	}
#endif
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
		return this->mNetThreads[index]->Context();
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
