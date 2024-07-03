//
// Created by leyi on 2023/10/25.
//

#ifndef APP_IEVENT_H
#define APP_IEVENT_H


#include"Event.h"

namespace help
{
	struct InnerLogoutEvent : public help::EventFactory<int> {};
	struct OuterLogoutEvent : public help::EventFactory<long long> {};
	struct PlayerLoginEvent : public help::EventFactory<long long> {};
	struct PlayerLogoutEvent : public help::EventFactory<long long> {};
	struct HttpPushTokenEvent : public help::EventFactory<std::string> { };
}


#endif //APP_IEVENT_H
