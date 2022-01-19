
#pragma once
#include"Component/Component.h"
namespace GameKeeper
{
    class HttpRespSession;
    class HttpServiceMethod;
    class HttpRequestHandler;	
	class HttpReqSession;
    class HttpHandlerClient;
    class HttpClientComponent : public Component, public ISocketListen, public ILoadData
    {
    public:
        HttpClientComponent() = default;
        ~HttpClientComponent() final = default;
    public:
        bool Awake() final;
        bool LateAwake() final;
        void OnLoadData() final;
    public:
        XCode Get(const std::string &url, int timeout = 5);

        XCode Post(const std::string &url, const std::string &data, int timeout = 5);

    public:

        void OnListen(std::shared_ptr<SocketProxy> socket) final;

		void OnRequest(HttpRespSession * remoteSession);

        HttpServiceMethod * GetHttpMethod(const std::string & service, const std::string & method);

    private:
        void Invoke(HttpRespSession *remoteRequest);
        void HandlerHttpData(std::shared_ptr<HttpHandlerClient> httpClient);
    private:
        class TaskComponent *mCorComponent;
        class ThreadPoolComponent * mThreadComponent;
    };
}