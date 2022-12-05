//
// Created by zmhy0073 on 2022/6/21.
//

#ifndef SERVER_HTTPSERVICECOMPONENT_H
#define SERVER_HTTPSERVICECOMPONENT_H
#include"HttpListenComponent.h"
#include"Component/RpcTaskComponent.h"
namespace Http
{
    class Request;
    class Response;
}
namespace Sentry
{
    class HttpMethodConfig;
    class HttpAsyncResponse;
    class HttpHandlerClient;
    class HttpWebComponent : public HttpListenComponent
    {
    public:
        HttpWebComponent() = default;
        ~HttpWebComponent() = default;
    public:
        void OnRequest(std::shared_ptr<HttpHandlerClient> httpClient);
        unsigned int GetWaitCount() const { return this->mWaitCount; }
    private:
        bool LateAwake() final;
        bool OnDelClient(const std::string& address);
        void Invoke(const std::string& address, const HttpMethodConfig* config, std::shared_ptr<Http::Request> request);
    private:
        unsigned int mWaitCount;
        TaskComponent * mTaskComponent;
        std::unordered_map<std::string, unsigned int> mTasks;
    };
}


#endif //SERVER_HTTPSERVICECOMPONENT_H
