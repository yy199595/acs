//
// Created by zmhy0073 on 2022/6/20.
//

#ifndef APP_RPCTASKCOMPONENT_H
#define APP_RPCTASKCOMPONENT_H
#include<unordered_map>
#include"Util/Tools/TimeHelper.h"
#include"Rpc/Async/RpcTaskSource.h"
#include"Entity/Component/Component.h"

namespace acs
{
    template<typename K,typename T>
	class RpcTaskComponent : public Component, public ILastFrameUpdate
    {
    public:
        RpcTaskComponent() = default;
        ~RpcTaskComponent() override = default;
        typedef IRpcTask<T> * RpcTask;
    public:
        template<typename T1>
        T1 * AddTask(K k, T1 * task, int timeout = 0)
        {
			auto iter = this->mTasks.find(k);
			if(iter != this->mTasks.end())
			{
				this->mDelTasks.emplace_back(task);
				LOG_ERROR("add task already exist");
				return nullptr;
			}
			if(timeout > 0)
			{
				long long target = help::Time::NowMil() + timeout;
				this->mTimeouts.emplace(k, target);
			}
			this->mTasks.emplace(k, task);
            return task;
        }
		inline bool OnResponse(K key, std::unique_ptr<T> message);
		size_t AwaitCount() const { return this->mTasks.size(); }
	protected:
		void OnLastFrameUpdate(long long) final;
		virtual void OnDelTask(K k) { }
        virtual void OnNotFindResponse(K key, std::unique_ptr<T> message);
    private:
		std::vector<RpcTask> mDelTasks;
		std::unordered_map<K, RpcTask> mTasks;
		std::unordered_map<K, long long> mTimeouts;
    };

	template<typename K,typename T>
	void RpcTaskComponent<K, T>::OnLastFrameUpdate(long long nowMS)
	{
		for (RpcTask& task: this->mDelTasks)
		{
			delete task;
		}
		this->mDelTasks.clear();
		for (auto iter = this->mTimeouts.begin(); iter != this->mTimeouts.end();)
		{
			const K& key = iter->first;
			long long targetTime = iter->second;
			if (nowMS >= targetTime)
			{
				auto iter1 = this->mTasks.find(key);
				if (iter1 != this->mTasks.end())
				{
					iter1->second->OnResponse(nullptr);
					this->mDelTasks.emplace_back(iter1->second);
					LOG_ERROR("rpc task:{} time out", iter1->first)
					this->mTasks.erase(iter1);
				}
				iter = this->mTimeouts.erase(iter);
				continue;
			}
			iter++;
		}
	}

    template<typename K,typename T>
    inline bool RpcTaskComponent<K, T>::OnResponse(K key, std::unique_ptr<T> message)
	{
		auto iter = this->mTimeouts.find(key);
		if (iter != this->mTimeouts.end())
		{
			this->mTimeouts.erase(iter);
		}

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

    template<typename K,typename T>
    void RpcTaskComponent<K, T>::OnNotFindResponse(K k, std::unique_ptr<T> message)
    {
        LOG_ERROR("{} not find rpc task id({}) ", this->GetName(), k);
    }
}


#endif //APP_RPCTASKCOMPONENT_H
