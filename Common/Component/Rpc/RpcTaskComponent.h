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
		virtual void OnAddTask(RpcTask task) { }
		virtual void OnDelTask(long long taskId, RpcTask task) { }
	private:
        std::unordered_map<long long, RpcTask> mTasks;
    };

    template<typename T>
    bool RpcTaskComponent<T>::AddTask(RpcTask task)
    {
        long long taskId = task->GetRpcId();
        auto iter = this->mTasks.find(taskId);
        if(iter == this->mTasks.end())
        {
            this->OnAddTask(task);
            this->mTasks.emplace(taskId, task);
            return true;
        }
        LOG_FATAL(this->GetName() << "add task error id = " << task->GetRpcId());
        return false;
    }

    template<typename T>
    bool RpcTaskComponent<T>::OnResponse(long long taskId, std::shared_ptr<T> message)
    {
        auto iter = this->mTasks.find(taskId);
        if(iter == this->mTasks.end())
        {
            LOG_ERROR(this->GetName() << " not find rpc task id " << taskId);
            return false;
        }
        RpcTask rpcTask = iter->second;
        this->mTasks.erase(iter);
        rpcTask->OnResponse(message);
		this->OnDelTask(taskId, rpcTask);
        LOG_WARN("invoke new rpc task id " << taskId);
        return true;
    }
}


#endif //SERVER_RPCTASKCOMPONENT_H
