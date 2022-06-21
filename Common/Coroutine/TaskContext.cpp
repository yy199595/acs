#include"TaskContext.h"

#ifdef _WIN32
#include<Windows.h>
#endif
namespace Sentry
{
	TaskContext::TaskContext()
	{
		memset(this, 0, (sizeof(TaskContext)));
	}

	void TaskContext::Invoke()
	{
		this->mFunction->run();
		if (this->mGroup != nullptr)
		{
			this->mGroup->FinishAny();
		}
		delete this->mFunction;
		this->mGroup = nullptr;
		this->mFunction = nullptr;
		this->mState = CorState::Finish;
	}
	TaskContext::~TaskContext()
	{
#ifndef __COROUTINE_BUFFER_STRING__
		if (this->mStack.p)
		{
			free(this->mStack.p);
		}
#endif
	}
}