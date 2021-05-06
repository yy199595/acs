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
		void AddItem(T & item);
		bool PopItem(T & item);
		void SwapQueueData();
	private:
		std::mutex mLock;
	private:
		std::queue<T> mWorkQueue;
		std::queue<T> mCacheQueue;
	};

	template<typename T>
	inline void DoubleBufferQueue<T>::AddItem(T & item)
	{
		mLock.lock();
		mCacheQueue.push(item);
		mLock.unlock();
	}
	template<typename T>
	inline void DoubleBufferQueue<T>::SwapQueueData()
	{
		if (!this->mCacheQueue.empty())
		{
			mLock.lock();
			while (!this->mCacheQueue.empty())
			{
				T item = this->mCacheQueue.front();
				this->mCacheQueue.pop();
				mWorkQueue.push(item);
			}
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