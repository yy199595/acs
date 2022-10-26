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
        return this->StartListen("web");
    }

    void HttpWebComponent::OnRequest(std::shared_ptr<HttpHandlerClient> httpClient)
    {
        assert(this->mApp->IsMainThread());
        std::shared_ptr<HttpHandlerRequest> request = httpClient->Request();
		std::shared_ptr<HttpHandlerResponse> response = httpClient->Response();
		Defer defer(std::bind(&HttpHandlerClient::StartWriter, httpClient));

		const HttpData & httpData = request->GetData();
        const HttpMethodConfig *httpConfig = HttpConfig::Inst()->GetMethodConfig(httpData.mPath);
        if (httpConfig == nullptr)
        {
			response->Str(HttpStatus::NOT_FOUND, "not find route");
            CONSOLE_LOG_ERROR("[" << request->GetUrl() << "] " << HttpStatusToString(HttpStatus::NOT_FOUND));
            return;
        }

        if (!httpConfig->Type.empty() && httpConfig->Type != httpData.mMethod)
        {
			response->Str(HttpStatus::METHOD_NOT_ALLOWED, "method error");
            CONSOLE_LOG_ERROR("[" << request->GetUrl() << "] " << HttpStatusToString(HttpStatus::METHOD_NOT_ALLOWED));
            return;
        }

        LocalHttpService *httpService = this->GetComponent<LocalHttpService>(httpConfig->Service);
        if (httpService == nullptr || !httpService->IsStartService())
        {
			response->Str(HttpStatus::NOT_FOUND, "not find service");
			CONSOLE_LOG_ERROR("[" << httpData.mPath << "] " << HttpStatusToString(HttpStatus::NOT_FOUND));
            return;
        }
        if (!httpConfig->IsAsync)
        {
            std::shared_ptr<HttpHandlerResponse> response = httpClient->Response();
            XCode code = httpService->Invoke(httpConfig->Method, request, response);
            {
                response->AddHead("code", (int) code);
            }
        }
        else
        {
			defer.Cancle();
            this->mTaskComponent->Start([httpService, httpClient, httpConfig]() {

                std::shared_ptr<HttpHandlerRequest> request = httpClient->Request();
                std::shared_ptr<HttpHandlerResponse> response = httpClient->Response();
                XCode code = httpService->Invoke(httpConfig->Method, request, response);
                {
                    response->AddHead("code", (int) code);
                    httpClient->StartWriter();
                }
            });
        }
    }
}