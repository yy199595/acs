//
// Created by zmhy0073 on 2022/8/1.
//

#include "JsonClientComponent.h"
#include"NetWork/JsonRpcClient.h"
namespace Sentry
{
    bool JsonClientComponent::OnListen(std::shared_ptr<SocketProxy> socket)
    {
        const std::string & address = socket->GetAddress();
        std::shared_ptr<Tcp::JsonRpcClient> jsonClient(new Tcp::JsonRpcClient(socket, this));

        jsonClient->StartReceive();
        this->mClients.emplace(address, jsonClient);
        return true;
    }

    void JsonClientComponent::OnRequest(std::shared_ptr<Tcp::JsonRequest> request)
    {

    }

    bool JsonClientComponent::SendToClient(const std::string &address, const std::string &json)
    {
        auto iter = this->mClients.find(address);
        if(iter != this->mClients.end())
        {
            return false;
        }
        iter->second->SendMesageData(json);
        return true;
    }
}