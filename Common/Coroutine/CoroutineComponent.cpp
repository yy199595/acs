#include"CoroutineComponent.h"
#include<memory.h>
#include<Core/App.h>
#include"Coroutine.h"
#include<Util/NumberHelper.h>
#include<Timer/TimerComponent.h>
#include<Timer/CorSleepTimer.h>
#include<fstream>
#ifdef JE_MALLOC
#include"jemalloc/jemalloc.h"
#endif
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

		Coroutine * logicCoroutine = pCoroutineMgr->GetCurCoroutine();
		Coroutine * mainCoroutine = pCoroutineMgr->GetMainCoroutine();
		Coroutine * fromCoroutine = (Coroutine*)parame.priv;
		assert(mainCoroutine == fromCoroutine);
        ((Coroutine*)parame.priv)->mCorContext = parame.ctx;
        pCoroutineMgr->GetCurCoroutine()->mFunction->run();
        tb_context_jump(parame.ctx, nullptr);
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
        this->mMainCoroutine = this->mCorPool.Pop();
#ifdef __COROUTINE_ASM__
        for(Stack & stack : this->mSharedStack)
        {
            stack.p = nullptr;
            stack.top = nullptr;
        }
#elif _WIN32
        this->mMainCoroutine->mContextStack = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
#else
        this->mTop = this->mSharedStack + STACK_SIZE;
#endif
    }

	CoroutineComponent::~CoroutineComponent() = default;

    bool CoroutineComponent::Awake()
    {
        this->mCurrentCorId = 0;
        for (int index = 10; index < 20; index++)
        {
            this->StartCoroutine(&CoroutineComponent::SleepTest, this, index);
			this->StartCoroutine(&CoroutineComponent::SleepTest1, this, index);
			this->StartCoroutine(&CoroutineComponent::SleepTest2, this, index);
        }
       GKAssertRetFalse_F(this->mTimerManager = this->GetComponent<TimerComponent>());
//        for(int index = 10; index < 1000; index++)
//        {
//            StaticMethod * sleepMethod = NewMethodProxy(&CoroutineComponent::SleepTest, this, index * 100);
//            this->mTimerManager->AddTimer(index * 100, sleepMethod);
//        }
        return true;
    }

    void CoroutineComponent::SleepTest(int ms)
    {
        this->Sleep(ms * 10);
        GKDebugWarning(TimeHelper::GetMilTimestamp() << "  " << ms);
    }

	void CoroutineComponent::SleepTest1(int ms)
	{
		long long num1 = 10;
		std::string str;
		int ni;
		this->Sleep(ms * 10);
	}
	void CoroutineComponent::SleepTest2(int ms)
	{
		float num;
		db::UserAccountData userData;
		this->Sleep(ms * 20);
	}

    void CoroutineComponent::Sleep(long long ms)
    {
        GKAssertRet_F(this->IsInLogicCoroutine());
        StaticMethod * sleepMethod = NewMethodProxy(&CoroutineComponent::Resume, this, this->mCurrentCorId);
        this->mTimerManager->AddTimer(ms, sleepMethod);
        this->YieldReturn();
    }

	void CoroutineComponent::ResumeCoroutine(unsigned int id)
	{
        this->mCorStack.push(this->mCurrentCorId);
        Coroutine * logicCoroutine = this->GetCoroutine(id);
        GKAssertRet(logicCoroutine, "not find coroutine : " << id);
        this->mCurrentCorId = logicCoroutine->mCoroutineId;
#ifdef __COROUTINE_ASM__
		Stack & stack = mSharedStack[logicCoroutine->sid];
		if (stack.p == nullptr)
		{
            stack.size = STACK_SIZE;
            stack.p = new char[STACK_SIZE];
			stack.co = logicCoroutine->mCoroutineId;
			stack.top = (char *)stack.p + STACK_SIZE;
		}
        logicCoroutine->mState = CorState::Running;
        if(logicCoroutine->mCorContext == nullptr)
        {			
            logicCoroutine->mCorContext =
                    tb_context_make(stack.p, stack.size, MainEntry);           
        }
        else
        {			
			Stack & ss = logicCoroutine->mStack;
			memcpy(logicCoroutine->mCorContext, ss.p, ss.size);
        }
		tb_context_from_t from = tb_context_jump(logicCoroutine->mCorContext, this->mMainCoroutine);

        this->mCurrentCorId = this->mCorStack.top();
        this->mCorStack.pop();

		if (stack.co != logicCoroutine->mCoroutineId)
		{
			this->SaveStack(stack.co);
			stack.co = logicCoroutine->mCoroutineId;
		}

        if (from.priv != nullptr)
        {
             auto lastCoroutine = (Coroutine*)from.priv;
             if(lastCoroutine != nullptr)
             {
                 lastCoroutine->mCorContext = from.ctx;
                 lastCoroutine->mState = CorState::Suspend;
                 //this->SaveStack(lastCoroutine->mCoroutineId);
             }
            return;
        }
        this->Destory(logicCoroutine);
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
		Coroutine *logicCoroutine = this->GetCurCoroutine();
		if (logicCoroutine == nullptr)
		{
			GKDebugFatal("not find coroutine context");
			return;
		}
		logicCoroutine->mState = CorState::Suspend;
#ifdef __COROUTINE_ASM__
        //this->SaveStack(logicCoroutine->mCoroutineId);
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
        GKAssertRet_F(id != 0);
        Coroutine *logicCoroutine = this->GetCoroutine(id);
        GKAssertRet(logicCoroutine, "not find coroutine id " << id);

        //this->ResumeCoroutine(id);
        this->mResumeCoroutines.push(id);
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

    Coroutine *CoroutineComponent::GetCoroutine(unsigned int id)
    {
		return this->mCorPool.Get(id);
    }

    Coroutine * CoroutineComponent::GetCurCoroutine()
    {
        if(this->mCurrentCorId == 0)
        {
            return nullptr;
        }
        return this->mCorPool.Get(this->mCurrentCorId);
    }

	void CoroutineComponent::Destory(Coroutine * coroutine)
	{
		if (coroutine->mGroupId != 0)
		{
			auto iter = this->mCoroutineGroups.find(coroutine->mGroupId);
			if (iter != this->mCoroutineGroups.end())
			{
				if (iter->second->SubCount())
				{
					delete iter->second;
					this->mCoroutineGroups.erase(iter);
				}
			}
		}
        //delete coroutine->mFunction;
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
        Coroutine *coroutine = this->GetCoroutine(id);
        if(coroutine == nullptr)
        {
            return;
        }
        char *top = this->mSharedStack[coroutine->sid].top;
        size_t size = top - (char *) coroutine->mCorContext;
        if(coroutine->mStack.size < size)
        {
#ifdef JE_MALLOC
            je_free(coroutine->mStack.p);
            coroutine->mStack.p = (char *)je_malloc(size);
#else
			free(coroutine->mStack.p);
			coroutine->mStack.p = (char *)malloc(size);
#endif
        }
        coroutine->mStack.size = size;
        GKDebugError("save stack size = " << size << "  id = " << id);
        memcpy(coroutine->mStack.p, coroutine->mCorContext, size);
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
		while (!this->mResumeCoroutines.empty())
		{
			this->ResumeCoroutine(this->mResumeCoroutines.front());
			this->mResumeCoroutines.pop();
		}
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
//        size_t size = this->mCorPool.GetMemorySize();
//		double memory = size / 1024.0f;
//		GKDebugWarning("使用内存" << memory << "kb" << "  协程总数 ：" << mCorPool.GetCorCount()
//			<< "平均使用内存 ：" << size / mCorPool.GetCorCount());
    }
}
