//
// Created by yjz on 2023/3/2.
//

#include "Log.h"
#include "Component/LogComponent.h"
namespace Sentry
{
	void Log::Init()
	{

	}

	bool Log::OnStart()
	{
		BIND_COMMON_RPC_METHOD(Log::Login);
		return true;
	}

	int Log::Login(const s2s::log::login& request)
	{
		return XCode::Successful;
	}
}