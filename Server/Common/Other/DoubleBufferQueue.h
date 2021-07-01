#pragma once
#pragma once
#include<queue>
#include<mutex>
#include<atomic>
namespace SoEasy
{
	template<typename T>
	class DoubleBufferQueue
	{
	public:
		DoubleBufferQueue() { }
	public:
		void AddItem(const T & item);
		bool PopItem(T & item);
		void SwapQueueData();
	private:
		std::mutex mLock;
	private:
		std::queue<T> mWorkQueue;
		std::queue<T> mCacheQueue;
	};

	template<typename T>
	inline void DoubleBufferQueue<T>::AddItem(const T & item)
	{
		mLock.lock();
		mCacheQueue.push(item);
		mLock.unlock();
	}
	template<typename T>
	inline void DoubleBufferQueue<T>::SwapQueueData()
	{
		if (!this->mCacheQueue.empty() && this->mWorkQueue.empty())
		{
			mLock.lock();
			std::swap(this->mCacheQueue, this->mWorkQueue);
			mLock.unlock();
		}

	}
	template<typename T>
	inline bool DoubleBufferQueue<T>::PopItem(T & item)
	{
		if (!this->mWorkQueue.empty())
		{
			item = this->mWorkQueue.front();
			this->mWorkQueue.pop();
			return true;
		}
		return false;
	}
}