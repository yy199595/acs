#include"TaskContext.h"
#include"memory.h"
namespace acs
{

#ifdef __MEMORY_POOL_OPERATOR__
	std::vector<void *> TaskContext::sAllocArray;
	void* TaskContext::operator new(size_t size)
	{
		if(!sAllocArray.empty())
		{
			void * ptr = sAllocArray.back();
			sAllocArray.pop_back();
			return ptr;
		}
		return std::malloc(size);
	}

	void TaskContext::operator delete(void* ptr) noexcept
	{
		if(sAllocArray.size() >= 100)
		{
			std::free(ptr);
			return;
		}
		sAllocArray.emplace_back(ptr);
	}
#endif

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