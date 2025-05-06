#include"TaskContext.h"
#include<memory>
namespace acs
{
	TaskContext::TaskContext()
	{
		this->id = 0;
        this->sid = 0;
		this->timerId = 0;
		this->mContext = nullptr;
        this->callback = nullptr;
		this->status = CorState::Ready;
        memset(&this->stack, 0, sizeof(Stack));
	}

	void TaskContext::Invoke()
	{
		this->callback->run();
		this->callback = nullptr;
		this->status = CorState::Finish;
	}

	TaskContext::~TaskContext()
	{
		if (this->stack.p)
		{
			std::free(this->stack.p);
			memset(&this->stack, 0, sizeof(Stack));
		}
	}
}