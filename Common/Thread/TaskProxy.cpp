#include "TaskProxy.h"
#include <Util/TimeHelper.h>
#include <Scene/ThreadPoolComponent.h>
#include "Object/App.h"
namespace Sentry
{
    TaskProxy::TaskProxy()
    {
		this->mTaskId = 0;
        this->mStartTime = Helper::Time::GetMilTimestamp();
    }

    CoroutineAsyncTask::CoroutineAsyncTask()
    {
        this->mCorId = 0;
        this->mCorComponent = App::Get().GetTaskComponent();
    }

    bool CoroutineAsyncTask::AwaitInvoke()
    {
        this->mCorComponent->Yield(this->mCorId);
        return true;
    }

    void CoroutineAsyncTask::RunFinish()
    {
        this->mCorComponent->Resume(this->mCorId);
    }
}// namespace Sentry