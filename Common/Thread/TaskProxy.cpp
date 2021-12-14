#include "TaskProxy.h"
#include <Util/TimeHelper.h>
#include <Scene/ThreadPoolComponent.h>
#include <Core/App.h>
namespace GameKeeper
{
    TaskProxy::TaskProxy()
    {
		this->mTaskId = 0;
        this->mStartTime = TimeHelper::GetMilTimestamp();
    }

    CoroutineAsyncTask::CoroutineAsyncTask()
    {
        this->mCorId = 0;
        this->mCorComponent = App::Get().GetTaskComponent();
    }

    bool CoroutineAsyncTask::AwaitInvoke()
    {
        if(this->mCorComponent->IsInMainCoroutine())
        {
            return false;
        }
        this->mCorComponent->Await(this->mCorId);
        return true;
    }

    void CoroutineAsyncTask::RunFinish()
    {
        this->mCorComponent->Resume(this->mCorId);
    }
}// namespace GameKeeper