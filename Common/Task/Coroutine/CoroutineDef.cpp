#include"CoroutineDef.h"
#include"TaskContext.h"
#include"App/App.h"
namespace Sentry
{
	TaskContextPool::~TaskContextPool()
	{
        auto iter = this->begin();
        for(;iter != this->end(); iter++)
        {
            delete iter->second;
        }
        this->clear();
	}
	TaskContext * TaskContextPool::Pop()
    {
        TaskContext *coroutine = nullptr;
        if(this->mCorPool.empty())
        {
            coroutine = new TaskContext();
        }
        else
        {
            coroutine = this->mCorPool.front();
            coroutine->mContext = nullptr;
            coroutine->mFunction = nullptr;
            coroutine->mState = CorState::Ready;
            this->mCorPool.pop();
        }
        coroutine->mCoroutineId = this->mNumPool.Pop();
        coroutine->sid = coroutine->mCoroutineId & (SHARED_STACK_NUM - 1);
        this->emplace(coroutine->mCoroutineId, coroutine);
        return coroutine;
    }

	void TaskContextPool::Push(TaskContext * coroutine)
	{
        unsigned int id = coroutine->mCoroutineId;
        auto iter = this->find(id);
        if(iter != this->end())
        {
            this->mNumPool.Push(id);
            this->erase(iter);
        }
        if(this->mCorPool.size() >=COR_POOL_COUNT)
        {
            delete coroutine;
            return;
        }
        this->mCorPool.push(coroutine);
	}
	TaskContext * TaskContextPool::Get(unsigned int id)
	{
        auto iter = this->find(id);
        return iter != this->end() ? iter->second : nullptr;
	}
}

namespace Sentry
{
	CoroutineGroup::CoroutineGroup()
	{
        this->mCoroutineId = 0;
        this->mCorComponent = nullptr;
		this->mCoroutineId = this->mCorComponent->GetContextId();
	}

    CoroutineGroup::~CoroutineGroup()
    {
        if(this->mCoroutineId != 0)
        {
            this->mCorComponent->Resume(this->mCoroutineId);
        }
    }

    void CoroutineGroup::WaitAll()
    {
        this->mCorComponent = App::Get()->GetTaskComponent();
        this->mCorComponent->YieldCoroutine(this->mCoroutineId);
    }

    void CoroutineGroup::WaitAll(std::vector<TaskContext *> &taskContexts)
    {
        if(!taskContexts.empty())
        {
            this->mCorComponent = App::Get()->GetTaskComponent();
            for(TaskContext * taskContext : taskContexts)
            {
                assert(taskContext->mGroup == nullptr);
                taskContext->mGroup = this->shared_from_this();
            }
            this->mCorComponent->YieldCoroutine(this->mCoroutineId);
        }
    }
}