//
// Created by zmhy0073 on 2021/10/27.
//

#ifndef GameKeeper_TCPLOCALSESSION_H
#define GameKeeper_TCPLOCALSESSION_H
#include "TcpClientSession.h"
namespace GameKeeper
{
    class TcpLocalSession : public TcpClientSession
    {
    public:
        TcpLocalSession(ISocketHandler * handler, const std::string & name, const std::string ip, const unsigned short port);
    public:
        SocketType GetSocketType() override { return SocketType::LocalSocket; }
    public:
        void ConnecRemote();
        void AsyncConnectRemote();
    private:
        void ConnectHandler();
        void AsyncConnectHandler(unsigned int od);
    private:
        std::string mIp;
        unsigned int mPort;
        unsigned int mConnectCount;
    };
}

#endif //GameKeeper_TCPLOCALSESSION_H
