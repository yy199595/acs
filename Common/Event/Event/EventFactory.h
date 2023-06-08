//
// Created by leyi on 2023/6/7.
//

#ifndef APP_EVENTFACTORY_H
#define APP_EVENTFACTORY_H
#include"Event.h"
namespace Tendo
{
	class EventFactory
	{
	public:
		EventFactory(const std::string & name);
	public:
		template<typename ... Args>
		int Invoke(const Args&&... args);
	private:
		const std::string mName;
		std::unordered_map<std::string, IEvent *> mEvents;
	};

	template<typename ... Args>
	int EventFactory::Invoke(const Args&& ...args)
	{
		int count = 0;
		typedef CEvent<Args ...> CppEvent;
		typedef LuaEvent<Args ...> TLuaEvent;
		auto iter = this->mEvents.begin();
		for(; iter != this->mEvents.end(); iter++)
		{
			IEvent* targetEvent = iter->second;
			if (targetEvent == nullptr)
			{
				continue;
			}
			count++;
			if (targetEvent->IsLuaEvent())
			{
				(static_cast<TLuaEvent*>(targetEvent))->Invoke(std::forward<Args>(args)...);
			}
			else
			{
				(static_cast<CppEvent*>(targetEvent))->Invoke(std::forward<Args>(args)...);
			}
		}
		return count;
	}
}

#endif //APP_EVENTFACTORY_H
