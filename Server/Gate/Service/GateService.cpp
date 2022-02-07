//
// Created by zmhy0073 on 2021/12/1.
//

#include"GateService.h"
#include"Object/App.h"
#include"Service/ServiceProxy.h"
#include"NetWork/RpcGateClient.h"
#include"Component/GateClientComponent.h"
#include"NetWork/Listener/NetworkListener.h"
#include"Network/Listener/TcpServerComponent.h"
namespace Sentry
{
    bool GateService::Awake()
    {
        BIND_RPC_FUNCTION(GateService::Ping);
        BIND_RPC_FUNCTION(GateService::Login);
        BIND_RPC_FUNCTION(GateService::Allot);
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

    XCode GateService::Allot(const s2s::AddToGate_Request &request, s2s::AddToGate_Response &response)
    {
        TcpServerComponent * tcpServerComponent = this->GetComponent<TcpServerComponent>();
        const NetworkListener * gateListener = tcpServerComponent->GetListener("gate");
        if(gateListener == nullptr)
        {
            return XCode::Failure;
        }
        const ListenConfig & listenConfig = gateListener->GetConfig();

        response.set_gate_ip(listenConfig.Ip);
        response.set_gate_port(listenConfig.Port);
        return XCode::Successful;
    }

    bool GateService::LateAwake()
    {
        return true;
    }
}