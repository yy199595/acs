//
// Created by zmhy0073 on 2022/6/6.
//

#include"LuaPhysicalHttpService.h"
#include"Script/Lua/Function.h"
#include"Script/Module/LuaModule.h"
#include"Script/Component/LuaScriptComponent.h"
namespace Sentry
{
	LuaPhysicalHttpService::LuaPhysicalHttpService()
	{
		this->mSumCount = 0;
		this->mWaitCount = 0;
	}

    bool LuaPhysicalHttpService::OnInit()
	{
		const std::string& name = this->GetName();
		LuaScriptComponent* luaScriptComponent = this->GetComponent<LuaScriptComponent>();
		return luaScriptComponent != nullptr && luaScriptComponent->GetModule(name) != nullptr;
	}

	int LuaPhysicalHttpService::Invoke(const string& name,
		const std::shared_ptr<Http::Request> &request, std::shared_ptr<Http::Response> &response)
	{
		HttpServiceRegister & httpRegister = this->GetRegister();
		std::shared_ptr<HttpServiceMethod> method = httpRegister.GetMethod(name);
		if(method == nullptr)
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