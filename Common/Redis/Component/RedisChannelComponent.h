//
// Created by mac on 2022/5/25.
//

#ifndef SERVER_NETEVENTCOMPONENT_H
#define SERVER_NETEVENTCOMPONENT_H
#include"Method/EventMethod.h"
#include"Component/Component.h"

namespace Sentry
{
	class RedisChannelComponent : public Component
	{
	public:
		RedisChannelComponent() = default;
		virtual ~RedisChannelComponent() = default;
	public:
		bool StartRegisterEvent();
		bool Invoke(const std::string & id, std::shared_ptr<Json::Reader> json);
	protected:
		virtual bool OnRegisterEvent(NetEventRegistry & eventRegister) = 0;
	private:
		NetEventRegistry mEventRegistry;
	};
}


#endif //SERVER_NETEVENTCOMPONENT_H
