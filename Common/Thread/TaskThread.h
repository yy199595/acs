#pragma once

#include"IThreadTask.h"
#include<condition_variable>
#include<mutex>
#include<queue>
#include<thread>
#include<atomic>
#include"Define/ThreadQueue.h"
#include<Define/CommonLogDef.h>
#include<Method/MethodProxy.h>
#include<Other/DoubleQueue.h>

namespace Sentry
{
	class StaticMethod;
    enum ThreadState
    {
        Idle,
        Run,
    };

    class IThread
    {
    public:
        explicit IThread(std::string  name);

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
        std::condition_variable mThreadVariable;
    private:
        std::string mName;
    };

    class TaskThread : public IThread
    {
    public:
        explicit TaskThread();

    public:
		int Start() final;

        void AddTask(std::shared_ptr<IThreadTask> task);
	private:
		void Update() final;
    private:
		std::thread * mThread;
        MultiThread::ConcurrentQueue<std::shared_ptr<IThreadTask>> mWaitInvokeTask;
    };

	class IAsioThread : public asio::io_context, public IThread
    {
    public:
        IAsioThread(std::string name) :
				IThread(name), asio::io_context(1), mWork(*this) {}
        //virtual AsioContext & GetContext() = 0;
        virtual void InvokeMethod(StaticMethod * task) = 0;
    public:
        template<typename F, typename T, typename ... Args>
        void Invoke(F && f, T * o, Args &&... args) {
            this->InvokeMethod(NewMethodProxy(std::forward<F>(f), o, std::forward<Args>(args)...));
        }
	private:
		asio::io_context::work mWork;
    };
#ifndef ONLY_MAIN_THREAD
    class NetWorkThread : public IAsioThread
    {
    public:
        explicit NetWorkThread();
    public:
		int Start() final;

		void InvokeMethod(StaticMethod * task) final;

		AsioContext & GetContext() final { return *mAsioContext; }
	protected:
		void Update() final;
    private:
		AsioWork * mAsioWork;
		std::thread * mThread;
		AsioContext * mAsioContext;
        DoubleQueue<StaticMethod *> mWaitInvokeMethod;
    };
#endif
    class MainTaskScheduler : public IAsioThread
	{
	public:
		explicit MainTaskScheduler(StaticMethod * method);
	public:
		int Start() final;
		void InvokeMethod(StaticMethod * method) final;
       // AsioContext & GetContext() final { return *mAsioContext; }
	private:
		void Update() final;
	private:
        //AsioWork * mAsioWork;
        //AsioContext * mAsioContext;
        StaticMethod * mMainMethod;
        DoubleQueue<StaticMethod*> mTaskQueue;
	};

}// namespace Sentry