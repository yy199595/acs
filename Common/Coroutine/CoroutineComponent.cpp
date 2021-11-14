#include"CoroutineComponent.h"
#include<chrono>
#include<memory.h>
#include<Core/App.h>
#include"Coroutine.h"
#include<Util/NumberHelper.h>
#include<Timer/TimerComponent.h>
#include<Timer/CorSleepTimer.h>

using namespace std::chrono;
#ifdef _WIN32
#include<Windows.h>
#endif
namespace GameKeeper
{
#ifdef __COROUTINE_ASM__
	void MainEntry(tb_context_from_t parame)
	{
		CoroutineComponent *pCoroutineMgr = App::Get().GetCorComponent();
		((Coroutine*)parame.priv)->mCorContext = parame.ctx;
#else
#if _WIN32
	void __stdcall MainEntry(LPVOID manager)
	{
		CoroutineComponent *pCoroutineMgr = (CoroutineComponent *)manager;
#else
	void MainEntry(void *manager)
	{
		CoroutineComponent *pCoroutineMgr = (CoroutineComponent *)manager;
#endif
#endif		
		Coroutine * logicCoroutine = pCoroutineMgr->GetCoroutine();
		if (logicCoroutine != nullptr)
		{
			logicCoroutine->mFunction->run();
			pCoroutineMgr->Destory(logicCoroutine);
		}
#ifdef __COROUTINE_ASM__		
		tb_context_jump(parame.ctx, nullptr);
#endif
	}

    CoroutineComponent::CoroutineComponent()
		:mCorPool(100)
    {		
		this->mCurrentCorId = 0;
		this->mMainCoroutine = this->mCorPool.Pop();
#ifdef __COROUTINE_ASM__
		this->mSharedStack = (Stack*)calloc(8, sizeof(Stack));
#elif _WIN32		
		this->mMainCoroutine->mContextStack = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
#else
		this->mTop = this->mSharedStack + STACK_SIZE;
#endif	
    }

	CoroutineComponent::~CoroutineComponent()
	{

	}

    bool CoroutineComponent::Awake()
    {
		GKAssertRetFalse_F(this->mTimerManager = this->GetComponent<TimerComponent>());
		for (int index = 0; index < 10000; index++)
		{
			this->StartCoroutine(&CoroutineComponent::Loop, this);
		}
        return true;
    }

    void CoroutineComponent::Start()
    {
		/*long long t1 = TimeHelper::GetMilTimestamp();
		CoroutineGroup * group = this->NewCoroutineGroup();
		for (size_t index = 0; index < 10; index++)
		{
			group->Add(this->StartCoroutine(&CoroutineComponent::Loop, this));
		}
		group->AwaitAll();
		long long t2 = TimeHelper::GetMilTimestamp();
		GKDebugError("时间 = " << t2 - t1);*/
		
    }

	void CoroutineComponent::Loop()
	{
		double val = 0.3;
		std::string str = __FUNCTION__;
        for (int i = 0; i < 10; ++i) 
		{
            this->Sleep(1000);
        }
	}

    void CoroutineComponent::Sleep(long long ms)
    {
        Coroutine *logicCoroutine = this->GetCoroutine();
        if (logicCoroutine != nullptr)
        {
            const long long id = logicCoroutine->mCoroutineId;
            this->mTimerManager->CreateTimer<CorSleepTimer>(this, id, ms);
            this->YieldReturn();
        }
    }

	void CoroutineComponent::YieldNextLoop()
	{
		Coroutine *logicCoroutine = this->GetCoroutine();
		if (logicCoroutine != nullptr)
		{
			this->mResumeCors.push(logicCoroutine->mCoroutineId);
			this->YieldReturn();
		}
	}

	void CoroutineComponent::YieldNextFrame()
	{
		Coroutine *logicCoroutine = this->GetCoroutine();
		if (logicCoroutine != nullptr)
		{
			this->mLateUpdateCors.push(logicCoroutine->mCoroutineId);
			this->YieldReturn();
		}
	}

	void CoroutineComponent::ResumeCoroutine()
	{
		Coroutine * logicCoroutine = this->GetCoroutine();
		if (logicCoroutine == nullptr)
		{
			return;
		}
#ifdef __COROUTINE_ASM__
		tb_context_from_t from;
		Stack & stack = mSharedStack[logicCoroutine->sid];
		if (stack.p == nullptr)
		{
			stack.p = (char*)malloc(STACK_SIZE);
			stack.top = stack.p + STACK_SIZE;
			stack.co = logicCoroutine;
		}
#endif
		if (logicCoroutine->mState == CorState::Ready)
		{
#ifdef __COROUTINE_ASM__
			if (stack.co != logicCoroutine)
			{
				this->SaveStack(stack.co);
				stack.co = logicCoroutine;
			}
			logicCoroutine->mCorContext =
				tb_context_make(stack.p, STACK_SIZE, MainEntry);
			from = tb_context_jump(logicCoroutine->mCorContext, this->mMainCoroutine);
#elif _WIN32
			logicCoroutine->mContextStack = CreateFiber(STACK_SIZE, MainEntry, this);
			SwitchToFiber(logicCoroutine->mContextStack);
#elif __linux__
			getcontext(&logicCoroutine->mCorContext);
			logicCoroutine->mState = CorState::Running;
			logicCoroutine->mCorContext.uc_stack.ss_size = STACK_SIZE;
			logicCoroutine->mCorContext.uc_stack.ss_sp = this->mSharedStack;
			logicCoroutine->mCorContext.uc_link = &this->mMainCoroutine->mCorContext;
			makecontext(&logicCoroutine->mCorContext, (void(*)(void)) MainEntry, 1, this);
			swapcontext(&this->mMainCoroutine->mCorContext, &logicCoroutine->mCorContext);
#endif
			}
		else if (logicCoroutine->mState == CorState::Suspend)
		{
			logicCoroutine->mState = CorState::Running;
#ifdef __COROUTINE_ASM__
			if (stack.co != logicCoroutine)
			{
				this->SaveStack(stack.co);
				const std::string & data = logicCoroutine->mStack;
				memcpy(logicCoroutine->mCorContext, data.c_str(), data.size()); // restore stack data
				stack.co = logicCoroutine;
			}
			from = tb_context_jump(logicCoroutine->mCorContext, this->mMainCoroutine);
#elif _WIN32
			SwitchToFiber(logicCoroutine->mContextStack);
#elif __linux__
			void *start = this->mSharedStack + STACK_SIZE - logicCoroutine->mStackSize;
			memcpy(start, logicCoroutine->mContextStack, logicCoroutine->mStackSize);
			swapcontext(&this->mMainCoroutine->mCorContext, &logicCoroutine->mCorContext);
#endif
		}
#ifdef __COROUTINE_ASM__
		logicCoroutine->mCorContext = from.ctx;
#endif
	}

	void CoroutineComponent::Resume(unsigned int id)
	{
		if (id == 0)
		{
			return;
		}
		Coroutine *logicCoroutine = this->GetCoroutine(id);
		if (logicCoroutine == nullptr)
		{
			GKDebugFatal("not find coroutine id : " << id);
			return;
		}
		this->mResumeCors.push(id);		
	}

	CoroutineGroup * CoroutineComponent::NewCoroutineGroup()
	{
		if (this->mCurrentCorId == 0)
		{
			return nullptr;
		}
        auto group = new CoroutineGroup(this);
		this->mCoroutineGroups.push_back(group);
		return group;
	}

	unsigned int CoroutineComponent::StartCoroutine(StaticMethod * func)
	{
		Coroutine * coroutine = this->mCorPool.Pop();
		if (coroutine != nullptr)
		{
#ifndef __COROUTINE_ASM__
	#ifdef _WIN32
			if (coroutine->mContextStack != nullptr)
			{
				DeleteFiber(coroutine->mContextStack);
				coroutine->mStackSize = 0;
				coroutine->mContextStack = nullptr;
			}
	#endif
#endif
			coroutine->mFunction = func;
			coroutine->mState = CorState::Ready;
			this->Resume(coroutine->mCoroutineId);
			return coroutine->mCoroutineId;
		}
		return 0;
	}

	void CoroutineComponent::YieldReturn()
    {
        Coroutine *logicCoroutine = this->GetCoroutine();
        if (logicCoroutine == nullptr)
        {
            GKDebugFatal("not find coroutine context");
            return;
        }
		if (logicCoroutine->mCoroutineId == 0)
		{
			GKDebugFatal("try yield main coroutine");
			return;
		}
        this->mCurrentCorId = 0;
        logicCoroutine->mState = CorState::Suspend;
#ifdef __COROUTINE_ASM__
		tb_context_jump(this->mMainCoroutine->mCorContext, this);
#elif _WIN32
        SwitchToFiber(this->mMainCoroutine->mContextStack);
#else
        this->SaveStack(logicCoroutine, this->mTop);
        swapcontext(&logicCoroutine->mCorContext, &this->mMainCoroutine->mCorContext);
#endif
    }

	void CoroutineComponent::YieldReturn(unsigned int & mCorId)
	{
		mCorId = this->mCurrentCorId;
		this->YieldReturn();
	}

	Coroutine *CoroutineComponent::GetCoroutine()
    {
		if (this->mCurrentCorId == 0)
		{
			return nullptr;
		}
		return this->mCorPool.Get(this->mCurrentCorId);     
    }

    Coroutine *CoroutineComponent::GetCoroutine(unsigned int id)
    {
		return this->mCorPool.Get(id);
    }

	void CoroutineComponent::Destory(Coroutine * coroutine)
	{
		unsigned int id = coroutine->mCoroutineId;
		if (!this->mCoroutineGroups.empty())
		{
			auto iter = this->mCoroutineGroups.begin();
			for (; iter != this->mCoroutineGroups.end(); iter++)
			{
				CoroutineGroup * group = (*iter);
				if (group->Remove(id))
				{
					this->mCoroutineGroups.erase(iter);
					delete group;
					break;
				}
			}
		}
		this->mCurrentCorId = 0;
		this->mCorPool.Push(coroutine);
#ifdef __COROUTINE_ASM__
		
#elif _WIN32
		SwitchToFiber(this->mMainCoroutine->mContextStack);
#else
		setcontext(&this->mMainCoroutine->mCorContext);
#endif
	}

#ifdef __COROUTINE_ASM__
	void CoroutineComponent::SaveStack(Coroutine * cor)
	{
		cor->mStack.clear();
		char * top = this->mSharedStack[cor->sid].top;
		cor->mStackSize = top - (char*)cor->mCorContext;
		//GKDebugWarning("size = " << cor->mStackSize);
		cor->mStack.append((char *)cor->mCorContext, cor->mStackSize);
	}
#else
    void CoroutineComponent::SaveStack(Coroutine *cor, char *top)
	{		
		char dummy = 0;
		size_t size = top - &dummy;
		if (cor->mStackSize < size)
		{
			free(cor->mContextStack);
			cor->mContextStack = malloc(size);
		}
		cor->mStackSize = size;
		memcpy(cor->mContextStack, &dummy, size);
    }
#endif 

	void CoroutineComponent::OnSystemUpdate()
    {
		while (!this->mResumeCors.empty())
		{
			this->mCurrentCorId = this->mResumeCors.front();
			this->ResumeCoroutine();
			this->mResumeCors.pop();
		}
		this->mCurrentCorId = 0;
    }
	void CoroutineComponent::OnLastFrameUpdate()
	{
		while (!this->mLateUpdateCors.empty())
		{
			this->mCurrentCorId = this->mLateUpdateCors.front();
			this->ResumeCoroutine();
			this->mLateUpdateCors.pop();
		}
		this->mCurrentCorId = 0;
	}

    void CoroutineComponent::OnSecondUpdate() {

		size_t index = 1;
		long long size = 0;
		for (size_t index = 1; index < mCorPool.GetCorCount(); index++)
		{
			Coroutine * cor = mCorPool.Get(index);
			size += cor->mStack.size();
			cor = mCorPool.Get(index);
		}
		
		double memory = size / 1024.0f / 1024.0f;
		GKDebugWarning("使用内存" << memory << "M" << "  协程总数 ：" << mCorPool.GetCorCount()
			<< "平均使用内存 ：" << size / mCorPool.GetCorCount());
    }
}
