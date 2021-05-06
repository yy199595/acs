#pragma once
#include<queue>
namespace SoEasy
{
	template<typename T>
	class MemoryPool
	{
	public:
		MemoryPool(size_t size = 100, size_t maxSize = 200);
	public:
		template<typename ... Args>
		T * CreateObject(Args &&... args);
		bool DestoryObject(T * object);
	private:
		size_t mMaxSize;
		std::queue<void *> mMemoryPoolQueue;
	};

	template<typename T>
	template<typename ...Args>
	inline T * MemoryPool<T>::CreateObject(Args && ...args)
	{
		if (!this->mMemoryPoolQueue.empty())
		{
			void * pMemory = this->mMemoryPoolQueue.front();
			this->mMemoryPoolQueue.pop();
			return new(pMemory)T(std::forward<Args>(args)...);
		}
		return new T(std::forward<Args>(args)...);
	}

	template<typename T>
	inline bool MemoryPool<T>::DestoryObject(T * object)
	{
		if (object == nullptr)
		{
			return true;
		}
		
		if (this->mMemoryPoolQueue.size() >= this->mMaxSize)
		{
			delete object;
			return true;
		}
		object->~T();
		this->mMemoryPoolQueue.push(object);
	}

	template<typename T>
	inline MemoryPool<T>::MemoryPool(size_t size, size_t maxSize)
	{
		for (size_t index = 0; index < size; index++)
		{
			void * pMemory = malloc(sizeof(T));
			this->mMemoryPoolQueue.push(pMemory);
		}
	}
}