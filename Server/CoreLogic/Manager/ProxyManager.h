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
	protected:
		void OnSessionErrorAfter(SharedTcpSession tcpSession) override;
		void OnSessionConnectAfter(SharedTcpSession tcpSession) override;
		bool OnRecvServerMessage(shared_ptr<TcpClientSession> session, shared_ptr<NetWorkPacket> msg);
	private:
		std::string mProxyIP;
		unsigned short mPorxyPort;
		std::string mProxyAddress;
	};
}