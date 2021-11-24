#include"Coroutine.h"

#ifdef _WIN32
#include<Windows.h>
#endif
namespace GameKeeper
{
	Coroutine::Coroutine()
	{
        this->mGroupId = 0;
		this->mStackSize = 0;
		this->mCoroutineId = 0;
        this->mStack.size = 0;
        this->mStack.p = nullptr;
        this->mStack.top = nullptr;
#ifdef __COROUTINE_ASM__
		this->mFunction = nullptr;
		this->mCorContext = nullptr;
#elif _WIN32
		this->mContextStack = nullptr;
#endif
        this->mState = CorState::Ready;
	}
	Coroutine::~Coroutine()
	{
#ifdef __COROUTINE_ASM__

#elif __linux__
		free(this->mContextStack);
#elif _WIN32
		DeleteFiber(this->mContextStack);
#endif
	}
}