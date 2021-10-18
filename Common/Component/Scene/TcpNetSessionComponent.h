#pragma once

#include <thread>
#include <Component/Component.h>
#include <Script/LuaTable.h>
#include <Pool/ObjectPool.h>

#include <Other/DoubleBufferQueue.h>
#include <Network/TcpClientSession.h>
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
        bool OnReceiveMessage(TcpClientSession * session, SharedMessage message) override;
        void OnSessionError(TcpClientSession * session, const asio::error_code & err) override;
		void OnConnectRemoteAfter(TcpClientSession * session, const asio::error_code & err) override;
        void OnSendMessageAfter(TcpClientSession *session, SharedMessage message, const asio::error_code &err) override;
	public:
        SessionBase * CreateSocket() { return new TcpClientSession(this);}
		TcpClientSession * ConnectRemote(const std::string &name, const std::string & ip, unsigned short port);

	protected:
		bool Awake() override;

		void OnDestory() override;

	public:
		bool CloseSession(const std::string &address);
		bool SendByAddress(const std::string & address, SharedMessage message);
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
		std::queue<std::string> mRecvSessionQueue;
        std::vector<TcpClientSession *> mSessionArray;
        std::unordered_map<std::string, TcpClientSession *> mSessionAdressMap;
	private:
		std::unordered_map<std::string, IRequestMessageHandler *> mRequestMsgHandlers;
		std::unordered_map<std::string, IResponseMessageHandler *> mResponseMsgHandlers;
	};
}
