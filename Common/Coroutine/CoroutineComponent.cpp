#include"CoroutineComponent.h"
#include<memory.h>
#include<Core/App.h>
#include"Coroutine.h"
#include<Util/NumberHelper.h>
#include<Timer/TimerComponent.h>
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
		assert(mainCoroutine == parame.priv);
        ((Coroutine*)parame.priv)->mContext = parame.ctx;
		if (logicCoroutine != nullptr)
		{
			logicCoroutine->mFunction->run();
			pCoroutineMgr->Destory(logicCoroutine);
		}
        tb_context_jump(mainCoroutine->mContext, nullptr);
		
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
        return true;
    }

    bool CoroutineComponent::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mTimerManager = this->GetComponent<TimerComponent>());
        return true;
    }

//	void CoroutineComponent::Start()
//	{
//		long long t1 = TimeHelper::GetMilTimestamp();
//		auto group = this->NewCoroutineGroup();
//
//		for (int index = 0; index < 10000; index++)
//		{
//			group->Add(this->StartCoroutine(&CoroutineComponent::SleepTest, this));
//		}
//		group->AwaitAll();
//		LOG_ERROR("use time = " << TimeHelper::GetMilTimestamp() - t1);
//	}

    void CoroutineComponent::SleepTest()
    {
		for (int index = 0; index < 1000; index++)
		{
			this->Resume(this->mCurrentCorId);
			this->WaitForYield();
		}
    }

    void CoroutineComponent::WaitForSleep(long long ms)
    {
        LOG_CHECK_RET(this->IsInLogicCoroutine());
        StaticMethod * sleepMethod = NewMethodProxy(&CoroutineComponent::Resume, this, this->mCurrentCorId);
        this->mTimerManager->AddTimer(ms, sleepMethod);
        this->WaitForYield();
    }

	void CoroutineComponent::ResumeCoroutine(Coroutine * co)
	{    
		co->mLastCoroutineId = this->mCurrentCorId;
        this->mCurrentCorId = co->mCoroutineId;
#ifdef __COROUTINE_ASM__
		Stack & stack = mSharedStack[co->sid];
		if (stack.p == nullptr)
		{
            stack.size = STACK_SIZE;
            stack.co = co->mCoroutineId;
            stack.p = new char[STACK_SIZE];
			stack.top = (char *)stack.p + STACK_SIZE;
		}
        co->mState = CorState::Running;
        if(co->mContext == nullptr)
        {
            if(stack.co != this->mCurrentCorId)
            {
                this->SaveStack(stack.co);
                stack.co = this->mCurrentCorId;
            }
            co->mContext = tb_context_make(stack.p, stack.size, MainEntry);
        }
        else if(stack.co != this->mCurrentCorId)
        {
            this->SaveStack(stack.co);
            stack.co = this->mCurrentCorId;
            memcpy(co->mContext, co->mStack.p, co->mStack.size);
        }
		tb_context_from_t from = tb_context_jump(co->mContext, this->mMainCoroutine);
		this->mCurrentCorId = co->mLastCoroutineId;
        if (from.priv != nullptr)
        {
             auto lastCoroutine = (Coroutine*)from.priv;
             if(lastCoroutine != nullptr)
             {
                 lastCoroutine->mContext = from.ctx;
                 lastCoroutine->mState = CorState::Suspend;
                 //this->SaveStack(lastCoroutine->mCoroutineId);
             }   
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
            getcontext(&logicCoroutine->mContext);
            logicCoroutine->mState = CorState::Running;
            logicCoroutine->mContext.uc_stack.ss_size = STACK_SIZE;
            logicCoroutine->mContext.uc_stack.ss_sp = this->mSharedStack;
            logicCoroutine->mContext.uc_link = &this->mMainCoroutine->mContext;
            makecontext(&logicCoroutine->mContext, (void (*)(void)) MainEntry, 1, this);
            swapcontext(&this->mMainCoroutine->mContext, &logicCoroutine->mContext);
        }
        else if (logicCoroutine->mState == CorState::Suspend)
        {
            void *start = this->mSharedStack + STACK_SIZE - logicCoroutine->mStackSize;
            memcpy(start, logicCoroutine->mContextStack, logicCoroutine->mStackSize);
            swapcontext(&this->mMainCoroutine->mContext, &logicCoroutine->mContext);
        }
#endif
    }

	void CoroutineComponent::WaitForYield()
	{
		Coroutine *logicCoroutine = this->GetCurCoroutine();
		if (logicCoroutine == nullptr)
		{
			LOG_FATAL("not find coroutine context");
			return;
		}
        logicCoroutine->mSwitchCount++;
		logicCoroutine->mState = CorState::Suspend;
#ifdef __COROUTINE_ASM__
        //this->SaveStack(logicCoroutine->mCoroutineId);
		tb_context_jump(this->mMainCoroutine->mContext, logicCoroutine);
#elif _WIN32
		SwitchToFiber(this->mMainCoroutine->mContextStack);
#elif __linux__
		this->SaveStack(logicCoroutine, this->mTop);
		swapcontext(&logicCoroutine->mContext, &this->mMainCoroutine->mContext);
#endif
	}

	void CoroutineComponent::Resume(unsigned int id)
    {
        LOG_CHECK_RET(id != 0);
        Coroutine *logicCoroutine = this->GetCoroutine(id);
        LOG_CHECK_ERROR_RET(logicCoroutine, "not find coroutine id " << id);
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

	Coroutine * CoroutineComponent::CreateCoroutine(StaticMethod *func)
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
		}
		return coroutine;
	}


	void CoroutineComponent::WaitForYield(unsigned int & mCorId)
	{
		mCorId = this->mCurrentCorId;
        this->WaitForYield();
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
        //LOG_ERROR(coroutine->mCoroutineId << " switch count " << coroutine->mSwitchCount);
        delete coroutine->mFunction;
		this->mCorPool.Push(coroutine);
#ifdef __COROUTINE_ASM__

#elif _WIN32
		SwitchToFiber(this->mMainCoroutine->mContextStack);
#else
		setcontext(&this->mMainCoroutine->mContext);
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
        size_t size = top - (char *) coroutine->mContext;
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
        //LOG_ERROR("save stack size = " << size << "  id = " << id);
        memcpy(coroutine->mStack.p, coroutine->mContext, size);
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
            unsigned int id = this->mResumeCoroutines.front();
            Coroutine * coroutine = this->GetCoroutine(id);
            if(coroutine != nullptr)
            {
                this->ResumeCoroutine(coroutine);
            }
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
        /*size_t size = this->mCorPool.GetMemorySize();
		double memory = size / (1024.0f * 1024);
		LOG_WARN("使用内存" << memory << "mb" << "  协程总数 ：" << mCorPool.GetCorCount()
			<< "平均使用内存 ：" << size / mCorPool.GetCorCount());*/
    }
}
