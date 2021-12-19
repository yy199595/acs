#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-member-init"
//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_PROTOPROXYCOMPONENT_H
#define GAMEKEEPER_PROTOPROXYCOMPONENT_H
#include"Component.h"
namespace GameKeeper
{
    class GateComponent : public Component,
                          public IClientProtoRpc<c2s::Rpc_Request, c2s::Rpc_Response>
    {
    public:
        GateComponent() = default;
        ~GateComponent() final = default;
    protected:
        bool Awake() final;
        bool LateAwake() final;
    public:
        XCode OnRequest(const c2s::Rpc_Request *request) final;
        XCode OnResponse(long long sockId, const c2s::Rpc_Response *response) final;
    private:
        std::string mProtoName;
        class RpcComponent * mRpcComponent;
        class RpcConfigComponent * mRpcConfigComponent;
        class NodeProxyComponent * mNodeProxyComponent;
        class GateClientComponent * mGateClientComponent;
    };
}


#endif //GAMEKEEPER_PROTOPROXYCOMPONENT_H

#pragma clang diagnostic pop