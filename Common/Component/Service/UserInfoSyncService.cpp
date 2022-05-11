//
// Created by yjz on 2022/4/17.
//

#include"UserInfoSyncService.h"
#include"Component/Redis/MainRedisComponent.h"
#include"Component/RpcService/LocalServiceComponent.h"
namespace Sentry
{
	bool UserInfoSyncService::LateAwake()
	{
		this->mRedisComponent = this->GetComponent<MainRedisComponent>();
		return true;
	}

	bool UserInfoSyncService::OnInitService(ServiceMethodRegister& methodRegister)
	{
		methodRegister.Bind("AddUser", &UserInfoSyncService::AddUser);
		methodRegister.Bind("DelUser", &UserInfoSyncService::DelUser);
		return true;
	}

	XCode UserInfoSyncService::DelUser(const sub::DelUser::Request& request)
	{
		const std::string & service = request.service();
		LocalServiceComponent * logicService = this->GetComponent<LocalServiceComponent>(service);
		if(logicService == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		logicService->DelEntity(request.user_id());
		return XCode::Successful;
	}

	XCode UserInfoSyncService::AddUser(const sub::AddUser::Request& request)
	{
		const std::string & service = request.service();
		LocalServiceComponent * logicService = this->GetComponent<LocalServiceComponent>(service);
		if(logicService == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		logicService->AddEntity(request.user_id(), request.address());
		return XCode::Successful;
	}
}
