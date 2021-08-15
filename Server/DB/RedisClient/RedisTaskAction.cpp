﻿#include "RedisTaskAction.h"
#include <Coroutine/CoroutineManager.h>
#include <Manager/RedisManager.h>

namespace Sentry
{
    RedisTaskAction::RedisTaskAction(RedisManager *mgr, const std::string &cmd)
        : RedisTaskBase(mgr, cmd)
    {
        Applocation *app = Applocation::Get();
        SayNoAssertRet_F(this->mCorManager = app->GetManager<CoroutineManager>());
        this->mCoreoutineId = this->mCorManager->GetCurrentCorId();
    }

    void RedisTaskAction::OnTaskFinish()
    {
        SayNoAssertRet_F(this->mCorManager);
        this->mCorManager->Resume(this->mCoreoutineId);
    }
}// namespace Sentry