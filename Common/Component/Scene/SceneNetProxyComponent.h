#pragma once

#include <Component/Component.h>

#include <NetWork/TcpProxySession.h>
#include <Other/DoubleBufferQueue.h>
#include <NetWork/SocketEveHandler.h>

namespace Sentry
{
    // session 代理管理器 负责与网络线程通信 处理网络事件
    class SceneNetProxyComponent : public Component, public ISystemUpdate
    {
    public:
        SceneNetProxyComponent() {}

        virtual ~SceneNetProxyComponent() {}

    public:
        bool PushEventHandler(SocketEveHandler *eve); //不要手动调用
    public:
        bool DestorySession(const std::string &address);

        bool SendMsgByAddress(const std::string &address, NetMessageProxy *msg);

		TcpProxySession * ConnectByAddress(const std::string &address, const std::string &name);

    public:
        TcpProxySession *GetProxySession(const std::string &address);

        TcpProxySession *DelProxySession(const std::string &address);
	public:
		void NewConnect(const std::string & address);
		void SessionError(const std::string & address);
		void ConnectFailure(const std::string & address);
		void ConnectSuccessful(const std::string & address);
		void ReceiveNewMessage(const std::string & address, NetMessageProxy * message);
    protected:
        bool Awake() override;

        void OnSystemUpdate() final;

        virtual void OnNewSessionConnect(TcpProxySession *session) {}

        virtual void OnConnectSuccessful(TcpProxySession *session) {}

        virtual bool OnRecvMessage(const std::string &address, NetMessageProxy *msg);
    private:
        int mReConnectTime;

        class TimerComponent *mTimerManager;

        class SceneActionComponent *mActionManager;

        class SceneSessionComponent *mNetWorkManager;

        DoubleBufferQueue<SocketEveHandler *> mNetEventQueue;
        std::unordered_map<std::string, TcpProxySession *> mSessionMap; //管理所有的session
        std::unordered_map<std::string, TcpProxySession *> mConnectSessionMap; //正在连接的session
		std::unordered_map<std::string, std::queue<unsigned int>> mConnectCoroutines;
    };
}