//
// Created by leyi on 2023/8/3.
//

#ifndef APP_DOUBLEBUFFERQUEUE_H
#define APP_DOUBLEBUFFERQUEUE_H

#include<queue>
#include<memory>
#include<mutex>
namespace custom
{
	template<typename T>
	class DoubleBufferQueue
	{
	public:
		bool Swap();
		bool Swap(size_t & size);
		size_t ReadSize() const;
		size_t WriteSize() const;
		bool TryPop(T & value);
		void Push(const T & value);
	private:
		std::mutex mReadLock;
		std::mutex mWriteLock;
		std::queue<T> mReadQueue;
		std::queue<T> mWriteQueue;
	};

	template<typename T>
	size_t DoubleBufferQueue<T>::ReadSize() const
	{
		std::lock_guard<std::mutex> lock(this->mReadLock);
		return this->mReadQueue.size();
	}

	template<typename T>
	size_t DoubleBufferQueue<T>::WriteSize() const
	{
		std::lock_guard<std::mutex> lock(this->mWriteLock);
		return this->mWriteQueue.size();
	}

	template<typename T>
	bool DoubleBufferQueue<T>::Swap()
	{
		std::lock_guard<std::mutex> lock1(this->mReadLock);
		if(!this->mReadQueue.empty())
		{
			return true;
		}
		std::lock_guard<std::mutex> lock2(this->mWriteLock);
		std::swap(this->mReadQueue, this->mWriteQueue);
		return !this->mReadQueue.empty();
	}

	template<typename T>
	bool DoubleBufferQueue<T>::Swap(size_t& size)
	{
		std::lock_guard<std::mutex> lock1(this->mReadLock);
		if(!this->mReadQueue.empty())
		{
			size = this->mReadQueue.size();
			return true;
		}
		std::lock_guard<std::mutex> lock2(this->mWriteLock);
		std::swap(this->mReadQueue, this->mWriteQueue);
		size = this->mReadQueue.size();
		return !this->mReadQueue.empty();
	}

	template<typename T>
	void DoubleBufferQueue<T>::Push(const T& value)
	{
		std::lock_guard<std::mutex> lock2(this->mWriteLock);
		this->mWriteQueue.push(value);
	}

	template<typename T>
	bool DoubleBufferQueue<T>::TryPop(T& value)
	{
		std::lock_guard<std::mutex> lock1(this->mReadLock);
		if(this->mReadQueue.empty())
		{
			return false;
		}
		value = this->mReadQueue.front();
		this->mReadQueue.pop();
		return true;
	}

}

#endif //APP_DOUBLEBUFFERQUEUE_H
