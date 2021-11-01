#pragma once
#include<set>
#include<queue>
#include<vector>
#include<functional>

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


namespace GameKeeper
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
        size_t GetCorCount() { return this->mAllCoroutine.size();}
	public:
		Coroutine * Get(unsigned int id);
	private:
		std::string * GetStack();
	private:
		unsigned int mId;
		std::queue<std::string *> mStringPool;
		std::vector<Coroutine *> mAllCoroutine;
		std::queue<unsigned int> mIdleCoroutines;
		
	};
}

namespace GameKeeper
{
	class CoroutineComponent;
	class CoroutineGroup
	{
	public:
		CoroutineGroup(CoroutineComponent *);
	public:
		bool Add(unsigned int id);
		bool Remove(unsigned int id);
		void AwaitAll();
	private:
		bool mIsYield;
		unsigned int mCoroutineId;
		CoroutineComponent * mCorComponent;
		std::set<unsigned int> mCoroutines;
	};
}