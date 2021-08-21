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
#ifdef SentryAsmCoroutine
		this->mFunction = nullptr;
		this->mCorContext = nullptr;
#else

#ifndef _WIN32
		ucontext_t mCorContext;
#else
		this->mCorContext = nullptr;
#endif

#endif
	}
	Coroutine::~Coroutine()
	{
#ifdef SentryAsmCoroutine
		if (this->mCorContext != nullptr)
		{
			delete this->mCorContext;
		}
#else
		if (this->mCorContext)
		{
#ifdef _WIN32
			DeleteFiber(this->mCorContext);
#else
			free(this->mContextStack);
#endif
			this->mCorContext = nullptr;
		}
#endif
	}
}