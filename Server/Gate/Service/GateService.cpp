//
// Created by zmhy0073 on 2021/12/1.
//

#include"GateService.h"
#include"Object/App.h"
#include"Service/ServiceProxy.h"
#include"NetWork/RpcProxyClient.h"
#include"Component/GateClientComponent.h"
namespace Sentry
{
    bool GateService::Awake()
    {
        BIND_RPC_FUNCTION(GateService::Ping);
        BIND_RPC_FUNCTION(GateService::Login);
        this->mGateComponent= this->GetComponent<GateClientComponent>();
        return true;
    }

    XCode GateService::Ping()
    {
        long long sockId = this->GetCurSocketId();
        auto gateClient = this->mGateComponent->GetGateClient(sockId);
        if (gateClient != nullptr)
        {
            LOG_WARN("{0} ping", gateClient->GetAddress());
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