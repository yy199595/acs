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
        long long t1 = TimeHelper::GetMilTimestamp();
        const std::string url = "http://lrs-oss.whitewolvesx.com/app/default/boy.png";
        std::string data = "fid=0&key=f5c417a28abf995d7ce6312b29556fd9";
        this->Get(url, json);

        SayNoDebugFatal(json.size());
        SayNoDebugError("time = " << (TimeHelper::GetMilTimestamp() - t1) / 1000.0f << "s");
    }

    HttpHandlerBase *HttpClientComponent::CreateMethodHandler(const std::string &method)
    {
        return nullptr;
    }

    XCode HttpClientComponent::Get(const std::string &url, std::string &json, int timeout)
    {
        HttpLolcalGetRequest httpGetRequest(this);
        return httpGetRequest.Get(url, json);
    }


    XCode HttpClientComponent::Post(const std::string &url, const std::string & data, std::string &response, int timeout)
    {
        HttpLocalPostRequest localPostRequest(this);
        return localPostRequest.Post(url, data, response);
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
        if(data.size() == 0)
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