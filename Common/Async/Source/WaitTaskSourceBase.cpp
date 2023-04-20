//
// Created by yjz on 2022/1/5.
//
#include"WaitTaskSourceBase.h"
#include "Util/Guid/Guid.h"
#include "Entity/Unit/App.h"
#include "Util/Time/TimeHelper.h"
namespace Tendo
{
    WaitTaskSourceBase::WaitTaskSourceBase()
    {
        this->mCorId = 0;
        this->mState = TaskState::TaskReady;
        this->mTaskId = Helper::Guid::Create();
        this->mCreateTime = Helper::Time::NowMilTime();
    }

	void WaitTaskSourceBase::Clear()
	{
		this->mCorId = 0;
		this->mState = TaskState::TaskReady;
		this->mTaskId = Helper::Guid::Create();
		this->mCreateTime = Helper::Time::NowMilTime();
	}

    bool WaitTaskSourceBase::ResumeTask(TaskState state)
    {
        switch(this->mState)
		{
		case TaskState::TaskReady:
			this->mState = state;
			return true;
		case TaskState::TaskAwait:
			this->mState = state;
			App::Inst()->GetTaskComponent()->Resume(this->mCorId);
			return true;
		case TaskState::TaskFinish:
			break;
		}
        return false;
    }

    bool WaitTaskSourceBase::YieldTask()
    {
		if(this->mState == TaskState::TaskReady)
        {
            this->mState = TaskState::TaskAwait;
			App::Inst()->GetTaskComponent()->YieldCoroutine(this->mCorId);
            return true;
        }
        return false;
    }
}

