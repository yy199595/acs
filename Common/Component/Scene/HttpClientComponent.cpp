//
// Created by 64658 on 2021/8/5.
//
#include <Core/App.h>
#include <Thread/TaskThread.h>
#include "HttpClientComponent.h"
#include "TaskComponent.h"
#include <Coroutine/CoroutineComponent.h>

namespace Sentry
{
    HttpRequestTask::HttpRequestTask(const std::string url, AsioContext & io)
        :mHttpUrl(url), mAsioContext(io)
    {
        this->mCorComponent = App::Get().GetCoroutineComponent();
        this->mCorId = this->mCorComponent->GetCurrentCorId();
    }

    void HttpRequestTask::Run()
    {
        tcp::resolver resolver(this->mAsioContext);
        tcp::resolver::query query(this->mHttpUrl);
        tcp::resolver::iterator endpoint = resolver.resolve(query);

        tcp::socket socket1(this->mAsioContext);

        asio::connect(socket1, endpoint);

        asio::streambuf  request;
        std::ostream requestStream(&request);
    }

    void HttpRequestTask::RunFinish()
    {

    }
}

namespace Sentry
{
    bool HttpClientComponent::Awake()
    {
        this->mTaskComponent = this->GetComponent<TaskComponent>();
        this->mCorComponent = this->GetComponent<CoroutineComponent>();
        return true;
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


      return XCode::Successful;
    }
}