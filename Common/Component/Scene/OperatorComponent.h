//
// Created by zmhy0073 on 2021/11/19.
//

#ifndef GAMEKEEPER_OPERATORCOMPONENT_H
#define GAMEKEEPER_OPERATORCOMPONENT_H
#include"Component.h"
namespace GameKeeper
{
    class OperatorComponent : public Component
    {
    public:
        OperatorComponent() = default;
        ~OperatorComponent() final = default;

    protected:
        bool Awake() override;
        void StartRefreshDay();
    public:
        void StartHotfix();
        bool StartLoadConnfig();
        int GetPriority() final { return 1000000;}

    private:
        unsigned int mRefreshTimerId;
        class TimerComponent * mTimerComponent;
    };
}
#endif //GAMEKEEPER_OPERATORCOMPONENT_H
