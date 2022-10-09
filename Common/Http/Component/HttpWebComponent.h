//
// Created by zmhy0073 on 2022/6/21.
//

#ifndef SERVER_HTTPSERVICECOMPONENT_H
#define SERVER_HTTPSERVICECOMPONENT_H
#include"HttpListenComponent.h"
#include"Component/RpcTaskComponent.h"
namespace Sentry
{
    class HttpAsyncResponse;
    class HttpHandlerClient;
    class HttpWebComponent : public HttpListenComponent, public IStart
    {
    public:
        HttpWebComponent() = default;
        ~HttpWebComponent() = default;
    public:
        void OnRequest(std::shared_ptr<HttpHandlerClient> httpClient);
    private:
        bool Start() final;
        bool LateAwake() final;
        const HttpMethodConfig * GetConfig(const std::string & path);
    private:
        TaskComponent * mTaskComponent;
        std::unordered_map<std::string, const HttpMethodConfig *> mHttpConfigs;
    };
}


#endif //SERVER_HTTPSERVICECOMPONENT_H
