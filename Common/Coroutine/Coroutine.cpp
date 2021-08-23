#include"Coroutine.h"
#include<memory.h>
#include"CoroutineManager.h"

#ifdef _WIN32
#include<Windows.h>
#endif
namespace Sentry
{
	Coroutine::Coroutine()
	{
		this->mStackSize = 0;
		this->mCoroutineId = 0;	
#ifdef __COROUTINE_ASM__
		this->mStack = nullptr;
		this->mFunction = nullptr;
		this->mCorContext = nullptr;
#elif _WIN32
		this->mContextStack = nullptr;
#endif
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