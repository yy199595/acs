#include "RedisTaskProxy.h"
#include <Coroutine/TaskComponent.h>
#include "Component/RedisComponent.h"
#include <Core/App.h>
namespace GameKeeper
{
    RedisTaskProxy::RedisTaskProxy(const std::string &cmd) : RedisTaskBase(cmd)
    {

    }

    void RedisTaskProxy::RunFinish()
    {
        this->mTask.SetResult(std::move(this->mResponse));
    }
}// namespace GameKeeper