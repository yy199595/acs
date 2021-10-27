//
// Created by zmhy0073 on 2021/10/27.
//

#ifndef SENTRY_TCPLOCALSESSION_H
#define SENTRY_TCPLOCALSESSION_H
#include "TcpClientSession.h"
namespace Sentry
{
    class TcpLocalSession : public TcpClientSession
    {
    public:
        using TcpClientSession::TcpClientSession;
    public:
        SocketType GetSocketType() override { return SocketType::LocalSocket; }
        void ConnectByAddress(const std::string & name, const std::string & ip, unsigned short port);
        void AsyncConnectByAddress(const std::string & name, const std::string & ip, unsigned short port);
    private:
        void ConnectHandler();
        void AsyncConnectHandler(unsigned int od);
    private:
        std::string mIp;
        std::string mName;
        unsigned int mPort;
    };
}

#endif //SENTRY_TCPLOCALSESSION_H
