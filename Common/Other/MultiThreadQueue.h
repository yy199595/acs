#pragma once


#ifdef __THREAD_LOCK__
#include <atomic>
#include <mutex>
#include <queue>
#else
#include<Define/ThreadQueue.h>
#endif
namespace GameKeeper
{
	template<typename T>
	class MultiThreadQueue
	{
	public:
		MultiThreadQueue()
		{}

	public:
		void Add(const T &item);

		void AddRange(std::queue<T> &items);

		bool PopItem(T &item);

		void SwapQueueData();

	private:

	private:
#ifdef __THREAD_LOCK__
        std::mutex mLock;
		std::queue<T> mReadQueue;
		std::queue<T> mWriteQueue;
#else
        MultiThread::ConcurrentQueue<T> mQeue;
#endif
	};

	template<typename T>
	inline void MultiThreadQueue<T>::Add(const T &item)
	{
#ifdef __THREAD_LOCK__
		mLock.lock();
		mWriteQueue.emplace(item);
		mLock.unlock();
#else
        mQeue.enqueue(item);
#endif
	}

	template<typename T>
	inline void MultiThreadQueue<T>::AddRange(std::queue<T> &item)
	{
#ifdef __THREAD_LOCK__
		mLock.lock();
		while (!item.empty())
		{
			mWriteQueue.emplace(item.front());
			item.pop();
		}
		mLock.unlock();
#else
        while(!item.empty())
        {
            this->mQeue.enqueue(item.front());
            item.pop();
        }
#endif
	}

	template<typename T>
	inline void MultiThreadQueue<T>::SwapQueueData()
	{
#ifdef __THREAD_LOCK__
		if (!this->mWriteQueue.empty() && this->mReadQueue.empty())
		{
			std::lock_guard<std::mutex> lock(this->mLock);
			this->mReadQueue.swap(this->mWriteQueue);
		}
#endif
	}

	template<typename T>
	inline bool MultiThreadQueue<T>::PopItem(T &item)
	{
#ifdef __THREAD_LOCK__
		if (!this->mReadQueue.empty())
		{
			item = this->mReadQueue.front();
			this->mReadQueue.pop();
			return true;
		}
		return false;
#else
        return this->mQeue.try_dequeue(item);
#endif
	}
}// namespace GameKeeper