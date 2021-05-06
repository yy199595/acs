#pragma once

#include<CommonProtocol/Common.pb.h>
#include<CommonManager/SessionManager.h>
#include<CommonCore/TcpSessionListener.h>
#include<CommonScript/LuaType/LuaTable.h>
#include<CommonProtocol/ServerCommon.pb.h>
#define ASIO_TCP_SEND_MAX_COUNT 1024 * 1024
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
		bool SendMessageByName(const std::string & func, shared_ptr<NetWorkPacket> returnPackage);
		bool SendMessageByAdress(const std::string & address, shared_ptr<NetWorkPacket> returnPackage);
	private:
		bool StartConnect(const std::string name, const std::string & address);
		XCode UpdateAction(shared_ptr<TcpClientSession> session, long long id, const PB::AreaActionInfo & actionInfos);
	protected:
		bool OnInit() override;
		void OnDestory() override;
		void OnFrameUpdateAfter() override;
		void OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession)override;
		void OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)override;
	public:
		void OnLoadLuaComplete(lua_State * luaEnv) override;
	private:
		std::mutex mSessionLock;
		AsioContext * mSessionContext;
		class ActionManager * mFunctionManager;
		std::unordered_map<std::string, std::queue<shared_ptr<NetWorkPacket>>> mWaitSendMessage;
	private:
		char mSendSharedBuffer[ASIO_TCP_SEND_MAX_COUNT + sizeof(unsigned int)];
		std::unordered_map<std::string, std::vector<std::string>> mRemoteAddressMap;		//方法对应的地址
		std::unordered_map<std::string, shared_ptr<TcpClientSession>> mSessionAdressMap;	//所有session
		std::unordered_map<std::string, shared_ptr<TcpClientSession>> mOnConnectSessionMap;	//正在连接的session
	private:
		LuaTable * mBindLuaTalbe;
	};
}
