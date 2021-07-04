#pragma once
#include"Manager.h"
namespace SoEasy
{
	// 处理外部连接进来的session
	class TcpSessionListener;
	class ListenerManager : public Manager
	{
	public:
		ListenerManager() { }
		~ListenerManager() { }
	public:
		const std::string & GetAddress() { return mListenAddress; }
	public:
		void StartAccept();
	protected:
		bool OnInit() override;
	private:
		std::string mListenerIp;	//监听的ip
		std::string mListenAddress;			//地址
		unsigned short mListenerPort;
		AsioTcpAcceptor * mBindAcceptor;
		NetSessionManager * mDispatchManager;	
		std::set<std::string> mWhiteList;	//白名单
		std::function<void(void)> mListenAction;
	};
}