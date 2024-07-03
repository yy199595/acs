//
// Created by leyi on 2023/8/3.
//

#ifndef APP_QUEUE_H
#define APP_QUEUE_H
#include<queue>
#include<memory>
namespace custom
{
	template<typename T>
	class Queue
	{
	public:
		explicit Queue(int size = 0) :mMaxSize(size) { }
	public:
		inline int Clear();
		inline bool Pop();
		inline bool Pop(T & value);
		inline bool Move(T & value);
		inline bool Front(T & value);
		inline bool Push(const T & value);
		inline bool Push(T && value);
		inline bool Empty() const { return this->mQueue.empty(); }
		inline size_t Size() const { return this->mQueue.size(); }
	private:
		int mMaxSize;
		std::queue<T> mQueue;
	};
	template<typename T>
	bool Queue<T>::Pop()
	{
		if(this->mQueue.empty())
		{
			return false;
		}
		this->mQueue.pop();
		return true;
	}

	template<typename T>
	bool Queue<T>::Front(T& value)
	{
		if(this->mQueue.empty())
		{
			return false;
		}
		value = this->mQueue.front();
		return true;
	}

	template<typename T>
	int Queue<T>::Clear()
	{
		int count = 0;
		while(!this->mQueue.empty())
		{
			count++;
			this->mQueue.pop();
		}
		return count;
	}

	template<typename T>
	bool Queue<T>::Move(T& value)
	{
		if(this->mQueue.empty())
		{
			return false;
		}
		value = std::move(this->mQueue.front());
		this->mQueue.pop();
		this->mQueue.push(value);
		return true;
	}


	template<typename T>
	inline bool Queue<T>::Push(const T& value)
	{
		if(this->mMaxSize >0 &&
			this->mQueue.size() >= this->mMaxSize)
		{
			return false;
		}
		this->mQueue.push(value);
		return true;
	}

	template<typename T>
	bool Queue<T>::Push(T&& value)
	{
		if(this->mMaxSize >0 &&
		   this->mQueue.size() >= this->mMaxSize)
		{
			return false;
		}
		this->mQueue.emplace(std::move(value));
		return true;
	}

	template<typename T>
	inline bool Queue<T>::Pop(T & value)
	{
		if(this->mQueue.empty())
		{
			return false;
		}
		value = std::move(this->mQueue.front());
		this->mQueue.pop();
		return true;
	}
}
#endif //APP_QUEUE_H
