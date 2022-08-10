//
// Created by zmhy0073 on 2022/6/21.
//

#ifndef SERVER_HTTPSERVICECOMPONENT_H
#define SERVER_HTTPSERVICECOMPONENT_H
#include"Component/Component.h"
#include"Component/Rpc/RpcTaskComponent.h"
namespace Sentry
{
    class HttpAsyncResponse;
    class HttpHandlerClient;
    class HttpServiceComponent : public Component, public ISocketListen
    {
    public:
        HttpServiceComponent() = default;
        ~HttpServiceComponent() = default;
    public:
        void ClosetHttpClient(const std::string & address);
        bool OnListen(std::shared_ptr<SocketProxy> socket) final;
        void OnRequest(std::shared_ptr<HttpHandlerClient> httpClient);
    private:
        bool LateAwake() final;
        const HttpInterfaceConfig * GetConfig(const std::string & path);
    private:
        TaskComponent * mTaskComponent;
    	class NetThreadComponent * mNetComponent;
        std::queue<std::shared_ptr<HttpHandlerClient>> mClientPools;
        std::unordered_map<std::string, const HttpInterfaceConfig *> mHttpConfigs;
        std::unordered_map<std::string, std::shared_ptr<HttpHandlerClient>> mHttpClients;
    };
}


#endif //SERVER_HTTPSERVICECOMPONENT_H
