//
// Created by yjz on 2022/1/5.
//
#include"WaitTaskSourceBase.h"
#include "Util/Tools/Guid.h"
#include "Entity/Actor/App.h"
#include "Util/Tools/TimeHelper.h"
namespace acs
{
    WaitTaskSourceBase::WaitTaskSourceBase()
		: mCoroutine(App::Coroutine())
    {
        this->mCorId = 0;
        this->mState = TaskState::TaskReady;
    }

	void WaitTaskSourceBase::Clear()
	{
		this->mCorId = 0;
		this->mState = TaskState::TaskReady;
	}

    bool WaitTaskSourceBase::ResumeTask(TaskState state)
	{
		switch (this->mState)
		{
			case TaskState::TaskReady:
				this->mState = state;
				return true;
			case TaskState::TaskAwait:
				this->mState = state;
				this->mCoroutine->Resume(this->mCorId);
				return true;
			case TaskState::TaskFinish:
				assert(false);
				break;
		}
		return false;
	}

    bool WaitTaskSourceBase::YieldTask()
    {
		if(this->mState == TaskState::TaskReady)
        {
            this->mState = TaskState::TaskAwait;
			this->mCoroutine->YieldCoroutine(this->mCorId);
            return true;
        }
        return false;
    }
}

