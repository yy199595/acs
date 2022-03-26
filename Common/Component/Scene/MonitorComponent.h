//
// Created by zmhy0073 on 2021/10/29.
//

#ifndef GameKeeper_DEAMONCOMPONENT_H
#define GameKeeper_DEAMONCOMPONENT_H
#include "Component/Component.h"
namespace Sentry
{
    class MonitorComponent : public Component
    {
    public:
        MonitorComponent() = default;
        ~MonitorComponent() final = default;
    protected:
        bool Awake() final;
        bool LateAwake() final;
    private:
        void Update();
    private:
        bool mIsClose;
        std::thread * mThread;		
        ThreadPoolComponent * mTaskComponent;
    };
}
#endif //GameKeeper_DEAMONCOMPONENT_H
