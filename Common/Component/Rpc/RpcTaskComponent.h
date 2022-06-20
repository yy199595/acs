//
// Created by zmhy0073 on 2022/6/20.
//

#ifndef SERVER_RPCTASKCOMPONENT_H
#define SERVER_RPCTASKCOMPONENT_H
#include"Component/Component.h"
#include"Async/RpcTask/RpcTaskSource.h"
namespace Sentry
{
    template<typename T>
    class RpcTaskComponent : public Component
    {
    public:
        RpcTaskComponent() = default;
        ~RpcTaskComponent() = default;
        typedef std::shared_ptr<IRpcTask<T>> RpcTask;
    public:
        bool AddTask(RpcTask task);
        bool OnResponse(long long taskId, std::shared_ptr<T> message);
    protected:
        virtual TimerComponent * GetTimerComponent() = 0;
    private:
         void OnTimeout(long long id);
    private:
        std::unordered_map<long long, RpcTask> mTasks;
        std::unordered_map<long long, long long> mTimers;
    };

    template<typename T>
    bool RpcTaskComponent<T>::AddTask(RpcTask task)
    {
        long long taskId = task->GetRpcId();
        auto iter = this->mTasks.find(taskId);
        if(iter == this->mTasks.end())
        {
            int timeout = task->GetTimeout();
            if(timeout > 0)
            {
                TimerComponent * timerComponent = this->GetTimerComponent();
                this->mTimers.emplace(taskId, timerComponent->DelayCall(
                        timeout / 1000.0f, &RpcTaskComponent<T>::OnTimeout, this, taskId));
            }
            this->mTasks.emplace(taskId, task);
            return true;
        }
        return false;
    }

    template<typename T>
    void RpcTaskComponent<T>::OnTimeout(long long id)
    {
        this->OnResponse(id, nullptr);
    }

    template<typename T>
    bool RpcTaskComponent<T>::OnResponse(long long taskId, std::shared_ptr<T> message)
    {
        auto iter = this->mTasks.find(taskId);
        if(iter == this->mTasks.end())
        {
            return false;
        }
        TimerComponent * timerComponent = this->GetTimerComponent();
        auto iter1 = this->mTimers.find(taskId);
        if(iter1 != this->mTimers.end())
        {
            long long timerId = iter1->second;
            this->mTimers.erase(iter1);
            timerComponent->CancelTimer(timerId);
        }
        RpcTask rpcTask = iter->second;
        this->mTasks.erase(iter);
        rpcTask->OnResponse(message);
        return true;
    }
}


#endif //SERVER_RPCTASKCOMPONENT_H
