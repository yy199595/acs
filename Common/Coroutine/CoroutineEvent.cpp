#include"CoroutineEvent.h"
#include"CoroutineComponent.h"
#include"Coroutine.h"

namespace Sentry
{
	CoroutinePool::CoroutinePool(int count)
		:mId(0)
	{
		for (int index = 0; index < count; index++)
		{
			Coroutine *coroutine = new Coroutine();
			if (coroutine != nullptr)
			{
				coroutine->mCoroutineId = mId++;
				this->mAllCoroutine.push_back(coroutine);
				this->mIdleCoroutines.push(coroutine->mCoroutineId);
			}
		}
	}

	CoroutinePool::~CoroutinePool()
	{
		for (size_t index = 0; index < this->mAllCoroutine.size(); index++)
		{
			Coroutine *coroutine = this->mAllCoroutine[index];
			delete coroutine;
		}
		this->mAllCoroutine.clear();
	}
	Coroutine * CoroutinePool::Pop()
	{
		if (!this->mIdleCoroutines.empty())
		{
			unsigned int id = this->mIdleCoroutines.front();
			this->mIdleCoroutines.pop();
			return this->mAllCoroutine[id];
		}
		else
		{
			Coroutine *coroutine = new Coroutine();
			if (coroutine != nullptr)
			{
				coroutine->mCoroutineId = mId++;
				coroutine->mState = CorState::Ready;
				this->mAllCoroutine.push_back(coroutine);
			}
			return coroutine;
		}
	}
	void CoroutinePool::Push(Coroutine * coroutine)
	{
		if (coroutine != nullptr)
		{
			if (coroutine->mFunction != nullptr)
			{
				delete coroutine->mFunction;
				coroutine->mFunction = nullptr;
			}
			coroutine->mState = CorState::Finish;
			this->mIdleCoroutines.push(coroutine->mCoroutineId);
		}
	}
	Coroutine * CoroutinePool::Get(unsigned int id)
	{
		if (id > this->mAllCoroutine.size())
		{
			return nullptr;
		}
		return this->mAllCoroutine[id];	
	}
}