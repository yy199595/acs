﻿#include"TaskComponent.h"
#include<memory.h>
#include<Core/App.h>
#include"Coroutine.h"
#include<Util/Guid.h>
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
		TaskComponent * taskComponent = (TaskComponent*)parame.priv;
        if(taskComponent != nullptr) {
            taskComponent->RunTask(parame.ctx);
        }
	}

    void TaskComponent::RunTask(tb_context_t context)
    {
        this->mMainContext = context;
        if (this->mRunCoroutine != nullptr)
        {
            this->mRunCoroutine->Invoke();
            this->Destory(this->mRunCoroutine);
        }
        tb_context_jump(this->mMainContext, nullptr);
    }

    bool TaskComponent::Awake()
    {
        this->mRunCoroutine = nullptr;
        for(Stack & stack : this->mSharedStack)
        {
            stack.co = 0;
            stack.size = STACK_SIZE;
            stack.p = new char[STACK_SIZE];
            stack.top = (char *)stack.p + STACK_SIZE;
        }
        return true;
    }

    bool TaskComponent::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mTimerManager = this->GetComponent<TimerComponent>());
        this->Start([this]() {
            ElapsedTimer timer;
            std::vector<Coroutine *> tasks;
            for (int index = 0; index < 100; index++) {
                tasks.push_back(this->Start(&TaskComponent::Test, this, index));
            }
            this->WhenAll(tasks);
            LOG_ERROR("use time = " << timer.GetSecond() << "s");
        });
        return true;
    }

    void TaskComponent::Test(int index)
    {
        ElapsedTimer timer;
        for (int x = 0; x < 10; x++)
        {
            this->Sleep(10 + 5 * index + x);
            this->Start([this, x]() {
                this->Sleep(100 + x * 100);
                //LOG_ERROR(__FUNCTION__ << "  " << __LINE__);
            });
        }
        //LOG_WARN("[" << index << "] use time = " << timer.GetSecond() << "s");
    }

    void TaskComponent::WhenAny(Coroutine *coroutine)
    {
        if(this->mRunCoroutine == nullptr)
        {
            LOG_FATAL("please in coroutine wait");
            return;
        }
        coroutine->mGroup = new CoroutineGroup(1);
        this->Yield();
    }

    void TaskComponent::WhenAll(std::vector<Coroutine *> &coroutines)
    {
        if(this->mRunCoroutine == nullptr)
        {
            LOG_FATAL("please in coroutine wait");
            return;
        }
        auto group = new CoroutineGroup(coroutines.size());
        for(auto coroutine : coroutines)
        {
            coroutine->mGroup = group;
        }
        this->Yield();
    }


    void TaskComponent::Sleep(long long ms)
    {
        unsigned int id = this->mRunCoroutine->mCoroutineId;
        StaticMethod * sleepMethod = NewMethodProxy(
                &TaskComponent::Resume, this, id);
        this->mTimerManager->AddTimer(ms, sleepMethod);
        this->Yield();
    }

	void TaskComponent::ResumeCoroutine(Coroutine * co)
    {
        co->mState = CorState::Running;
        Stack &stack = mSharedStack[co->sid];
        if (co->mContext == nullptr)
        {
            if (stack.co != co->mCoroutineId)
            {
                this->SaveStack(stack.co);
                stack.co = co->mCoroutineId;
            }
            this->mRunCoroutine->mContext = tb_context_make(stack.p, stack.size, MainEntry);
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
            this->mRunCoroutine->mContext = from.ctx;
        }
    }

	bool TaskComponent::Yield()
	{
        LOG_CHECK_RET_FALSE(this->mRunCoroutine);
        LOG_CHECK_RET_FALSE(this->mRunCoroutine->mState == CorState::Running);

        this->mRunCoroutine->mSwitchCount++;
		this->mRunCoroutine->mState = CorState::Suspend;
		tb_context_jump(this->mMainContext, this->mRunCoroutine);
        return true;
	}

	void TaskComponent::Resume(unsigned int id)
    {
        Coroutine *logicCoroutine = this->GetCoroutine(id);
        LOG_CHECK_RET(logicCoroutine);
        if(logicCoroutine->mState == CorState::Ready
            || logicCoroutine->mState == CorState::Suspend)
        {
            this->mResumeCoroutines.push(logicCoroutine);
            return;
        }
        LOG_FATAL("coroutine " << id << " state is not resume");
    }

	Coroutine * TaskComponent::CreateCoroutine(StaticMethod *func)
	{
		Coroutine * coroutine = this->mCorPool.Pop();
		if (coroutine != nullptr)
		{
			coroutine->mFunction = func;
			coroutine->mState = CorState::Ready;
		}
		return coroutine;
	}

	bool TaskComponent::Yield(unsigned int & mCorId)
    {
        LOG_CHECK_RET_FALSE(mRunCoroutine);
        mCorId = this->mRunCoroutine->mCoroutineId;
        return this->Yield();
    }

    Coroutine *TaskComponent::GetCoroutine(unsigned int id)
    {
		return this->mCorPool.Get(id);
    }

	void TaskComponent::Destory(Coroutine * coroutine)
	{
        delete coroutine->mFunction;
		this->mCorPool.Push(coroutine);
	}

	void TaskComponent::SaveStack(unsigned int id)
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

	void TaskComponent::OnSystemUpdate()
    {
		while (!this->mResumeCoroutines.empty())
		{
            Coroutine * coroutine = this->mResumeCoroutines.front();
            if(coroutine != nullptr)
            {
                this->mRunCoroutine = coroutine;
                this->ResumeCoroutine(coroutine);
            }
            this->mResumeCoroutines.pop();
            this->mRunCoroutine = nullptr;
        }
    }
	void TaskComponent::OnLastFrameUpdate()
	{
        while(!this->mLastQueues.empty())
        {
            unsigned int id = this->mLastQueues.front();
            Coroutine *coroutine = this->GetCoroutine(id);
            if (coroutine != nullptr) {
                this->mResumeCoroutines.push(coroutine);
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