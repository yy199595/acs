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
        Coroutine * lastCoroutine = ((Coroutine*)parame.priv);

        lastCoroutine->mCorContext = parame.ctx;
        Coroutine * logicCoroutine = pCoroutineMgr->GetCoroutine();
        if (logicCoroutine != nullptr)
        {
            logicCoroutine->mFunction->run();
        }
        tb_context_jump(lastCoroutine->mCorContext, nullptr);
#elif _WIN32
        void __stdcall MainEntry(LPVOID manager)
	    {
		    CoroutineComponent *pCoroutineMgr = (CoroutineComponent *)manager;
        }
#elif __linux__
void MainEntry(void *manager)
	{
		CoroutineComponent *pCoroutineMgr = (CoroutineComponent *)manager;
    }
#endif
	}

    CoroutineComponent::CoroutineComponent()
    {
        this->mCurrentCorId = 0;

        this->mMainCoroutine = this->mCorPool.Pop();
#ifdef __COROUTINE_ASM__
        this->mSharedStack.co = 0;
        this->mSharedStack.p = nullptr;
        this->mSharedStack.top = nullptr;
#elif _WIN32
        this->mMainCoroutine->mContextStack = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
#else
        this->mTop = this->mSharedStack + STACK_SIZE;
#endif
    }

	CoroutineComponent::~CoroutineComponent() = default;

    bool CoroutineComponent::Awake()
    {
        GKAssertRetFalse_F(this->mTimerManager = this->GetComponent<TimerComponent>());

//        for (size_t index = 0; index < 20; index++)
//        {
//            this->StartCoroutine(&CoroutineComponent::Loop, this);
//        }
        return true;
    }

    void CoroutineComponent::Start()
    {
		long long t1 = TimeHelper::GetMilTimestamp();

		long long t2 = TimeHelper::GetMilTimestamp();
		GKDebugError("时间 = " << t2 - t1);
		
    }

	void CoroutineComponent::Loop()
	{
		double val = 0.3;
		std::string str = __FUNCTION__;
        for (int i = 0; i < 10; ++i) 
		{
            this->Sleep(100);
        }
        GKDebugFatal("==============");
	}

    void CoroutineComponent::Sleep(long long ms)
    {
        GKAssertRet_F(this->IsInLogicCoroutine());
        this->mTimerManager->AddTimer(new CorSleepTimer(this, this->mCurrentCorId, ms));
        this->YieldReturn();
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
			this->mLastQueues1.push(logicCoroutine->mCoroutineId);
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
		Stack & stack = mSharedStack;
		if (stack.p == nullptr)
		{		
			stack.p = new char[STACK_SIZE];
			stack.top = stack.p + STACK_SIZE;
			stack.co = logicCoroutine->mCoroutineId;
		}
        if(logicCoroutine->mCorContext == nullptr)
        {
            if(stack.co != logicCoroutine->mCoroutineId)
            {
                this->SaveStack(stack.co);
                stack.co = logicCoroutine->mCoroutineId;
            }
            logicCoroutine->mCorContext =
                    tb_context_make(stack.p, STACK_SIZE, MainEntry);
            GKDebugFatal(logicCoroutine->mCoroutineId << " stack top = " << logicCoroutine->mCorContext << "  " << (void*)stack.p);
            from = tb_context_jump(logicCoroutine->mCorContext, this->mMainCoroutine);
        }
        else
        {
            if (stack.co != logicCoroutine->mCoroutineId)
            {
                this->SaveStack(stack.co);
                stack.co = logicCoroutine->mCoroutineId;
                const std::string & data = logicCoroutine->mStack;
                memcpy(logicCoroutine->mCorContext, data.c_str(), data.size());
            }
            logicCoroutine->mState = CorState::Running;
            from = tb_context_jump(logicCoroutine->mCorContext, this->mMainCoroutine);
        }
        if (from.priv)
        {
            assert(logicCoroutine == from.priv);
            GKDebugWarning("coroutine " << logicCoroutine->mCoroutineId
                                        << " from " << logicCoroutine->mCorContext << " switch to " << from.ctx);
            logicCoroutine->mCorContext = from.ctx;
        }
        else
        {
            this->mCurrentCorId = 0;
            this->Destory(logicCoroutine);
        }
#elif _WIN32
        if (logicCoroutine->mState == CorState::Ready)
        {
            logicCoroutine->mContextStack = CreateFiber(STACK_SIZE, MainEntry, this);
            SwitchToFiber(logicCoroutine->mContextStack);
        }
        else if (logicCoroutine->mState == CorState::Suspend)
        {
            logicCoroutine->mState = CorState::Running;
            SwitchToFiber(logicCoroutine->mContextStack);
        }
#elif __linux__
        if (logicCoroutine->mState == CorState::Ready)
        {
            getcontext(&logicCoroutine->mCorContext);
            logicCoroutine->mState = CorState::Running;
            logicCoroutine->mCorContext.uc_stack.ss_size = STACK_SIZE;
            logicCoroutine->mCorContext.uc_stack.ss_sp = this->mSharedStack;
            logicCoroutine->mCorContext.uc_link = &this->mMainCoroutine->mCorContext;
            makecontext(&logicCoroutine->mCorContext, (void (*)(void)) MainEntry, 1, this);
            swapcontext(&this->mMainCoroutine->mCorContext, &logicCoroutine->mCorContext);
        }
        else if (logicCoroutine->mState == CorState::Suspend)
        {
            void *start = this->mSharedStack + STACK_SIZE - logicCoroutine->mStackSize;
            memcpy(start, logicCoroutine->mContextStack, logicCoroutine->mStackSize);
            swapcontext(&this->mMainCoroutine->mCorContext, &logicCoroutine->mCorContext);
        }
#endif
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
		tb_context_jump(this->mMainCoroutine->mCorContext, logicCoroutine);
#elif _WIN32
		SwitchToFiber(this->mMainCoroutine->mContextStack);
#elif __linux__
		this->SaveStack(logicCoroutine, this->mTop);
		swapcontext(&logicCoroutine->mCorContext, &this->mMainCoroutine->mCorContext);
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
        GKAssertRet_F(logicCoroutine->mState == CorState::Suspend
                      || logicCoroutine->mState == CorState::Ready);
        this->mResumeCors.push(id);
    }

	CoroutineGroup * CoroutineComponent::NewCoroutineGroup()
	{
		if (this->mCurrentCorId == 0)
		{
			return nullptr;
		}
        auto group = new CoroutineGroup(this);
        this->mCoroutineGroups.emplace(group->GetGroupId(), group);
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
		if (!this->mCoroutineGroups.empty())
		{
            unsigned int id = coroutine->mGroupId;
            if(coroutine->mGroupId != 0)
            {
                auto iter = this->mCoroutineGroups.find(coroutine->mGroupId);
                if(iter != this->mCoroutineGroups.end())
                {
                    if(iter->second->SubCount())
                    {
                        this->mCoroutineGroups.erase(iter);
                    }
                }
            }
		}
		this->mCorPool.Push(coroutine);
#ifdef __COROUTINE_ASM__
		
#elif _WIN32
		SwitchToFiber(this->mMainCoroutine->mContextStack);
#else
		setcontext(&this->mMainCoroutine->mCorContext);
#endif
	}

#ifdef __COROUTINE_ASM__
	void CoroutineComponent::SaveStack(unsigned int id)
	{
		Coroutine * coroutine = this->GetCoroutine(id);
		if (coroutine != nullptr)
		{
			coroutine->mStack.clear();
			char * top = this->mSharedStack.top;
			coroutine->mStackSize = top - (char*)coroutine->mCorContext;
			coroutine->mStack.append((char *)coroutine->mCorContext, coroutine->mStackSize);
		}
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
		if (!this->mLastQueues1.empty())
		{
			std::swap(this->mLastQueues1, this->mLastQueues2);
			while (!this->mLastQueues2.empty())
			{
				this->Resume(this->mLastQueues2.front());
				this->mLastQueues2.pop();
			}
		}
	}

    void CoroutineComponent::OnSecondUpdate()
    {

		size_t index = 1;
		long long size = 0;
		for (size_t index = 1; index < mCorPool.GetCorCount(); index++)
		{
			Coroutine * cor = mCorPool.Get(index);
            if(cor != nullptr)
            {
                //size += cor->mStack.size();
            }
		}

		double memory = size / 1024.0f / 1024.0f;
		GKDebugWarning("使用内存" << memory << "M" << "  协程总数 ：" << mCorPool.GetCorCount()
			<< "平均使用内存 ：" << size / mCorPool.GetCorCount());
    }
}
