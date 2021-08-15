#pragma once

#include <thread>
#include <Protocol/com.pb.h>
#include <Manager/Manager.h>
#include <Script/LuaTable.h>
#include <Protocol/s2s.pb.h>
#include <Pool/ObjectPool.h>
#include <NetWork/SocketEvent.h>
#include <Other/DoubleBufferQueue.h>
#include <NetWork/NetMessageProxy.h>

namespace Sentry
{
				// 管理所有session  在网络线程中运行
				class TcpClientSession;

				class NetSessionManager : public Manager, public INetSystemUpdate
				{
				public:
								NetSessionManager();

								virtual ~NetSessionManager()
								{}

				public:
								friend Applocation;

				public: //网络线程调用
								void OnConnectSuccess(TcpClientSession *session);

								void OnSessionError(TcpClientSession *session, Net2MainEventType type);

								bool OnRecvMessage(TcpClientSession *session, const char *message, const size_t size);

								bool OnSendMessageError(TcpClientSession *session, const char *message, const size_t size);

				public:
								bool AddNetSessionEvent(Main2NetEvent *eve);

				public:
								TcpClientSession *Create(shared_ptr<AsioTcpSocket> socket);

								TcpClientSession *Create(const std::string &name, const std::string &address);

				protected:
								bool OnInit() override;

								void OnDestory() override;

								void OnNetSystemUpdate(AsioContext & io) final;

				private:

								void HandlerMainThreadEvent(Main2NetEvent *eve);

				private:
								bool DescorySession(const std::string &address);

								TcpClientSession *GetSession(const std::string &address);

				private:

								class ActionManager *mLocalActionManager;

								class NetProxyManager *mNetProxyManager;

				private:
								std::queue<std::string> mRecvSessionQueue;
								DoubleBufferQueue<Main2NetEvent *> mNetEventQueue;
								char mSendSharedBuffer[ASIO_TCP_SEND_MAX_COUNT + sizeof(unsigned int)];
								std::unordered_map<std::string, TcpClientSession *> mSessionAdressMap; //所有session
				};
}
