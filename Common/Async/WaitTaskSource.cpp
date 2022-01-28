//
// Created by zmhy0073 on 2022/1/28.
//

#include"WaitTaskSource.h"

namespace Sentry
{
    void WaitTaskSource::WaitSecond(float s)
    {
        long long ms = s * 1000;
        this->mTimerComponent = App::Get().GetTimerComponent();
        this->mTimerComponent->AsyncWait(ms, &TaskSource<void>::SetResult, &mTaskSource);
        this->mTaskSource.Await();
    }

    void WaitTaskSource::WaitFrame(int count)
    {

    }
}