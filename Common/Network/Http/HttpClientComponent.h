
#pragma once
#include<Component/Component.h>
#include <Network/Http/HttpSessionBase.h>
namespace GameKeeper
{
    class HttpWriteContent;
    class HttpRemoteSession;
    class HttpServiceMethod;
    class HttpRemoteRequestHandler;
    class HttpClientComponent : public Component, public SocketHandler<HttpSessionBase>
    {
    public:
        HttpClientComponent() = default;

        ~HttpClientComponent() = default;

    protected:
        SessionBase *CreateSocket() override;

    public:
        bool Awake() final;

        void Start() final;

    public:
        XCode Get(const std::string &url, std::string &json, int timeout = 5);

        XCode Post(const std::string &url, const std::string &data, std::string &response, int timeout = 5);

        XCode Post(const std::string &url, const std::unordered_map<std::string, std::string> &data, std::string &response,
             int timeout = 5);

    public:
        void HandlerHttpRequest(HttpRemoteRequestHandler *remoteRequest);

        HttpRemoteRequestHandler *CreateMethodHandler(const std::string &method, HttpRemoteSession *session);

    private:
        XCode Post(const std::string &url, HttpWriteContent &content, std::string &response, int timeout);

        void Invoke(HttpServiceMethod *method, HttpRemoteRequestHandler *remoteRequest);

    private:
        class CoroutineComponent *mCorComponent;
    };
}