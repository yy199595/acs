#include "TaskProxy.h"
#include <Util/TimeHelper.h>
#include <Scene/TaskPoolComponent.h>
#include <Core/App.h>
namespace Sentry
{
    TaskProxy::TaskProxy()
    {
		this->mTaskId = 0;
        this->mStartTime = TimeHelper::GetMilTimestamp();
    }

    CoroutineAsyncTask::CoroutineAsyncTask()
    {
        this->mCorId = 0;
        this->mCorComponent = App::Get().GetCoroutineComponent();
    }

    bool CoroutineAsyncTask::AwaitInvoke()
    {
        if(this->mCorComponent->IsInMainCoroutine())
        {
            return false;
        }
        this->mCorComponent->YieldReturn(this->mCorId);
        return true;
    }

    void CoroutineAsyncTask::RunFinish()
    {
        this->mCorComponent->Resume(this->mCorId);
    }
}// namespace Sentry