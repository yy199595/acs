//
// Created by 64658 on 2021/8/5.
//
#include "Core/App.h"
#include "Thread/TaskThread.h"
#include "HttpClientComponent.h"
#include "Network/Http/HttpRespSession.h"
#include"Http/Request/HttpGetRequest.h"
#include"Http/Request/HttpPostRequest.h"
#include "Method/HttpServiceMethod.h"
#include "Http/Service/HttpServiceComponent.h"
#include "Other/ProtocolConfig.h"
#include"Other/ElapsedTimer.h"
#include"Network/Http/HttpReqSession.h"

#include"Scene/LoggerComponent.h"
#include"Scene/ThreadPoolComponent.h"
namespace GameKeeper
{

    bool HttpClientComponent::Awake()
    {
        this->mCorComponent = nullptr;
        return true;
    }

    bool HttpClientComponent::LateAwake()
    {
        this->mCorComponent = App::Get().GetTaskComponent();
        this->mThreadComponent = this->GetComponent<ThreadPoolComponent>();

        std::string url1 = "http://v.juhe.cn/telecode/to_telecodes.php";
        this->mCorComponent->Start([this, url1]() {
            std::string url = "http://langrens.oss-cn-shenzhen.aliyuncs.com/res/area/city-config.json";
            this->Get(url);
        });
        return true;
    }

    void HttpClientComponent::OnListen(SocketProxy *socket)
    {

    }

	void HttpClientComponent::OnRequest(HttpRespSession * remoteSession)
	{
        auto requestHandler = remoteSession->GetReuqestHandler();
        const std::string & method = requestHandler->GetMethod();
        const std::string & service = requestHandler->GetComponent();
        auto httpMethod = this->GetHttpMethod(service, method);

        if(httpMethod == nullptr)
        {		
            requestHandler->SetResponseCode(HttpStatus::NOT_FOUND);
			LOG_ERROR("not find http method " << service << "." << method);
        }
        else
        {
            requestHandler->SetResponseCode(httpMethod->Invoke(remoteSession));
        }
        remoteSession->StartSendHttpMessage();
	}

    HttpServiceMethod *HttpClientComponent::GetHttpMethod(const std::string &service, const std::string &method)
    {
        auto component = this->GetComponent<HttpServiceComponent>(service);
        if(component == nullptr)
        {
            return nullptr;
        }
        return component->GetMethod(method);
    }

    XCode HttpClientComponent::Get(const std::string &url, int timeout)
    {
        ElapsedTimer timer;
        std::shared_ptr<HttpGetRequest> getRequest(new HttpGetRequest(url));
        if (getRequest->HasParseError())
        {
            return XCode::HttpUrlParseError;
        }
        NetWorkThread &netWorkThread = this->mThreadComponent->AllocateNetThread();
        std::shared_ptr<HttpReqSession> httpLocalSession(new HttpReqSession(netWorkThread));
        auto httpRespTask = httpLocalSession->NewTask<HttpGetRequest, HttpRespTask>(getRequest);

        HttpStatus code = httpRespTask->AwaitGetCode();

        LOG_ERROR(httpRespTask->Await());

        LOG_DEBUG("time = " << timer.GetSecond() << "s");
        return XCode::Successful;
    }


    XCode HttpClientComponent::Post(const std::string &url, const std::string & data, int timeout)
    {
        ElapsedTimer timer;
        std::shared_ptr<HttpPostRequest> postRequest(new HttpPostRequest(url, data));
        if (postRequest->HasParseError())
        {
            return XCode::HttpUrlParseError;
        }
        NetWorkThread &netWorkThread = this->mThreadComponent->AllocateNetThread();
        std::shared_ptr<HttpReqSession> httpLocalSession(new HttpReqSession(netWorkThread));
        auto httpRespTask = httpLocalSession->NewTask<HttpPostRequest, HttpRespTask>(postRequest);

        LOG_ERROR(httpRespTask->Await());

        LOG_DEBUG("time = " << timer.GetSecond() << "s");
        return XCode::Successful;
    }

    void HttpClientComponent::Invoke(HttpRespSession *remoteRequest)
    {			
//		 HttpRequestHandler * requestHandler = remoteRequest->GetReuqestHandler();
//		 const HttpServiceConfig * httpConfig = requestHandler->GetHttpConfig();
//		 if (requestHandler != nullptr)
//		 {
//			 const std::string & method = httpConfig->Method;
//			 const std::string & service = httpConfig->Service;
//			 auto httpMethod = this->GetHttpMethod(service, method);
//			 requestHandler->SetResponseCode(httpMethod->OnResponse(remoteRequest));
//		 }
    }
}