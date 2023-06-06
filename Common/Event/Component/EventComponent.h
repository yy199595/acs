//
// Created by leyi on 2023/6/6.
//

#ifndef APP_EVENTCOMPONENT_H
#define APP_EVENTCOMPONENT_H
#include<unordered_map>
#include"Event/Event/Event.h"
#include"Entity/Component/Component.h"
namespace Tendo
{
	class EventComponent : public Component
	{
	public:

	private:
		std::unordered_map<std::string, std::unique_ptr<IEvent>> mEvents;
	};
}
#endif //APP_EVENTCOMPONENT_H
