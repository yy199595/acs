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
	enum class ListenCode
	{
		Ok = 0,
		Close = 1, //关闭
		Shield = 2, //拉黑
		StopListen = 3, //关闭监听
		SwitchPort = 4, //切换端口
	};


	class ListenerComponent : public Component, public IDestroy, public INetListen
	{
	public:
		explicit ListenerComponent();
		~ListenerComponent() override;
	public:
		bool StopListen() final;
		bool StartListen(const ListenConfig & config) final;
	private:
		void OnDestroy() final;
	private:
		void Accept();
		void OnAcceptSocket(tcp::Socket* sock);
	private:
		int mOffsetPort;
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