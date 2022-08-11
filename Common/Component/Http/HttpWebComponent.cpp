//
// Created by zmhy0073 on 2022/6/21.
//

#include"HttpWebComponent.h"
#include"Util/FileHelper.h"
#include"Network/Http/HttpHandlerClient.h"
#include"Component/Scene/NetThreadComponent.h"
#include"Component/HttpService/LocalHttpService.h"
namespace Sentry
{
    bool HttpWebComponent::LateAwake()
    {
        auto iter = this->GetApp()->ComponentBegin();
        for(; iter != this->GetApp()->ComponentEnd(); iter++)
        {
            Component * component = iter->second;
            LocalHttpService * localHttpService = component->Cast<LocalHttpService>();
            if(localHttpService != nullptr)
            {
                std::vector<const HttpInterfaceConfig *> httpInterConfigs;
                localHttpService->GetServiceConfig().GetConfigs(httpInterConfigs);
                for(const HttpInterfaceConfig * httpInterfaceConfig : httpInterConfigs)
                {
                    this->mHttpConfigs.emplace(httpInterfaceConfig->Path, httpInterfaceConfig);
                }
            }
        }
        this->mTaskComponent = this->GetApp()->GetTaskComponent();
        this->mNetComponent = this->GetComponent<NetThreadComponent>();
        return HttpListenComponent::LateAwake();
    }


    const HttpInterfaceConfig *HttpWebComponent::GetConfig(const std::string &path)
    {
        auto iter = this->mHttpConfigs.find(path);
        return iter != this->mHttpConfigs.end() ? iter->second : nullptr;
    }

    void HttpWebComponent::OnRequest(std::shared_ptr<HttpHandlerClient> httpClient)
    {
        assert(this->GetApp()->IsMainThread());
        std::shared_ptr<HttpHandlerRequest> request = httpClient->Request();
        std::shared_ptr<HttpHandlerResponse> response = httpClient->Response();

        const std::string & path = request->GetPath();
        const std::string & type = request->GetMethod();
        const std::string & data = request->GetContent();
        const std::string & address = request->GetAddress();

        const HttpInterfaceConfig *httpConfig = this->GetConfig(path);
        if (httpConfig == nullptr)
        {
            httpClient->StartWriter(HttpStatus::NOT_FOUND);
            CONSOLE_LOG_ERROR("[" << path << "] " << HttpStatusToString(HttpStatus::NOT_FOUND));
            return;
        }
        if (httpConfig->Type != type)
        {
            httpClient->StartWriter(HttpStatus::METHOD_NOT_ALLOWED);
            CONSOLE_LOG_ERROR("[" << path << "] " << HttpStatusToString(HttpStatus::METHOD_NOT_ALLOWED));
            return;
        }

        LocalHttpService *httpService = this->GetComponent<LocalHttpService>(httpConfig->Service);
        if (httpService == nullptr || !httpService->IsStartService())
        {
            httpClient->StartWriter(HttpStatus::NOT_FOUND);
            CONSOLE_LOG_ERROR("[" << path << "] " << HttpStatusToString(HttpStatus::NOT_FOUND));
            return;
        }
        if (!httpConfig->IsAsync)
        {
            httpService->Invoke(httpConfig->Method, request, response);
            httpClient->StartWriter(HttpStatus::OK);
        }
        else
        {
            this->mTaskComponent->Start([httpService, httpClient, httpConfig]() {

                std::shared_ptr<HttpHandlerRequest> request = httpClient->Request();
                std::shared_ptr<HttpHandlerResponse> response = httpClient->Response();

                httpService->Invoke(httpConfig->Method, request, response);
                httpClient->StartWriter(HttpStatus::OK);
            });
        }
    }
}