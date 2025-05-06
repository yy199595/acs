//
// Created by yy on 2023/8/7.
//

#ifndef APP_THREADSYNC_H
#define APP_THREADSYNC_H

#ifndef ONLY_MAIN_THREAD

#include <queue>
#include <mutex>
#include <condition_variable>


namespace custom
{
	template<typename T>
	class ThreadSync
	{
	public:
		T Wait();
		void SetResult(const T & val);
	private:
		std::queue<T> mQueue;
		mutable std::mutex mMutex;
		std::condition_variable mVar;
	};

	template<typename T>
	void ThreadSync<T>::SetResult(const T& val)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		this->mQueue.push(val);
		this->mVar.notify_one();
	}

	template<typename T>
	T ThreadSync<T>::Wait()
	{
		std::unique_lock<std::mutex> lock(this->mMutex);
		this->mVar.wait(lock, [this] { return !this->mQueue.empty(); });
		return this->mQueue.front();
	}
}
#endif //APP_THREADSYNC_H

#endif