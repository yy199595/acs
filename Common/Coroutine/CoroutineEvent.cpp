#include"CoroutineEvent.h"
#include"CoroutineComponent.h"
#include"Coroutine.h"
#include<Core/App.h>
namespace GameKeeper
{
	CoroutinePool::~CoroutinePool()
	{
        auto iter = this->mCorMap.begin();
        for(;iter != this->mCorMap.end(); iter++)
        {
            delete iter->second;
        }
        this->mCorMap.clear();
	}
	Coroutine * CoroutinePool::Pop()
    {
        Coroutine *coroutine = nullptr;
        if (!this->mCorPool.empty())
        {
            coroutine = this->mCorPool.front();
            this->mCorPool.pop();
        }
        else
        {
            coroutine = new Coroutine();
        }
        coroutine->mCoroutineId = this->mNumPool.Pop();
#ifdef __COROUTINE_ASM__
        coroutine->sid = coroutine->mCoroutineId & (SHARED_STACK_NUM - 1);
#endif
        this->mCorMap.emplace(coroutine->mCoroutineId, coroutine);
        return coroutine;
    }

    size_t CoroutinePool::GetMemorySize()
    {
        size_t size = 0;
        auto iter = this->mCorMap.begin();
        for(; iter != this->mCorMap.end(); iter++)
        {
            size += iter->second->mStack.size();
        }
        return size;
    }

	void CoroutinePool::Push(Coroutine * coroutine)
	{
        unsigned int id = coroutine->mCoroutineId;
        auto iter = this->mCorMap.find(id);
        if(iter != this->mCorMap.end())
        {
            this->mNumPool.Push(id);
            this->mCorMap.erase(iter);
        }
        if(this->mCorPool.size() >=COR_POOL_COUNT)
        {
            delete coroutine;
            return;
        }
        coroutine->sid = 0;
        coroutine->mGroupId = 0;
        coroutine->mStackSize = 0;
        coroutine->mStack.clear();
        coroutine->mCoroutineId = 0;
        coroutine->mFunction = nullptr;
        coroutine->mCorContext = nullptr;
        coroutine->mState = CorState::Ready;
        this->mCorPool.push(coroutine);
	}
	Coroutine * CoroutinePool::Get(unsigned int id)
	{
        auto iter = this->mCorMap.find(id);
        return iter != this->mCorMap.end() ? iter->second : nullptr;
	}
}

namespace GameKeeper
{
	CoroutineGroup::CoroutineGroup(CoroutineComponent * cor)
    {
        this->mCount = 0;
        this->mCorComponent = cor;
        this->mCoroutineId = cor->GetCurrentCorId();
    }

	bool CoroutineGroup::Add(unsigned int id)
    {
        Coroutine *coroutine = this->mCorComponent->GetCoroutine(id);
        if (coroutine == nullptr)
        {
            return false;
        }
        this->mCount++;
        coroutine->mGroupId = this->mCoroutineId;
        return true;
	}
	bool CoroutineGroup::SubCount()
    {
        this->mCount--;
        if(this->mCount == 0)
        {
            this->mCorComponent->Resume(this->mCoroutineId);
            return true;
        }
        return false;
    }

	void CoroutineGroup::AwaitAll()
    {
        if(this->mCount > 0)
        {
            unsigned int id = 0;
            this->mCorComponent->YieldReturn(id);
            GKDebugInfo("coroutine group finish  id = " << id << "  corid = " << this->mCoroutineId);
        }
    }
}