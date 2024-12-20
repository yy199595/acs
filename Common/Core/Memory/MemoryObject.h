//
// Created by 64658 on 2024/12/3.
//

#ifndef APP_MEMORYOBJECT_H
#define APP_MEMORYOBJECT_H

#ifdef __SHARE_PTR_COUNTER__
#include<string>
#include<mutex>
#include<unordered_set>
namespace memory
{
	template<typename T>
	class Object
	{
	public:
		Object();
		virtual ~Object();
	public:
		static size_t GetObjectCount();
	private:
		static std::mutex sMutex;
		static std::unordered_set<Object*> sObjects;
	};
	template<typename T>
	Object<T>::Object()
	{
		std::lock_guard<std::mutex> lock(sMutex);
		sObjects.emplace(this);
	}

	template<typename T>
	Object<T>::~Object()
	{
		std::lock_guard<std::mutex> lock(sMutex);
		auto iter = sObjects.find(this);
		if(iter != sObjects.end())
		{
			sObjects.erase(iter);
		}
	}

	template<typename T>
	size_t Object<T>::GetObjectCount()
	{
		std::lock_guard<std::mutex> lock(sMutex);
		size_t index = 0;
		for (memory::Object<T> * object : sObjects)
		{
			index++;
			if(index >= 40000 && index >= sObjects.size() - 10)
			{
				printf("[%d] %p\n", (int)index, object);
			}
		}
		return sObjects.size();
	}

	template<typename T>
	std::mutex Object<T>::sMutex;

	template<typename T>
	std::unordered_set<Object<T>*> Object<T>::sObjects;
}

#endif

#endif //APP_MEMORYOBJECT_H
