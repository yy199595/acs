//
// Created by zmhy0073 on 2022/8/11.
//

#ifndef APP_HTTPLISTENCOMPONENT_H
#define APP_HTTPLISTENCOMPONENT_H
#include"Component/Component.h"
#include"Listener/TcpListenerComponent.h"
namespace Sentry
{
    class HttpHandlerClient;
    class HttpListenComponent : public TcpListenerComponent
    {
    public:
        bool LateAwake() override;
        void ClosetHttpClient(const std::string & address);
        bool OnListen(std::shared_ptr<SocketProxy> socket) final;
        virtual void OnRequest(std::shared_ptr<HttpHandlerClient> httpClient) = 0;
    private:
        class NetThreadComponent * mNetComponent;
        std::queue<std::shared_ptr<HttpHandlerClient>> mClientPools;
        std::unordered_map<std::string, std::shared_ptr<HttpHandlerClient>> mHttpClients;
    };
}


#endif //APP_HTTPLISTENCOMPONENT_H
