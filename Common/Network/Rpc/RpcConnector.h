//
// Created by zmhy0073 on 2021/10/27.
//

#ifndef GameKeeper_TCPLOCALSESSION_H
#define GameKeeper_TCPLOCALSESSION_H
#include "RpcClient.h"
namespace GameKeeper
{
    class RpcConnector : public RpcClient
    {
    public:
        RpcConnector(RpcComponent * component, const std::string & ip, const unsigned short port);
    public:
        SocketType GetSocketType() override { return SocketType::LocalSocket; }
    public:
        void StartConnect();
        void StartAsyncConnect();
    private:
        void ConnectHandler();
        void AsyncConnectHandler(unsigned int id);
    private:
        std::string mIp;
        unsigned int mPort;
        unsigned int mConnectCount;
    };
}

#endif //GameKeeper_TCPLOCALSESSION_H
