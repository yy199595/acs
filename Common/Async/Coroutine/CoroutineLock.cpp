//
// Created by mac on 2022/4/25.
//

#include"CoroutineLock.h"
#include"Entity/App/App.h"
namespace Tendo
{
	CoroutineLock::CoroutineLock()
	{
		this->mIsLock = false;
		this->mTaskComponent = App::Inst()->GetTaskComponent();
	}

	void CoroutineLock::Lock()
	{
		if(this->mIsLock)
		{
			this->mWaitTasks.push(this->mTaskComponent->GetContextId());
			this->mTaskComponent->YieldCoroutine();
		}
		this->mIsLock = true;
	}

	void CoroutineLock::UnLock()
	{
		if(!this->mWaitTasks.empty())
		{
			unsigned int id = this->mWaitTasks.front();
			this->mTaskComponent->Resume(id);
			this->mWaitTasks.pop();
		}
		this->mIsLock = !this->mWaitTasks.empty();
	}
}