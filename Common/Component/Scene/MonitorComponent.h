//
// Created by zmhy0073 on 2021/10/29.
//

#ifndef GameKeeper_DEAMONCOMPONENT_H
#define GameKeeper_DEAMONCOMPONENT_H
#include "Component.h"
namespace GameKeeper
{
    class MonitorComponent : public Component
    {
    public:
        MonitorComponent() = default;
        ~MonitorComponent() final = default;

    public:
        long long GetNowTime() const { return this->mStartTime;}
    protected:
        bool Awake() override;
    private:
        void Update();
    private:
        bool mIsClose;
        long long mStartTime;
        std::thread * mThread;
        TaskPoolComponent * mTaskComponent;
    };
}
#endif //GameKeeper_DEAMONCOMPONENT_H
