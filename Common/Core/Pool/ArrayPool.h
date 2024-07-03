//
// Created by leyi on 2023/9/5.
//

#ifndef APP_ARRAYPOOL_H
#define APP_ARRAYPOOL_H

#include <queue>

namespace custom
{
	template<typename T, size_t size>
	class ArrayPool
	{
	public:
		ArrayPool() = default;
		ArrayPool(ArrayPool<T, size> &&) = delete;
		ArrayPool(const ArrayPool<T, size> &) = delete;
	public:
		inline T* Pop();
		inline void Clear();
		inline bool Push(T* data);
		inline size_t Size() const { return this->mArray.size(); }
	private:
		std::queue<T*> mArray;
	};

	template<typename T, size_t size>
	inline void ArrayPool<T, size>::Clear()
	{
		while(!this->mArray.empty())
		{
			delete this->mArray.front();
			this->mArray.pop();
		}
	}

	template<typename T, size_t size>
	inline T* ArrayPool<T, size>::Pop()
	{
		if(this->mArray.empty())
		{
			return nullptr;
		}
		T * data = this->mArray.front();
		{
			this->mArray.pop();
			return data;
		}
	}

	template<typename T, size_t size>
	inline bool ArrayPool<T, size>::Push(T* data)
	{
		if(this->mArray.size() >= size)
		{
			delete data;
			return false;
		}
		this->mArray.emplace(data);
		return true;
	}
}

#endif //APP_ARRAYPOOL_H
