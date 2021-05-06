#include"Coroutine.h"
#include<memory.h>
#include"CoroutineManager.h"
#ifdef _WIN32
#include<Windows.h>
#else
#include<ucontext.h>
#endif
namespace SoEasy
{
	Coroutine::Coroutine(long long id, CoroutineAction func)
	{
		this->mStackSize = 0;
		this->mBindFunc = func;
		this->mCoroutineId = id;
		this->mContextStack = nullptr;
		this->mState = CorState::Ready;
	}

	Coroutine::~Coroutine()
	{
		if (this->mContextStack)
		{
#ifdef _WIN32
			DeleteFiber(this->mContextStack);
#else
			free(this->mContextStack);
#endif
			this->mContextStack = nullptr;
		}
	}



}