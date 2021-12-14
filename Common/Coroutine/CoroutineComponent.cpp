#include"CoroutineComponent.h"
#include<memory.h>
#include<Core/App.h>
#include"Coroutine.h"
#include<Util/NumberHelper.h>
#include<Timer/TimerComponent.h>
#include"Other/ElapsedTimer.h"
#ifdef JE_MALLOC
#include"jemalloc/jemalloc.h"
#endif
using namespace std::chrono;
namespace GameKeeper
{
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
	}

    CoroutineComponent::CoroutineComponent()
    {
        this->mMainCoroutine = this->mCorPool.Pop();
        for(Stack & stack : this->mSharedStack)
        {
            stack.co = 0;
            stack.size = STACK_SIZE;
            stack.p = new char[STACK_SIZE];
            stack.top = (char *)stack.p + STACK_SIZE;
        }
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
        for (int index = 0; index < 100; index++)
        {
            this->StartCoroutine(&CoroutineComponent::Test, this, index);
        }
        return true;
    }

    void CoroutineComponent::Test(int index)
    {
        ElapsedTimer timer;
        for (int x = 0; x < 10; x++)
        {
            this->WaitForSleep(10 + 10 * index + 10 * x);
        }
        LOG_WARN("[" << index << "] use time = " << timer.GetSecond() << "s");
    }


    void CoroutineComponent::WaitForSleep(long long ms)
    {
        StaticMethod * sleepMethod = NewMethodProxy(
                &CoroutineComponent::Resume, this, this->mCurrentCorId);
        this->mTimerManager->AddTimer(ms, sleepMethod);
        this->WaitForYield();
    }

	void CoroutineComponent::ResumeCoroutine(Coroutine * co)
	{    
		co->mLastCoroutineId = this->mCurrentCorId;
        this->mCurrentCorId = co->mCoroutineId;


        co->mState = CorState::Running;
		Stack & stack = mSharedStack[co->sid];

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
             }   
        }
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
		tb_context_jump(this->mMainCoroutine->mContext, logicCoroutine);
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
        delete coroutine->mFunction;
		this->mCorPool.Push(coroutine);
	}

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
        memcpy(coroutine->mStack.p, coroutine->mContext, size);
    }

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
