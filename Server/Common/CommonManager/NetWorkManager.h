#pragma once
#include<CommonManager/Manager.h>
#include<CommonProtocol/Common.pb.h>
#include<CommonCore/TcpSessionListener.h>
#include<CommonScript/LuaType/LuaTable.h>
#define ASIO_TCP_SEND_MAX_COUNT 1024 * 1024
namespace SoEasy
{

	class TcpClientSession;
	class NetWorkManager : public Manager
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
		bool SendMessageByAdress(const std::string & address, const NetWorkPacket & returnPackage);
	protected:
		bool OnInit() override;
		void OnDestory() override;
	public:
		void OnLoadLuaComplete(lua_State * luaEnv) override;
	protected:
		void OnFrameUpdateAfter() override;
	private:
		std::mutex mSessionLock;
		class ActionManager * mFunctionManager;
		std::queue<SharedNetPacket> mSendMessageQueue;
	private:
		char mSendSharedBuffer[ASIO_TCP_SEND_MAX_COUNT + sizeof(unsigned int)];
		std::unordered_map<std::string, shared_ptr<TcpClientSession>> mSessionAdressMap;
	private:
		LuaTable * mBindLuaTalbe;
	};
}
