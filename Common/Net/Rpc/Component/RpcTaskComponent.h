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
    template<typename K,typename T, bool Del = true>
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
		inline bool OnResponse(K key, T * message);
		size_t AwaitCount() const { return this->mTasks.size(); }
	protected:
		void OnLastFrameUpdate(long long) final;
		virtual void OnDelTask(K k) { }
        virtual void OnNotFindResponse(K key, T * message);
    private:
		std::queue<T *> mDelMessages;
		std::vector<RpcTask> mDelTasks;
		std::unordered_map<K, RpcTask> mTasks;
		std::unordered_map<K, long long> mTimeouts;
    };

	template<typename K,typename T, bool Del>
	void RpcTaskComponent<K, T, Del>::OnLastFrameUpdate(long long nowMS)
	{
		for(RpcTask & task : this->mDelTasks)
		{
			delete task;
		}
		T * data = nullptr;
		while(!this->mDelMessages.empty())
		{
			data = this->mDelMessages.front();
			{
				delete data;
				this->mDelMessages.pop();
			}
		}
		this->mDelTasks.clear();
		if(!this->mTimeouts.empty())
		{
			for(auto iter = this->mTimeouts.begin(); iter != this->mTimeouts.end(); )
			{
				const K & key = iter->first;
				long long targetTime = iter->second;
				if(nowMS >= targetTime)
				{
					auto iter1 = this->mTasks.find(key);
					if (iter1 != this->mTasks.end())
					{
						iter1->second->OnResponse(nullptr);
						this->mDelTasks.emplace_back(iter1->second);
						this->mTasks.erase(iter1);
					}
					this->mTimeouts.erase(iter++);
					continue;
				}
				iter++;
			}
		}
	}

    template<typename K,typename T, bool Del>
    inline bool RpcTaskComponent<K, T, Del>::OnResponse(K key, T * message)
	{
		auto iter = this->mTimeouts.find(key);
		if(iter != this->mTimeouts.end());
		{
			this->mTimeouts.erase(iter);
		}
		if(Del && message != nullptr)
		{
			this->mDelMessages.emplace(message);
		}
		auto iter1 = this->mTasks.find(key);
		if(iter1 == this->mTasks.end())
		{
			this->OnNotFindResponse(key, message);
			return false;
		}
		{
			this->OnDelTask(key);
			iter1->second->OnResponse(message);
			this->mDelTasks.emplace_back(iter1->second);
		}
		this->mTasks.erase(iter1);
		return true;
	}

    template<typename K,typename T, bool Del>
    void RpcTaskComponent<K, T, Del>::OnNotFindResponse(K k, T * message)
    {
        LOG_ERROR("{} not find rpc task id({}) ", this->GetName(), k);
    }
}


#endif //APP_RPCTASKCOMPONENT_H
