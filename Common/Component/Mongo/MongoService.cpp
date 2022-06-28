//
// Created by mac on 2022/5/19.
//

#include"MongoService.h"
#include"MongoComponent.h"
namespace Sentry
{
	bool MongoService::OnStartService(ServiceMethodRegister& methodRegister)
	{
		return true;
	}

	bool MongoService::LateAwake()
	{
		this->mMongoComponent = this->GetComponent<MongoComponent>();
		return this->mMongoComponent != nullptr;
	}

}