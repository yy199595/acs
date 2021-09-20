#pragma once

#include<Component/Component.h>
#include <Thread/TaskThread.h>
#include <Other/DoubleBufferQueue.h>
#include <Util/NumberBuilder.h>
namespace Sentry
{
    class TaskProxy;

    class TaskComponent : public Component, public ISystemUpdate
    {
    public:
        TaskComponent();

        ~TaskComponent() {}

    public:
        bool Awake() final;

        void Start() final;

        void OnSystemUpdate() final;

		int GetPriority() override { return 2; }
    public:
		void GetThreads(std::vector<std::thread::id> & threads);

        long long CreateTaskId();

        void PushFinishTask(unsigned int taskId);

		void PushFinishTask(std::queue<unsigned int> & tasks);

        bool StartTask(TaskProxy * taskAction);

    private:
        DoubleBufferQueue<unsigned int> mFinishTaskQueue;                 //在其他线程完成的任务存储
        std::unordered_map<unsigned int, TaskProxy *> mTaskMap;
    private:
        int mThreadIndex;
        int mThreadCount;
        std::vector<TaskThread *> mThreadArray;
		NumberBuilder<unsigned int> mTaskNumberPool;
    };
}