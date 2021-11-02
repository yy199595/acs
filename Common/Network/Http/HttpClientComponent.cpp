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
#include <Network/Http/Response/HttpContent.h>
#include <Network/Http/Response/HttpRemoteGetRequest.h>
#include <HttpService/HttpServiceComponent.h>
namespace GameKeeper
{
    SessionBase *HttpClientComponent::CreateSocket()
    {
        return new HttpRemoteSession(this);
    }

    bool HttpClientComponent::Awake()
    {
        this->mTaskComponent = this->GetComponent<TaskPoolComponent>();
        this->mCorComponent = this->GetComponent<CoroutineComponent>();
        return true;
    }

	void HttpClientComponent::Start()
    {
        std::string json;
        const std::string url = "http://lrs-oss.whitewolvesx.com/app/default/boy.png";
        std::string data = "fid=0&key=f5c417a28abf995d7ce6312b29556fd9";
        this->Get(url, json);

        GKDebugFatal(json.size());
        this->Get("http://yjz199595.com/api/account/login?account=646585122@qq.com&passwd=11223344", json);

        GKDebugFatal(json);
    }

    HttpRemoteRequest *HttpClientComponent::CreateMethodHandler(const std::string &method, HttpRemoteSession * session)
    {
        if (method == "GET")
        {
            return new HttpRemoteGetRequest(this, session);
        }
        return nullptr;
    }

    void HttpClientComponent::HandlerHttpRequest(HttpRemoteRequest *remoteRequest)
    {
		const std::string & method = remoteRequest->GetMethodName();
		const std::string & service = remoteRequest->GetServiceName();
		auto httpService = App::Get().GetComponent<HttpServiceComponent>(service);
		if (httpService == nullptr)
		{
			remoteRequest->SetCode(HttpStatus::NOT_FOUND);
			return;
		}

		HttpServiceMethod * httpMethod = httpService->GetMethod(method);
		if (httpMethod == nullptr)
		{
			remoteRequest->SetCode(HttpStatus::NOT_FOUND);
			return;
		}

		HttpStatus code = httpMethod->Invoke(remoteRequest);
		remoteRequest->SetCode(code);
    }

    HttpServiceMethod *HttpClientComponent::GetHttpMethod(const std::string &path)
    {
		std::vector<std::string> tempArray;
		StringHelper::SplitString(path, "/", tempArray);
		if (tempArray.size() != 3)
		{
			return nullptr;
		}
		const std::string & service = tempArray[1];
        auto httpService = App::Get().GetComponent<HttpServiceComponent>(service);
        if (httpService == nullptr)
        {
            return nullptr;
        }
        return httpService->GetMethod(path);
    }


    XCode HttpClientComponent::Get(const std::string &url, std::string &json, int timeout)
    {
        HttpLolcalGetRequest httpGetRequest(this);
#ifdef __DEBUG__
        long long t1 = TimeHelper::GetMilTimestamp();
        XCode code = httpGetRequest.Get(url, json);
        long long t2 = TimeHelper::GetMilTimestamp();
        GKDebugInfo("get " << url << " use time [" << (t2 - t1) / 1000.f << "s]");
        return code;
#else
        return httpGetRequest.Get(url, json);
#endif
    }


    XCode HttpClientComponent::Post(const std::string &url, const std::string & data, std::string &response, int timeout)
    {
        HttpLocalPostRequest localPostRequest(this);
#ifdef __DEBUG__
        long long t1 = TimeHelper::GetMilTimestamp();
        XCode code = localPostRequest.Post(url, data, response);
        long long t2 = TimeHelper::GetMilTimestamp();
        GKDebugInfo("post " << url << " use time [" << (t2 - t1) / 1000.f << "s]");
		return code;
#else
        return localPostRequest.Post(url, data, response);
#endif
    }

    XCode HttpClientComponent::Post(const std::string &url, std::string &response, int timeout)
    {
        const size_t pos = url.find('?');
        if (pos == std::string::npos)
        {
            return XCode::HttpUrlParseError;
        }
        std::string heard = url.substr(0, pos);
        std::string data = url.substr(pos + 1, url.size() - pos);
        return this->Post(heard, data, response);
    }

    XCode HttpClientComponent::Post(const std::string &url, const std::unordered_map<std::string, std::string> &data,
                                    std::string &response, int timeout)
    {
        if(data.empty())
        {
            return XCode::HttpUrlParseError;
        }
        std::string parameter;
        for(auto iter = data.begin(); iter != data.end(); iter++)
        {
            if (iter != data.begin())
            {
                parameter.append("&");
            }
            const std::string &key = iter->first;
            const std::string &val = iter->second;
            parameter += (key + '=' + val);
        }
        return this->Post(url, parameter, response);
    }

}