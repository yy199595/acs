#pragma once

#include <memory>
#include<Method/MethodProxy.h>
namespace Sentry
{
    class TaskPoolComponent;

    class TaskProxy
    {
    public:
        TaskProxy();
        virtual ~TaskProxy() {}
    public:
		friend class TaskPoolComponent;
        virtual bool Run() = 0; //在线程池执行的任务
		virtual void RunFinish() { };
	public:
		long long GetTaskId() { return this->mTaskId; }
        long long GetStartTime() { return this->mStartTime; }
    private:
        long long mStartTime;
        unsigned int mTaskId;
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

	class MethodTaskProxy : TaskProxy
	{
	public:
		MethodTaskProxy(StaticMethod * method)
			: mMethodProxy(method) { }
		~MethodTaskProxy() { delete mMethodProxy; }
	public:
		bool Run() final { mMethodProxy->run(); }
	private:
		StaticMethod * mMethodProxy;
	};

	template<typename T>
	class ThreadFucntionTask : public TaskProxy
	{
	public:
		typedef void(T::*TaskFunc)();
		ThreadFucntionTask(TaskFunc && func, T * o) :
			_o(o), _func(std::forward<TaskFunc>(func)) { }
	public:
		bool Run() final
		{
			(_o->*_f)();
			delete this;
			return false;
		}
	private:
		T * _o;
		TaskFunc _func;
	};
	
}
