//
// Created by mac on 2022/4/25.
//

#include"CoroutineLock.h"
#include"Entity/Actor/App.h"
namespace acs
{
	CoroutineLock::CoroutineLock(CoroutineComponent * cor)
	{
		this->mIsLock = false;
		this->mCoroutine = cor;
	}

	void CoroutineLock::Lock()
	{
		if(this->mIsLock)
		{
			unsigned int id = this->mCoroutine->GetContextId();
			{
				this->mWaitTasks.push(id);
				this->mCoroutine->YieldCoroutine();
			}
		}
		this->mIsLock = true;
	}

	void CoroutineLock::UnLock()
	{
		if(!this->mWaitTasks.empty())
		{
			unsigned int id = this->mWaitTasks.front();
			this->mCoroutine->Resume(id);
			this->mWaitTasks.pop();
		}
		this->mIsLock = !this->mWaitTasks.empty();
	}
}