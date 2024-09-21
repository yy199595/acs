//
// Created by leyi on 2023/8/25.
//

#ifndef APP_TASKQUEUE_H
#define APP_TASKQUEUE_H
#include<queue>
#include<mutex>
#include"Rpc/Method/MethodProxy.h"

namespace custom
{
	class TaskQueue
	{
	public:
		template<typename F, typename T, typename ... Args>
		inline void Start(F&& f, T* o, Args&& ... args);
		inline void Start(std::function<void()>&& func);
	public:
		inline size_t Swap();
		inline void Invoke(unsigned int maxCount = 500);
	private:
		std::mutex mMutex;
		std::queue<acs::StaticMethod *> mReadQueue;
		std::queue<acs::StaticMethod *> mWriteQueue;
	};

	inline void TaskQueue::Invoke(unsigned int maxCount)
	{
		unsigned int index = 0;
		while(!this->mReadQueue.empty() && index < maxCount)
		{
			++index;
			this->mReadQueue.front()->run();
			{
				delete this->mReadQueue.front();
			}
			this->mReadQueue.pop();
		}
	}

	inline size_t TaskQueue::Swap()
	{
		std::lock_guard<std::mutex> lock(this->mMutex);
		if(!this->mReadQueue.empty())
		{
			return this->mReadQueue.size();
		}
		if(this->mWriteQueue.empty())
		{
			return 0;
		}
		this->mReadQueue.swap(this->mWriteQueue);
		return this->mReadQueue.size();
	}

	template<typename F, typename T, typename ... Args>
	inline void TaskQueue::Start(F&& f, T* o, Args&& ...args)
	{
		this->mMutex.lock();
		acs::StaticMethod* taskMethod =
				NewMethodProxy(std::forward<F>(f), o, std::forward<Args>(args)...);
		this->mWriteQueue.emplace(taskMethod);
		this->mMutex.unlock();
	}

	inline void TaskQueue::Start(std::function<void()>&& func)
	{
		this->mMutex.lock();
		acs::StaticMethod * taskMethod =
				new acs::LambdaMethod(std::move(func));
		this->mWriteQueue.emplace(taskMethod);
		this->mMutex.unlock();
	}
}



#endif //APP_TASKQUEUE_H
