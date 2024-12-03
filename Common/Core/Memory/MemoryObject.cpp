//
// Created by 64658 on 2024/12/3.
//
#include "MemoryObject.h"

namespace memory
{
	Object::Object()
	{
		std::lock_guard<std::mutex> lock(sMutex);
		sObjects.insert(this);
	}

	Object::~Object()
	{
		std::lock_guard<std::mutex> lock(sMutex);
		auto iter = sObjects.find(this);
		if(iter != sObjects.end())
		{
			sObjects.erase(iter);
		}
	}


	size_t Object::GetObjectCount()
	{
		std::lock_guard<std::mutex> lock(sMutex);
		return sObjects.size();
	}


	std::mutex Object::sMutex;
	std::unordered_set<Object*> Object::sObjects;
}