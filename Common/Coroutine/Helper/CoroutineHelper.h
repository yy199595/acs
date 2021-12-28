//
// Created by zmhy0073 on 2021/12/28.
//

#ifndef GAMEKEEPER_COROUTINEHELPER_H
#define GAMEKEEPER_COROUTINEHELPER_H

#endif //GAMEKEEPER_COROUTINEHELPER_H
#include"Coroutine/Coroutine.h"
using namespace GameKeeper;

namespace GameKeeper
{
    namespace CoroutineHelper
    {
        extern void WhenAny(Coroutine *coroutine);
        extern void WhenAll(std::vector<Coroutine *> &tasks);
    }
}