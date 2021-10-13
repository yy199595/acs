
#pragma once

#include <Component/Component.h>
#include <Thread/TaskProxy.h>

namespace Sentry
{
    class TaskComponent;
    class CoroutineComponent;
    class HttpRequestTask : public TaskProxy
    {
    public:
        HttpRequestTask(const std::string url, AsioContext & io);
    public:
        XCode GetCode() { return this->mCode;}
        const std::string & GetData() { return this->mData;}

    public:
         void Run() final; //在线程池执行的任务
         void RunFinish() final;
    private:
        XCode mCode;
        std::string mData;
        unsigned int mCorId;
        const std::string mHttpUrl;
        AsioContext & mAsioContext;
        CoroutineComponent * mCorComponent;
    };
}

namespace Sentry
{
    class HttpClientComponent : public Component, public IHttpContextUpdate
    {
    public:
        HttpClientComponent()
        {}

        ~HttpClientComponent()
        {}

    public:
        bool Awake() final;

        void OnHttpContextUpdate(AsioContext &ctx) final;

    public:
        XCode Get(const std::string &url, std::string &json, int timeout = 5);

    private:
        TaskComponent *mTaskComponent;
        CoroutineComponent *mCorComponent;
    };
}