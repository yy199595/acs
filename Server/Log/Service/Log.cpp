//
// Created by yjz on 2023/3/2.
//

#include "Log.h"

namespace Sentry
{
	void Log::Init()
	{

	}

	bool Log::OnStart()
	{
		BIND_COMMON_RPC_METHOD(Log::Push);
		return true;
	}

	bool Log::OnClose()
	{
		return true;
	}

	XCode Log::Push(const s2s::log::push& request)
	{
		return XCode::Successful;
	}
}