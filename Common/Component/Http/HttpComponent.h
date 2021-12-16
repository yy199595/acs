
#pragma once
#include<Component/Component.h>
namespace GameKeeper
{
	class HttpSessionBase;
    class HttpReadContent;
    class HttpWriteContent;
    class HttpRespSession;
    class HttpServiceMethod;
    class HttpRequestHandler;	
	class HttpReqSession;

    class HttpComponent : public Component, public ISocketListen
    {
    public:
        HttpComponent() = default;
        ~HttpComponent() final = default;
    public:
        bool Awake() final;
        bool LateAwake() final;
    public:
        XCode Get(const std::string &url, int timeout = 5);

        XCode Post(const std::string &url, const std::string &data, int timeout = 5);

    public:

        void OnListen(SocketProxy *socket) final;

		void OnRequest(HttpRespSession * remoteSession);

        HttpServiceMethod * GetHttpMethod(const std::string & service, const std::string & method);

    private:
        void Invoke(HttpRespSession *remoteRequest);
    private:
        class TaskComponent *mCorComponent;
        class ThreadPoolComponent * mThreadComponent;
    };
}