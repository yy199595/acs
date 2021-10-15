//
// Created by zmhy0073 on 2021/10/15.
//

#ifndef SENTRY_HTTPREQUESTTASK_H
#define SENTRY_HTTPREQUESTTASK_H
#include <Thread/TaskProxy.h>
#include <XCode/XCode.h>
#include <Define/CommonTypeDef.h>
#include <Http/HttpClientSession.h>
namespace Sentry
{
    class HttpUrlHelper
    {
    public:
        static bool TryParse(const std::string & url, std::string & host, std::string & port, std::string & path);
    };
}


namespace Sentry
{
    class TaskPoolComponent;
    class CoroutineComponent;
    class HttpRequestTask : public TaskProxy
    {
    public:
        HttpRequestTask(const std::string & url, AsioContext & io, std::string & res);
        ~HttpRequestTask();
    public:
        int GetCode() { return this->mCode;}
        const std::string & GetData() { return this->mData;}

    public:
        bool Run() final; //在线程池执行的任务
        void RunFinish() final;

    private:
        void OnResponse(EHttpError err, std::istream & message);
    private:
        int mCode;
        std::string & mData;
        unsigned int mCorId;
        const std::string mHttpUrl;
        AsioContext & mAsioContext;
        HttpClientSession * mHttpClient;
        TaskPoolComponent * mTaskComponent;
        CoroutineComponent * mCorComponent;
    };
}


#endif //SENTRY_HTTPREQUESTTASK_H
