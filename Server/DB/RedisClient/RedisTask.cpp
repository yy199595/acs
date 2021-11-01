#include "RedisTask.h"
#include <Coroutine/CoroutineComponent.h>
#include <Scene/RedisComponent.h>
#include <Core/App.h>
namespace GameKeeper
{
    RedisTask::RedisTask(const std::string &cmd) : RedisTaskBase(cmd)
    {
		CoroutineComponent * corComponent = App::Get().GetCorComponent();
		GKAssertRet_F(this->mCoreoutineId = corComponent->GetCurrentCorId());
    }

    void RedisTask::RunFinish()
    {
        if(this->GetErrorCode() != XCode::Successful)
        {
            GKDebugError(this->GetErrorStr());
        }
		CoroutineComponent * corComponent = App::Get().GetCorComponent();
		if (this->mCoreoutineId != 0)
		{
			corComponent->Resume(this->mCoreoutineId);
		}
        this->DebugInvokeInfo();
    }
}// namespace GameKeeper