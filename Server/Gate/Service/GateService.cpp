//
// Created by zmhy0073 on 2021/12/1.
//

#include"GateService.h"
#include"Object/App.h"
#include"Util/MD5.h"
#include"Util/Guid.h"
#include"Service/ServiceProxy.h"
#include"NetWork/RpcGateClient.h"
#include"Scene/EntityMgrComponent.h"
#include"Component/GateClientComponent.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Listener/TcpServerComponent.h"

namespace Sentry
{
    bool GateService::Awake()
    {
        BIND_RPC_FUNCTION(GateService::Ping);
        BIND_RPC_FUNCTION(GateService::Login);
        BIND_RPC_FUNCTION(GateService::Allot);
        return true;
    }

    bool GateService::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mTimerComponent = this->GetComponent<TimerComponent>());
        TcpServerComponent * tcpServerComponent = this->GetComponent<TcpServerComponent>();
        LOG_CHECK_RET_FALSE(this->mGateListener = tcpServerComponent->GetListener("gate"));

        this->mGateComponent = this->GetComponent<GateClientComponent>();
        this->mEntityComponent = this->GetComponent<EntityMgrComponent>();
        if(this->mGateComponent == nullptr)
        {
            this->mGateComponent = this->mEntity->GetOrAddComponent<GateClientComponent>();
            LOG_CHECK_RET_FALSE(this->mGateComponent->LateAwake());
        }

        if(this->mEntityComponent == nullptr)
        {
            this->mEntityComponent = this->mEntity->GetOrAddComponent<EntityMgrComponent>();
            LOG_CHECK_RET_FALSE(this->mEntityComponent->LateAwake());
        }
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

    XCode GateService::Login(const c2s::GateLogin::Request &request)
    {
        auto iter = this->mTokenMap.find(request.token());
        LOGIC_THROW_ERROR(iter != this->mTokenMap.end());

        long long userId = iter->second;
        long long socketId = this->GetCurSocketId();
#ifdef __DEBUG__
        LOG_DEBUG("{0} player login to gate", userId);
#endif
        std::shared_ptr<Entity> player(new Entity(userId, socketId));
        LOGIC_THROW_ERROR(this->mEntityComponent->Add(player));
		return XCode::Successful;
    }

    void GateService::OnTokenTimeout(const std::string &token)
    {
        auto iter = this->mTokenMap.find(token);
        if(iter != this->mTokenMap.end())
        {
            this->mTokenMap.erase(iter);
        }
    }

    XCode GateService::Allot(const s2s::AddToGate_Request &request, s2s::AddToGate_Response &response)
    {
        if(this->mEntityComponent->GetEntityCount() >= 10000)
        {
            return XCode::Failure;
        }
        long long value = Helper::Guid::Create();
        const ListenConfig & listenConfig = this->mGateListener->GetConfig();
        std::string token = Helper::Md5::GetMd5(std::to_string(value));

        this->mTokenMap.emplace(token, request.user_id());
        this->mTimerComponent->AsyncWait(5000, &GateService::OnTokenTimeout, this, token);

        response.set_login_token(token);
        response.set_gate_ip(listenConfig.Ip);
        response.set_gate_port(listenConfig.Port);
        return XCode::Successful;
    }
}