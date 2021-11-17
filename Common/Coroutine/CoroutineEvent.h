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
#include <unordered_map>
#include <Util/NumberBuilder.h>
#include<Method/MethodProxy.h>
#ifdef __COROUTINE_ASM__
	#include"Context/context.h"	
	#define STACK_SIZE 1024 * 10
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
		explicit CoroutinePool() = default;
		virtual ~CoroutinePool();
	public:
		Coroutine * Pop();
		void Push(Coroutine * coroutine);
        size_t GetCorCount() { return this->mCorMap.size();}
	public:
		Coroutine * Get(unsigned int id);
	private:
        NumberBuilder<unsigned int> mNumPool;
        std::unordered_map<unsigned int , Coroutine *> mCorMap;
    };
}

namespace GameKeeper
{
	class CoroutineComponent;
	class CoroutineGroup
	{
	public:
		explicit CoroutineGroup(CoroutineComponent *);
	public:
        bool SubCount();
        void AwaitAll();
		bool Add(unsigned int id);
        unsigned int GetGroupId() const { return this->mCoroutineId;}
	private:
        unsigned int mCount;
		unsigned int mCoroutineId;
		CoroutineComponent * mCorComponent;
	};
}