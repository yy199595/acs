//
// Created by zmhy0073 on 2022/6/20.
//

#ifndef SERVER_RPCTASKCOMPONENT_H
#define SERVER_RPCTASKCOMPONENT_H
#include"Core/Component/Component.h"
#include"Rpc/Async/RpcTaskSource.h"
namespace Sentry
{
    template<typename K,typename T>
    class RpcTaskComponent : public Component
    {
    public:
        RpcTaskComponent() = default;
        ~RpcTaskComponent() = default;
        typedef std::shared_ptr<IRpcTask<T>> RpcTask;
    public:
        void OnTimeout(K k);
        template<typename T1>
        T1 * AddTask(K k, std::shared_ptr<T1> task)
        {
            auto iter = this->mTasks.find(k);
            if(iter != this->mTasks.end())
            {
                LOG_ERROR("add task already exist");
                return nullptr;
            }
			this->mTasks.emplace(k, task);
            return task.get();
        }
        bool OnResponse(K key, std::shared_ptr<T> message);
	protected:
		virtual void OnTaskComplete(K key) { }
        virtual void OnNotFindResponse(K key, std::shared_ptr<T> message);
    private:
        std::unordered_map<K, RpcTask> mTasks;
    };

    template<typename K,typename T>
    void RpcTaskComponent<K, T>::OnTimeout(K key)
    {
        auto iter = this->mTasks.find(key);
        if(iter != this->mTasks.end())
        {
            RpcTask rpcTask = iter->second;
            rpcTask->OnTimeout();
            this->mTasks.erase(iter);
            LOG_ERROR(this->GetName() << " rpc task time out id " << key);
        }
    }

    template<typename K,typename T>
    bool RpcTaskComponent<K, T>::OnResponse(K key, std::shared_ptr<T> message)
    {
        auto iter = this->mTasks.find(key);
        if(iter == this->mTasks.end())
        {
			this->OnNotFindResponse(key, message);
            return false;
        }
        RpcTask rpcTask = iter->second;
		if(rpcTask != nullptr)
		{
			rpcTask->OnResponse(message);
			this->OnTaskComplete(key);
		}
		this->mTasks.erase(iter);
		return true;
    }

    template<typename K,typename T>
    void RpcTaskComponent<K, T>::OnNotFindResponse(K k, std::shared_ptr<T> message)
    {
        LOG_ERROR(this->GetName() << " not find rpc task id " << k);
    }
}


#endif //SERVER_RPCTASKCOMPONENT_H
