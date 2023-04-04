#include"AsyncMgrComponent.h"
#include"Entity/App/App.h"
#include"Timer/Component/TimerComponent.h"
#ifdef __DEBUG__
#include"Util/Time/TimeHelper.h"
#endif
using namespace std::chrono;
namespace Sentry
{
	void MainEntry(tb_context_from_t context)
	{
		AsyncMgrComponent * taskComponent = (AsyncMgrComponent*)context.priv;
		if (taskComponent != nullptr)
		{
			taskComponent->RunTask(context.ctx);
		}
	}

	AsyncMgrComponent::AsyncMgrComponent()
	{
		this->mRunContext = nullptr;
		this->mMainContext = nullptr;
	}

	void AsyncMgrComponent::RunTask(tb_context_t context)
	{
		this->mMainContext = context;
		if (this->mRunContext != nullptr)
		{
			this->mRunContext->Invoke();
			int sid = this->mRunContext->sid;
			Stack& stack = this->mSharedStack[sid];
			if (stack.co == this->mRunContext->mCoroutineId)
			{
				stack.co = 0;
			}
			this->mCorPool.Push(this->mRunContext);
		}
		tb_context_jump(this->mMainContext, nullptr);
	}

	bool AsyncMgrComponent::Awake()
	{
		this->mRunContext = nullptr;
		for (Stack& stack : this->mSharedStack)
		{
			stack.co = 0;
			stack.size = STACK_SIZE;
			stack.p = new char[STACK_SIZE];
			stack.top = (char*)stack.p + STACK_SIZE;
		}
        return true;
	}

	void AsyncMgrComponent::Sleep(long long ms)
	{
        TimerComponent * timerComponent = this->mApp->GetTimerComponent();
        if(timerComponent != nullptr && ms > 0)
        {
            unsigned int id = this->mRunContext->mCoroutineId;
            StaticMethod *sleepMethod = NewMethodProxy(
                &AsyncMgrComponent::Resume, this, id);
            timerComponent->AddTimer(ms, sleepMethod);
            this->YieldCoroutine();
        }
	}

	void AsyncMgrComponent::ResumeContext(TaskContext* co)
	{
		co->mState = CorState::Running;
		Stack& stack = mSharedStack[co->sid];
		if (co->mContext == nullptr)
		{
			if (stack.co != co->mCoroutineId)
			{
				this->SaveStack(stack.co);
				stack.co = co->mCoroutineId;
			}
			this->mRunContext->mContext = tb_context_make(stack.p, stack.size, MainEntry);
		}
		else if (stack.co != co->mCoroutineId)
		{
			this->SaveStack(stack.co);
			stack.co = co->mCoroutineId;
            memcpy(co->mContext, co->mStack.p, co->mStack.size);
		}
		tb_context_from_t from = tb_context_jump(co->mContext, this);
		if (from.priv != nullptr)
		{
			this->mRunContext->mContext = from.ctx;
		}
	}

	bool AsyncMgrComponent::YieldCoroutine()
	{
		assert(this->mRunContext);
		assert(this->mRunContext->mState == CorState::Running);
		this->mRunContext->mSwitchCount++;
		this->mRunContext->mState = CorState::Suspend;
#ifdef __DEBUG__
      this->mRunContext->mSwitchTime = Helper::Time::NowSecTime();
#endif
		tb_context_jump(this->mMainContext, this->mRunContext);
		return true;
	}

	void AsyncMgrComponent::Resume(unsigned int id)
	{
        assert(this->mApp->IsMainThread());
        if(this->mCorPool.Get(id) == nullptr)
        {
            LOG_FATAL("try resume context id : " << id);
            return;
        }
        this->mResumeContexts.push(id);
	}

	TaskContext* AsyncMgrComponent::MakeContext(StaticMethod* func)
	{
		TaskContext* coroutine = this->mCorPool.Pop();
		if (coroutine != nullptr)
		{
			coroutine->mFunction = func;
			coroutine->mState = CorState::Ready;
#ifdef __DEBUG__
			coroutine->mSwitchTime = Helper::Time::NowSecTime();
#endif
		}
		return coroutine;
	}

	bool AsyncMgrComponent::YieldCoroutine(unsigned int& mCorId)
	{
		if (this->mRunContext != nullptr)
		{
			mCorId = this->mRunContext->mCoroutineId;
			return this->YieldCoroutine();
		}
		LOG_FATAL("not coroutine context");
		return false;
	}

	void AsyncMgrComponent::SaveStack(unsigned int id)
	{
		if (id == 0) return;
		TaskContext* coroutine = this->mCorPool.Get(id);
		if (coroutine == nullptr)
		{
			return;
		}
        char* top = this->mSharedStack[coroutine->sid].top;
		const size_t size = top - (char*)coroutine->mContext;
        if (coroutine->mStack.size < size)
        {
            coroutine->mStack.p = (char*)realloc(coroutine->mStack.p, size);
            assert(coroutine->mStack.p);
        }
        coroutine->mStack.size = size;
        memcpy(coroutine->mStack.p, coroutine->mContext, coroutine->mStack.size);
    }

	void AsyncMgrComponent::OnSystemUpdate()
	{
		unsigned int contextId = 0;
        while(!this->mResumeContexts.empty())
        {
            contextId = this->mResumeContexts.front();
            TaskContext* logicCoroutine = this->mCorPool.Get(contextId);
            if(logicCoroutine == nullptr)
            {
                LOG_FATAL("not find task context : " << contextId);
                continue;
            }
            if (logicCoroutine->mState == CorState::Ready
                || logicCoroutine->mState == CorState::Suspend)
            {
                this->mRunContext = logicCoroutine;
                this->ResumeContext(logicCoroutine);
            }
            this->mRunContext = nullptr;
            this->mResumeContexts.pop();
        }
	}
	void AsyncMgrComponent::OnLastFrameUpdate()
	{
		while (!this->mLastQueues.empty())
		{
			unsigned int id = this->mLastQueues.front();
			TaskContext* coroutine = this->mCorPool.Get(id);
			if (coroutine != nullptr)
			{
				this->mResumeContexts.push(id);
			}
			this->mLastQueues.pop();
		}
	}
}
