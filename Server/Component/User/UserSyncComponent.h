//
// Created by yjz on 2022/5/15.
//

#ifndef _USERSYNCCOMPONENT_H_
#define _USERSYNCCOMPONENT_H_
#include"Component/Component.h"
namespace Sentry
{
	class UserSyncComponent final : public Component
	{
	 public:
		UserSyncComponent() = default;
		~UserSyncComponent() = default;
	 public:
		int GetUserState(long long userId);
		bool SetUserState(long long userId, int state);
		long long GetUserId(const std::string & token);
		long long AddNewUser(const std::string & account);
		bool SetToken(const std::string & token, long long userId, int time);
		const std::string GetAddress(long long userId, const std::string & service);
		bool SeAddress(long long userId, const std::string & service, const std::string & address);
	 public:
		bool LateAwake() final;
	 private:
		class MainRedisComponent * mRedisComponent;
	};
}

#endif //_USERSYNCCOMPONENT_H_
