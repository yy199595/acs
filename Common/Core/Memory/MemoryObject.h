//
// Created by 64658 on 2024/12/3.
//

#ifndef APP_MEMORYOBJECT_H
#define APP_MEMORYOBJECT_H
#include<string>
#include<mutex>
#include<unordered_set>
namespace memory
{
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
}



#endif //APP_MEMORYOBJECT_H
