#pragma once
#include<set>
#include<queue>
#include<vector>
#include<memory>
#include<functional>
#include<unordered_map>
#include"Util/Guid/NumberBuilder.h"
#include"Rpc/Method/MethodProxy.h"
#include"Context/context.h"
#define STACK_SIZE 1024 * 1024
#ifdef __DEBUG__
	#define SHARED_STACK_NUM 2 //共享栈个数
#else
	#define SHARED_STACK_NUM 8 //共享栈个数
#endif
#define COR_POOL_COUNT 100


namespace Sentry
{

	class TaskContext;
    class TaskContextPool
	{
	public:
		explicit TaskContextPool() = default;
		virtual ~TaskContextPool();
	public:
		TaskContext * Pop();
		void Push(TaskContext * coroutine);
		TaskContext* Get(unsigned int id);
		size_t GetCorCount() { return this->mCoroutines.size(); }
		
	private:
        std::queue<TaskContext *> mCorPool;
        Util::NumberBuilder<unsigned int, 10> mNumPool;
		std::unordered_map<unsigned int, TaskContext*> mCoroutines;
    };
}

namespace Sentry
{
	class AsyncMgrComponent;
    class CoroutineGroup : public std::enable_shared_from_this<CoroutineGroup>
	{
	 public:
		explicit CoroutineGroup();
        ~CoroutineGroup();
	 public:
        void WaitAll();
        void WaitAll(std::vector<TaskContext *> & taskContexts);
    private:
		unsigned int mCoroutineId;
		AsyncMgrComponent* mCorComponent;
	};
}