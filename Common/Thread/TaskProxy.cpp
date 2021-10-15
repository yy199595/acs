#include "TaskProxy.h"
#include <Util/TimeHelper.h>
#include <Scene/TaskPoolComponent.h>

namespace Sentry
{
    TaskProxy::TaskProxy()
    {
		this->mTaskId = 0;
        this->mStartTime = TimeHelper::GetMilTimestamp();
    }
}// namespace Sentry