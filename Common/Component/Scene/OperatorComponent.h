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
        bool Awake() final;
        bool LateAwake() final;
        void StartRefreshDay(const std::string & component);
    public:
        void StartHotfix();
        bool StartLoadConfig();
        int GetPriority() final { return 1000000;}

    private:
        void AddRefreshTimer(Component * component);

    private:
        unsigned int mRefreshTimerId;
        class TimerComponent * mTimerComponent;
    };
}
#endif //GAMEKEEPER_OPERATORCOMPONENT_H
