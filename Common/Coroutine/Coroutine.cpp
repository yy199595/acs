#include"Coroutine.h"

#ifdef _WIN32
#include<Windows.h>
#endif
namespace GameKeeper
{
	Coroutine::Coroutine()
	{
		this->mStackSize = 0;
		this->mCoroutineId = 0;	
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
		delete this->mFunction;
        this->mFunction = nullptr;
#elif __linux__
		free(this->mContextStack);
#elif _WIN32
		DeleteFiber(this->mContextStack);
#endif
	}
}