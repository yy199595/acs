#pragma once

#include <thread>
#include <Component/Component.h>
#include <Script/LuaTable.h>
#include <Pool/ObjectPool.h>

#include <Other/DoubleBufferQueue.h>
#include <NetWork/SocketEveHandler.h>

namespace Sentry
{
	// 管理所有session  在网络线程中运行
	class TcpClientSession;

    class NetSessionComponent : public Component, public ITcpContextUpdate, public ISessionHandler
	{
	public:
		NetSessionComponent();

		virtual ~NetSessionComponent()
		{}

	public: //网络线程调用

		void OnSessionError(TcpClientSession *session) override;

        void OnConnectComplete(TcpClientSession *session, bool isSuc) override;

        bool OnRecvMessage(TcpClientSession *session, const char *message, const size_t size) override;


    public:
		bool PushEventHandler(SocketEveHandler *eve);

	public:
		TcpClientSession *Create(shared_ptr<AsioTcpSocket> socket);

		TcpClientSession *Create(const std::string &name, const std::string &address);

	protected:
		bool Awake() override;

		void OnDestory() override;

		void OnTcpContextUpdate(AsioContext & io) final;

	public:
		bool StartClose(const std::string &address);
		bool StartConnect(const std::string & address, const std::string & name);
        bool StartSendMessage(const std::string & address, SharedMessage message);

    private:
		

		TcpClientSession *GetSession(const std::string &address);

	private:

		class NetProxyComponent *mNetProxyComponent;

	private:
		std::queue<std::string> mRecvSessionQueue;
        class ProtocolComponent * mProtocolComponent;
        DoubleBufferQueue<SocketEveHandler *> mNetEventQueue;
		std::unordered_map<std::string, TcpClientSession *> mSessionAdressMap; //所有session
	};
}
