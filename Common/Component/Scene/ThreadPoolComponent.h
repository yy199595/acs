#pragma once

#include<Component/Component.h>
#include <Thread/TaskThread.h>
#include <Other/MultiThreadQueue.h>
#include <Util/NumberBuilder.h>
namespace GameKeeper
{
    class TaskProxy;

    class ThreadPoolComponent : public Component, public ISystemUpdate
    {
    public:
        ThreadPoolComponent() = default;
        ~ThreadPoolComponent() final = default;

    public:
        bool Awake() final;


        bool LateAwake() final;

        void OnSystemUpdate() final;

		int GetPriority() final { return 0; }
    public:

        void PushFinishTask(unsigned int taskId);

        bool StartTask(TaskProxy * taskAction);

        void GetAllThread(std::vector<const IThread *> & threads);

        const std::vector<TaskThread *> & GetThreads() { return this->mThreadArray;}

    private:
        void Update();

	public:	
		NetWorkThread & AllocateNetThread();
    private:
        MultiThreadQueue<unsigned int> mFinishTaskQueue;                 //在其他线程完成的任务存储
        std::unordered_map<unsigned int, TaskProxy *> mTaskMap;
    private:
        size_t mIndex;
        std::mutex mLock;
        std::thread * mMonitorThread;
        std::vector<TaskThread *> mThreadArray;
		std::vector<NetWorkThread *> mNetThreads;
		NumberBuilder<unsigned int> mTaskNumberPool;       
    };
}