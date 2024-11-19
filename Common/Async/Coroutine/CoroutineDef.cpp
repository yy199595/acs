#include"CoroutineDef.h"
#include"TaskContext.h"
#include"Entity/Actor/App.h"

namespace acs
{
	TaskContextPool::~TaskContextPool()
	{
        auto iter = this->mCoroutines.begin();
        for(;iter != this->mCoroutines.end(); iter++)
        {
            delete iter->second;
        }
        this->mCoroutines.clear();
	}

	TaskContext * TaskContextPool::Pop()
	{
		TaskContext* coroutine = this->mCorPool.Pop();
		if (coroutine == nullptr)
		{
			coroutine = new TaskContext();
		}

		coroutine->mContext = nullptr;
		coroutine->mFunction = nullptr;
		coroutine->mState = CorState::Ready;
		coroutine->mCoroutineId = this->mNumPool.BuildNumber();
		coroutine->sid = coroutine->mCoroutineId & (SHARED_STACK_NUM - 1);
		coroutine->mStack.size = STACK_SIZE;
		coroutine->mStack.p = new char[STACK_SIZE];
		assert(coroutine->mStack.p);
		//memset(coroutine->mStack.p, STACK_SIZE, 0);
		this->mCoroutines.emplace(coroutine->mCoroutineId, coroutine);
		return coroutine;
	}

	void TaskContextPool::Push(TaskContext * coroutine)
	{
		long long id = coroutine->mCoroutineId;
		auto iter = this->mCoroutines.find(id);
		if (iter != this->mCoroutines.end())
		{
			this->mCoroutines.erase(iter);
		}
		this->mCorPool.Push(coroutine);
	}

	size_t TaskContextPool::GetWaitCount() const
	{
		size_t count = 0;
		auto iter = this->mCoroutines.begin();
		for(; iter != this->mCoroutines.end(); iter++)
		{
			if(iter->second->mState == CorState::Suspend)
			{
				count++;
			}
		}
		return count;
	}

	size_t TaskContextPool::GetMemory() const
	{
		size_t size = 0;
		auto iter = this->mCoroutines.begin();
		for(; iter != this->mCoroutines.end(); iter++)
		{
			size += iter->second->mStack.size;
		}
		return size;
	}

	TaskContext * TaskContextPool::Get(unsigned int id)
	{
        auto iter = this->mCoroutines.find(id);
        return iter != this->mCoroutines.end() ? iter->second : nullptr;
	}
}
