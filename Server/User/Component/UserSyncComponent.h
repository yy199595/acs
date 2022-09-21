//
// Created by yjz on 2022/5/15.
//

#ifndef _USERSYNCCOMPONENT_H_
#define _USERSYNCCOMPONENT_H_
#include"Component/RedisChannelComponent.h"
namespace Sentry
{
	class UserSyncComponent final : public RedisChannelComponent
	{
	 public:
		UserSyncComponent() = default;
		~UserSyncComponent() = default;

    public:
		bool LateAwake() final;
		bool OnRegisterEvent(NetEventRegistry &eventRegister) final;
	private:
		bool OnUserJoin(const Json::Reader & json);
		bool OnUserExit(const Json::Reader & json);
	private:
		class RedisDataComponent * mRedisComponent;
	};
}

#endif //_USERSYNCCOMPONENT_H_
