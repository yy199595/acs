//
// Created by zmhy0073 on 2022/8/11.
//

#ifndef APP_HTTPLISTENCOMPONENT_H
#define APP_HTTPLISTENCOMPONENT_H
#include"Component/TcpListenerComponent.h"
namespace Http
{
	class Request;
	class Response;
}
namespace Sentry
{
    class HttpHandlerClient;
    class HttpListenComponent : public TcpListenerComponent
    {
    public:
        bool LateAwake() override;
        void ClosetHttpClient(const std::string & address);
        bool OnListen(std::shared_ptr<SocketProxy> socket) final;
        virtual bool OnDelClient(const std::string& address) = 0;
        virtual void OnRequest(const std::string & address, std::shared_ptr<Http::Request> request) = 0;
    protected:
        HttpHandlerClient* GetClient(const std::string& address);
    private:
        class NetThreadComponent * mNetComponent;
        std::queue<std::shared_ptr<HttpHandlerClient>> mClientPools;
        std::unordered_map<std::string, std::shared_ptr<HttpHandlerClient>> mHttpClients;
    };
}


#endif //APP_HTTPLISTENCOMPONENT_H
