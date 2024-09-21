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
		tcp::Socket* CreateSocket();
		tcp::Socket* CreateSocket(const std::string & addr);
		tcp::Socket* CreateSocket(const std::string& ip, unsigned short port);
		void CreateSockets(std::queue<tcp::Socket*> & sockets, int count = 10);
#ifdef __ENABLE_OPEN_SSL__
		tcp::Socket* CreateSocket(Asio::ssl::Context & ssl);
		void CreateSockets(std::queue<tcp::Socket*>& sockets, Asio::ssl::Context& ssl, int count = 10);
#endif
	public:
		void CloseThread();
	private:
		bool Awake() final;
	private:
#ifndef ONLY_MAIN_THREAD
		int mThreadCount;
		custom::ThreadQueue<custom::AsioThread *> mNetThreads;
#else
		std::unique_ptr<Asio::Context> mContext;
		std::unique_ptr<Asio::ContextWork> mWork;
#endif
	};
}