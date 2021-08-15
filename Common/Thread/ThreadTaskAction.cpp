#include "ThreadTaskAction.h"
#include <Manager/ThreadTaskManager.h>

namespace Sentry
{
    ThreadTaskAction::ThreadTaskAction()
    {
        this->mStartTime = TimeHelper::GetMilTimestamp();
    }

    bool ThreadTaskAction::InitTaskAction(ThreadTaskManager *taskManager)
    {
        if (taskManager == nullptr)
        {
            return false;
        }
        this->mTaskActionId = taskManager->CreateTaskId();
        return true;
    }
}// namespace Sentry