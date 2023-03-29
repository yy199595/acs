//
// Created by yjz on 2023/3/23.
//

#include"PhysicalHttpService.h"
#include"Component/LuaScriptComponent.h"
#include "Method/LuaHttpServiceMethod.h"
namespace Sentry
{
	PhysicalHttpService::PhysicalHttpService()
	{
		this->mSumCount = 0;
		this->mWaitCount = 0;
	}

	int PhysicalHttpService::Invoke(const std::string& name,
		const std::shared_ptr<Http::Request>& request, std::shared_ptr<Http::Response>& response)
	{
		HttpServiceRegister& httpRegister = this->GetRegister();
		std::shared_ptr<HttpServiceMethod> method = httpRegister.GetMethod(name);
		if (method == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		this->mSumCount++;
		this->mWaitCount++;
		int code = method->Invoke(*request, *response);
		{
			this->mWaitCount--;
		}
		return code;
	}
}