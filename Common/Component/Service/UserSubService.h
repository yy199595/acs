//
// Created by yjz on 2022/4/17.
//

#ifndef _USERSUBSERVICE_H_
#define _USERSUBSERVICE_H_
#include"Component/RpcService/LocalServiceComponent.h"
namespace Sentry
{
	class UserSubService : public LocalServiceComponent
	{
	 public:
		UserSubService() = default;
		~UserSubService() = default;
	 private:
		void DelUser(const Json::Reader & jsonReader);
		void AddUser(const Json::Reader & jsonReader);
	 protected:
		bool OnInitService(ServiceMethodRegister &methodRegister)final;
	 private:
		class RedisComponent * mRedisComponent;
	};
}

#endif //_USERSUBSERVICE_H_
