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
#include<Util/StringHelper.h>
namespace GameKeeper
{

    bool HttpClientComponent::Awake()
    {
        this->mCorComponent = this->GetComponent<CoroutineComponent>();
        return true;
    }

	void HttpClientComponent::Start()
    {
        std::string json;
        const std::string path = App::Get().GetDownloadPath();
        const std::string url = "http://langrensha01.oss-cn-shenzhen.aliyuncs.com/res/area/city-config.json";
        std::string data = "fid=0&key=f5c417a28abf995d7ce6312b29556fd9";
        //this->Download(url, path + "city-config.json");

        //GKDebugFatal(json.size());

        HttpJsonContent jsonContent;
        this->Get("http://127.0.0.1:80/app/account/login?{}", json, 5);

        //this->Get("http://lrs-oss.whitewolvesx.com/app/default/boy.png", json);
	
        GKDebugFatal(StringHelper::FormatJson(json));
    }

    HttpRequestHandler *HttpClientComponent::CreateMethodHandler(const std::string &method, HttpRemoteSession * session)
    {
        if (method == "GET")
        {
            return new HttpGettHandler(this, session);
        }
        if(method == "POST")
        {
            return new HttpPostHandler(this, session);
        }
        return nullptr;
    }

    void HttpClientComponent::HandlerHttpRequest(HttpRequestHandler *remoteRequest)
    {
        if(remoteRequest->GetErrorCode() != XCode::Successful)
        {
            delete remoteRequest;
            return;
        }
		const HttpServiceConfig * config =  remoteRequest->GetHttpConfig();    
		GKDebugWarning("http call " << config->Service << "." << config->Method);
        if(!config->IsAsync)
        {
            HttpServiceMethod *httpMethod = this->GetHttpMethod(config->Service, config->Method);
            remoteRequest->SetCode(httpMethod->Invoke(remoteRequest));
        }
        this->mCorComponent->StartCoroutine(&HttpClientComponent::Invoke, this, remoteRequest);
    }

    void HttpClientComponent::OnListen(SocketProxy *socket)
    {
        auto httpSession = new HttpRemoteSession(this);
        httpSession->SetSocketProxy(socket);
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

    XCode HttpClientComponent::Download(const std::string &url, const std::string &path, int timeout)
    {
        HttpReadFileContent content(path);
        if(!content.OpenFile())
        {
            return XCode::Failure;
        }
        HttpGetRequest httpGetRequest(this);
#ifdef __DEBUG__
        long long t1 = TimeHelper::GetMilTimestamp();
        XCode code = httpGetRequest.Get(url, content);
        long long t2 = TimeHelper::GetMilTimestamp();
        GKDebugInfo("get " << url << " use time [" << (t2 - t1) / 1000.f << "s]");
        return code;
#else
        return httpGetRequest.Get(url, content);
#endif
    }

    XCode HttpClientComponent::Get(const std::string &url, std::string &json, int timeout)
    {
        HttpReadStringContent content(json);
        HttpGetRequest httpGetRequest(this);
#ifdef __DEBUG__
        long long t1 = TimeHelper::GetMilTimestamp();
        XCode code = httpGetRequest.Get(url, content);
        long long t2 = TimeHelper::GetMilTimestamp();
        GKDebugInfo("get " << url << " use time [" << (t2 - t1) / 1000.f << "s]");
        return code;
#else
        return httpGetRequest.Get(url, content);
#endif
    }


    XCode HttpClientComponent::Post(const std::string &url, const std::string & data, std::string &response, int timeout)
    {
        HttpWriteStringContent content(data);
        return this->Post(url, content, response, timeout);
    }

    XCode HttpClientComponent::Post(const std::string &url, const std::unordered_map<std::string, std::string> &data,
                                    std::string &response, int timeout)
    {
        if (data.empty())
        {
            return XCode::HttpUrlParseError;
        }
        HttpJsonContent jsonContent;
        for (const auto &iter: data)
        {
            const std::string &key = iter.first;
            const std::string &val = iter.second;
            jsonContent.Add(key.c_str(), val);
        }
        return this->Post(url, jsonContent, response, timeout);
    }

    XCode HttpClientComponent::Post(const std::string & url, HttpWriteContent & content, std::string & response, int timeout)
    {
        HttpPostRequest localPostRequest(this);
#ifdef __DEBUG__
        long long t1 = TimeHelper::GetMilTimestamp();
        XCode code = localPostRequest.Post(url, content, response);
        long long t2 = TimeHelper::GetMilTimestamp();
        GKDebugInfo("post " << url << " use time [" << (t2 - t1) / 1000.f << "s]");
        return code;
#else
        return localPostRequest.Post(url, content, response);
#endif
    }

    void HttpClientComponent::Invoke(HttpRequestHandler *remoteRequest)
    {
        const HttpServiceConfig *config = remoteRequest->GetHttpConfig();
        HttpServiceMethod *httpMethod = this->GetHttpMethod(config->Service, config->Method);
        remoteRequest->SetCode(httpMethod->Invoke(remoteRequest));
    }

}