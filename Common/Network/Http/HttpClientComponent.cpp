//
// Created by 64658 on 2021/8/5.
//
#include <Core/App.h>
#include <Thread/TaskThread.h>
#include "HttpClientComponent.h"
#include <Coroutine/CoroutineComponent.h>
#include <Network/Http/HttpRemoteSession.h>
#include "Component/Scene/TaskPoolComponent.h"
#include <Network/Http/Request/HttpGetRequest.h>
#include <Network/Http/Request/HttpPostRequest.h>
#include <Method/HttpServiceMethod.h>
#include <Network/Http/Content/HttpWriteContent.h>
#include <Network/Http/Response/HttpGettHandler.h>
#include <Network/Http/Response/HttpPostHandler.h>
#include <HttpService/HttpServiceComponent.h>
#include <Other/ProtocolConfig.h>
#include<Network/Http/HttpLocalsession.h>
namespace GameKeeper
{

    bool HttpClientComponent::Awake()
    {
        this->mCorComponent = this->GetComponent<CoroutineComponent>();
		this->mTaskComponent = this->GetComponent<TaskPoolComponent>();
        return true;
    }

	void HttpClientComponent::Start()
    {
        std::string json;
        const std::string path = App::Get().GetDownloadPath();
        const std::string url = "http://langrensha01.oss-cn-shenzhen.aliyuncs.com/res/area/city-config.json";
        std::string data = "fid=0&key=f5c417a28abf995d7ce6312b29556fd9";
        XCode code = this->Get(url, json);

        GKDebugFatal(code << "\n" << json.size());

        HttpJsonContent jsonContent;
        //this->Get("http://127.0.0.1:80/app/account/login?{}", json, 5);

        //this->Get("http://lrs-oss.whitewolvesx.com/app/default/boy.png", json);
	
        //GKDebugFatal(StringHelper::FormatJson(json));
    }

    void HttpClientComponent::OnListen(SocketProxy *socket)
    {
        auto httpSession = this->CreateRemoteSession();
        if(httpSession != nullptr)
        {
            httpSession->Start(socket);
        }
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

    XCode HttpClientComponent::Get(const std::string &url, std::string &json, int timeout)
    {
        HttpReadStringContent content(json);
        HttpLocalSession httpLocalSession(this);
#ifdef __DEBUG__
        long long t1 = TimeHelper::GetMilTimestamp();
        XCode code = httpLocalSession.Get(url, content);
        GKDebugInfo("get " << url << " use time [" << (TimeHelper::GetMilTimestamp() - t1) / 1000.f << "s]");
        return code;
#else
        return httpLocalSession.Get(url, content);
#endif
    }


    XCode HttpClientComponent::Post(const std::string &url, const std::string & request, std::string &response, int timeout)
    {
        HttpWriteStringContent writeContent(request);
        HttpReadStringContent readContent(response);
        HttpLocalSession httpLocalSession(this);
        return this->Post(url, writeContent, readContent, timeout);
    }

    XCode HttpClientComponent::Post(const std::string & url, HttpWriteContent & content, HttpReadContent & response, int timeout)
    {
        HttpLocalSession httpLocalSession(this);
#ifdef __DEBUG__
        long long t1 = TimeHelper::GetMilTimestamp();
        XCode code = httpLocalSession.Post(url, content, response);
        GKDebugInfo("post " << url << " use time [" << (TimeHelper::GetMilTimestamp() - t1) / 1000.f << "s]");
        return code;
#else
        return httpLocalSession.Post(url, content, response);
#endif
    }

    void HttpClientComponent::Invoke(HttpRequestHandler *remoteRequest)
    {
        const HttpServiceConfig *config = remoteRequest->GetHttpConfig();
        HttpServiceMethod *httpMethod = this->GetHttpMethod(config->Service, config->Method);
        //remoteRequest->SetCode(httpMethod->Invoke(remoteRequest));
    }

    HttpLocalSession *HttpClientComponent::CreateLocalSession()
    {
        HttpLocalSession * httpLocalSession = nullptr;
        if(!this->mLocalSessionPool.empty())
        {
            httpLocalSession = this->mLocalSessionPool.front();
            this->mLocalSessionPool.pop();
            return  httpLocalSession;
        }
        return new HttpLocalSession(this);
    }

    HttpRemoteSession *HttpClientComponent::CreateRemoteSession()
    {
        HttpRemoteSession * remoteSession = nullptr;
        if(!this->mRemoteSessionPool.empty())
        {
            remoteSession = this->mRemoteSessionPool.front();
            this->mRemoteSessionPool.pop();
            return remoteSession;
        }
        return new HttpRemoteSession(this);
    }

    void HttpClientComponent::DeleteSession(HttpLocalSession *session)
    {
        if(this->mLocalSessionPool.size() >= 10)
        {
            delete session;
            return;
        }
        session->Clear();
        this->mLocalSessionPool.push(session);
    }

    void HttpClientComponent::DeleteSession(HttpRemoteSession *session)
    {
        if(this->mRemoteSessionPool.size() >= 10)
        {
            delete session;
            return;
        }
        session->Clear();
        this->mRemoteSessionPool.push(session);
    }

}