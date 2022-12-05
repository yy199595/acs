//
// Created by zmhy0073 on 2022/8/11.
//

#include"HttpListenComponent.h"
#include"Client/HttpHandlerClient.h"
#include"Component/NetThreadComponent.h"

namespace Sentry
{
    bool HttpListenComponent::LateAwake()
    {
        this->mNetComponent = this->GetComponent<NetThreadComponent>();
        return this->mNetComponent != nullptr;
    }
    bool HttpListenComponent::OnListen(std::shared_ptr<SocketProxy> socket)
    {
#ifdef __DEBUG__
        //LOG_DEBUG("handler http socket count = " << count++);
#endif
        assert(this->mApp->IsMainThread());
        std::shared_ptr<HttpHandlerClient> handlerClient;
        if(!this->mClientPools.empty())
        {
            handlerClient = this->mClientPools.front();
            handlerClient->Reset(socket);
            this->mClientPools.pop();
        }
        else
        {
            handlerClient = std::make_shared<HttpHandlerClient>(this, socket);
        }

        handlerClient->StartReceive();
        const std::string &address = socket->GetAddress();
        this->mHttpClients.emplace(address, handlerClient);
        return true;
    }

    void HttpListenComponent::ClosetHttpClient(const std::string &address)
    {
        assert(this->mApp->IsMainThread());
        auto iter = this->mHttpClients.find(address);
        if(iter != this->mHttpClients.end())
        {
            if (this->OnDelClient(address))
            {
                std::shared_ptr<HttpHandlerClient> handlerClient = iter->second;
                if (this->mClientPools.size() <= 100)
                {
                    this->mClientPools.push(handlerClient);
                }
                this->mHttpClients.erase(iter);
            }
        }
    }

    HttpHandlerClient * HttpListenComponent::GetClient(const std::string& address)
    {
        auto iter = this->mHttpClients.find(address);
        return iter != this->mHttpClients.end() ? iter->second.get() : nullptr;
    }
}