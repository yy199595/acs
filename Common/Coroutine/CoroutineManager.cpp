#include"CoroutineManager.h"
#include<chrono>
#include<memory.h>
#include"Coroutine.h"
#include<Util/NumberHelper.h>
#include<Timer/TimerManager.h>
#include<Timer/CorSleepTimer.h>
#include<NetWork/NetWorkRetAction.h>

using namespace std::chrono;
#ifdef _WIN32
#include<Windows.h>
#endif
namespace Sentry
{
#ifdef SentryAsmCoroutine
	void AsmEntry(tb_context_from_t parame)
	{
		CoroutineManager *pCoroutineMgr = (CoroutineManager*)parame.priv;
#else
#if _WIN32
	void __stdcall WinEntry(LPVOID manager)
	{
		CoroutineManager *pCoroutineMgr = (CoroutineManager *)manager;
#else
	void LinuxEntry(void *manager)
	{
		CoroutineManager *pCoroutineMgr = (CoroutineManager *)manager;
#endif
#endif
		Coroutine *pCoroutine = pCoroutineMgr->GetCoroutine();
		if (pCoroutine != nullptr)
		{
			pCoroutine->mFunction->run();
			pCoroutineMgr->Destory(pCoroutine);
		}
	}

    CoroutineManager::CoroutineManager()
		:mCorPool(100)
    {
#ifdef SentryAsmCoroutine
		this->mMainCoroutine = this->mCorPool.Pop();
		this->mMainCoroutine->mStackSize = STACK_SIZE;
		this->mMainCoroutine->mCorContext = this->mSharedStack;
#elif _WIN32
		this->mMainCoroutine = this->mCorPool.Pop();
		this->mMainCoroutine->mCorContext = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
#endif
		this->mCurrentCorId = 0;
    }

    bool CoroutineManager::OnInit()
    {
		SayNoAssertRetFalse_F(this->mTimerManager = this->GetManager<TimerManager>());
        return true;
    }

    void CoroutineManager::OnInitComplete()
    {
		this->Start(&CoroutineManager::Loop, this);
		this->Start(&CoroutineManager::Loop2, this);
    }

    void CoroutineManager::Sleep(long long ms)
    {
        Coroutine *pCoroutine = this->GetCoroutine();
        if (pCoroutine != nullptr)
        {
            const long long id = pCoroutine->mCoroutineId;
            this->mTimerManager->CreateTimer<CorSleepTimer>(this, id, ms);
            this->YieldReturn();
        }
    }

    void CoroutineManager::Resume(unsigned int id)
    {
        if (this->mCurrentCorId != 0)
        {
            SayNoDebugFatal("logic error");
            return;
        }
        Coroutine *pCoroutine = this->GetCoroutine(id);
        if (pCoroutine != nullptr)
        {
            if (pCoroutine->mState == CorState::Ready)
            {
                this->mCurrentCorId = id;
				pCoroutine->mStackSize = STACK_SIZE;
#ifdef SentryAsmCoroutine
				char * stack = (char*)this->mMainCoroutine->mCorContext;
				pCoroutine->mCorContext = tb_context_make(stack, STACK_SIZE, AsmEntry);		
				tb_context_jump(pCoroutine->mCorContext, this);
#elif _WIN32
                pCoroutine->mCorContext = CreateFiber(STACK_SIZE, WinEntry, this);
                SwitchToFiber(pCoroutine->mCorContext);
#else
                getcontext(&pCoroutine->mCorContext);
                pCoroutine->mState = CorState::Running;
                pCoroutine->mCorContext.uc_stack.ss_size = STACK_SIZE;
                pCoroutine->mCorContext.uc_link = &this->mMainContext;
                pCoroutine->mCorContext.uc_stack.ss_sp = this->mSharedStack;
                makecontext(&pCoroutine->mCorContext, (void (*)(void)) LinuxEntry, 1, this);
                swapcontext(&this->mMainContext, &pCoroutine->mCorContext);
#endif
            } 
			else if (pCoroutine->mState == CorState::Suspend)
            {
                this->mCurrentCorId = id;
                pCoroutine->mState = CorState::Running;
#ifdef SentryAsmCoroutine
				tb_context_jump(pCoroutine->mCorContext, this);
#elif _WIN32
                SwitchToFiber(pCoroutine->mCorContext);
#else
                void *start = this->mSharedStack + STACK_SIZE - pCoroutine->mStackSize;
                memcpy(start, pCoroutine->mContextStack, pCoroutine->mStackSize);
                swapcontext(&this->mMainContext, &pCoroutine->mCorContext);
#endif
            }
        }
    }

	unsigned int CoroutineManager::StartCoroutine(Closure * func)
	{
		Coroutine * coroutine = this->mCorPool.Pop();
		if (coroutine != nullptr)
		{
#ifndef SentryAsmCoroutine
	#ifdef _WIN32
			if (coroutine->mCorContext != nullptr)
			{
				DeleteFiber(coroutine->mCorContext);
				coroutine->mStackSize = 0;
				coroutine->mCorContext = nullptr;
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

	void CoroutineManager::Loop()
	{
		while (true)
		{
			this->Sleep(1000);
			SayNoDebugFatal("================");
		}
	}

	void CoroutineManager::Loop2()
	{
		while (true)
		{
			this->Sleep(1000);
			SayNoDebugWarning("================");
		}
	}

	void CoroutineManager::YieldReturn()
    {
        Coroutine *pCoroutine = this->GetCoroutine();
        if (pCoroutine == nullptr)
        {
            SayNoDebugFatal("not find coroutine context");
            return;
        }
		if (pCoroutine->mCoroutineId == 0)
		{
			SayNoDebugFatal("try yield main coroutine");
			return;
		}
        this->mCurrentCorId = 0;
        pCoroutine->mState = CorState::Suspend;
#ifdef SentryAsmCoroutine
		this->SaveStack(pCoroutine, this->mSharedStack + STACK_SIZE);
		tb_context_jump(pCoroutine->mCorContext, this->mMainCoroutine->mCorContext);
#elif _WIN32
        SwitchToFiber(this->mMainCoroutine->mCorContext);
#else
        this->SaveStack(pCoroutine, this->mSharedStack + STACK_SIZE);
        swapcontext(&pCoroutine->mCorContext, &this->mMainContext);
#endif
    }

    Coroutine *CoroutineManager::GetCoroutine()
    {
		return this->mCorPool.Get(this->mCurrentCorId);     
    }

    Coroutine *CoroutineManager::GetCoroutine(unsigned int id)
    {
		return this->mCorPool.Get(id);
    }

	void CoroutineManager::Destory(Coroutine * coroutine)
	{
		this->mCurrentCorId = 0;
		this->mCorPool.Push(coroutine);
#ifdef SentryAsmCoroutine
		tb_context_jump(this->mMainCoroutine->mCorContext, this);
#elif _WIN32
		SwitchToFiber(this->mMainCoroutine->mCorContext);
#else
		setcontext(&mMainContext);
#endif
	}

    long long CoroutineManager::GetNowTime()
    {
        auto timeNow = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        return timeNow.count();
    }

    void CoroutineManager::SaveStack(Coroutine *cor, char *top)
    {
        char dummy = 0;
        size_t size = top - &dummy;
        if (cor->mStackSize < size)
        {
            free(cor->mCorContext);
            cor->mCorContext = malloc(size);
        }
        cor->mStackSize = size;
        memcpy(cor->mCorContext, &dummy, cor->mStackSize);
    }


    void CoroutineManager::OnFrameUpdate(float t)
    {

    }
}