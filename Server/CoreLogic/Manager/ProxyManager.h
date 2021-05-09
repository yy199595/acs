#pragma once
#include<Manager/SessionManager.h>

namespace SoEasy
{
	class ProxyManager : public SessionManager
	{
	public:
		ProxyManager() { }
		~ProxyManager() { }
	public:

	protected:
		bool OnInit() override;
		void OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession) override;
		void OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession) override;
		void OnRecvNewMessageAfter(const std::string & address, const char * msg, size_t size) override;
	public:
		shared_ptr<TcpClientSession> GetClientSession(const std::string & address);
	private:
		std::string mListenIp;
		unsigned short mListenPort;
		shared_ptr<TcpSessionListener> mTpcListener;
		class ActionQueryManager * mActionQueryManager;
		std::unordered_map<std::string, long long> mClientSessionIdMap;
		std::unordered_map<std::string, shared_ptr<TcpClientSession>> mServerSessionMap;	//服务器session
		std::unordered_map<std::string, shared_ptr<TcpClientSession>> mClientSessionMap;	//客户端session
	};
}