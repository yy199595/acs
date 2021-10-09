#pragma once

#include <thread>
#include <Component/Component.h>
#include <Script/LuaTable.h>
#include <Pool/ObjectPool.h>

#include <Other/DoubleBufferQueue.h>
#include <NetWork/PacketMapper.h>
#include <NetWork/SocketEveHandler.h>

namespace Sentry
{
	// 管理所有session  在网络线程中运行
	class TcpClientSession;

    class NetSessionComponent : public Component, public INetSystemUpdate, public ISessionHandler
	{
	public:
		NetSessionComponent();

		virtual ~NetSessionComponent()
		{}

	public: //网络线程调用

		void OnSessionError(TcpClientSession *session) override;

		void OnSendMessageAfter(std::string * message) override;

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

		void OnNetSystemUpdate(AsioContext & io) final;

	public:
		bool StartClose(const std::string &address);
		bool StartSendMessage(PacketMapper * message);
		bool StartConnect(const std::string & address, const std::string & name);
		
	private:
		

		TcpClientSession *GetSession(const std::string &address);

	private:

		class NetProxyComponent *mNetProxyComponent;

	private:
        ObjectPool<std::string> mSendBufferPool;
		std::queue<std::string> mRecvSessionQueue;
		DoubleBufferQueue<SocketEveHandler *> mNetEventQueue;
		char mSendSharedBuffer[TCP_SEND_MAX_COUNT + sizeof(unsigned int)];
		std::unordered_map<std::string, TcpClientSession *> mSessionAdressMap; //所有session
	};
}
