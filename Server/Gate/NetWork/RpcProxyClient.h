//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_RPCPROXYCLIENT_H
#define GAMEKEEPER_RPCPROXYCLIENT_H

#include"Network/Rpc/RpcClient.h"
using namespace google::protobuf;
namespace GameKeeper
{
    class GateClientComponent;
    class RpcProxyClient : public RpcClient
    {
    public:
        RpcProxyClient(SocketProxy * socket, SocketType type, GateClientComponent * component);
        ~RpcProxyClient() final =default;

    public:
        void StartClose();
        unsigned int GetQps() const { return this->mQps;}
        bool SendToClient(const c2s::Rpc_Request * message);
        bool SendToClient(const c2s::Rpc_Response * message);
        unsigned int GetCallCount() const { return this->mCallCount;}
    protected:
        void OnClose(XCode code) final;
        void OnConnect(XCode code) final;
        void OnSendData(XCode code, const Message *) final;
        XCode OnRequest(const char *buffer, size_t size) final;
        XCode OnResponse(const char *buffer, size_t size) final;
    private:
        unsigned int mQps;
        unsigned int mCallCount;
        GateClientComponent * mProxyComponent;
    };
}


#endif //GAMEKEEPER_RPCPROXYCLIENT_H
