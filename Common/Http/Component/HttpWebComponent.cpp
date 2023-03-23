//
// Created by zmhy0073 on 2022/6/21.
//

#include"HttpWebComponent.h"
#include"App/App.h"
#include"Config/CodeConfig.h"
#include"Config/ServiceConfig.h"
#include"Client/HttpHandlerClient.h"
#include"Service/LocalHttpService.h"
#include"Component/ThreadComponent.h"
namespace Sentry
{
    HttpWebComponent::HttpWebComponent()
        : mSumCount(0), mWaitCount(0),
          mTaskComponent(nullptr)
    {

    }
    bool HttpWebComponent::LateAwake()
    {
        this->mWaitCount = 0;
        std::vector<LocalHttpService *> httpServices;
        this->mTaskComponent = this->mApp->GetTaskComponent();
        return this->mApp->GetComponents(httpServices) && this->StartListen("http");
    }

    void HttpWebComponent::OnRequest(const std::string& address, std::shared_ptr<Http::Request> request)
    {
        const HttpMethodConfig *httpConfig = HttpConfig::Inst()->GetMethodConfig(request->Path());
        if (httpConfig == nullptr)
        {
			//this->ClosetHttpClient(address);
            this->Send(address, HttpStatus::NOT_FOUND);
            LOG_ERROR("[" << address << "] <<" << request->Path() << ">>" << HttpStatusToString(HttpStatus::NOT_FOUND));
            return;
        }

        if (!httpConfig->Type.empty() && httpConfig->Type != request->Method())
        {			
			//this->ClosetHttpClient(address);
            this->Send(address, HttpStatus::METHOD_NOT_ALLOWED);
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
        const HttpMethodConfig* config, const std::shared_ptr<Http::Request>& request)
    {
		this->mSumCount++;
        this->mWaitCount++;
        LocalHttpService* httpService = this->GetComponent<LocalHttpService>(config->Service);
        if (httpService == nullptr || !httpService->IsStartService())
        {           
            this->Send(address, HttpStatus::NOT_FOUND);
            LOG_ERROR("[" << address << "] <<" << request->Url() << ">>" << HttpStatusToString(HttpStatus::NOT_FOUND));         
        }
        else
        {
            const std::string& method = config->Method;
            std::shared_ptr<Http::Response> response(new Http::Response());
            int code = httpService->Invoke(method, request, response);
            if (code != XCode::Successful)
            {
#ifdef __DEBUG__
                LOG_ERROR("[" << config->Type << "] " << config->Path 
                    << " : " << CodeConfig::Inst()->GetDesc(code));
#endif
            }
            this->Send(address, response);
        }            
        this->mWaitCount--;
    }

    void HttpWebComponent::OnRecord(Json::Writer&document)
    {
        document.Add("sum").Add(this->mSumCount);
        document.Add("wait").Add(this->mWaitCount);
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