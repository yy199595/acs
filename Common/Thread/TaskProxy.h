#pragma once

#include <memory>
namespace Sentry
{
    class SceneTaskComponent;

    class TaskProxy
    {
    public:
        TaskProxy();
        virtual ~TaskProxy() {}
    public:
		friend class SceneTaskComponent;
        virtual void Run() = 0; //在线程池执行的任务
		virtual void RunFinish() = 0;
	public:
		long long GetTaskId() { return this->mTaskId; }
        long long GetStartTime() { return this->mStartTime; }

    private:
        long long mStartTime;
        unsigned int mTaskId;
    };
}
