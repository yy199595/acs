#pragma once

#include <memory>
namespace GameKeeper
{

    class ThreadPoolComponent;
    class CoroutineComponent;

    class TaskProxy
    {
    public:
        TaskProxy();
        virtual ~TaskProxy() {}
    public:
		friend class ThreadPoolComponent;
        virtual bool Run() = 0; //在线程池执行的任务
		virtual void RunFinish() { };
	public:
		long long GetTaskId() { return this->mTaskId; }
        long long GetStartTime() { return this->mStartTime; }
    private:
        long long mStartTime;
        unsigned int mTaskId;
    };

    class CoroutineAsyncTask : public TaskProxy
    {
    public:
        CoroutineAsyncTask();
    public:
        void RunFinish() override;
        bool AwaitInvoke();
    private:
        unsigned int mCorId;
        CoroutineComponent * mCorComponent;
    };

	template<typename T>
	class FucntionTask : public TaskProxy
	{
	public:
		typedef void(T::*TaskFunc)();
		FucntionTask(TaskFunc && func, T * o) :
			_o(o), _func(std::forward<TaskFunc>(func)) { }
	public:
		bool Run() final 
		{ 
			(_o->*_func)(); 
			delete this;
			return false; 
		}
		void RunFinish() { }
	private:
		T * _o;
		TaskFunc _func;
	};
}
