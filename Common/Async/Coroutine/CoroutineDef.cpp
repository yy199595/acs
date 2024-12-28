#include"CoroutineDef.h"
#include"TaskContext.h"

namespace acs
{
	TaskContextPool::~TaskContextPool()
	{
        this->mCoroutines.clear();
	}

	TaskContext * TaskContextPool::Pop()
	{
		TaskContext * newCoroutine = nullptr;
		std::unique_ptr<TaskContext> coroutine  = std::make_unique<TaskContext>();
		{
			newCoroutine = coroutine.get();
			coroutine->mContext = nullptr;
			coroutine->mFunction = nullptr;
			coroutine->mState = CorState::Ready;
			coroutine->mCoroutineId = this->mNumPool.BuildNumber();
			coroutine->sid = coroutine->mCoroutineId & (SHARED_STACK_NUM - 1);
		}
		this->mCoroutines.emplace(coroutine->mCoroutineId, std::move(coroutine));
		return newCoroutine;
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
        return iter != this->mCoroutines.end() ? iter->second.get(): nullptr;
	}

	bool TaskContextPool::Remove(unsigned int id)
	{
		auto iter = this->mCoroutines.find(id);
		if(iter == this->mCoroutines.end())
		{
			return false;
		}
		this->mCoroutines.erase(iter);
		return true;
	}
}
