#pragma once

#include <thread>
#include <Component/Component.h>
#include <Script/LuaTable.h>

#include <Other/MultiThreadQueue.h>
#include <TcpNetwork/TcpClientSession.h>
namespace Sentry
{
	// 管理所有session
	class TcpNetSessionComponent : public Component, public ScoketHandler<TcpClientSession>
	{
	public:
		TcpNetSessionComponent();

		virtual ~TcpNetSessionComponent() {}

    protected:

        void OnCloseSession(TcpClientSession * socket) override;
        bool OnListenNewSession(TcpClientSession * session) override;
        bool OnReceiveMessage(TcpClientSession * session, const string & message) override;
        void OnSessionError(TcpClientSession * session, const asio::error_code & err) override;
		void OnConnectRemoteAfter(TcpClientSession * session, const asio::error_code & err) override;
	public:
        SessionBase * CreateSocket() { return new TcpClientSession(this);}
		TcpClientSession * ConnectRemote(const std::string &name, const std::string & ip, unsigned short port);

	protected:
		bool Awake() override;

		void OnDestory() override;

	public:
		bool CloseSession(const std::string &address);
		bool SendByAddress(const std::string & address, std::string * message);
		bool SendByAddress(const std::string & address, com::DataPacket_Request & message);
		bool SendByAddress(const std::string & address, com::DataPacket_Response & message);
	private:
		bool SendByAddress(const std::string & address, DataMessageType type, Message & message);
    private:

		TcpClientSession *GetSession(const std::string &address);

	private:
		class ActionComponent * mActionComponent;
		class ServiceMgrComponent * mServiceComponent;

		class TcpNetSessionComponent *mNetWorkManager;
		class ProtocolComponent *mProtocolComponent;

	private:
        char mMessageBuffer[1024 * 1024];
        com::DataPacket_Request mRequestData;
        com::DataPacket_Response mResponseData;
		std::queue<std::string> mRecvSessionQueue;
        std::vector<TcpClientSession *> mSessionArray;
        std::unordered_map<std::string, TcpClientSession *> mSessionAdressMap;
	};
}
