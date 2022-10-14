//
// Created by zmhy0073 on 2022/10/9.
//

#ifndef APP_SERVICELAUNCHCOMPONENT_H
#define APP_SERVICELAUNCHCOMPONENT_H
#include"Component/Component.h"
namespace Sentry
{
    class ServiceLaunchComponent final : public Component, public IStart
    {
    public:
        ServiceLaunchComponent() = default;
        ~ServiceLaunchComponent() = default;
    private:
        bool Awake() final;
        bool Start() final;
    };
}

#endif //APP_SERVICELAUNCHCOMPONENT_H
