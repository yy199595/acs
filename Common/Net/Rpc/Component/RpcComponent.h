//
// Created by zmhy0073 on 2022/6/20.
//

#ifndef APP_RPCCOMPONENT_H
#define APP_RPCCOMPONENT_H
#include<unordered_map>
#include"Rpc/Async/RpcTaskSource.h"
#include"Entity/Component/Component.h"

namespace acs
{
    template<typename T>
	class RpcComponent : public Component, public ILastFrameUpdate
    {
    public:
        RpcComponent() = default;
        ~RpcComponent() override = default;
        typedef IRpcTask<T> * RpcTask;
    public:
        template<typename T1>
		inline T1 * AddTask(T1 * task, int timeout = 0)
        {
			int k = task->GetRpcId();
			assert(k != 0);
			auto iter = this->mTasks.find(k);
			if(iter != this->mTasks.end())
			{
				this->mDelTasks.emplace_back(task);
				LOG_ERROR("add task already exist");
				return nullptr;
			}
			this->mTasks.emplace(k, task);
            return task;
        }
		template<typename T1>
		inline T1 * BuildRpcTask(int rpcId, int timeout = 0)
		{
			T1 * task = new T1(rpcId);
			return this->AddTask(task, timeout);
		}

		inline bool OnResponse(int key, std::unique_ptr<T> message);
		inline size_t AwaitCount() const { return this->mTasks.size(); }
		inline int BuildRpcId() { return this->mNumberPool.BuildNumber(); }
		inline int CurrentRpcCount() { return this->mNumberPool.CurrentNumber(); }
	protected:
		void OnLastFrameUpdate(long long) noexcept final;
		virtual void OnDelTask(int k) { }
        virtual void OnNotFindResponse(int key, std::unique_ptr<T> message);
    private:
		std::vector<RpcTask> mDelTasks;
		math::NumberPool<int> mNumberPool;
		std::unordered_map<int, RpcTask> mTasks;
    };

	template<typename T>
	void RpcComponent<T>::OnLastFrameUpdate(long long nowMS) noexcept
	{
		for (RpcTask& task: this->mDelTasks)
		{
			delete task;
		}
		this->mDelTasks.clear();
	}

    template<typename T>
    inline bool RpcComponent<T>::OnResponse(int key, std::unique_ptr<T> message)
	{
		auto iter1 = this->mTasks.find(key);
		if (iter1 == this->mTasks.end())
		{
			this->OnNotFindResponse(key, std::move(message));
			return false;
		}
		{
			this->OnDelTask(key);
			iter1->second->OnResponse(std::move(message));
			this->mDelTasks.emplace_back(iter1->second);
		}
		this->mTasks.erase(iter1);
		return true;
	}

    template<typename T>
    void RpcComponent<T>::OnNotFindResponse(int k, std::unique_ptr<T> message)
    {
        LOG_ERROR("{} not find rpc task id({}) ", this->GetName(), k);
    }
}


#endif //APP_RPCCOMPONENT_H
