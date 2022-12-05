//
// Created by zmhy0073 on 2022/6/21.
//

#include"HttpWebComponent.h"
#include"File/FileHelper.h"
#include"Defer/Defer.h"
#include"Config/CodeConfig.h"
#include"Config/ServiceConfig.h"
#include"Client/HttpHandlerClient.h"
#include"Service/LocalHttpService.h"
#include"Component/NetThreadComponent.h"
namespace Sentry
{
    bool HttpWebComponent::LateAwake()
    {
        this->mWaitCount = 0;
        std::vector<LocalHttpService *> httpServices;
        this->mTaskComponent = this->mApp->GetTaskComponent();
        if (this->mApp->GetComponents(httpServices) <= 0)
        {
            return false;
        }
        return this->StartListen("http");
    }

    void HttpWebComponent::OnRequest(std::shared_ptr<HttpHandlerClient> httpClient)
    {
        assert(this->mApp->IsMainThread());
        const std::string& address = httpClient->GetAddress();
        std::shared_ptr<Http::Request> request = httpClient->Request();		
        const HttpMethodConfig *httpConfig = HttpConfig::Inst()->GetMethodConfig(request->Path());
        if (httpConfig == nullptr)
        {
            httpClient->StartWriter(HttpStatus::NOT_FOUND);
            LOG_ERROR("[" << address << "] <<" << request->Url() << ">>" << HttpStatusToString(HttpStatus::NOT_FOUND));
            return;
        }

        if (!httpConfig->Type.empty() && httpConfig->Type != request->Method())
        {			
            httpClient->StartWriter(HttpStatus::METHOD_NOT_ALLOWED);
            LOG_ERROR("[" << address << "] <<" << request->Url() << ">>" << HttpStatusToString(HttpStatus::METHOD_NOT_ALLOWED));
            return;
        }

        if (!httpConfig->IsAsync)
        {
            this->Invoke(address, httpConfig, request);
            return;
        }       
        this->mTaskComponent->Start(&HttpWebComponent::Invoke, this, address, httpConfig, request);
    }
    void HttpWebComponent::Invoke(const std::string& address, 
        const HttpMethodConfig* config, std::shared_ptr<Http::Request> request)
    {
        this->mWaitCount++;
        std::shared_ptr<Http::Response> response(new Http::Response());
        LocalHttpService* httpService = this->GetComponent<LocalHttpService>(config->Service);
        if (httpService == nullptr || !httpService->IsStartService())
        {
            response->SetCode(HttpStatus::NOT_FOUND);
            LOG_ERROR("[" << address << "] <<" << request->Url() << ">>" << HttpStatusToString(HttpStatus::NOT_FOUND));         
        }
        else
        {
            const std::string& method = config->Method;
            XCode code = httpService->Invoke(method, request, response);
            if (code != XCode::Successful)
            {
                LOG_ERROR("[" << config->Type << "] " << config->Path 
                    << " : " << CodeConfig::Inst()->GetDesc(code));
            }
        }
        HttpHandlerClient* httpHandlerClient = this->GetClient(address);
        if (httpHandlerClient != nullptr)
        {
            httpHandlerClient->StartWriter(response);
        }
        this->mWaitCount--;
    }

    bool HttpWebComponent::OnDelClient(const std::string& address)
    {
        auto iter = this->mTasks.find(address);
        if (iter != this->mTasks.end())
        {           
            this->mTasks.erase(iter);
        }
        return true;
    }
}