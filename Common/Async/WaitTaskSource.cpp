//
// Created by zmhy0073 on 2022/1/28.
//

#include"WaitTaskSource.h"

namespace Sentry
{
    WaitTaskSource::WaitTaskSource()
    {
        this->mTimerId = 0;
        this->mTimerComponent = App::Get()->GetTimerComponent();
    }

    WaitTaskSource::~WaitTaskSource()
    {
        if(this->mTimerId != 0)
        {
			this->mTimerComponent->CancelTimer(this->mTimerId);
        }
    }
    void WaitTaskSource::WaitSecond(float s)
    {
        long long ms = s * 1000;
        this->mTimerId = this->mTimerComponent->DelayCall(
				ms, &TaskSource<void>::SetResult, &mTaskSource);
        this->mTaskSource.Await();
        this->mTimerId = 0;
    }

    void WaitTaskSource::WaitFrame(int count)
    {

    }
}