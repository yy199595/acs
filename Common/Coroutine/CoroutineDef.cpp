#include"CoroutineDef.h"
#include"TaskComponent.h"
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
        coroutine->sid = coroutine->mCoroutineId & (SHARED_STACK_NUM - 1);
        this->mCorMap.emplace(coroutine->mCoroutineId, coroutine);
        return coroutine;
    }

    size_t CoroutinePool::GetMemorySize()
    {
        size_t size = 0;
        auto iter = this->mCorMap.begin();
        for(; iter != this->mCorMap.end(); iter++)
        {
            size += iter->second->mStack.size;
        }
        return size;
    }

	void CoroutinePool::Push(Coroutine * coroutine)
	{
        unsigned int id = coroutine->mCoroutineId;
        //LOG_FATAL("destory coroutine : " << id);
        auto iter = this->mCorMap.find(id);
        if(iter != this->mCorMap.end())
        {
            //this->mNumPool.Push(id);
            this->mCorMap.erase(iter);
        }
        if(this->mCorPool.size() >=COR_POOL_COUNT)
        {
            delete coroutine;
            return;
        }
        coroutine->sid = 0;
        coroutine->mGroupId = 0;
        coroutine->mCoroutineId = 0;
        coroutine->mFunction = nullptr;
        coroutine->mContext = nullptr;
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
	CoroutineGroup::CoroutineGroup(TaskComponent * cor)
    {
        this->mCount = 0;
        this->mCorComponent = cor;
        this->mCoroutineId = cor->GetCurrentCorId();
    }

	void CoroutineGroup::Add(Coroutine * coroutine)
    {
        this->mCount++;
        coroutine->mGroupId = this->mCoroutineId;
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
            this->mCorComponent->Await(id);
            LOG_INFO("coroutine group finish  id = " << id << "  corid = " << this->mCoroutineId);
        }
    }
}