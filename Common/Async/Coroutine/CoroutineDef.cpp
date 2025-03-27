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
		TaskContext* newCoroutine = nullptr;
		std::unique_ptr<TaskContext> coroutine;
		if (!this->mObjectPool.empty())
		{
			coroutine = std::move(this->mObjectPool.front());
			this->mObjectPool.pop();
		}
		else
		{
			coroutine = std::make_unique<TaskContext>();
		}

		newCoroutine = coroutine.get();
		coroutine->mContext = nullptr;
		coroutine->callback = nullptr;
		coroutine->status = CorState::Ready;
		coroutine->id = this->mNumPool.BuildNumber();
		coroutine->sid = coroutine->id & (cor::SHARED_STACK_NUM - 1);
		this->mCoroutines.emplace(coroutine->id, std::move(coroutine));
		return newCoroutine;
	}

	size_t TaskContextPool::GetWaitCount() const
	{
		size_t count = 0;
		auto iter = this->mCoroutines.begin();
		for(; iter != this->mCoroutines.end(); iter++)
		{
			if(iter->second->status == CorState::Suspend)
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
			size += iter->second->stack.size;
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
		std::unique_ptr<TaskContext> coroutine = std::move(iter->second);
		if(this->mObjectPool.size() < cor::COROUTINE_CONTEXT_COUNT)
		{
			this->mObjectPool.emplace(std::move(coroutine));
		}
		this->mCoroutines.erase(iter);
		return true;
	}
}
