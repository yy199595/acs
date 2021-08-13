#pragma once

#include <atomic>
#include <mutex>
#include <queue>

namespace Sentry
{
				template<typename T>
				class DoubleBufferQueue
				{
				public:
								DoubleBufferQueue()
								{}

				public:
								void AddItem(const T &item);

								bool PopItem(T &item);

								void SwapQueueData();

				private:
								std::mutex mLock;

				private:
								std::queue<T> mReadQueue;
								std::queue<T> mWrterQueue;
				};

				template<typename T>
				inline void DoubleBufferQueue<T>::AddItem(const T &item)
				{
								mLock.lock();
								mWrterQueue.emplace(item);
								mLock.unlock();
				}

				template<typename T>
				inline void DoubleBufferQueue<T>::SwapQueueData()
				{
								if (!this->mWrterQueue.empty() && this->mReadQueue.empty())
								{
												std::lock_guard<std::mutex> lock(this->mLock);
												this->mReadQueue.swap(this->mWrterQueue);
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