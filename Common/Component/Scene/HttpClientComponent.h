
#pragma once

#include <Component/Component.h>
namespace Sentry
{
    class HttpClientComponent : public Component, public ISystemUpdate
    {
    public:
        HttpClientComponent() {}

        ~HttpClientComponent() {}

    public:
        bool Awake() final;
        void OnSystemUpdate() final;

    public:
        XCode Get(const std::string & url, std::string & json, int timeout = 5);

    private:
        AsioContext mHttpAsioContext;

        class CoroutineComponent *mCorComponent;
    };
}