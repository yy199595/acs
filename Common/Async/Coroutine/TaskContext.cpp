#include"TaskContext.h"
#include"memory.h"
namespace Tendo
{
	TaskContext::TaskContext()
	{
        this->sid = 0;
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
        std::move(this->mGroup);
		this->mState = CorState::Finish;
	}
	TaskContext::~TaskContext()
	{
		if (this->mStack.p)
		{
			free(this->mStack.p);
		}
	}
}