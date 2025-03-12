#pragma once
#include<unordered_set>
#include<unordered_map>
#include "ITcpComponent.h"
#include"Core/Thread/AsioThread.h"
#include"Entity/Component/Component.h"
#ifdef ONLY_MAIN_THREAD
#include"Core/Queue/Queue.h"
#else
#include"Core/Queue/DoubleBufferQueue.h"
#endif

namespace acs
{
	class ListenerComponent final : public Component, public INetListen
	{
	public:
		explicit ListenerComponent();
		~ListenerComponent() override;
	public:
		bool StopListen() final;
		bool StartListen(const ListenConfig & config) final;
	private:
		void Accept();
		tcp::Socket* CreateSocket();
		void OnAcceptSocket(tcp::Socket* sock);
	private:
		ListenConfig mConfig;
#ifdef __ENABLE_OPEN_SSL__
		Asio::ssl::Context mSslCtx;
#endif
		class ITcpListen * mTcpListen;
		std::queue<tcp::Socket*> mSocketPool;
		class ThreadComponent* mThreadComponent;
		std::unique_ptr<Asio::Acceptor> mAcceptor;
		std::unordered_set<std::string> mBlackList; //黑名单
	};
}