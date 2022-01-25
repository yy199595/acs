#pragma once

#include<Component/Component.h>
#include <Thread/TaskThread.h>
#include <Other/MultiThreadQueue.h>
#include <Util/NumberBuilder.h>
namespace Sentry
{
    class TaskProxy;

    class ThreadPoolComponent : public Component
    {
    public:
        ThreadPoolComponent() = default;
        ~ThreadPoolComponent() final = default;

    public:
        bool Awake() final;

        bool LateAwake() final;

    public:

        bool StartTask(TaskProxy * taskAction);

        void GetAllThread(std::vector<const IThread *> & threads);

        const std::vector<TaskThread *> & GetThreads() { return this->mThreadArray;}

    private:
        void Update();

	public:
        IAsioThread & AllocateNetThread();
    private:
        size_t mIndex;
        std::mutex mLock;
        std::thread * mMonitorThread;
        std::vector<TaskThread *> mThreadArray;
		std::vector<NetWorkThread *> mNetThreads;
		NumberBuilder<unsigned int> mTaskNumberPool;       
    };
}