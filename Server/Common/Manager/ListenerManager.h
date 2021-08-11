#pragma once

#include"Manager.h"

namespace Sentry
{
    // 处理外部连接进来的session
    class TcpSessionListener;

				class ListenerManager : public Manager, public INetSystemUpdate
    {
    public:
        ListenerManager() {}

        ~ListenerManager() {}

    public:
        const std::string &GetAddress() { return mListenAddress; }

    protected:
        bool OnInit() override;
        void OnNetSystemUpdate(AsioContext & io) final;

    private:
        bool mIsAccept;
        std::string mListenerIp;    //监听的ip
        std::string mListenAddress;            //地址
        unsigned short mListenerPort;
        AsioTcpAcceptor *mBindAcceptor;
        NetSessionManager *mDispatchManager;
        std::set<std::string> mWhiteList;    //白名单
    };
}