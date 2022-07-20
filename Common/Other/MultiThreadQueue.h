#pragma once


#ifdef __THREAD_LOCK__
#include <atomic>
#include <mutex>
#include <queue>
#else
#include<Define/ThreadQueue.h>
#endif
namespace Sentry
{
	template<typename T>
	class MultiThreadQueue
	{
	public:
		MultiThreadQueue() = default;

	public:
		void Add(const T &item);

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
}// namespace Sentry