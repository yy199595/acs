//
// Created by zmhy0073 on 2022/1/28.
//

#ifndef SENTRY_WAITTASKSOURCE_H
#define SENTRY_WAITTASKSOURCE_H
#include"TaskSource.h"
namespace Sentry
{
    class WaitTaskSource
    {
    public:
        WaitTaskSource();
        ~WaitTaskSource();
    public:
        void WaitSecond(float s);
        void WaitFrame(int count = 1);
    private:
        unsigned int mTimerId;
        TaskSource<void> mTaskSource;
        TimerComponent * mTimerComponent;
    };
}
#endif //SENTRY_WAITTASKSOURCE_H
