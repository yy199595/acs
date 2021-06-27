#pragma once
#include"TcpClientSession.h"
#include<Protocol/com.pb.h>

namespace SoEasy
{
	class SessionManager;
	class NetWorkManager;

	class RemoteActionProxy
	{
	public:
		RemoteActionProxy(const std::string name, const std::string & address, int areaId);
	public:
		XCode Invoke(shared_ptr<PB::NetWorkPacket> message);
		bool BindSession(shared_ptr<TcpClientSession> session);
	public:
		bool IsAction();
		const int GetActionAreaId() { return this->mActionAreaId; }
		const std::string & GetActionName() { return this->mActionName; }
		const size_t GetMessageSize() { return this->mSendQueue.size(); }
		const std::string & GetActionAddress() { return this->mActionAddress; }
	private:
		int mActionAreaId;
		std::string mActionName;
		std::string mActionAddress;	
		NetWorkManager * mNetWorkManager;
		shared_ptr<TcpClientSession> mActionSession;
		std::queue<shared_ptr<PB::NetWorkPacket>> mSendQueue; //等待发送的消息
	};
}