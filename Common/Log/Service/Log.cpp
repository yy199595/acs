//
// Created by yjz on 2023/3/2.
//

#include"Log.h"
namespace acs
{
	bool Log::OnInit()
	{
		BIND_RPC_METHOD(Log::Login);
		return true;
	}

	int Log::Login(const s2s::log::login& request)
	{
		return XCode::Ok;
	}
}