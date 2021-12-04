//
// Created by zmhy0073 on 2021/12/1.
//

#include"ProxyService.h"
#include"Core/App.h"
#include"Service/RpcNodeProxy.h"
#include"Rpc/RpcProxyClient.h"
#include"ProxyRpc/ProtoProxyClientComponent.h"
namespace GameKeeper
{
    ProxyService::ProxyService()
    {

    }

    bool ProxyService::Awake()
    {
        BIND_RPC_FUNCTION(ProxyService::Ping);
        BIND_RPC_FUNCTION(ProxyService::Login);
        this->mProxyComponent= this->GetComponent<ProtoProxyClientComponent>();
        return true;
    }

    void ProxyService::OnAddProxyNode(RpcNodeProxy *node)
    {

    }

    void ProxyService::OnDelProxyNode(RpcNodeProxy *node)
    {

    }

    XCode ProxyService::Ping()
    {
        long long sockId = this->GetCurSocketId();
        auto proxyClient = this->mProxyComponent->GetProxyClient(sockId);
        if (proxyClient != nullptr)
        {
            LOG_WARN(proxyClient->GetAddress() << " ping");
            return XCode::Successful;
        }
        return XCode::Failure;
    }

    XCode ProxyService::Login(const c2s::ProxyLogin_Request &request)
    {
		return XCode::Successful;
    }
}