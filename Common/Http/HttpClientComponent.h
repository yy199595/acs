
#pragma once

#include <Component/Component.h>

namespace Sentry
{
    class HttpClientComponent : public Component
    {
    public:
        HttpClientComponent()
        {}

        ~HttpClientComponent()
        {}

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