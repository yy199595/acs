
#pragma once

#include <Component/Component.h>
namespace Sentry
{
    class SceneHttpComponent : public Component, public ISystemUpdate
    {
    public:
        SceneHttpComponent() {}

        ~SceneHttpComponent() {}

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