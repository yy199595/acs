#pragma once
#include"TcpClientSession.h"
#include<Protocol/Common.pb.h>

namespace SoEasy
{
	class SessionManager;
	class NetWorkManager;

	class RemoteActionProxy
	{
	public:
		RemoteActionProxy(NetWorkManager * mgr, const std::string & address);
	public:
		bool StartConnect(SessionManager * sMgr);
		XCode CallAction(shared_ptr<PB::NetWorkPacket> message);
	private:
		void OnConnectBack(shared_ptr<TcpClientSession> session, bool hasError);
	private:
		std::string mActionIp;
		unsigned short mActionPort;
		std::string mActionAddress;
		SessionManager * mSessionManager;
		NetWorkManager * mNetWorkManager;
		shared_ptr<TcpClientSession> mActionSession;
		std::queue<shared_ptr<PB::NetWorkPacket>> mSendQueue;
	};
}