#pragma once

#include "TaskProxy.h"
#include<condition_variable>
#include<mutex>
#include<queue>
#include<thread>
#include<atomic>
#include<Define/CommonLogDef.h>
#include<Other/MultiThreadQueue.h>
#include<Method/MethodProxy.h>
namespace GameKeeper
{
	class StaticMethod;
    class ThreadPoolComponent;
    enum ThreadState
    {
        Idle,
        Run,
    };

    class IThread
    {
    public:
        explicit IThread(std::string  name, ThreadPoolComponent *taskComponent);

		virtual ~IThread() = default;

    public:

        void Stop();

        void HangUp();

		virtual int Start() = 0;

        bool IsWork() const { return this->mIsWork; };

        const std::string & GetName() const { return this->mName;}

        long long GetLastOperTime() const { return this->mLastOperTime; };

        const std::thread::id & GetThreadId() const { return this->mThreadId;};

        bool IsCurrentThread() const { return std::this_thread::get_id() == this->mThreadId; }

	protected:
		virtual void Update() = 0;
    protected:
		bool mIsClose;		
        std::mutex mThreadLock;
        long long mLastOperTime;
        std::thread::id mThreadId;
		std::atomic_bool mIsWork;
        ThreadPoolComponent *mTaskComponent;
        std::condition_variable mThreadVariable;
    private:
        std::string mName;
    };

    class TaskThread : public IThread
    {
    public:
        explicit TaskThread(ThreadPoolComponent * taskComponent);

    public:
		int Start() final;

        void AddTask(TaskProxy * task);
	private:
		void Update() final;
    private:
		std::thread * mThread;
        ThreadState mTaskState;
        MultiThreadQueue<TaskProxy *> mWaitInvokeTask;
    };

    class NetWorkThread : public IThread
    {
    public:
        explicit NetWorkThread(ThreadPoolComponent * taskComponent);
    public:
		int Start() final;
		void AddTask(StaticMethod * task);
		
		AsioContext & GetContext() { return *mAsioContext; }

	public:
		template<typename F, typename T, typename ... Args>
		void Invoke(F && f, T * o, Args &&... args) {
			this->AddTask(NewMethodProxy(std::forward<F>(f), o, std::forward<Args>(args)...));
		}
	protected:
		void Update() final;
    private:		
		AsioWork * mAsioWork;
		std::thread * mThread;
		AsioContext * mAsioContext;
		MultiThreadQueue<StaticMethod *> mWaitInvokeMethod;
    };

	class MainTaskScheduler : public IThread
	{
	public:
		explicit MainTaskScheduler(StaticMethod * method);
	public:
		int Start() final;
		void Invoke(StaticMethod * task);

		template<typename F, typename T, typename ... Args>
		void Invoke(F && f, T * o, Args &&... args) {
            this->Invoke(NewMethodProxy(std::forward<F>(f), o, std::forward<Args>(args)...));
		}
	private:
		void Update() final;
	private:
		StaticMethod * mMainMethod;
		MultiThreadQueue<StaticMethod *> mTaskQueue;
	};
}// namespace GameKeeper