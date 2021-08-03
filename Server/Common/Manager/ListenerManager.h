#pragma once

#include"Manager.h"

namespace Sentry
{
    // 处理外部连接进来的session
    class TcpSessionListener;

    class ListenerManager : public Manager
    {
    public:
        ListenerManager() {}

        ~ListenerManager() {}

    public:
        const std::string &GetAddress() { return mListenAddress; }

    public:
        bool StartAccept();

    protected:
        bool OnInit() override;

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