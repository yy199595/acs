#pragma once
#include<Object/GameObject.h>
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
		shared_ptr<GameObject> GetClientObject(const long long id);
	private:
		void OnRecvServerMessage(shared_ptr<TcpClientSession> session, shared_ptr<NetWorkPacket> msg);
	private:
		std::string mListenIp;
		unsigned short mListenPort;
		shared_ptr<TcpSessionListener> mTpcListener;
		class RemoteActionManager * mRemoteActionManager;
		std::unordered_map<long long, shared_ptr<GameObject>> mClientObjectMap;	//¿Í»§¶Ësession
	};
}