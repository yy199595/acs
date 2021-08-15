﻿#pragma once

#include "Manager.h"
#include <Thread/TaskThread.h>
#include <Other/DoubleBufferQueue.h>

namespace Sentry
{
    class ThreadTaskAction;

    class ThreadTaskManager : public Manager, public ISystemUpdate
    {
    public:
        ThreadTaskManager();

        ~ThreadTaskManager() {}

    public:
        bool OnInit() final;

        void OnInitComplete() final;

        void OnSystemUpdate() final;

    public:
        int GetThreadCount();

        long long CreateTaskId();

        void OnTaskFinish(long long taskId);

        bool StartInvokeTask(std::shared_ptr<ThreadTaskAction> taskAction);

    private:
        DoubleBufferQueue<long long> mFinishTaskQueue;                 //在其他线程完成的任务存储
        std::unordered_map<long long, shared_ptr<ThreadTaskAction>> mThreadTaskMap;
    private:
        int mThreadIndex;
        int mThreadCount;
        std::vector<TaskThread *> mThreadArray;
    };
}