
#ifndef APP_COMMON_CORE_QUEUE_THREADQUEUE_H
#define APP_COMMON_CORE_QUEUE_THREADQUEUE_H


#include <queue>
#include <mutex>
#include <condition_variable>

namespace custom
{
	template<typename T>
	class ThreadQueue
	{
	public:
		ThreadQueue() = default;

		void Push(T value)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			queue_.push(std::move(value));
			condition_variable_.notify_one();
		}

		void WaitPop(T& value)
		{
			std::unique_lock<std::mutex> lock(mutex_);
			condition_variable_.wait(lock, [this]
			{ return !queue_.empty(); });
			value = std::move(queue_.front());
			queue_.pop();
		}

		bool Move(T & value)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			if (queue_.empty())
			{
				return false;
			}
			value = std::move(queue_.front());
			queue_.pop();
			queue_.push(value);
			return true;
		}

		bool TryPop(T& value)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			if (queue_.empty())
			{
				return false;
			}
			value = std::move(queue_.front());
			queue_.pop();
			return true;
		}

		bool Empty() const
		{
			std::lock_guard<std::mutex> lock(mutex_);
			return queue_.empty();
		}

		int Size() const
		{
			std::lock_guard<std::mutex> lock(mutex_);
			return (int)queue_.size();
		}
	private:
		std::queue<T> queue_;
		mutable std::mutex mutex_;
		std::condition_variable condition_variable_;
	};
}

#endif //APP_COMMON_CORE_QUEUE_THREADQUEUE_H
