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

	class SceneSessionComponent : public Component, public INetSystemUpdate
	{
	public:
		SceneSessionComponent();

		virtual ~SceneSessionComponent()
		{}

	public: //网络线程调用
		void OnConnectComplate(TcpClientSession *session, bool isSuc);

		void OnSessionError(TcpClientSession *session);

		bool OnRecvMessage(TcpClientSession *session, const char *message, const size_t size);

		bool OnSendMessageError(TcpClientSession *session, const char *message, const size_t size);

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
		bool StartConnect(const std::string & address, const std::string & name);
		bool StartSendMessage(const std::string & address, PacketMapper * message);
	private:
		

		TcpClientSession *GetSession(const std::string &address);

	private:

		class SceneNetProxyComponent *mNetProxyComponent;

	private:
		std::queue<std::string> mRecvSessionQueue;
		DoubleBufferQueue<SocketEveHandler *> mNetEventQueue;
		char mSendSharedBuffer[ASIO_TCP_SEND_MAX_COUNT + sizeof(unsigned int)];
		std::unordered_map<std::string, TcpClientSession *> mSessionAdressMap; //所有session
	};
}
