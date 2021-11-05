#pragma once

#include <thread>
#include <Component/Component.h>
#include <Script/LuaTable.h>
#include <Other/MultiThreadQueue.h>
#include <Network/Tcp/TcpLocalSession.h>
namespace GameKeeper
{
	// 管理所有session
	class TcpClientComponent : public Component, public ISocketListen
	{
	public:
		friend TcpClientSession;
		TcpClientComponent() = default;

		~TcpClientComponent() override = default;
	public:
		void OnCloseSession(TcpClientSession * socket);
		void OnReceiveMessage(TcpClientSession * session, string * message);
		void OnSendMessageAfter(TcpClientSession *session, std::string * message, bool);
		void OnConnectRemoteAfter(TcpLocalSession * session, const asio::error_code & err);
    protected:
		void OnListen(SocketProxy * socket) final;
	public:
        TcpLocalSession * GetLocalSession(long long id);
        TcpClientSession * GetRemoteSession(long long id);
        long long NewSession(const std::string &name, const std::string & ip, unsigned short port);
	protected:
		bool Awake() override;
		void OnDestory() override;

	public:
		bool CloseSession(long long id);
		bool SendByAddress(long long id, std::string * message);
		bool SendByAddress(long long id, com::DataPacket_Request & message);
		bool SendByAddress(long long id, com::DataPacket_Response & message);

    public:
        std::string * Serialize(const com::DataPacket_Request & message);
        std::string * Serialize(const com::DataPacket_Response & message);
    private:
		
		TcpClientSession *GetSession(long long id);
		bool OnReceive(TcpClientSession *, const std::string & message);

	private:
		class TaskPoolComponent * mTaskComponent;
		class TcpClientComponent *mNetWorkManager;
		class ProtocolComponent *mProtocolComponent;
        class ServiceMgrComponent * mServiceComponent;
        class CallHandlerComponent * mCallHandlerComponent;
	private:		
        char mMessageBuffer[1024 * 1024];
        com::DataPacket_Request mRequestData;
        com::DataPacket_Response mResponseData;
		std::queue<std::string> mRecvSessionQueue;
		std::queue<TcpClientSession *> mSessionQueue;		
        std::unordered_map<long long, TcpClientSession *> mSessionAdressMap;
	};
}
