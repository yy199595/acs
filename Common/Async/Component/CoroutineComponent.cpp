#include"CoroutineComponent.h"
#include"Entity/Actor/App.h"
#include"Timer/Component/TimerComponent.h"
#ifdef __DEBUG__
#include"Util/Tools/TimeHelper.h"
#endif

namespace acs
{
	void MainEntry(tb_context_from_t context)
	{
		CoroutineComponent * taskComponent = (CoroutineComponent*)context.priv;
		if (taskComponent != nullptr)
		{
			taskComponent->RunTask(context.ctx);
		}
	}

	CoroutineComponent::CoroutineComponent()
	{
		this->mRunContext = nullptr;
		this->mMainContext = nullptr;
		this->mTimerComponent = nullptr;
	}

	void CoroutineComponent::RunTask(tb_context_t context)
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
			unsigned int id = this->mRunContext->mCoroutineId;
			{
				this->mCorPool.Remove(id);
				this->mRunContext = nullptr;
			}
		}
		tb_context_jump(this->mMainContext, nullptr);
	}

	bool CoroutineComponent::Awake()
	{
		this->mRunContext = nullptr;
		for (Stack& stack : this->mSharedStack)
		{
			stack.co = 0;
			stack.size = STACK_SIZE;
			stack.p = new char[STACK_SIZE];
			memset(stack.p, 0, STACK_SIZE);
			stack.top = (char*)stack.p + STACK_SIZE;
		}
        return true;
	}

	bool CoroutineComponent::LateAwake()
	{
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		return true;
	}

	void CoroutineComponent::OnRecord(json::w::Document& document)
	{
		std::unique_ptr<json::w::Value> data = document.AddObject("coroutine");
		{
			data->Add("count", this->mCorPool.GetCount());
			data->Add("memory", this->mCorPool.GetMemory());
			data->Add("wait", this->mCorPool.GetWaitCount());
		}
	}

	void CoroutineComponent::Sleep(unsigned int ms)
	{
		unsigned int id = this->mRunContext->mCoroutineId;
		this->mTimerComponent->DelayCall((int)ms, [this, id]
		{
			this->Resume(id);
		});
		this->YieldCoroutine();
	}

	void CoroutineComponent::RunCoroutine(TaskContext * coroutine)
	{
		this->mRunContext = coroutine;
		this->mRunContext->mState = CorState::Running;
		Stack& stack = mSharedStack[this->mRunContext->sid];
		if (this->mRunContext->mContext == nullptr)
		{
			if (stack.co != this->mRunContext->mCoroutineId)
			{
				this->SaveStack(stack.co);
				stack.co = this->mRunContext->mCoroutineId;
			}
			this->mRunContext->mContext = tb_context_make(stack.p, stack.size, MainEntry);
		}
		else if (stack.co != this->mRunContext->mCoroutineId)
		{
			this->SaveStack(stack.co);
			stack.co = this->mRunContext->mCoroutineId;
            memcpy(this->mRunContext->mContext, this->mRunContext->mStack.p, this->mRunContext->mStack.size);
		}
		tb_context_from_t from = tb_context_jump(this->mRunContext->mContext, this);
		if (from.priv != nullptr)
		{
			this->mRunContext->mContext = from.ctx;
		}
	}

	bool CoroutineComponent::YieldCoroutine() const noexcept
	{
		assert(this->mRunContext);
		this->mRunContext->mState = CorState::Suspend;
		tb_context_jump(this->mMainContext, this->mRunContext);
		return true;
	}

	unsigned int CoroutineComponent::Resume(unsigned int id) noexcept
	{
		TaskContext* coroutine = this->mCorPool.Get(id);
		if (coroutine == nullptr)
		{
			LOG_FATAL("try resume context id : {}", id);
			return 0;
		}
		switch (coroutine->mState)
		{
			case CorState::Ready:
			case CorState::Suspend:
				this->mResumeContexts.push(coroutine);
				return coroutine->mCoroutineId;
			default:
#ifdef __DEBUG__
				assert(false);
#else
				LOG_FATAL("coroutine id:{} status:{}", id, (int)coroutine->mState);
				break;
#endif
		}
		return 0;
	}

	TaskContext* CoroutineComponent::MakeContext(std::unique_ptr<StaticMethod> func)
	{
		TaskContext* coroutine = this->mCorPool.Pop();
		if (coroutine != nullptr)
		{
			coroutine->mState = CorState::Ready;
			coroutine->mFunction = std::move(func);
		}
		return coroutine;
	}

	bool CoroutineComponent::YieldCoroutine(unsigned int& mCorId) const noexcept
	{
		if (this->mRunContext != nullptr)
		{
			mCorId = this->mRunContext->mCoroutineId;
			return this->YieldCoroutine();
		}
		LOG_FATAL("not coroutine context");
		return false;
	}

	void CoroutineComponent::SaveStack(unsigned int id)
	{
		if(id == 0)
		{
			return;
		}
		TaskContext* coroutine = this->mCorPool.Get(id);
		if (coroutine == nullptr)
		{
			LOG_FATAL("coroutine context is null {}", id);
			return;
		}
        char* top = this->mSharedStack[coroutine->sid].top;
		const size_t size = top - (char*)coroutine->mContext;
        if (coroutine->mStack.size < size)
        {
			void * newPtr = std::realloc(coroutine->mStack.p, size);
			if(newPtr == nullptr)
			{
				LOG_ERROR("alloc memory:{} is null", size);
				return;
			}
			coroutine->mStack.p =  (char*)newPtr;
        }
        coroutine->mStack.size = size;
        memcpy(coroutine->mStack.p, coroutine->mContext, coroutine->mStack.size);
    }

	void CoroutineComponent::OnSystemUpdate() noexcept
	{
        while(!this->mResumeContexts.empty())
        {
            TaskContext * coroutine = this->mResumeContexts.front();
			{
				this->mResumeContexts.pop();
				if(coroutine == nullptr)
				{
					LOG_FATAL("not find task context");
					continue;
				}
			}
			this->RunCoroutine(coroutine);
        }
	}
	void CoroutineComponent::OnLastFrameUpdate(long long) noexcept
	{
		while (!this->mLastQueues.empty())
		{
			unsigned int id = this->mLastQueues.front();
			TaskContext* coroutine = this->mCorPool.Get(id);
			if (coroutine != nullptr)
			{
				this->RunCoroutine(coroutine);
			}
			this->mLastQueues.pop();
		}
	}
}
