#include"TaskComponent.h"
#include"App/App.h"
#include"Util/Guid.h"
#include"Other/ElapsedTimer.h"
#include"Coroutine/TaskContext.h"
#include"Component/Timer/TimerComponent.h"
using namespace std::chrono;
namespace Sentry
{
	void MainEntry(tb_context_from_t parame)
	{
		auto taskComponent = (TaskComponent*)parame.priv;
		if (taskComponent != nullptr)
		{
			taskComponent->RunTask(parame.ctx);
		}
	}

	void TaskComponent::RunTask(tb_context_t context)
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

	void TaskComponent::Awake()
	{
		this->mRunContext = nullptr;
		for (Stack& stack : this->mSharedStack)
		{
			stack.co = 0;
			stack.size = STACK_SIZE;
			stack.p = new char[STACK_SIZE];
			stack.top = (char*)stack.p + STACK_SIZE;
		}
	}

	bool TaskComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mTimerManager = this->GetComponent<TimerComponent>());
//        this->Start([this]() {
//            ElapsedTimer timer;
//            std::vector<TaskContext *> tasks;
//            for (int index = 0; index < 100; index++) {
//                tasks.push_back(this->Start(&TaskComponent::Test, this, index));
//            }
//            this->WhenAll(tasks);
//            LOG_ERROR("use time = " << timer.GetSecond() << "s");
//        });
		return true;
	}

	void TaskComponent::Test(int index)
	{
		ElapsedTimer timer;
		for (int x = 0; x < 10; x++)
		{
			this->Sleep(10 + 5 * index + x);
			this->Start([this, x]()
			{
				this->Sleep(100 + x * 100);
				//LOG_ERROR(__FUNCTION__ << "  " << __LINE__);
			});
		}
		//LOG_WARN("[" << index << "] use time = " << timer.GetSecond() << "s");
	}

	void TaskComponent::WhenAny(TaskContext* coroutine)
	{
		if (this->mRunContext == nullptr)
		{
			LOG_FATAL("please in coroutine wait");
			return;
		}
		coroutine->mGroup = new CoroutineGroup(1);
		this->YieldCoroutine();
	}

	void TaskComponent::WhenAll(std::vector<TaskContext*>& coroutines)
	{
		if (this->mRunContext == nullptr)
		{
			LOG_FATAL("please in coroutine wait");
			return;
		}
		auto group = new CoroutineGroup(coroutines.size());
		for (auto coroutine : coroutines)
		{
			coroutine->mGroup = group;
		}
		this->YieldCoroutine();
	}

	void TaskComponent::Sleep(long long ms)
	{
		unsigned int id = this->mRunContext->mCoroutineId;
		StaticMethod* sleepMethod = NewMethodProxy(
			&TaskComponent::Resume, this, id);
		this->mTimerManager->AddTimer(ms, sleepMethod);
		this->YieldCoroutine();
	}

	void TaskComponent::ResumeContext(TaskContext* co)
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

	bool TaskComponent::YieldCoroutine()
	{
		LOG_CHECK_RET_FALSE(this->mRunContext);
		LOG_CHECK_RET_FALSE(this->mRunContext->mState == CorState::Running);

		this->mRunContext->mSwitchCount++;
		this->mRunContext->mState = CorState::Suspend;
		tb_context_jump(this->mMainContext, this->mRunContext);
		return true;
	}

	void TaskComponent::Resume(unsigned int id)
	{
		if (id != 0)
		{
			this->mResumeContexts.Push(id);
			return;
		}
		std::cerr << "try resume context id : " << id << std::endl;
	}

	TaskContext* TaskComponent::MakeContext(StaticMethod* func)
	{
		TaskContext* coroutine = this->mCorPool.Pop();
		if (coroutine != nullptr)
		{
			coroutine->mFunction = func;
			coroutine->mState = CorState::Ready;
		}
		return coroutine;
	}

	bool TaskComponent::YieldCoroutine(unsigned int& mCorId)
	{
		if (this->mRunContext != nullptr)
		{
			mCorId = this->mRunContext->mCoroutineId;
			return this->YieldCoroutine();
		}
		LOG_FATAL("not coroutine context");
		return false;
	}

	TaskContext* TaskComponent::GetContext(unsigned int id)
	{
		return this->mCorPool.Get(id);
	}

	void TaskComponent::SaveStack(unsigned int id)
	{
		if (id == 0) return;
		TaskContext* coroutine = this->GetContext(id);
		if (coroutine == nullptr)
		{
			return;
		}
		char* top = this->mSharedStack[coroutine->sid].top;
		size_t size = top - (char*)coroutine->mContext;
		if (coroutine->mStack.size < size)
		{
			free(coroutine->mStack.p);
			coroutine->mStack.p = (char*)malloc(size);
			assert(coroutine->mStack.p);
		}
		coroutine->mStack.size = size;
		memcpy(coroutine->mStack.p, coroutine->mContext, size);
	}

	void TaskComponent::OnSystemUpdate()
	{
		unsigned int contextId = 0;
		this->mResumeContexts.Swap();
		while (this->mResumeContexts.Pop(contextId))
		{
			TaskContext* logicCoroutine = this->GetContext(contextId);
			LOG_CHECK_RET(logicCoroutine);
			if (logicCoroutine->mState == CorState::Ready
				|| logicCoroutine->mState == CorState::Suspend)
			{
				this->mRunContext = logicCoroutine;
				this->ResumeContext(logicCoroutine);
			}
			this->mRunContext = nullptr;
		}
	}
	void TaskComponent::OnLastFrameUpdate()
	{
		while (!this->mLastQueues.empty())
		{
			unsigned int id = this->mLastQueues.front();
			TaskContext* coroutine = this->GetContext(id);
			if (coroutine != nullptr)
			{
				this->mResumeContexts.Push(id);
			}
			this->mLastQueues.pop();
		}
	}

	void TaskComponent::OnSecondUpdate()
	{
		/*size_t size = this->mCorPool.GetMemorySize();
		double memory = size / (1024.0f * 1024);
		LOG_WARN("使用内存" << memory << "mb" << "  协程总数 ：" << mCorPool.GetCorCount()
			<< "平均使用内存 ：" << size / mCorPool.GetCorCount());*/
	}
}
