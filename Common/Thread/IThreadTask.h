#pragma once

#include<memory>
namespace Sentry
{

    class NetThreadComponent;
    class TaskComponent;

    class IThreadTask
    {
    public:
        IThreadTask();
        virtual ~IThreadTask() = default;
    public:
		friend class NetThreadComponent;
        virtual void Run() = 0; //在线程池执行的任务
		virtual void RunFinish() { };
	public:
		long long GetTaskId() const { return this->mTaskId; }
        long long GetStartTime() const { return this->mStartTime; }
    private:
        long long mStartTime;
        unsigned int mTaskId;
    };

    class CoroutineAsyncTask : public IThreadTask
    {
    public:
        CoroutineAsyncTask();
    public:
        void RunFinish() override;
        bool AwaitInvoke();
    private:
        unsigned int mCorId;
        TaskComponent * mCorComponent;
    };

	template<typename T>
	class FucntionTask : public IThreadTask
	{
	public:
		typedef void(T::*TaskFunc)();
		FucntionTask(TaskFunc && func, T * o) :
			_o(o), _func(std::forward<TaskFunc>(func)) { }
	public:
		void Run() final
		{ 
			(_o->*_func)(); 
			delete this;
		}
		void RunFinish() { }
	private:
		T * _o;
		TaskFunc _func;
	};
}
