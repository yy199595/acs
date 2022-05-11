//
// Created by yjz on 2022/4/17.
//

#ifndef _USERSUBSERVICE_H_
#define _USERSUBSERVICE_H_
#include"Protocol/sub.pb.h"
#include"Component/RpcService/LocalServiceComponent.h"
namespace Sentry
{
	class UserInfoSyncService final : public LocalServiceComponent
	{
	 public:
		UserInfoSyncService() = default;
		~UserInfoSyncService() = default;
	 private:
		XCode DelUser(const sub::DelUser::Request & request);
		XCode AddUser(const sub::AddUser::Request & request);
	 protected:
		bool LateAwake() final;
		bool OnInitService(ServiceMethodRegister &methodRegister) final;
	 private:
		class MainRedisComponent * mRedisComponent;
	};
}

#endif //_USERSUBSERVICE_H_
