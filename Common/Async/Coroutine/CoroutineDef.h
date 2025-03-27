#pragma once
#include<memory>
#include<unordered_map>
#include"Context/context.h"
#include"Util/Tools/NumberBuilder.h"
#include"Entity/Component/IComponent.h"

namespace cor
{
	constexpr int STACK_SIZE = 1024 * 1024; //共享栈大小
	constexpr int SHARED_STACK_NUM = 8; //共享栈个数
	constexpr int COROUTINE_CONTEXT_COUNT = 100; //TaskContext 对象池
}

namespace acs
{

	class TaskContext;
    class TaskContextPool final
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
		std::queue<std::unique_ptr<TaskContext>> mObjectPool;
		std::unordered_map<unsigned int, std::unique_ptr<TaskContext>> mCoroutines;
	};
}