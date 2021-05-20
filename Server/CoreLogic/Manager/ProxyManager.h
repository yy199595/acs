#pragma once
#include<Object/GameObject.h>
#include<Manager/ListenerManager.h>
namespace SoEasy
{
	class ProxyManager : public ListenerManager
	{
	public:
		ProxyManager() { }
		~ProxyManager() { }
	public:

	protected:
		bool OnInit() override;
		void OnSessionErrorAfter(SharedTcpSession tcpSession) override;
		void OnSessionConnectAfter(SharedTcpSession tcpSession) override;
		void OnRecvNewMessageAfter(SharedTcpSession tcpSession, shared_ptr<NetWorkPacket>) override;
	public:
		shared_ptr<GameObject> GetClientObject(const long long id);
	private:
		bool OnRecvServerMessage(shared_ptr<TcpClientSession> session, shared_ptr<NetWorkPacket> msg);
	private:
		class ServiceQuery * mRemoteActionManager;
		std::unordered_map<long long, shared_ptr<GameObject>> mClientObjectMap;	//¿Í»§¶Ësession
	};
}