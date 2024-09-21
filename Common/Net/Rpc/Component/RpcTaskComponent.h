//
// Created by zmhy0073 on 2022/6/20.
//

#ifndef APP_RPCTASKCOMPONENT_H
#define APP_RPCTASKCOMPONENT_H
#include"Core/Map/HashMap.h"
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
        T1 * AddTask(K k, T1 * task)
        {
			if(!this->mTasks.Add(k, task))
			{
				LOG_ERROR("add task already exist");
				return nullptr;
			}
            return task;
        }
		inline bool OnResponse(K key, T * message);
		size_t AwaitCount() const { return this->mTasks.Size(); }
	protected:
		void OnLastFrameUpdate() final;
		virtual void OnTaskComplete(K k) { }
        virtual void OnNotFindResponse(K key, T * message);
    private:
		std::queue<K> mTimeouts;
		std::queue<T *> mDelMessages;
		std::vector<RpcTask> mDelTasks;
		custom::HashMap<K, RpcTask> mTasks;
    };

	template<typename K,typename T, bool Del>
	void RpcTaskComponent<K, T, Del>::OnLastFrameUpdate()
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
	}

    template<typename K,typename T, bool Del>
    inline bool RpcTaskComponent<K, T, Del>::OnResponse(K key, T * message)
	{
		RpcTask rpcTask = nullptr;
		assert(this->mApp->IsMainThread());
		if(Del)
		{
			this->mDelMessages.emplace(message);
		}
		if (!this->mTasks.Del(key, rpcTask))
		{
			this->OnNotFindResponse(key, message);
			return false;
		}
		this->OnTaskComplete(key);
		rpcTask->OnResponse(message);
		this->mDelTasks.emplace_back(rpcTask);
		return true;
	}

    template<typename K,typename T, bool Del>
    void RpcTaskComponent<K, T, Del>::OnNotFindResponse(K k, T * message)
    {
        LOG_ERROR("{} not find rpc task id({}) ", this->GetName(), k);
    }
}


#endif //APP_RPCTASKCOMPONENT_H
