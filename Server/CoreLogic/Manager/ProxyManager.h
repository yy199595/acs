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
	public:
		shared_ptr<GameObject> GetClientObject(const long long id);
	private:
		bool OnRecvServerMessage(shared_ptr<TcpClientSession> session, shared_ptr<NetWorkPacket> msg);
	private:
		std::unordered_map<long long, shared_ptr<GameObject>> mClientObjectMap;	//¿Í»§¶Ësession
	};
}