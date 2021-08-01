#pragma once
#include<queue>
#include<Protocol/com.pb.h>
namespace Sentry
{
	template<typename T>
	class ObjectPool
	{
	public:
		ObjectPool(size_t maxCount = 100);
		~ObjectPool();
	public:
		template<typename ... Args>
		T * Create(Args &&... args);
		bool Destory(T * data);
	private:
		const size_t mMaxCount;
		std::queue<T *> mObjectQueue;
	};
	template<typename T>
	inline ObjectPool<T>::ObjectPool(size_t maxCount)
		: mMaxCount(maxCount)
	{
		for (size_t index = 0; index < maxCount; index++)
		{
			T * object = new T();
			this->mObjectQueue.push(object);
		}
	}
	template<typename T>
	inline ObjectPool<T>::~ObjectPool()
	{
		while (!this->mObjectQueue.empty())
		{
			T * object = this->mObjectQueue.front();
			this->mObjectQueue.pop();
			delete object;
		}
	}
	
	template<typename T>
	inline bool ObjectPool<T>::Destory(T * data)
	{
		if (data != nullptr)
		{
			if (this->mObjectQueue.size() >= this->mMaxCount)
			{
				delete data;
				return true;
			}
			data->Clear();
			this->mObjectQueue.push(data);
			return false;
		}
		return true;
	}
	template<typename T>
	template<typename ...Args>
	inline T * ObjectPool<T>::Create(Args &&... args)
	{
		if (!this->mObjectQueue.empty())
		{
			T * object = this->mObjectQueue.front();
			this->mObjectQueue.pop();
			return object;
		}
		return new T();
	}
}