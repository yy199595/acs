#pragma once
#include"SessionManager.h"
#include<Protocol/ServerCommon.pb.h>

using namespace PB;
namespace SoEasy
{
	// 处理外部连接进来的session
	class TcpSessionListener;
	class ListenerManager : public SessionManager
	{
	public:
		ListenerManager() { }
		~ListenerManager() { }
	public:
		const std::string & GetAddress() { return mListenAddress; }
	protected:
		bool OnInit() override;
		void OnInitComplete() override;
	protected:
		void OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession) override;
		void OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession) override;
	private:
		std::string mListenerIp;	//监听的ip
		std::string mListenAddress;	//地址
		unsigned short mListenerPort; //监听的端口号
		std::set<std::string> mWhiteList;	//白名单
		shared_ptr<TcpSessionListener> mTcpSessionListener;
	};
}