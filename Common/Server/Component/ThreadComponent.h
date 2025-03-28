﻿#pragma once

#include"Network/Tcp/Socket.h"
#include"Core/Queue/ThreadQueue.h"
#include"Core/Thread/AsioThread.h"
#include"Entity/Component/Component.h"

#ifdef __ENABLE_OPEN_SSL__
#include<asio/ssl.hpp>
#endif

namespace acs
{
	class ThreadComponent final : public Component, public IServerRecord
	{
	public:
		ThreadComponent();
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
		void OnRecord(json::w::Document &document) final;
	private:
		std::mutex mLock;
#ifdef ONLY_MAIN_THREAD
		std::unique_ptr<Asio::Context> mContext;
		std::unique_ptr<Asio::ContextWork> mWork;
#else
		size_t mIndex = 0;
		std::vector<std::shared_ptr<custom::AsioThread>> mNetThreads;
#endif
	};

}