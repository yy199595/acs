//
// Created by zmhy0073 on 2022/6/21.
//

#ifndef SERVER_HTTPHANDLERCOMPONENT_H
#define SERVER_HTTPHANDLERCOMPONENT_H
#include"Component/Component.h"
#include"Component/Rpc/RpcTaskComponent.h"
namespace Sentry
{
    class HttpAsyncResponse;
    class HttpHandlerClient;
    class HttpHandlerComponent : public Component, public ISocketListen
    {
    public:
        HttpHandlerComponent() = default;
        ~HttpHandlerComponent() = default;
    public:
        void ClosetHttpClient(const std::string & address);
        void OnListen(std::shared_ptr<SocketProxy> socket) final;
        void OnRequest(std::shared_ptr<HttpHandlerClient> httpClient);
    private:
        bool LateAwake() final;
        const HttpInterfaceConfig * GetConfig(const std::string & path);
    private:
#ifndef ONLY_MAIN_THREAD
        class NetThreadComponent* mThreadComponent;
#endif
        TaskComponent * mTaskComponent;
        std::unordered_map<std::string, const HttpInterfaceConfig *> mHttpConfigs;
        std::unordered_map<std::string, std::shared_ptr<HttpHandlerClient>> mHttpClients;
    };
}


#endif //SERVER_HTTPHANDLERCOMPONENT_H
