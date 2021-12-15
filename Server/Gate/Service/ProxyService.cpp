//
// Created by zmhy0073 on 2021/12/1.
//

#include"GateService.h"
#include"Core/App.h"
#include"Service/RpcNode.h"
#include"Rpc/RpcProxyClient.h"
#include"Component/ProtoGateClientComponent.h"
namespace GameKeeper
{
    GateService::GateService()
    {

    }

    bool GateService::Awake()
    {
        BIND_RPC_FUNCTION(GateService::Ping);
        BIND_RPC_FUNCTION(GateService::Login);
        this->mGateComponent= this->GetComponent<ProtoGateClientComponent>();
        return true;
    }

    void GateService::OnAddRpcNode(RpcNode *node)
    {

    }

    void GateService::OnDelRpcNode(RpcNode *node)
    {

    }

    XCode GateService::Ping()
    {
        long long sockId = this->GetCurSocketId();
        auto gateClient = this->mGateComponent->GetProxyClient(sockId);
        if (gateClient != nullptr)
        {
            LOG_WARN(gateClient->GetAddress() << " ping");
            return XCode::Successful;
        }
        return XCode::Failure;
    }

    XCode GateService::Login(const c2s::ProxyLogin_Request &request)
    {
		return XCode::Successful;
    }

    bool GateService::LateAwake()
    {
        return true;
    }
}