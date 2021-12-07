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
	#define STACK_SIZE 1024 * 20
    #define SHARED_STACK_NUM 1 //共享栈个数
#elif __linux__
	#include<ucontext.h>
	#define STACK_SIZE 1024 * 1024
#elif _WIN32
	#define STACK_SIZE 1024 * 2
#endif
#define COR_POOL_COUNT 100


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

    public:
        size_t GetMemorySize();
        size_t GetCorCount() { return this->mCorMap.size();}
	public:
		Coroutine * Get(unsigned int id);
	private:
        std::queue<Coroutine *> mCorPool;
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
		void Add(Coroutine * coroutine);
        unsigned int GetGroupId() const { return this->mCoroutineId;}
	private:
        unsigned int mCount;
		unsigned int mCoroutineId;
		CoroutineComponent * mCorComponent;
	};
}