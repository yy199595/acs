#include"CoroutineEvent.h"
#include"CoroutineComponent.h"
#include"Coroutine.h"
#include<Core/App.h>
namespace GameKeeper
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
#ifdef __COROUTINE_ASM__
				coroutine->sid = coroutine->mCoroutineId & 7;
#endif
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
		Coroutine * coroutine = nullptr;
		if (!this->mIdleCoroutines.empty())
		{
			unsigned int id = this->mIdleCoroutines.front();
			this->mIdleCoroutines.pop();
			coroutine = this->mAllCoroutine[id];
		}
		else
		{
			coroutine = new Coroutine();
			if (coroutine != nullptr)
			{
				coroutine->mCoroutineId = mId++;
#ifdef __COROUTINE_ASM__
				coroutine->sid = coroutine->mCoroutineId & 7;				
#endif
				coroutine->mState = CorState::Ready;
				this->mAllCoroutine.push_back(coroutine);
			}
		}
#ifdef __COROUTINE_ASM__
		coroutine->mStack.clear();
#endif
		return coroutine;
	}
	void CoroutinePool::Push(Coroutine * coroutine)
	{
		if (coroutine != nullptr)
		{
#ifdef __COROUTINE_ASM__
			coroutine->mStack.clear();
#endif
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
		if (id >= this->mAllCoroutine.size())
		{
			return nullptr;
		}
		return this->mAllCoroutine[id];	
	}
}

namespace GameKeeper
{
	
	CoroutineGroup::CoroutineGroup(CoroutineComponent * cor)
	{
		this->mIsYield = false;
		this->mCorComponent = cor;
		this->mCoroutineId = cor->GetCurrentCorId();
	}

	bool CoroutineGroup::Add(unsigned int id)
	{
		if (this->mIsYield == false)
		{
			this->mCoroutines.insert(id);
			return true;
		}
		return false;
	}
	bool CoroutineGroup::Remove(unsigned int id)
	{
		auto iter = this->mCoroutines.find(id);
		if (iter == this->mCoroutines.end())
		{
			return false;
		}
		this->mCoroutines.erase(iter);
		if (this->mCoroutines.empty())
		{
			this->mCorComponent->Resume(this->mCoroutineId);
			return true;
		}
		return false;
	}
	void CoroutineGroup::AwaitAll()
	{
		if (this->mIsYield == false)
		{
			this->mIsYield = true;
			this->mCorComponent->YieldReturn();
		}
	}
}