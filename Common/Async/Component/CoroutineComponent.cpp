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
		this->mTimer = nullptr;
		this->mConfig.pool = 100;
		this->mConfig.share = 8;
		this->mConfig.stack = 1024 * 1024;

		this->mRunContext = nullptr;
		this->mMainContext = nullptr;
		this->mCoroutines.max_load_factor(0.75);
		REGISTER_JSON_CLASS_FIELD(coroutine::Config, pool);
		REGISTER_JSON_CLASS_FIELD(coroutine::Config, share);
		REGISTER_JSON_CLASS_FIELD(coroutine::Config, stack);
	}

	void CoroutineComponent::RunTask(tb_context_t context)
    {
		this->mMainContext = context;
		if (this->mRunContext != nullptr)
		{
			this->mRunContext->Invoke();
			int sid = this->mRunContext->sid;
			Stack& stack = this->mSharedStack[sid];
			if (stack.co == this->mRunContext->id)
			{
				stack.co = 0;
			}
			unsigned int id = this->mRunContext->id;
			{
				this->Remove(id);
				this->mRunContext = nullptr;
			}
		}
		tb_context_jump(this->mMainContext, nullptr);
	}

	bool CoroutineComponent::Awake()
	{
		this->mRunContext = nullptr;
		ServerConfig::Inst()->Get("coroutine", this->mConfig);
		this->mSharedStack = std::make_unique<Stack[]>(this->mConfig.share);
		for(int index = 0; index < this->mConfig.share; index++)
		{
			Stack& stack = this->mSharedStack[index];
			{
				stack.co = 0;
				stack.size = this->mConfig.stack;
				stack.p = (char *)std::malloc(this->mConfig.stack);
				stack.top = (char*)stack.p + this->mConfig.stack;
			}
			std::memset(stack.p, 0, this->mConfig.stack);
		}
		this->mObjectPool.reserve(this->mConfig.pool);
		this->mCoroutines.reserve(this->mConfig.pool * 2);
		LOG_INFO("coroutine stack:{:.2f}MB share_num:{} pool:{}",
				this->mConfig.stack / (1024 * 1024.0f), this->mConfig.share, this->mConfig.pool);
        return true;
	}

	bool CoroutineComponent::LateAwake()
	{
		this->mTimer = this->GetComponent<TimerComponent>();
		return true;
	}

	bool CoroutineComponent::OnRefresh()
	{
		while(!this->mObjectPool.empty())
		{
			this->mObjectPool.pop_back();
		}
		return true;
	}

	size_t CoroutineComponent::GetMemory() const
	{
		size_t size = 0;
		for(const std::unique_ptr<TaskContext> & taskContext : this->mObjectPool)
		{
			size += taskContext->stack.size;
		}
		for(auto iter = this->mCoroutines.begin(); iter != this->mCoroutines.end(); iter++)
		{
			size += iter->second->stack.size;
		}
		return size;
	}

	void CoroutineComponent::OnRecord(json::w::Document& document)
	{
		size_t memory = this->GetMemory();
		for(int index = 0; index < this->mConfig.share; index++)
		{
			memory += this->mSharedStack[index].size;
		}
		constexpr double MB = 1024 * 1024.0f;
		std::unique_ptr<json::w::Value> data = document.AddObject("coroutine");
		{
			data->Add("wait", this->GetWaitCount());
			data->Add("pool", this->mObjectPool.size());
			data->Add("count", this->mCoroutines.size());
			data->Add("memory", fmt::format("{:.2f}MB", (double )memory / MB));
		}
	}

	void CoroutineComponent::Sleep(unsigned int ms)
	{
		unsigned int id = this->mRunContext->id;
		this->mTimer->DelayCall((int)ms, [this, id]
		{
			this->Resume(id);
		});
		this->YieldCoroutine();
	}

	void CoroutineComponent::SetTimeout(unsigned int timeout)
	{
		if(this->mRunContext == nullptr)
		{
			return;
		}
		int id = this->mRunContext->id;
		if(this->mRunContext->timerId > 0)
		{
			this->mTimer->CancelTimer(this->mRunContext->timerId);
		}
		this->mRunContext->timerId = this->mTimer->DelayCall(timeout, &CoroutineComponent::Resume, this, id);;
	}

	void CoroutineComponent::RunCoroutine(TaskContext * coroutine)
	{
		this->mRunContext = coroutine;
		this->mRunContext->status = CorState::Running;
		Stack& stack = mSharedStack[this->mRunContext->sid];
		if (this->mRunContext->mContext == nullptr)
		{
			if (stack.co != this->mRunContext->id)
			{
				this->SaveStack(stack.co);
				stack.co = this->mRunContext->id;
			}
			this->mRunContext->mContext = tb_context_make(stack.p, stack.size, MainEntry);
		}
		else if (stack.co != this->mRunContext->id)
		{
			this->SaveStack(stack.co);
			stack.co = this->mRunContext->id;
			Stack & curStack = this->mRunContext->stack;
            memcpy(this->mRunContext->mContext, curStack.p, curStack.size);
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
		this->mRunContext->status = CorState::Suspend;
		tb_context_jump(this->mMainContext, this->mRunContext);
		return true;
	}

	void CoroutineComponent::Resume(unsigned int id) noexcept
	{
		TaskContext* coroutine = this->Get(id);
		if (coroutine == nullptr)
		{
			LOG_FATAL("try resume context id : {}", id);
			return;
		}
		if (coroutine->status == CorState::Ready || coroutine->status == CorState::Suspend)
		{
			this->mResumeContexts.emplace(coroutine);
			return;
		}
#ifdef __DEBUG__
		assert(false);
#else
		LOG_FATAL("coroutine id:{} status:{}", id, (int)coroutine->status);
		break;
#endif
	}

	unsigned int CoroutineComponent::MakeContext(std::unique_ptr<StaticMethod> func, bool resume)
	{
		std::unique_ptr<TaskContext> coroutine;
		if (!this->mObjectPool.empty())
		{
			coroutine = std::move(this->mObjectPool.back());
			this->mObjectPool.pop_back();
		}
		else
		{
			coroutine = std::make_unique<TaskContext>();
		}
		unsigned int coroutineId = this->mNumPool.BuildNumber();
		{
			coroutine->timerId = 0;
			coroutine->id = coroutineId;
			coroutine->mContext = nullptr;
			coroutine->callback = nullptr;
			coroutine->status = CorState::Ready;
			coroutine->callback = std::move(func);
			coroutine->sid = (coroutineId - 1) & (this->mConfig.share - 1);
			this->mCoroutines.emplace(coroutineId, std::move(coroutine));
		}
		if(resume) {
			this->Resume(coroutineId);
		}
		return coroutineId;
	}

	bool CoroutineComponent::YieldCoroutine(unsigned int& coroutineId) const noexcept
	{
		if (this->mRunContext != nullptr)
		{
			coroutineId = this->mRunContext->id;
			return this->YieldCoroutine();
		}
		LOG_FATAL("not coroutine context");
		return false;
	}

	void CoroutineComponent::SaveStack(unsigned int id)
	{
		if(id == 0) return;
		TaskContext* coroutine = this->Get(id);
		if (coroutine == nullptr)
		{
			LOG_FATAL("coroutine context is null {}", id);
			return;
		}
        char* top = this->mSharedStack[coroutine->sid].top;
		const size_t size = top - (char*)coroutine->mContext;
		//CONSOLE_LOG_WARN("coroutine size => {}", size)
        if (coroutine->stack.size < size)
        {
			void * newPtr = std::realloc(coroutine->stack.p, size);
			if(newPtr == nullptr)
			{
				LOG_ERROR("alloc memory:{} is null", size);
				return;
			}
			coroutine->stack.p =  (char*)newPtr;
        }
        coroutine->stack.size = size;
        std::memcpy(coroutine->stack.p, coroutine->mContext, coroutine->stack.size);
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
			TaskContext* coroutine = this->Get(id);
			if (coroutine != nullptr)
			{
				this->RunCoroutine(coroutine);
			}
			this->mLastQueues.pop();
		}
	}

	bool CoroutineComponent::Remove(unsigned int id)
	{
		auto iter = this->mCoroutines.find(id);
		if(iter == this->mCoroutines.end())
		{
			return false;
		}
		std::unique_ptr<TaskContext> coroutine = std::move(iter->second);
		{
			if(coroutine->timerId > 0)
			{
				this->mTimer->CancelTimer(coroutine->timerId);
				coroutine->timerId = 0;
			}
			if(this->mObjectPool.size() < this->mConfig.pool)
			{
				this->mObjectPool.emplace_back(std::move(coroutine));
			}
			this->mCoroutines.erase(iter);
		}
		return true;
	}

	TaskContext* CoroutineComponent::Get(unsigned int id)
	{
		auto iter = this->mCoroutines.find(id);
		return iter != this->mCoroutines.end() ? iter->second.get(): nullptr;
	}

	size_t CoroutineComponent::GetWaitCount() const
	{
		size_t count = 0;
		auto iter = this->mCoroutines.begin();
		for(; iter != this->mCoroutines.end(); iter++)
		{
			if(iter->second->status == CorState::Suspend)
			{
				count++;
			}
		}
		return count;
	}
}
