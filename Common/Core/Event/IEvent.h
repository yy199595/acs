//
// Created by leyi on 2023/10/25.
//

#ifndef APP_IEVENT_H
#define APP_IEVENT_H


#include"Event.h"
#define DEFINE_STATIC_EVENT(name, ...) struct name : public help::EventFactory<name, __VA_ARGS__> {}
namespace help
{
	DEFINE_STATIC_EVENT(InnerLogoutEvent, int);
	DEFINE_STATIC_EVENT(PlayerLoginEvent, long long, int);
	DEFINE_STATIC_EVENT(PlayerLogoutEvent, long long, int);
	DEFINE_STATIC_EVENT(AddNodeEvent, const std::string &, int);
	DEFINE_STATIC_EVENT(DelNodeEvent, int);
}





#endif //APP_IEVENT_H
