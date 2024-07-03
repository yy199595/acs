#pragma once
#include<list>
#include<memory>
#include<unordered_map>
#include"Core/Pool/ArrayPool.h"
#include"Util/Guid/NumberBuilder.h"
#include"Entity/Component/IComponent.h"
#include"Context/context.h"
#ifdef 	__COR_SHARED_STACK__
#define STACK_SIZE (1024 * 1024)
#define SHARED_STACK_NUM 1 //共享栈个数
#else
#define STACK_SIZE (1024 * 1024)
#endif


#define COR_POOL_COUNT 100


namespace joke
{

	class TaskContext;
    class TaskContextPool
	{
	public:
		explicit TaskContextPool() = default;
		virtual ~TaskContextPool();
	public:
		TaskContext * Pop();
		size_t GetMemory() const;
		TaskContext* Get(unsigned int id);
		void Push(TaskContext * coroutine);
		size_t GetWaitCount() const;
		size_t GetCount() const { return this->mCoroutines.size(); }
	private:
		math::NumberPool<unsigned int> mNumPool;
		custom::ArrayPool<TaskContext, 100> mCorPool;
		std::unordered_map<unsigned int, TaskContext*> mCoroutines;
	};
}

namespace joke
{
	class CoroutineComponent;
    class CoroutineGroup : public std::enable_shared_from_this<CoroutineGroup>
	{
	 public:
		explicit CoroutineGroup();
        ~CoroutineGroup();
	 public:
		void WaitComplete();
		void Add(unsigned int id);
		void Add(TaskContext * coroutine);
    private:
		unsigned int mCoroutineId;
		CoroutineComponent* mCorComponent;
	};
}