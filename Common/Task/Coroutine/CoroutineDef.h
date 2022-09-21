#pragma once
#include<set>
#include<queue>
#include<vector>
#include<functional>
#include<unordered_map>
#include"Guid/NumberBuilder.h"
#include"Method/MethodProxy.h"
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
    typedef std::unordered_map<unsigned int, TaskContext *>::iterator MapIter;
    class TaskContextPool : protected std::unordered_map<unsigned int, TaskContext *>
	{
	public:
		explicit TaskContextPool() = default;
		virtual ~TaskContextPool();
	public:
		TaskContext * Pop();
		void Push(TaskContext * coroutine);
    public:
        MapIter End() { return this->end(); }
        MapIter Begin() { return this->begin(); }
        size_t GetCorCount() { return this->size();}
	public:
		TaskContext * Get(unsigned int id);
	private:
        std::queue<TaskContext *> mCorPool;
        Util::NumberBuilder<unsigned int, 10> mNumPool;
    };
}

namespace Sentry
{
	class TaskComponent;
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
		TaskComponent* mCorComponent;
	};
}