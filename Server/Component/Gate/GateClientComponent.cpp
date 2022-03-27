//
// Created by mac on 2021/11/28.
//

#include"GateClientComponent.h"
#include"App/App.h"
#include"NetWork/RpcGateClient.h"
#include"GateComponent.h"
#include"Component/Rpc/RpcComponent.h"
#ifdef __DEBUG__
#include"Util/StringHelper.h"
#include"Pool/MessagePool.h"
#include"google/protobuf/util/json_util.h"
#include"Component/Rpc//RpcConfigComponent.h"
#endif
namespace Sentry
{
    bool GateClientComponent::Awake()
    {
        this->mRpcComponent = nullptr;
        this->mTimerComponent = nullptr;
        this->mGateComponent = nullptr;
        return true;
    }

    bool GateClientComponent::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mTimerComponent = App::Get()->GetTimerComponent());
        LOG_CHECK_RET_FALSE(this->mRpcComponent = this->GetComponent<RpcComponent>());
        LOG_CHECK_RET_FALSE(this->mGateComponent = this->GetComponent<GateComponent>());
        return true;
    }

    void GateClientComponent::OnListen(std::shared_ptr<SocketProxy> socket)
    {
        long long id = socket->GetSocketId();
        auto iter = this->mGateClientMap.find(id);
        LOG_CHECK_RET(iter == this->mGateClientMap.end());
        const std::string ip = socket->GetSocket().remote_endpoint().address().to_string();
        if(this->mBlackList.find(ip) == this->mBlackList.end())
        {
            std::shared_ptr<RpcGateClient> gateClient(
                    new RpcGateClient(socket, SocketType::RemoteSocket, this));

            gateClient->StartReceive();
            this->mGateClientMap.emplace(id, gateClient);
        }
    }

    void GateClientComponent::OnRequest(std::shared_ptr<c2s::Rpc_Request> request) //客户端调过来的
    {
        XCode code = this->mGateComponent->OnRequest(request);
        if(code != XCode::Successful)
        {
            std::shared_ptr<c2s::Rpc_Response> responseMessage(new c2s::Rpc_Response());
#ifdef __DEBUG__
			RpcConfigComponent * configCom = this->GetComponent<RpcConfigComponent>();
            LOG_ERROR("player call", request->method_name(), "failure error = ", configCom->GetCodeDesc(code));
#endif
            responseMessage->set_code((int)code);
            responseMessage->set_rpc_id(request->rpc_id());
            this->SendToClient(request->sock_id(), responseMessage);
        }
    }

    void GateClientComponent::OnCloseSocket(long long id, XCode code)
    {
        auto iter = this->mGateClientMap.find(id);
        if(iter != this->mGateClientMap.end())
        {
#ifdef __DEBUG__
			RpcConfigComponent * configCom = this->GetComponent<RpcConfigComponent>();
            LOG_WARN("remove player session code = ", configCom->GetCodeDesc(code));
#endif
            this->mGateClientMap.erase(iter);
        }
    }

    bool GateClientComponent::SendToClient(long long sockId, std::shared_ptr<c2s::Rpc_Response> message)
    {
        auto proxyClient = this->GetGateClient(sockId);
        if(proxyClient == nullptr)
        {
            return false;
        }
        return proxyClient->SendToClient(message);
    }

    std::shared_ptr<RpcGateClient> GateClientComponent::GetGateClient(long long int sockId)
    {
        auto iter = this->mGateClientMap.find(sockId);
        return iter != this->mGateClientMap.end() ? iter->second : nullptr;
    }

    void GateClientComponent::StartClose(long long int id)
    {
        auto proxyClient = this->GetGateClient(id);
        if(proxyClient != nullptr)
        {
            proxyClient->StartClose();
        }
    }

    void GateClientComponent::CheckPlayerLogout(long long sockId)
    {
        auto proxyClient = this->GetGateClient(sockId);
        if(proxyClient != nullptr)
        {
            long long nowTime = Helper::Time::GetSecTimeStamp();
            if(nowTime - proxyClient->GetLastOperatorTime() >= 5)
            {
                proxyClient->StartClose();
                LOG_ERROR("{[0]} logout",  proxyClient->GetAddress());
                return;
            }
        }
        this->mTimerComponent->AsyncWait(5000, &GateClientComponent::CheckPlayerLogout, this, sockId);
    }
}