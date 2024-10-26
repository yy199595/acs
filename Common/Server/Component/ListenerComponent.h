#pragma once
#include<unordered_set>
#include<unordered_map>
#include"Network/Tcp/Socket.h"
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

	class ITcpListen
	{
	public:
		virtual bool OnListen(tcp::Socket * socket) = 0;
		virtual bool OnListenOk(const char * name) { return true; };
	};

	struct ListenConfig
	{
	public:
		int Port;			//监听
		int MaxConn;		//最大连接数
#ifdef __ENABLE_OPEN_SSL__
		std::string Key;	//私钥
		std::string Cert;	//证书地址
		std::string Verify;
#endif
		std::string Name;	//名字
		std::string Addr;  //地址 xxx://xxx:xxx
		std::string Protocol; //使用的协议
		std::string Component; //处理消息的component
	};

	struct ListenData
	{
	public:
		ListenData(ListenConfig  conf);
	public:
		tcp::Socket * Create(class ThreadComponent * component);

#ifdef __ENABLE_OPEN_SSL__
		inline bool IsEnableSsl() const { return !this->Config.Cert.empty(); }
#endif
	public:
		ListenConfig Config;
		ITcpListen * Listener;
		std::unique_ptr<Asio::Acceptor> Acceptor;
#ifdef __ENABLE_OPEN_SSL__
		Asio::ssl::Context SslCtx;
#endif
	};

	class ListenerComponent : public Component, public IDestroy
	{
	public:
		explicit ListenerComponent();
		~ListenerComponent() override;
	public:
		bool StopListen(const char * name);
		bool StartListen(const char* name);
	private:
		bool StopListen(ListenData * listenData);
		void AcceptConnect(ListenData * listenData);
		bool StartListen(const ListenConfig & config);
	private:
		bool Awake() final;
		bool LateAwake() final;
		void OnDestroy() final;
	private:
		void OnAcceptSocket(ListenData* listenData, tcp::Socket* sock);
	private:
		int mOffsetPort;
		//custom::AsioThread mThread;
		std::queue<tcp::Socket*> mSocketPool;
		class ThreadComponent* mThreadComponent;
		std::unique_ptr<Asio::Acceptor> mAcceptor;
		std::unordered_set<std::string> mBlackList; //黑名单
		std::unordered_map<std::string, std::string> mListenInfos;
		std::unordered_map<std::string, ListenConfig> mListenConfigs;
		std::unordered_map<std::string, std::unique_ptr<ListenData>> mListenDatas;
	};
}