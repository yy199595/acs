#pragma once
#include"RpcService.h"
namespace acs
{
	class LuaRpcService : public RpcService, public ILogin
	{
	private:
		void OnLogin(long long playerId) final;
		void OnLogout(long long playerId) final;
		bool OnInit() final { return this->GetLuaModule() != nullptr; }
	};
}// namespace Sentry