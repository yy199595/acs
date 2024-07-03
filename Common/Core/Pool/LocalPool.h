//
// Created by leyi on 2023/9/6.
//

#ifndef APP_LOCALPOOL_H
#define APP_LOCALPOOL_H

#include <queue>
#include <atomic>
#include <mutex>
#include <thread>

namespace custom
{
//	template<typename T>
//	class IObject
//	{
//	public:
//		virtual ~IObject() = default;
//		void * operator new(size_t size)
//		{
//			++Counter;
//			return malloc(size);
//		}
//		void operator delete (void * memory)
//		{
//			--Counter;
//			free(memory);
//		}
//	public:
//		static int GetCounter() { return Counter.load(); }
//	private:
//		static std::atomic_int Counter;
//	};
//
//	template<typename T>
//	std::atomic_int IObject<T>::Counter(0);

}

#endif //APP_LOCALPOOL_H
