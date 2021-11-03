//
// Created by 64658 on 2021/8/5.
//
#include <Core/App.h>
#include <Thread/TaskThread.h>
#include "HttpClientComponent.h"
#include <Util/StringHelper.h>
#include <Coroutine/CoroutineComponent.h>
#include <Network/Http/HttpRemoteSession.h>
#include "Component/Scene/TaskPoolComponent.h"
#include <Network/Http/Request/HttpLolcalGetRequest.h>
#include <Network/Http/Request/HttpLocalPostRequest.h>
#include <Method/HttpServiceMethod.h>
#include <Network/Http/Content/HttpWriteContent.h>
#include <Network/Http/Response/HttpRemoteGetRequestHandler.h>
#include <HttpService/HttpServiceComponent.h>

namespace GameKeeper
{
    SessionBase *HttpClientComponent::CreateSocket()
    {
        return new HttpRemoteSession(this);
    }

    bool HttpClientComponent::Awake()
    {
        this->mCorComponent = this->GetComponent<CoroutineComponent>();
        return true;
    }

	void HttpClientComponent::Start()
    {
        std::string json;
        const std::string url = "http://langrensha01.oss-cn-shenzhen.aliyuncs.com/res/area/city-config.json";
        std::string data = "fid=0&key=f5c417a28abf995d7ce6312b29556fd9";
        //this->Get(url, json);

        GKDebugFatal(json.size());
        const std::string path = App::Get().GetDownloadPath() + "1122.mp4";
        //this->Download("http://127.0.0.1:80/App/HttpDownloadService/Download/1-2.mp4", path);

        //GKDebugFatal(json);
    }

    HttpRemoteRequestHandler *HttpClientComponent::CreateMethodHandler(const std::string &method, HttpRemoteSession * session)
    {
        if (method == "GET")
        {
            return new HttpRemoteGetRequestHandler(this, session);
        }
        return nullptr;
    }

    void HttpClientComponent::HandlerHttpRequest(HttpRemoteRequestHandler *remoteRequest)
    {
        if(remoteRequest->GetErrorCode() != XCode::Successful)
        {
            delete remoteRequest;
            return;
        }
        const std::string &method = remoteRequest->GetMethodName();
        const std::string &service = remoteRequest->GetServiceName();
		GKDebugWarning("http call " << service << "." << method);
        auto httpService = App::Get().GetComponent<HttpServiceComponent>(service);
        if (httpService == nullptr)
        {
            remoteRequest->SetCode(HttpStatus::NOT_FOUND);
            return;
        }

        HttpServiceMethod *httpMethod = httpService->GetMethod(method);
        if (httpMethod == nullptr)
        {
            remoteRequest->SetCode(HttpStatus::NOT_FOUND);
            return;
        }
        remoteRequest->SetCode(httpMethod->Invoke(remoteRequest));
        //this->mCorComponent->StartCoroutine(&HttpClientComponent::Invoke, this, httpMethod, remoteRequest);
    }

    XCode HttpClientComponent::Download(const std::string &url, const std::string &path, int timeout)
    {
        HttpReadFileContent content(path);
        if(!content.OpenFile())
        {
            return XCode::Failure;
        }
        HttpLolcalGetRequest httpGetRequest(this);
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
        HttpLolcalGetRequest httpGetRequest(this);
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
            jsonContent.AddParameter(key.c_str(), val);
        }
        return this->Post(url, jsonContent, response, timeout);
    }

    XCode HttpClientComponent::Post(const std::string & url, HttpWriteContent & content, std::string & response, int timeout)
    {
        HttpLocalPostRequest localPostRequest(this);
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

    void HttpClientComponent::Invoke(HttpServiceMethod *method, HttpRemoteRequestHandler *remoteRequest)
    {
        HttpStatus code = method->Invoke(remoteRequest);
        this->mCorComponent->YieldReturn();
        remoteRequest->SetCode(code);

    }

}