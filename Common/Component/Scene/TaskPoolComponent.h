#pragma once

#include<Component/Component.h>
#include <Thread/TaskThread.h>
#include <Other/MultiThreadQueue.h>
#include <Util/NumberBuilder.h>
namespace GameKeeper
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

        void PushFinishTask(std::queue<unsigned int> & tasks);

        const std::vector<TaskThread *> GetThreads() { return this->mThreadArray;}

	public:	
		NetWorkThread & GetNetThread();		
    private:
        MultiThreadQueue<unsigned int> mFinishTaskQueue;                 //在其他线程完成的任务存储
        std::unordered_map<unsigned int, TaskProxy *> mTaskMap;
    private:
		size_t mIndex;
		std::mutex mLock;
        std::vector<TaskThread *> mThreadArray;
		std::vector<NetWorkThread *> mNetThreads;
		NumberBuilder<unsigned int> mTaskNumberPool;       
    };
}