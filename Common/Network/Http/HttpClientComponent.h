
#pragma once
#include<Component/Component.h>
#include <Network/Http/HttpSessionBase.h>
namespace Sentry
{
    class HttpClientComponent : public Component, public SocketHandler<HttpSessionBase>
    {
    public:
        HttpClientComponent()
        {}

        ~HttpClientComponent()
        {}

    protected:
         SessionBase * CreateSocket() override;
    public:
        bool Awake() final;
        void Start() final;
    public:
        XCode Get(const std::string &url, std::string &json, int timeout = 5);
        XCode DownLoad(const std::string & url, const std::string & path);
    private:
        class TaskPoolComponent *mTaskComponent;
        class CoroutineComponent *mCorComponent;
    };
}