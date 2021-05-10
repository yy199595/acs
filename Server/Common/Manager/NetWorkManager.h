#pragma once

#include<Protocol/Common.pb.h>
#include<Manager/SessionManager.h>
#include<Core/TcpSessionListener.h>
#include<Script/LuaType/LuaTable.h>
#include<Protocol/ServerCommon.pb.h>

namespace SoEasy
{

	class TcpClientSession;
	class NetWorkManager : public SessionManager
	{
	public:
		NetWorkManager();
		virtual ~NetWorkManager() {}
	public:
		friend Applocation;
	public:
		bool AddTcpSession(shared_ptr<TcpClientSession> tcpSession);
		bool CloseTcpSession(const std::string & address);
		bool CloseTcpSession(shared_ptr<TcpClientSession> tcpSession);

		bool RemoveTcpSession(const std::string & tcpSession);
		bool RemoveTcpSession(shared_ptr<TcpClientSession> tcpSession);
		shared_ptr<TcpClientSession> GetSessionByAdress(const std::string & adress);
		XCode SendMessageByName(const std::string & func, shared_ptr<NetWorkPacket> returnPackage);
		XCode SendMessageByAdress(const std::string & address, shared_ptr<NetWorkPacket> returnPackage);
	protected:
		bool OnInit() override;
		void OnDestory() override;
		void OnFrameUpdateAfter() override;
		void OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession)override;
		void OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)override;
	private:
		std::mutex mSessionLock;
		AsioContext * mSessionContext;
		class RemoteActionManager * mActionQueryManager;
	private:
		char mSendSharedBuffer[ASIO_TCP_SEND_MAX_COUNT + sizeof(unsigned int)];
		std::unordered_map<std::string, shared_ptr<TcpClientSession>> mSessionAdressMap;	//所有session
		std::unordered_map<std::string, shared_ptr<TcpClientSession>> mOnConnectSessionMap;	//正在连接的session
	};
}
