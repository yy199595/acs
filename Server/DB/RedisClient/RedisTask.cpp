#include "RedisTask.h"
#include <Coroutine/CoroutineComponent.h>
#include <Scene/RedisComponent.h>
#include <Core/App.h>
namespace Sentry
{
    RedisTask::RedisTask(const std::string &cmd) : RedisTaskBase(cmd)
    {
		CoroutineComponent * corComponent = App::Get().GetCoroutineComponent();
		SayNoAssertRet_F(this->mCoreoutineId = corComponent->GetCurrentCorId() != 0);
    }

    void RedisTask::RunFinish()
    {
        if(this->GetErrorCode() != XCode::Successful)
        {
            SayNoDebugError(this->GetErrorStr());
        }
		CoroutineComponent * corComponent = App::Get().GetCoroutineComponent();
		if (this->mCoreoutineId != 0)
		{
			corComponent->Resume(this->mCoreoutineId);
		}		
    }
}// namespace Sentry