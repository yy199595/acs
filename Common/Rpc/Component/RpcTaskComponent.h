//
// Created by zmhy0073 on 2022/6/20.
//

#ifndef SERVER_RPCTASKCOMPONENT_H
#define SERVER_RPCTASKCOMPONENT_H
#include"Component/Component.h"
#include"Async/RpcTaskSource.h"
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
        void OnTimeout(long long taskId);
        template<typename T1>
        T1 * AddTask(std::shared_ptr<T1> task)
        {
            long long taskId = task->GetRpcId();
            auto iter = this->mTasks.find(taskId);
            if(iter == this->mTasks.end())
            {
                this->OnAddTask(task);
                this->mTasks.emplace(taskId, task);
            }
            return task.get();
        }
        bool OnResponse(long long taskId, std::shared_ptr<T> message);
	protected:
		virtual void OnAddTask(RpcTask task) { }
		virtual void OnDelTask(long long taskId, RpcTask task) { }
        virtual void OnNotFindResponse(long long taskId, std::shared_ptr<T> message);
    private:
        std::unordered_map<long long, RpcTask> mTasks;
    };

    template<typename T>
    void RpcTaskComponent<T>::OnTimeout(long long taskId)
    {
        auto iter = this->mTasks.find(taskId);
        if(iter != this->mTasks.end())
        {
            RpcTask rpcTask = iter->second;
            rpcTask->OnTimeout();
            this->mTasks.erase(iter);
            LOG_ERROR(this->GetName() << " rpc task time out id " << taskId);
        }
    }

    template<typename T>
    bool RpcTaskComponent<T>::OnResponse(long long taskId, std::shared_ptr<T> message)
    {
        auto iter = this->mTasks.find(taskId);
        if(iter == this->mTasks.end())
        {
            this->OnNotFindResponse(taskId, message);
            return false;
        }
        RpcTask rpcTask = iter->second;
        this->mTasks.erase(iter);
        rpcTask->OnResponse(message);
		this->OnDelTask(taskId, rpcTask);
        return true;
    }

    template<typename T>
    void RpcTaskComponent<T>::OnNotFindResponse(long long taskId, std::shared_ptr<T> message)
    {
        LOG_ERROR(this->GetName() << " not find rpc task id " << taskId);
    }
}


#endif //SERVER_RPCTASKCOMPONENT_H
