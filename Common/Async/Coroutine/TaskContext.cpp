#include"TaskContext.h"
#include"memory.h"
namespace acs
{
	TaskContext::TaskContext()
	{
#ifdef __COR_SHARED_STACK__
        this->sid = 0;
#endif
        this->mCoroutineId = 0;
        this->mContext = nullptr;
        this->mFunction = nullptr;
        memset(&this->mStack, 0, sizeof(Stack));
	}

	void TaskContext::Invoke()
	{
		this->mFunction->run();
		delete this->mFunction;
		this->mFunction = nullptr;
		this->mState = CorState::Finish;
	}

	TaskContext::~TaskContext()
	{
		if (this->mStack.p)
		{
			free(this->mStack.p);
			memset(&this->mStack, 0, sizeof(Stack));
		}
	}
}