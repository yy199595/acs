//
// Created by 64658 on 2021/8/5.
//
#include <Core/App.h>
#include <Thread/TaskThread.h>
#include "HttpClientComponent.h"
#include "Component/Scene/TaskPoolComponent.h"
#include<Util/StringHelper.h>
#include <Coroutine/CoroutineComponent.h>
#include <Http/HttpRequestTask.h>
#include<Util/JsonHelper.h>
namespace Sentry
{
    bool HttpClientComponent::Awake()
    {
        this->mTaskComponent = this->GetComponent<TaskPoolComponent>();
        this->mCorComponent = this->GetComponent<CoroutineComponent>();
        return true;
    }

//    void HttpClientComponent::OnCloseSession(HttpClientSession * session)
//    {
//
//    }
//
//    bool HttpClientComponent::OnListenNewSession(HttpClientSession * session)
//    {
//        return false;
//    }
//
//    bool HttpClientComponent::OnReceiveMessage(HttpClientSession * session, SharedMessage message)
//    {
//        return true;
//    }
//
//    void HttpClientComponent::OnSessionError(HttpClientSession * session, const asio::error_code & err)
//    {
//
//    }
//    void HttpClientComponent::OnConnectRemoteAfter(HttpClientSession * session, const asio::error_code & err)
//    {
//
//    }

    void HttpClientComponent::Start()
    {
        std::string json ;
		long long t1 = TimeHelper::GetMilTimestamp();
		
		
		
       this->Get("http://apis.juhe.cn/xzqh/query?fid=0&key=f5c417a28abf995d7ce6312b29556fd9", json);
		rapidjson::Document doc;

		if (!doc.Parse(json.c_str(), json.size()).HasParseError())
		{
			rapidjson::Value & jsonValue = doc["result"];
			for (unsigned int index = 0; index < jsonValue.Size(); index++)
			{
				char buffer[256] = { 0 };
				std::string id = jsonValue[index]["id"].GetString();
				size_t size = sprintf(buffer, "http://apis.juhe.cn/xzqh/query?fid=%s&key=f5c417a28abf995d7ce6312b29556fd9", id.c_str());
				this->Get(std::string(buffer, size), json);
				SayNoDebugFatal(jsonValue[index]["name"].GetString() << " = "  << json);
			}
		}
		SayNoDebugError("time = " << (TimeHelper::GetMilTimestamp() - t1) / 1000.0f);
    }

    XCode HttpClientComponent::Get(const std::string &url, std::string &json, int timeout)
    {
		json.clear();
        if (this->mCorComponent->IsInMainCoroutine())
        {
            return XCode::NoCoroutineContext;
        }  
        HttpRequestTask httpTask(url, App::Get().GetHttpContext(), json);
		if (!this->mTaskComponent->StartTask("Http", &httpTask))
		{
			return XCode::HttpTaskStarFail;
		}		
        this->mCorComponent->YieldReturn();  
		return httpTask.GetCode();
    }
}