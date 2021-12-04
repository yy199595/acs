//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_PROTOPROXYCOMPONENT_H
#define GAMEKEEPER_PROTOPROXYCOMPONENT_H
#include"Component.h"
namespace GameKeeper
{
    class ProtoProxyComponent : public Component,
        public IClientProtoRpc<c2s::Rpc_Request, c2s::Rpc_Response>
    {
    public:
        ProtoProxyComponent();
        ~ProtoProxyComponent() final = default;
    protected:
        bool Awake() final;
    public:
        XCode OnRequest(const c2s::Rpc_Request *request) final;
        XCode OnResponse(long long sockId, const c2s::Rpc_Response *response) final;
    private:
        std::string mProtoName;
        class ProtoRpcComponent * mRpcComponent;
        class RpcConfigComponent * mRpcConfigComponent;
        class NodeProxyComponent * mNodeProxyComponent;
        class ProtoProxyClientComponent * mProxyClientComponent;
    };
}


#endif //GAMEKEEPER_PROTOPROXYCOMPONENT_H
