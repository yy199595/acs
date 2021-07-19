#pragma once
#include <memory>
namespace Sentry
{
	class ThreadTaskManager;
	class ThreadTaskAction
	{
	public:
		ThreadTaskAction();
		virtual ~ThreadTaskAction() {}
	public:
		bool InitTaskAction(ThreadTaskManager *taskManager);
	public:
		long long GetTaskId() { return this->mTaskActionId; }
	public:
		virtual void OnTaskFinish() = 0;
		virtual void InvokeInThreadPool(long long threadId) = 0; //在线程池执行的任务
		long long GetStartTime() { return this->mStartTime; }

	private:
		long long mStartTime;
		long long mTaskActionId;
	};
	typedef std::shared_ptr<ThreadTaskAction> SharedThreadTask;
}
