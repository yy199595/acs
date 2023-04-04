#include"CoroutineDef.h"
#include"TaskContext.h"
#include"Entity/App/App.h"
namespace Tendo
{
	TaskContextPool::~TaskContextPool()
	{
        auto iter = this->mCoroutines.begin();
        for(;iter != this->mCoroutines.end(); iter++)
        {
            delete iter->second;
        }
        this->mCoroutines.clear();
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
        this->mCoroutines.emplace(coroutine->mCoroutineId, coroutine);
        return coroutine;
    }

	void TaskContextPool::Push(TaskContext * coroutine)
	{
        unsigned int id = coroutine->mCoroutineId;
        auto iter = this->mCoroutines.find(id);
        if(iter != this->mCoroutines.end())
        {
            this->mNumPool.Push(id);
            this->mCoroutines.erase(iter);
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
        auto iter = this->mCoroutines.find(id);
        return iter != this->mCoroutines.end() ? iter->second : nullptr;
	}
}

namespace Tendo
{
	CoroutineGroup::CoroutineGroup()
	{
        this->mCoroutineId = 0;
        this->mCorComponent = App::Inst()->GetTaskComponent();
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
        this->mCorComponent = App::Inst()->GetTaskComponent();
        this->mCorComponent->YieldCoroutine(this->mCoroutineId);
    }

    void CoroutineGroup::WaitAll(std::vector<TaskContext *> &taskContexts)
    {
        if(!taskContexts.empty())
        {
            this->mCorComponent = App::Inst()->GetTaskComponent();
            for(TaskContext * taskContext : taskContexts)
            {
                assert(taskContext->mGroup == nullptr);
                taskContext->mGroup = this->shared_from_this();
            }
            this->mCorComponent->YieldCoroutine(this->mCoroutineId);
        }
    }
}