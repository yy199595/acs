#pragma once
#include"TcpClientSession.h"
#include<Protocol/Common.pb.h>

namespace SoEasy
{
	class SessionManager;
	class NetWorkManager;

	class ActionAddressProxy
	{
	public:
		ActionAddressProxy(NetWorkManager * mgr, SessionManager * sMgr, const std::string & address);
	public:
		bool CallAction(shared_ptr<PB::NetWorkPacket> message);
	private:
		bool StartConnect();
		void OnConnectBack(shared_ptr<TcpClientSession> session, bool hasError);
	private:
		std::queue<shared_ptr<PB::NetWorkPacket>> mSendQueue;
	private:
		std::string mActionIp;
		unsigned short mActionPort;
		std::string mActionAddress;
		SessionManager * mSessionManager;
		NetWorkManager * mNetWorkManager;
		shared_ptr<TcpClientSession> mActionSession;
	};
}