#pragma once

#include <Component/Component.h>

#include <NetWork/TcpProxySession.h>
#include <Other/DoubleBufferQueue.h>
#include <NetWork/SocketEveHandler.h>
#include <Network/MessageStream.h>

namespace Sentry
{
    // session 代理管理器 负责与网络线程通信 处理网络事件
    class NetProxyComponent : public Component, public ISystemUpdate
    {
    public:
        NetProxyComponent()
        {}

        virtual ~NetProxyComponent()
        {}

    public:
        bool PushEventHandler(SocketEveHandler *eve); //不要手动调用
    public:
        bool DestorySession(const std::string &address);

        bool SendNetMessage(const std::string &address, com::DataPacket_Request &message);

        bool SendNetMessage(const std::string &address, com::DataPacket_Response &message);

        TcpProxySession *Create(const std::string &address, const std::string &name);

    public:
        MessageStream &GetSendStream();

        TcpProxySession *GetProxySession(const std::string &address);

        TcpProxySession *DelProxySession(const std::string &address);


    public:
        void NewConnect(const std::string &address);

        void SessionError(const std::string &address);

        void ConnectAfter(const std::string &address, bool isSuc);

        bool ReceiveNewMessage(const std::string &address, SharedMessage message);

    protected:
        bool Awake() override;

        void OnSystemUpdate() final;

        virtual void OnNewSessionConnect(TcpProxySession *session)
        {}

        virtual void OnConnectSuccessful(TcpProxySession *session)
        {}

    private:
        int mReConnectTime;
        int mReConnectCount;
        MessageStream mSendStream;

        class ActionComponent * mActionComponent;
        class ServiceMgrComponent * mServiceComponent;

        class NetSessionComponent *mNetWorkManager;
        class ProtocolComponent *mProtocolComponent;

        DoubleBufferQueue<SocketEveHandler *> mNetEventQueue;
        std::unordered_map<std::string, TcpProxySession *> mSessionMap; //管理所有的session
        std::unordered_map<std::string, IRequestMessageHandler *> mRequestMsgHandlers;
        std::unordered_map<std::string, IResponseMessageHandler *> mResponseMsgHandlers;
    };
}