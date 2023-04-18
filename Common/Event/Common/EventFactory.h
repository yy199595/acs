//
// Created by MyPC on 2023/4/18.
//

#ifndef APP_EVENTFACTORY_H
#define APP_EVENTFACTORY_H
#include<vector>
#include<unordered_map>
namespace Tendo
{
	class IEventBase
	{

	};
	template<typename T>
	class IEvent : public IEventBase
	{
	public:
		virtual void OnEvent(const T & data) = 0;
	};
}


#endif //APP_EVENTFACTORY_H
