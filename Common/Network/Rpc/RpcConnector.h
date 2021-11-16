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
        RpcConnector(RpcComponent * component, const std::string & ip, unsigned short port);
    public:
        bool IsConnected() const { return this->mIsConnect; } //是不是正在连接
        SocketType GetSocketType() override { return SocketType::LocalSocket; }
    public:
        void StartConnect(StaticMethod * method = nullptr);
        bool StartAsyncConnect();
    private:
        void ConnectHandler(StaticMethod * method);
        void AsyncConnectHandler(unsigned int id);
    private:
        unsigned int mPort;
        atomic_bool mIsConnect;
        unsigned int mConnectCount;
    };
}

#endif //GameKeeper_TCPLOCALSESSION_H
