//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_PROTOPROXYCLIENTCOMPONENT_H
#define GAMEKEEPER_PROTOPROXYCLIENTCOMPONENT_H
#include"Component.h"
namespace GameKeeper
{
    class RpcProxyClient;
    class ProtoProxyClientComponent : public Component, public ISocketListen,
        public IRpc<c2s::Rpc_Request, c2s::Rpc_Response>
    {
    public:
        ProtoProxyClientComponent() = default;
        ~ProtoProxyClientComponent() final= default;
    public:
        void StartClose(long long id) final;
        void OnRequest(c2s::Rpc_Request * request) final;
        void OnCloseSocket(long long id, XCode code) final;
        void OnResponse(c2s::Rpc_Response * response) final { }
        void OnConnectAfter(long long id, XCode code) final { }
    public:
        RpcProxyClient * GetProxyClient(long long sockId);
        bool SendProtoMessage(long long sockId, const c2s::Rpc_Response * message);
    protected:
        bool Awake() final;
        void OnListen(SocketProxy *socket) final;

    private:
        void CheckPlayerLogout(long long sockId);
    private:
        std::set<std::string> mBlackList;
        class TimerComponent * mTimerComponent;
        class ProtoRpcComponent * mRpcComponent;
        class ProtoProxyComponent * mProxyComponent;
        std::unordered_map<long long, RpcProxyClient *> mProxyClientMap;
    };
}


#endif //GAMEKEEPER_PROTOPROXYCLIENTCOMPONENT_H
