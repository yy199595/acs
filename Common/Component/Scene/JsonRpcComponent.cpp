//
// Created by mac on 2021/11/20.
//
#include"JsonRpcComponent.h"
#include"Scene/RpcConfigComponent.h"
#include"Scene/RpcRequestComponent.h"
#include"Scene/RpcResponseComponent.h"
namespace GameKeeper
{
    bool JsonRpcComponent::Awake()
    {
        this->mRpcConfigComponent = this->GetComponent<RpcConfigComponent>();
        this->mRequestComponent = this->GetComponent<RpcRequestComponent>();
        this->mResponseComponent = this->GetComponent<RpcResponseComponent>();
        return true;
    }

    void JsonRpcComponent::OnRequest(long long id, RapidJsonReader * request)
    {

    }

    void JsonRpcComponent::OnResponse(long long id, RapidJsonReader * response)
    {

    }

    void JsonRpcComponent::StartClose(long long id)
    {

    }

    void JsonRpcComponent::OnConnectAfter(long long id, XCode code)
    {

    }

    void JsonRpcComponent::OnCloseSocket(long long id, XCode code)
    {

    }

    JsonRpcClient *JsonRpcComponent::GetSession(long long int id)
    {
        auto iter = this->mClientMap.find(id);
        return iter != this->mClientMap.end() ? iter->second : nullptr;
    }

    void JsonRpcComponent::OnListen(SocketProxy *socket)
    {

    }

}
