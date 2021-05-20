#include"CoroutineManager.h"
#include<chrono>
#include<memory.h>
#include"Coroutine.h"
#include"CoroutineEvent.h"
#include<Core/Applocation.h>
#include<Util/NumberHelper.h>
#include<Timer/CorSleepTimer.h>
#include<Manager/TimerManager.h>
#include<Manager/NetWorkManager.h>
#include<Manager/ActionManager.h>
#include<NetWork/NetWorkRetAction.h>
using namespace std::chrono;

namespace SoEasy
{
#ifdef _WIN32
#include<Windows.h>
	void __stdcall WinEntry(LPVOID manager)
#else
	void LinuxEntry(void * manager)
#endif
	{
		CoroutineManager * pCoroutineMgr = (CoroutineManager*)manager;
		if (pCoroutineMgr != nullptr)
		{
			Coroutine * pCoroutine = pCoroutineMgr->GetCoroutine();
			if (pCoroutine != nullptr)
			{
				pCoroutine->mBindFunc();
				pCoroutineMgr->Destory(pCoroutine->mCoroutineId);
			}
		}
	}

	CoroutineManager::CoroutineManager()
	{
#ifdef _WIN32
		this->mMainCoroutineStack = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
#endif
	}

	bool CoroutineManager::OnInit()
	{		
		mCoroutinePoolMaxSize = CoroutinePoolMaxCount;
		this->GetConfig().GetValue("CoroutinePoolMaxSize", mCoroutinePoolMaxSize);
		SayNoAssertRetFalse_F(this->mTimerManager = this->GetManager<TimerManager>());

		for (int index = 0; index < mCoroutinePoolMaxSize; index++)
		{
			Coroutine * coroutine = new Coroutine();

			coroutine->mStackSize = 0;
			coroutine->mScheduler = this;
			coroutine->mCoroutineId = 0;
			coroutine->mContextStack = nullptr;
			this->mCoroutinePool.push(coroutine);
		}
		SayNoDebugLog("create " << mCoroutinePoolMaxSize << " coroutine");
		return true;
	}

	void CoroutineManager::OnInitComplete()
	{	
			
	}

	long long CoroutineManager::Start(const std::string & name, CoroutineAction func)
	{
		long long id = this->Create(name, func);
		this->Resume(id);
		return id;
	}

	long long CoroutineManager::Create(const std::string & name, CoroutineAction func)
	{
		long long id = NumberHelper::Create();
		Coroutine *	pCoroutine = nullptr;
		if (!this->mCoroutinePool.empty())
		{
			pCoroutine = this->mCoroutinePool.front();
			this->mCoroutinePool.pop();
		}
		else
		{
			pCoroutine = new Coroutine();
			pCoroutine->mStackSize = 0;
			pCoroutine->mContextStack = nullptr;		
		}
		pCoroutine->mBindFunc = func;
		pCoroutine->mCoroutineId = id;
		pCoroutine->mCoroutineName = name;
		pCoroutine->mState = CorState::Ready;
		this->mCoroutineMap.insert(std::make_pair(id, pCoroutine));
		return pCoroutine->mCoroutineId;
	}

	void CoroutineManager::Sleep(long long ms)
	{
		Coroutine * pCoroutine = this->GetCoroutine();
		if (pCoroutine != nullptr)
		{
			const long long id = pCoroutine->mCoroutineId;
			this->mTimerManager->CreateTimer<CorSleepTimer>(this, id, ms);
			this->YieldReturn();
		}
	}

	void CoroutineManager::Resume(long long id)
	{
		if (this->mCurrentCorId != 0)
		{
			SayNoDebugFatal("logic fail");
			return;
		}
		Coroutine * pCoroutine = this->GetCoroutine(id);
		if (pCoroutine != nullptr)
		{
			if (pCoroutine->mState == CorState::Ready)
			{
				this->mCurrentCorId = id;
#ifdef _WIN32
				pCoroutine->mStackSize = STACK_SIZE;
				pCoroutine->mContextStack = CreateFiber(STACK_SIZE, WinEntry, this);
				SwitchToFiber(pCoroutine->mContextStack);
#else
				getcontext(&pCoroutine->mCorContext);
				pCoroutine->mState = CorState::Running;
				pCoroutine->mCorContext.uc_stack.ss_size = STACK_SIZE;
				pCoroutine->mCorContext.uc_link = &this->mMainContext;
				pCoroutine->mCorContext.uc_stack.ss_sp = this->mSharedStack;
				makecontext(&pCoroutine->mCorContext, (void(*)(void))LinuxEntry, 1, this);
				swapcontext(&this->mMainContext, &pCoroutine->mCorContext);
#endif
			}
			else if (pCoroutine->mState == CorState::Suspend)
			{
				this->mCurrentCorId = id;
				pCoroutine->mState = CorState::Running;
#ifdef _WIN32
				SwitchToFiber(pCoroutine->mContextStack);
#else
				void * start = this->mSharedStack + STACK_SIZE - pCoroutine->mStackSize;
				memcpy(start, pCoroutine->mContextStack, pCoroutine->mStackSize);
				swapcontext(&this->mMainContext, &pCoroutine->mCorContext);
#endif
			}
		}
	}

	void CoroutineManager::YieldReturn()
	{
		Coroutine * pCoroutine = this->GetCoroutine();
		if (pCoroutine != nullptr)
		{
			this->mCurrentCorId = 0;
			pCoroutine->mState = CorState::Suspend;
#ifdef _WIN32
			SwitchToFiber(this->mMainCoroutineStack);
#else
			this->SaveStack(pCoroutine, this->mSharedStack + STACK_SIZE);
			swapcontext(&pCoroutine->mCorContext, &this->mMainContext);
#endif
		}
	}

	Coroutine * CoroutineManager::GetCoroutine()
	{
		auto iter = this->mCoroutineMap.find(this->mCurrentCorId);
		return iter != this->mCoroutineMap.end() ? iter->second : nullptr;
	}

	Coroutine * CoroutineManager::GetCoroutine(long long id)
	{
		auto iter = this->mCoroutineMap.find(id);
		return iter != this->mCoroutineMap.end() ? iter->second : nullptr;
	}

	void CoroutineManager::Destory(long long id)
	{
		auto iter = this->mCoroutineMap.find(id);
		if (iter != this->mCoroutineMap.end())
		{
			this->mCurrentCorId = 0;
			Coroutine * cor = iter->second;
			this->mCoroutineMap.erase(iter);
			mDestoryCoroutine.push(cor);
#ifdef _WIN32	
			SwitchToFiber(this->mMainCoroutineStack);
#else
			setcontext(&mMainContext);
#endif
		}
	}

	long long CoroutineManager::GetNowTime()
	{
		auto timeNow = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		return timeNow.count();
	}

	void CoroutineManager::SaveStack(Coroutine * cor, char * top)
	{
		char dummy = 0;
		size_t size = top - &dummy;
		if (cor->mStackSize < size)
		{
			free(cor->mContextStack);
			cor->mContextStack = malloc(size);
		}
		cor->mStackSize = size;
		memcpy(cor->mContextStack, &dummy, cor->mStackSize);
	}

	
	void CoroutineManager::OnFrameUpdate(float t)
	{
		while (!this->mDestoryCoroutine.empty())
		{
			Coroutine * pCoroutine = this->mDestoryCoroutine.front();
			this->mDestoryCoroutine.pop();
			if (this->mCoroutinePool.size() >= mCoroutinePoolMaxSize)
			{
				delete pCoroutine;
				continue;
			}
			else
			{
#ifdef _WIN32
				if (pCoroutine->mContextStack != nullptr)
				{
					DeleteFiber(pCoroutine->mContextStack);
					pCoroutine->mContextStack = nullptr;
					pCoroutine->mStackSize = 0;
				}
#endif
				this->mCoroutinePool.push(pCoroutine);
			}
		}
	}
}