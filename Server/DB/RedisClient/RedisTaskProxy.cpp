#include "RedisTaskProxy.h"
#include <Coroutine/CoroutineComponent.h>
#include <Scene/RedisComponent.h>
#include <Core/App.h>
namespace GameKeeper
{
    RedisTaskProxy::RedisTaskProxy(const std::string &cmd) : RedisTaskBase(cmd)
    {
		CoroutineComponent * corComponent = App::Get().GetCorComponent();
		LOG_CHECK_RET(this->mCoroutineId = corComponent->GetCurrentCorId());
    }

    void RedisTaskProxy::RunFinish()
    {
		App::Get().GetCorComponent()->Resume(this->mCoroutineId);		
    }
}// namespace GameKeeper