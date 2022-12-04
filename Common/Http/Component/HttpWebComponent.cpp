//
// Created by zmhy0073 on 2022/6/21.
//

#include"HttpWebComponent.h"
#include"File/FileHelper.h"
#include"Defer/Defer.h"
#include"Config/ServiceConfig.h"
#include"Client/HttpHandlerClient.h"
#include"Service/LocalHttpService.h"
#include"Component/NetThreadComponent.h"
namespace Sentry
{
    bool HttpWebComponent::LateAwake()
    {
        std::vector<LocalHttpService *> httpServices;
        if (this->mApp->GetComponents(httpServices) <= 0)
        {
            return false;
        }
        this->mTaskComponent = this->mApp->GetTaskComponent();
        return this->StartListen("http");
    }

    void HttpWebComponent::OnRequest(std::shared_ptr<HttpHandlerClient> httpClient)
    {
        assert(this->mApp->IsMainThread());
        std::shared_ptr<Http::Request> request = httpClient->Request();
		std::shared_ptr<Http::Response> response = httpClient->Response();
		Defer defer(std::bind(&HttpHandlerClient::StartWriter, httpClient));

        const HttpMethodConfig *httpConfig = HttpConfig::Inst()->GetMethodConfig(request->Path());
        if (httpConfig == nullptr)
        {
			response->Str(HttpStatus::NOT_FOUND, "not find route");
            CONSOLE_LOG_ERROR("[" << request->Url() << "] " << HttpStatusToString(HttpStatus::NOT_FOUND));
            return;
        }

        if (!httpConfig->Type.empty() && httpConfig->Type != request->Method())
        {
			response->Str(HttpStatus::METHOD_NOT_ALLOWED, "method error");
            CONSOLE_LOG_ERROR("[" << request->Url() << "] " << HttpStatusToString(HttpStatus::METHOD_NOT_ALLOWED));
            return;
        }

        LocalHttpService *httpService = this->GetComponent<LocalHttpService>(httpConfig->Service);
        if (httpService == nullptr || !httpService->IsStartService())
        {
			response->Str(HttpStatus::NOT_FOUND, "not find service");
			CONSOLE_LOG_ERROR("[" << request->Path() << "] " << HttpStatusToString(HttpStatus::NOT_FOUND));
            return;
        }
        if (!httpConfig->IsAsync)
        {
            XCode code = httpService->Invoke(httpConfig->Method, request, response);
            {
                response->Header().Add("code", (int) code);
            }
        }
        else
        {
			defer.Cancle();
            TaskContext * taskContext = this->mTaskComponent->Start(
                [httpService, httpClient, httpConfig, this]() 
            {
                const std::string& address = httpClient->GetAddress();
                std::shared_ptr<Http::Request> request = httpClient->Request();
                std::shared_ptr<Http::Response> response = httpClient->Response();
                XCode code = httpService->Invoke(httpConfig->Method, request, response);
                {
                    response->Header().Add("code", (int) code);
                    httpClient->StartWriter();
                }
                auto iter = this->mTasks.find(address);
                if (iter != this->mTasks.end())
                {
                    this->mTasks.erase(iter);
                }
            });
            const std::string& address = httpClient->GetAddress();
            this->mTasks.emplace(address, taskContext->mCoroutineId);
        }
    }

    bool HttpWebComponent::OnDelClient(const std::string& address)
    {
        auto iter = this->mTasks.find(address);
        if (iter != this->mTasks.end())
        {
            this->mTaskComponent->Resume(iter->second);
            this->mTasks.erase(iter);
        }
        return true;
    }
}