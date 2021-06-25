#pragma once
#include<memory>
#include<Manager/Manager.h>
namespace SoEasy
{
	class ThreadTaskAction
	{
	public:
		ThreadTaskAction(Manager * manager, long long id);
		virtual ~ThreadTaskAction() { }
	public:
		long long GetTaskId() { return this->mTaskActionId; }
	public:
		void NoticeToMainThread();
		virtual void OnTaskFinish() = 0;
		virtual void InvokeInThreadPool(long long threadId) = 0;	//在线程池执行的任务
		long long GetStartTime() { return this->mStartTime; }
	private:
		long long mStartTime;
		Manager * mBindManager;
		long long mTaskActionId;		
	};
	typedef std::shared_ptr<ThreadTaskAction> SharedThreadTask;
}
