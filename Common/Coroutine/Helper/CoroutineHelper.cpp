//
// Created by zmhy0073 on 2021/12/28.
//
#include"CoroutineHelper.h"
#include"Coroutine/CoroutineDef.h"
#include"Core/App.h"
namespace GameKeeper
{
    void CoroutineHelper::WhenAny(Coroutine *coroutine)
    {
        coroutine->mGroup = new CoroutineGroup(1);
        App::Get().GetTaskComponent()->Await();
    }

    void CoroutineHelper::WhenAll(std::vector<Coroutine *> &coroutines)
    {
        auto group = new CoroutineGroup(coroutines.size());
        for (Coroutine *coroutine: coroutines)
        {
            coroutine->mGroup = group;
        }
        App::Get().GetTaskComponent()->Await();
    }
}