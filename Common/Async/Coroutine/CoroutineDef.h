#pragma once
#include<list>
#include<memory>
#include<unordered_map>
#include"Core/Pool/ArrayPool.h"
#include"Util/Tools/NumberBuilder.h"
#include"Entity/Component/IComponent.h"
#include"Context/context.h"

#define STACK_SIZE (1024 * 1024)
#define SHARED_STACK_NUM 8 //共享栈个数

#define COR_POOL_COUNT 100


namespace acs
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
		bool Remove(unsigned int id);
		TaskContext* Get(unsigned int id);
		size_t GetWaitCount() const;
		size_t GetCount() const { return this->mCoroutines.size(); }
	private:
		math::NumberPool<unsigned int> mNumPool;
		std::unordered_map<unsigned int, TaskContext*> mCoroutines;
	};
}