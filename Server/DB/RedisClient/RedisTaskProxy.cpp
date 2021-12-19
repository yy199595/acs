#include "RedisTaskProxy.h"
#include <Coroutine/TaskComponent.h>
#include "Component/RedisComponent.h"
#include <Core/App.h>
namespace GameKeeper
{
    RedisTaskProxy::RedisTaskProxy(const std::string &cmd) : RedisTaskBase(cmd)
    {
		TaskComponent * corComponent = App::Get().GetTaskComponent();
		LOG_CHECK_RET(this->mCoroutineId = corComponent->GetCurrentCorId());
    }

    void RedisTaskProxy::RunFinish()
    {
        App::Get().GetTaskComponent()->Resume(this->mCoroutineId);
    }
}// namespace GameKeeper