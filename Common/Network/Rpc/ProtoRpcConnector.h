//
// Created by zmhy0073 on 2021/10/27.
//

#ifndef GameKeeper_TCPLOCALSESSION_H
#define GameKeeper_TCPLOCALSESSION_H
#include "ProtoRpcClient.h"
namespace GameKeeper
{
    class ProtoRpcConnector : public ProtoRpcClient
    {
    public:
        ProtoRpcConnector(ProtoRpcComponent * component, SocketProxy * socket);
    public:
        bool IsConnected() const { return this->mIsConnect; } //是不是正在连接
        SocketType GetSocketType() override { return SocketType::LocalSocket; }
    public:
        void StartConnect(std::string & ip, unsigned short port, StaticMethod * method = nullptr);
    private:  
		void ConnectHandler(std::string & ip, unsigned short port, StaticMethod * method);
    private:
        unsigned int mPort;
        atomic_bool mIsConnect;
        unsigned int mConnectCount;
    };
}

#endif //GameKeeper_TCPLOCALSESSION_H
