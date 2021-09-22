#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <vector>
#include <list>
namespace Sentry
{
	template<typename T>
	class DoubleBufferQueue
	{
	public:
		DoubleBufferQueue()
		{}

	public:
		void Add(const T &item);

		void AddRange(std::list<T> &items);

		void AddRange(std::queue<T> &items);

		void AddRange(std::vector<T> &items);

		bool PopItem(T &item);

		void SwapQueueData();

	private:
		std::mutex mLock;

	private:
		std::queue<T> mReadQueue;
		std::queue<T> mWriteQueue;
	};

	template<typename T>
	inline void DoubleBufferQueue<T>::Add(const T &item)
	{
		mLock.lock();
		mWriteQueue.emplace(item);
		mLock.unlock();
	}

	template<typename T>
	void DoubleBufferQueue<T>::AddRange(std::list<T> &items)
	{
		mLock.lock();
		for (auto iter = items.begin(); iter != items.end(); iter++)
		{
			mWriteQueue.emplace(*iter);
		}
		items.clear();
		mLock.unlock();
	}

	template<typename T>
	void DoubleBufferQueue<T>::AddRange(std::vector<T> &items)
	{
		mLock.lock();
		for (size_t index = 0; index < items.size(); index++)
		{
			mWriteQueue.emplace(items[index]);
		}
		items.clear();
		mLock.unlock();
	}

	template<typename T>
	inline void DoubleBufferQueue<T>::AddRange(std::queue<T> &item)
	{
		mLock.lock();
		while (!item.empty())
		{
			mWriteQueue.emplace(item.front());
			item.pop();
		}
		mLock.unlock();
	}

	template<typename T>
	inline void DoubleBufferQueue<T>::SwapQueueData()
	{
		if (!this->mWriteQueue.empty() && this->mReadQueue.empty())
		{
			std::lock_guard<std::mutex> lock(this->mLock);
			this->mReadQueue.swap(this->mWriteQueue);
		}
	}

	template<typename T>
	inline bool DoubleBufferQueue<T>::PopItem(T &item)
	{
		if (!this->mReadQueue.empty())
		{
			item = this->mReadQueue.front();
			this->mReadQueue.pop();
			return true;
		}
		return false;
	}
}// namespace Sentry