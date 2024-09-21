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
	template<typename ... Args>
	class EventFactory
	{
	public:
		template<typename T>
		static int Add(T * obj, void (T::*func)(Args ...)) {
			int id = mNumPool.Pop();
			std::function<void(Args ...)> callback = [obj, func](Args ... args) {
				(obj->*func)(args...);
			};
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

	template<typename ... Args>
	help::NumberBuilder<int, 1> EventFactory<Args...>::mNumPool;
	template<typename ... Args>
	std::unordered_map<int, std::function<void(Args ...)>> EventFactory<Args...>::mEvents;
}
#endif //APP_EVENT_H
