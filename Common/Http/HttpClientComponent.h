
#pragma once

#include <Component/Component.h>
#include <Http/HttpClientSession.h>
namespace Sentry
{
    class HttpClientComponent : public Component//, public ScoketHandler<HttpClientSession>
    {
    public:
        HttpClientComponent()
        {}

        ~HttpClientComponent()
        {}

    protected:
//        SessionBase * CreateSocket() override { return nullptr; }
//         void OnCloseSession(HttpClientSession * session) override;
//         bool OnListenNewSession(HttpClientSession * session) override;
//         bool OnReceiveMessage(HttpClientSession * session, SharedMessage message) override;
//         void OnSessionError(HttpClientSession * session, const asio::error_code & err) override;
//         void OnConnectRemoteAfter(HttpClientSession * session, const asio::error_code & err) override;
    public:
        bool Awake() final;
        void Start() final;
    public:
        XCode Get(const std::string &url, std::string &json, int timeout = 5);

    private:
        class TaskPoolComponent *mTaskComponent;
        class CoroutineComponent *mCorComponent;
    };
}