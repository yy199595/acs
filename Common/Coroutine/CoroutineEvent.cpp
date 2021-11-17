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
		 auto coroutine = new Coroutine();
         coroutine->mCoroutineId = this->mNumPool.Pop();
#ifdef __COROUTINE_ASM__
        coroutine->sid = coroutine->mCoroutineId & 7;
#endif
        this->mCorMap.emplace(coroutine->mCoroutineId, coroutine);
		return coroutine;
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
        delete coroutine;
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
		this->mIsYield = false;
		this->mCorComponent = cor;
		this->mCoroutineId = cor->GetCurrentCorId();
	}

	bool CoroutineGroup::Add(unsigned int id)
	{
		if (!this->mIsYield)
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
		if (!this->mIsYield)
		{
			this->mIsYield = true;
			this->mCorComponent->YieldReturn();
		}
	}
}