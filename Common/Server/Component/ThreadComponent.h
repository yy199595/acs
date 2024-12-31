#pragma once

#include"Network/Tcp/Socket.h"
#include"Core/Queue/ThreadQueue.h"
#include"Core/Thread/AsioThread.h"
#include"Entity/Component/Component.h"

#ifdef __ENABLE_OPEN_SSL__
#include<asio/ssl.hpp>
#endif

namespace acs
{
	class ThreadComponent final : public Component
	{
	public:
		ThreadComponent() = default;
		~ThreadComponent() final;
	public:
		Asio::Context& GetContext();
		tcp::Socket * CreateSocket();
		tcp::Socket * CreateSocket(const std::string & addr);
		tcp::Socket * CreateSocket(const std::string& ip, unsigned short port);
#ifdef __ENABLE_OPEN_SSL__
		tcp::Socket * CreateSocket(Asio::ssl::Context & ssl);
#endif
	private:
		bool Awake() final;
		void CloseThread();
	private:
#ifdef ONLY_MAIN_THREAD
		std::unique_ptr<Asio::Context> mContext;
		std::unique_ptr<Asio::ContextWork> mWork;
#else
		std::queue<std::unique_ptr<custom::AsioThread>> mNetThreads;
#endif
	};

}