#pragma once

#include<Component/Component.h>
#include <Thread/TaskThread.h>
#include <Other/MultiThreadQueue.h>
#include <Util/NumberBuilder.h>
namespace Sentry
{
    class TaskProxy;

    class TaskPoolComponent : public Component, public ISystemUpdate
    {
    public:
        TaskPoolComponent();

        ~TaskPoolComponent() {}

    public:
        bool Awake() final;

        void Start() final;

        void OnSystemUpdate() final;

		int GetPriority() override { return 2; }
    public:

        long long CreateTaskId();

        void PushFinishTask(unsigned int taskId);

        bool StartTask(TaskProxy * taskAction);

        bool StartTask(const std::string & name, TaskProxy * task);

        void PushFinishTask(std::queue<unsigned int> & tasks);

        NetWorkThread * NewNetworkThread(const std::string & name, class StaticMethod * method);

        const std::vector<TaskThread *> GetThreads() { return this->mThreadArray;}
    private:
        MultiThreadQueue<unsigned int> mFinishTaskQueue;                 //在其他线程完成的任务存储
        std::unordered_map<unsigned int, TaskProxy *> mTaskMap;
    private:
        int mThreadCount;
        std::vector<TaskThread *> mThreadArray;
		NumberBuilder<unsigned int> mTaskNumberPool;
        std::unordered_map<std::string, NetWorkThread *> mNetThread;
    };
}