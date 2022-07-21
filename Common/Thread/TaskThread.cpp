#include "TaskThread.h"
#include"Util/TimeHelper.h"
#include"Method/MethodProxy.h"
#include<memory>
#include<utility>
#include"Component/Scene/NetThreadComponent.h"

using namespace std::chrono;

namespace Sentry
{
	IThread::IThread(std::string  name)
		: mName(std::move(name)), mIsClose(false)
    {		
		this->mIsWork = true;
    }

    void IThread::HangUp()
    {
        if(this->mIsWork)
        {
            this->mIsWork = false;
            std::unique_lock<std::mutex> waitLock(this->mThreadLock);
            this->mThreadVariable.wait(waitLock);
        }
    }

    void IThread::Stop()
    {
        this->mIsClose = true;
        this->mThreadVariable.notify_one();
    }
#ifdef __ENABLE_OPEN_SSL__
	bool IAsioThread::LoadVeriftFile(const std::string & path)
	{
		this->mSslContext = new asio::ssl::context(asio::ssl::context::sslv23);
		if(path.empty())
		{
			this->mSslContext->set_default_verify_paths();
			return true;
		}
		try
		{
			this->mSslContext->load_verify_file(path);
			return true;
		}
		catch (asio::system_error & code)
		{
			printf("load ssl file error : %s\n", code.what());
			return false;
		}
	}
#endif
}

#ifndef ONLY_MAIN_THREAD
namespace Sentry
{
    NetWorkThread::NetWorkThread()
        : IAsioThread("net"),
		  mThread(new std::thread(std::bind(&NetWorkThread::Update, this)))
	{
		this->mThread->detach();
    }

	int NetWorkThread::Start()
	{
        this->mIsWork = true;
        this->mThreadVariable.notify_one();
		return 0;
	}

	void NetWorkThread::InvokeMethod(StaticMethod *task)
	{
        this->mWaitInvokeMethod.Push(task);
	}

    void NetWorkThread::Update()
    {
        this->mThreadId = std::this_thread::get_id();

        this->HangUp();
        asio::error_code err;
        StaticMethod * taskMethod = nullptr;
        std::chrono::milliseconds time(1);
        while(!this->mIsClose)
        {
            this->poll(err);
            this->mWaitInvokeMethod.Swap();
             while (this->mWaitInvokeMethod.Pop(taskMethod) && taskMethod != nullptr)
            {
                taskMethod->run();
                delete taskMethod;
				taskMethod = nullptr;
                this->mLastOperTime = Helper::Time::GetNowSecTime();
            }
            std::this_thread::sleep_for(time);
        }
    }
}
#endif
namespace Sentry
{
	MainTaskScheduler::MainTaskScheduler(StaticMethod * method)
		:IAsioThread("main"), mMainMethod(method)
	{
		this->mThreadId = std::this_thread::get_id();
	}

	int MainTaskScheduler::Start()
	{
        this->mIsWork = true;
		std::chrono::milliseconds time(1);
		while (!this->mIsClose)
		{
			this->Update();
			this_thread::sleep_for(time);			
		}
		return 0;
	}

	void MainTaskScheduler::Update()
    {
        this->poll();
        this->mTaskQueue.Swap();
        this->mMainMethod->run();
        StaticMethod *task = nullptr;
        while (this->mTaskQueue.Pop(task) && task != nullptr)
        {
            task->run();
            delete task;
            task = nullptr;
        }
        this->mLastOperTime = Helper::Time::GetNowSecTime();
    }
	
	void MainTaskScheduler::InvokeMethod(StaticMethod * task)
    {
        if (task != nullptr)
        {
            this->mTaskQueue.Push(task);
        }
    }
}

// namespace Sentry
