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
namespace Sentry
{
    bool HttpClientComponent::Awake()
    {
        this->mTaskComponent = this->GetComponent<TaskPoolComponent>();
        this->mCorComponent = this->GetComponent<CoroutineComponent>();
        return true;
    }

    void HttpClientComponent::Start()
    {
        std::string json ;
        this->Get("http://langrens.oss-cn-shenzhen.aliyuncs.com/res/area/city-config.json", json);
    }

    void HttpClientComponent::OnHttpContextUpdate(AsioContext & ctx)
    {

    }

    XCode HttpClientComponent::Get(const std::string &url, std::string &json, int timeout)
    {
      if (this->mCorComponent->IsInMainCoroutine())
      {
        return XCode::NoCoroutineContext;
      }
      HttpRequestTask httpTask(url, App::Get().GetHttpContext());
      this->mTaskComponent->StartTask("Http", &httpTask);
      this->mCorComponent->YieldReturn();


      return XCode::Successful;
    }
}