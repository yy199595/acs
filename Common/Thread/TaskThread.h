#pragma once

#include "TaskProxy.h"
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <Define/CommonDef.h>
#include <Other/MultiThreadQueue.h>
namespace Sentry
{
	class StaticMethod;
    class TaskPoolComponent;
    enum ThreadState
    {
        Idle,
        Run,
    };

    class IThread
    {
    public:
        IThread(TaskPoolComponent *taskComponent);

		virtual ~IThread() = default;

    public:
		virtual int Start() = 0;

        void Stop() { this->mIsClose = true; }	

       

        std::thread::id GetThreadId() { return this->mThreadId; }

        bool IsCurrentThread() { return std::this_thread::get_id() == this->mThreadId; }
	protected:
		virtual void Update() = 0;
    protected:
		bool mIsClose;
		std::thread::id mThreadId;
        TaskPoolComponent *mTaskComponent;
    };

    class TaskThread : public IThread
    {
    public:
        TaskThread(TaskPoolComponent * taskComponent);

    public:
		int Start() final;
        void AddTask(TaskProxy * task);
        ThreadState GetTaskState() { return this->mTaskState; }
        bool IsRunning() { return this->mTaskState == ThreadState::Run; }
	private:
		void Update() final;
    private:
		std::thread * mThread;
        std::mutex mThreadLock;
        ThreadState mTaskState;
		std::queue<unsigned int> mFinishTasks;
        std::condition_variable mThreadVariable;
        MultiThreadQueue<TaskProxy *> mWaitInvokeTask;
    };

    class NetWorkThread : public IThread
    {
    public:
        NetWorkThread(TaskPoolComponent * taskComponent, class StaticMethod * method = nullptr);
    public:
		int Start();
        void AddTask(TaskProxy * task);
		void AddTask(StaticMethod * task);
		
		AsioContext & GetContext() { return *mAsioContext; }
	public:
		template<typename F, typename T, typename ... Args>
		void AddTask(F && f, T * o, Args &&... args) {
			this->AddTask(NewMethodProxy(std::forward<F>(f), o, std::forward<Args>(args)...));
		}
	protected:
		void Update() final;
    private:		
		AsioWork * mAsioWork;
		std::thread * mThread;
		AsioContext * mAsioContext;
        StaticMethod * mMethodProxy;
        MultiThreadQueue<TaskProxy *> mWaitInvokeTask;
		MultiThreadQueue<StaticMethod *> mWaitInvokeMethod;
    };

	class MainTaskScheduler : public IThread
	{
	public:
		MainTaskScheduler(StaticMethod * method);
	public:
		int Start() final;
		void AddMainTask(StaticMethod * task);		

		template<typename F, typename T, typename ... Args>
		void AddMainTask(F && f, T * o, Args &&... args) {
			this->AddMainTask(NewMethodProxy(std::forward<F>(f), o, std::forward<Args>(args)...));
		}
	private:
		void Update() final;
	private:
		StaticMethod * mMainMethod;
		MultiThreadQueue<StaticMethod *> mTaskQueue;
	};
}// namespace Sentry