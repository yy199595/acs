#pragma once
#include<queue>
#include<vector>
#include<functional>

#define __COROUTINE_ASM__

#ifdef __APPLE__
#ifndef __COROUTINE_ASM__
#define __COROUTINE_ASM__
#endif // !1
#endif
#include<Method/MethodProxy.h>
#ifdef __COROUTINE_ASM__
	#include"Context/context.h"	
	#define STACK_SIZE 1024 * 1024
#elif __linux__
	#include<ucontext.h>
	#define STACK_SIZE 1024 * 1024
#elif _WIN32
	#define STACK_SIZE 1024 * 2
#endif


namespace Sentry
{
	struct Coroutine;
	class CoroutinePool
	{
	public:
		CoroutinePool(int count);
		virtual ~CoroutinePool();
	public:
		Coroutine * Pop();
		void Push(Coroutine * coroutine);
	public:
		Coroutine * Get(unsigned int id);
	private:
		unsigned int mId;
		std::vector<Coroutine *> mAllCoroutine;
		std::queue<unsigned int> mIdleCoroutines;
	};
}