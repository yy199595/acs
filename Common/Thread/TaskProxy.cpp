#include"TaskProxy.h"
#include"App/App.h"
#include"Util/TimeHelper.h"
#include"Component/Scene/ThreadPoolComponent.h"
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
        this->mCorComponent = App::Get()->GetTaskComponent();
    }

    bool CoroutineAsyncTask::AwaitInvoke()
    {
        this->mCorComponent->YieldCoroutine(this->mCorId);
        return true;
    }

    void CoroutineAsyncTask::RunFinish()
    {
        this->mCorComponent->Resume(this->mCorId);
    }
}// namespace Sentry