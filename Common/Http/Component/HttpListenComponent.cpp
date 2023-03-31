//
// Created by zmhy0073 on 2022/8/11.
//

#include"HttpListenComponent.h"
#include"Http/Client/HttpHandlerClient.h"
#include"Server/Component/ThreadComponent.h"
#include"Http/Common/HttpResponse.h"
#include"Entity/App/App.h"
namespace Sentry
{
    void HttpListenComponent::OnListen(std::shared_ptr<SocketProxy> socket)
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

    bool HttpListenComponent::Send(const std::string& address, HttpStatus code)
    {
        auto iter = this->mHttpClients.find(address);
        if (iter == this->mHttpClients.end())
        {
            return false;
        }
        iter->second->StartWriter(code);
        return true;
    }
    bool HttpListenComponent::Send(const std::string& address, const std::string& str)
    {
        auto iter = this->mHttpClients.find(address);
        if (iter == this->mHttpClients.end())
        {
            return false;
        }
        std::shared_ptr<Http::Response> response
            = std::make_shared<Http::Response>();
        response->Str(HttpStatus::OK, str);
        iter->second->StartWriter(response);
        return true;
    }
    bool HttpListenComponent::Send(const std::string& address, std::shared_ptr<Http::Response> response)
    {
        auto iter = this->mHttpClients.find(address);
        if (iter == this->mHttpClients.end())
        {
            return false;
        }
        iter->second->StartWriter(response);
        return true;
    }
}