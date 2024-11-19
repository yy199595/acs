#include"ThreadComponent.h"
#include"Entity/Actor/App.h"
#include"Network/Tcp/Socket.h"
#include"Core/System/System.h"
#include"Log/Output/ShowOutput.h"
#include"Util/Tools/String.h"
#include"Util/Tools/TimeHelper.h"
namespace acs
{
	ThreadComponent::ThreadComponent()
	#ifndef ONLY_MAIN_THREAD
		: mThreadCount(0)
	#endif
	{

	}

    bool ThreadComponent::Awake()
	{
#ifndef ONLY_MAIN_THREAD
		std::unique_ptr<json::r::Value> jsonObject;
#ifdef __OS_LINUX__
		if(ServerConfig::Inst()->Get("core", jsonObject))
		{
			std::string str = jsonObject->ToString();
			jsonObject->Get("thread", this->mThreadCount);
		}
		this->mThreadCount = std::thread::hardware_concurrency();
#else
		this->mThreadCount = 1;
#endif
		for (int index = 0; index < this->mThreadCount; index++)
		{
			custom::AsioThread * t = new custom::AsioThread();
			{
				t->Start(index + 1, "net");
				this->mNetThreads.Push(t);
			}
		}
		LOG_INFO("thread count = {}", this->mThreadCount)
#endif
		return true;
	}

	void ThreadComponent::CloseThread()
	{
#ifndef ONLY_MAIN_THREAD
		custom::AsioThread * asioThread = nullptr;
		while(!this->mNetThreads.Empty())
		{
			if(this->mNetThreads.TryPop(asioThread))
			{
				asioThread->Stop();
				delete asioThread;
				asioThread = nullptr;
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
		this->mNetThreads.Move(asioThread);
		{
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
		custom::AsioThread * asioThread;
		this->mNetThreads.Move(asioThread);
		Asio::Context& io = asioThread->Context();
		return new tcp::Socket(io);
#endif
	}

#ifdef __ENABLE_OPEN_SSL__

	tcp::Socket* ThreadComponent::CreateSocket(Asio::ssl::Context& ssl)
	{
#ifdef ONLY_MAIN_THREAD
		asio::io_service & io = this->mApp->GetContext();
		return new tcp::Socket(io, ssl);
#else
		custom::AsioThread * asioThread;
		this->mNetThreads.Move(asioThread);
		Asio::Context& io = asioThread->Context();
		return new tcp::Socket(io, ssl);
#endif
	}
#endif

	tcp::Socket* ThreadComponent::CreateSocket(const std::string& addr)
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
		custom::AsioThread * asioThread;
		while(this->mNetThreads.TryPop(asioThread))
		{
			asioThread->Stop();
			delete asioThread;
			asioThread = nullptr;
		}
#endif
	}

}
