#include"CoroutineDef.h"
#include"TaskComponent.h"
#include"TaskContext.h"
#include<Core/App.h>
namespace GameKeeper
{
	TaskContextPool::~TaskContextPool()
	{
        auto iter = this->mCorMap.begin();
        for(;iter != this->mCorMap.end(); iter++)
        {
            delete iter->second;
        }
        this->mCorMap.clear();
	}
	TaskContext * TaskContextPool::Pop()
    {
        TaskContext *coroutine = nullptr;
        if(this->mCorPool.empty())
        {
            coroutine = new TaskContext();
        }
        else
        {
            coroutine = this->mCorPool.front();
            coroutine->mContext = nullptr;
            coroutine->mFunction = nullptr;
            coroutine->mState = CorState::Ready;
            this->mCorPool.pop();
        }
        coroutine->mCoroutineId = this->mNumPool.Pop();
        coroutine->sid = coroutine->mCoroutineId & (SHARED_STACK_NUM - 1);
        this->mCorMap.emplace(coroutine->mCoroutineId, coroutine);
        return coroutine;
    }

    size_t TaskContextPool::GetMemorySize()
    {
        size_t size = 0;
        auto iter = this->mCorMap.begin();
        for(; iter != this->mCorMap.end(); iter++)
        {
            size += iter->second->mStack.size;
        }
        return size;
    }

	void TaskContextPool::Push(TaskContext * coroutine)
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
        this->mCorPool.push(coroutine);
	}
	TaskContext * TaskContextPool::Get(unsigned int id)
	{
        auto iter = this->mCorMap.find(id);
        return iter != this->mCorMap.end() ? iter->second : nullptr;
	}
}

namespace GameKeeper
{
	CoroutineGroup::CoroutineGroup(size_t size)
    {
        this->mCount = size;
        this->mCorComponent = App::Get().GetTaskComponent();
        this->mCoroutineId = this->mCorComponent->GetContextId();
    }

    void CoroutineGroup::FinishAny()
    {
        this->mCount--;
        if(this->mCount == 0)
        {
            this->mCorComponent->Resume(this->mCoroutineId);
            delete this;
        }
    }
}