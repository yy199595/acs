//
// Created by zmhy0073 on 2022/10/12.
//

#ifndef APP_LAUNCHCOMPONENT_H
#define APP_LAUNCHCOMPONENT_H
#include"Component/Component.h"

namespace Sentry
{
    class LaunchComponent final : public Component, public IStart
    {
    public:
        LaunchComponent() = default;
        ~LaunchComponent() = default;
    private:
        bool Awake() final;
        bool Start() final;
		bool LateAwake() final;
    };
}


#endif //APP_LAUNCHCOMPONENT_H
