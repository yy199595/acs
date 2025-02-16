//
// Created by leyi on 2023/6/6.
//

#ifndef APP_EVENT_H
#define APP_EVENT_H
#include<string>
#include<functional>
#include<unordered_map>
#include"Lua/Engine/LuaParameter.h"
#include"Util/Tools/NumberBuilder.h"
namespace help
{
	template<typename T, typename ... Args>
	class EventFactory
	{
	public:
		template<typename T1>
		static int Add(T1 * obj, void (T1::*func)(Args ...)) {
			int id = mNumPool.Pop();
			std::function<void(Args ...)> callback = [obj, func](Args ... args) {
				(obj->*func)(args...);
			};
			mEvents.emplace(id, callback);
			return id;
		}

		static int Add(std::function<void(Args ...)> && callback) {
			int id = mNumPool.Pop();
			mEvents.emplace(id, callback);
			return id;
		}

		static bool Remove(int id)
		{
			auto iter = mEvents.find(id);
			if(iter == mEvents.end())
			{
				return false;
			}
			mEvents.erase(iter);
			return true;
		}

		static int Trigger(Args ... args)
		{
			int count = 0;
			auto iter = mEvents.begin();
			for(; iter != mEvents.end(); iter++)
			{
				count++;
				iter->second(std::forward<Args>(args)...);
			}
			return count;
		}
	private:
		static help::NumberBuilder<int, 1> mNumPool;
		static std::unordered_map<int, std::function<void(Args ...)>> mEvents;
	};

	template<typename T, typename ... Args>
	help::NumberBuilder<int, 1> EventFactory<T, Args...>::mNumPool;
	template<typename T, typename ... Args>
	std::unordered_map<int, std::function<void(Args ...)>> EventFactory<T, Args...>::mEvents;
}
#endif //APP_EVENT_H
