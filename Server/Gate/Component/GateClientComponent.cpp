//
// Created by mac on 2021/11/28.
//

#include "GateClientComponent.h"
#include"Object/App.h"
#include"NetWork/RpcProxyClient.h"
#include"GateComponent.h"
#include"Rpc/RpcComponent.h"
#ifdef __DEBUG__
#include"Util/StringHelper.h"
#include"Scene/RpcConfigComponent.h"
#include"Pool/MessagePool.h"
#include"google/protobuf/util/json_util.h"
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
        LOG_CHECK_RET_FALSE(this->mTimerComponent = App::Get().GetTimerComponent());
        LOG_CHECK_RET_FALSE(this->mRpcComponent = this->GetComponent<RpcComponent>());
        LOG_CHECK_RET_FALSE(this->mGateComponent = this->GetComponent<GateComponent>());
        return true;
    }

    void GateClientComponent::OnListen(std::shared_ptr<SocketProxy> socket)
    {
        long long id = socket->GetSocketId();
        auto iter = this->mProxyClientMap.find(id);
        LOG_CHECK_RET(iter == this->mProxyClientMap.end());
        const std::string ip = socket->GetSocket().remote_endpoint().address().to_string();
        if(this->mBlackList.find(ip) == this->mBlackList.end())
        {
            auto rpcClient = new RpcProxyClient(socket, SocketType::RemoteSocket, this);
#ifdef __DEBUG__
            LOG_INFO("new player connect proxy component ip : ", ip);
#endif
            rpcClient->StartReceive();
            this->mProxyClientMap.insert(std::make_pair(id, rpcClient));
        }
    }

    void GateClientComponent::OnRequest(std::shared_ptr<c2s::Rpc_Request> request) //客户端调过来的
    {
#ifdef __DEBUG__
        LOG_WARN("**********[client request]**********");
        LOG_WARN("func = ", request->method_name());
        LOG_WARN("json = ", Helper::Proto::ToJson(*request));
        LOG_WARN("*****************************************");
#endif
        XCode code = this->mGateComponent->OnRequest(request);
        if(code != XCode::Successful)
        {
            std::shared_ptr<c2s::Rpc_Response> responseMessage(new c2s::Rpc_Response());
#ifdef __DEBUG__
            auto configCom = App::Get().GetComponent<RpcConfigComponent>();
            LOG_ERROR("player call", request->method_name(), "failure error = ", configCom->GetCodeDesc(code));
#endif
            responseMessage->set_code((int)code);
            responseMessage->set_rpc_id(request->rpc_id());
            this->SendToClient(request->sock_id(), responseMessage);
        }
    }

    void GateClientComponent::OnCloseSocket(long long id, XCode code)
    {
        auto iter = this->mProxyClientMap.find(id);
        if(iter != this->mProxyClientMap.end())
        {
            RpcProxyClient *proxyClient = iter->second;
            this->mProxyClientMap.erase(iter);
            if (code == XCode::UnKnowPacket) //恶意消息
            {
                this->mBlackList.emplace(proxyClient->GetIp());
            }
            delete proxyClient;
#ifdef __DEBUG__
            auto configCom = App::Get().GetComponent<RpcConfigComponent>();
            LOG_WARN("remove player session code = ", configCom->GetCodeDesc(code));
#endif
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

    RpcProxyClient *GateClientComponent::GetGateClient(long long int sockId)
    {
        auto iter = this->mProxyClientMap.find(sockId);
        return iter != this->mProxyClientMap.end() ? iter->second : nullptr;
    }

    void GateClientComponent::StartClose(long long int id)
    {
        RpcProxyClient * proxyClient = this->GetGateClient(id);
        if(proxyClient != nullptr)
        {
            proxyClient->StartClose();
        }
    }

    void GateClientComponent::CheckPlayerLogout(long long sockId)
    {
        RpcProxyClient * proxyClient = this->GetGateClient(sockId);
        if(proxyClient != nullptr)
        {
            long long nowTime = Helper::Time::GetSecTimeStamp();
            if(nowTime - proxyClient->GetLastOperatorTime() >= 5)
            {
                proxyClient->StartClose();
                LOG_ERROR(sockId,  " logout ");
                return;
            }
        }
        this->mTimerComponent->AsyncWait(5000, &GateClientComponent::CheckPlayerLogout, this, sockId);
    }
}