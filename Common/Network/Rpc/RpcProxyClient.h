//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_RPCPROXYCLIENT_H
#define GAMEKEEPER_RPCPROXYCLIENT_H

#include"RpcClient.h"
using namespace google::protobuf;
namespace GameKeeper
{
    class ProtoProxyClientComponent;
    class RpcProxyClient : public RpcClient
    {
    public:
        RpcProxyClient(SocketProxy * socket,SocketType type, ProtoProxyClientComponent * component);
        ~RpcProxyClient() final =default;

    public:
        void StartClose();
        unsigned int GetQps() const { return this->mQps;}
        bool StartSendData(char type, const Message * message);
        unsigned int GetCallCount() const { return this->mCallCount;}
    protected:
        void OnClose(XCode code) final;
        void OnConnect(XCode code) final;
        XCode OnRequest(const char *buffer, size_t size) final;
        XCode OnResponse(const char *buffer, size_t size) final;
    private:
        void SendData(char type, const Message * message);
    private:
        unsigned int mQps;
        unsigned int mCallCount;
        ProtoProxyClientComponent * mProxyComponent;
    };
}


#endif //GAMEKEEPER_RPCPROXYCLIENT_H
